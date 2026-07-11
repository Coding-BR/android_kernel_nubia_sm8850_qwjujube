# SM8850 Reconstructed ZTE Driver Integration Audit

Date: 2026-07-11

Baseline: `feature/nx809j-source-hygiene` at
`d45fd4d31dac4f6bc69497b9e92657b4ceb50265`

Validation branch: `feature/sm8850-driver-validation`

Scope:

- The eleven single-translation-unit ZTE reconstructions under
  `kernel_platform/common/drivers/soc/qcom/zte/`
- The `zte_tpd` reconstruction in the same subtree
- The parallel standalone reconstruction layer under
  `vendor/qcom/opensource/zte-drivers/`
- The build and packaging scripts that are expected to produce or deploy these
  modules

This is a source and build-integration audit. It is not a claim of module ABI,
KCFI, boot, suspend/resume, or hardware validation.

## Verdict

The reconstructed source is already present. The primary baseline failure is
not an absent driver payload and it is not a raw C parser failure in the eleven
small drivers. In the `super_build.sh` in-tree graph, those eleven non-TPD child
objects were orphaned by a parent `obj-m` directory descent combined with child
`obj-y` declarations. Kbuild emitted no mismatch diagnostic and generated no
modules for them.

`zte_tpd` follows a different Kbuild path and does reach the compiler. Its
normal build succeeds only with a broad diagnostic suppression set. Removing
that suppression set in a temporary strict-build copy produced 1,251 compiler
errors, dominated by missing prototypes, integer/pointer conversions, implicit
function declarations, uninitialized values, and incompatible pointer types.

The repository also has two unsynchronized source roots, no ZTE Kconfig layer,
output-name mismatches, and a deployment manifest that cannot receive all of
the modules advertised by the source tree.

## Reproducible Probe Environment

| Item | Value |
|---|---|
| Kernel baseline | `d45fd4d31dac4f6bc69497b9e92657b4ceb50265` |
| Configuration | `op_wild_defconfig` followed by `olddefconfig` |
| `.config` SHA-256 | `c78b15727bf8c455920e68ce20fea5976d8002aca95d468c1869d4350d752fdf` |
| Compiler | Android Clang `r536225`, Clang 19.0.1 |
| Architecture | `arm64` |
| Relevant config | `CONFIG_MODULES=y`, `CONFIG_MODVERSIONS=y`, `CONFIG_MODULE_SIG=y`, `CONFIG_LTO_CLANG=y` |
| CFI in this config | `CONFIG_CFI_CLANG` is not set |

The output tree was prepared with:

```bash
make -C kernel_platform/common O="$OUT" ARCH=arm64 LLVM=1 LLVM_IAS=1 op_wild_defconfig
make -C kernel_platform/common O="$OUT" ARCH=arm64 LLVM=1 LLVM_IAS=1 olddefconfig
make -C kernel_platform/common O="$OUT" ARCH=arm64 LLVM=1 LLVM_IAS=1 prepare modules_prepare
```

The focused in-tree traversal probe was:

```bash
make -C kernel_platform/common O="$OUT" ARCH=arm64 LLVM=1 LLVM_IAS=1 \
  drivers/soc/qcom/zte/
```

## Finding F-1: Eleven Intended Modules Were Orphaned by the Kbuild Graph

Severity: critical

The parent Makefile enters each intended driver directory as a module:

```make
# kernel_platform/common/drivers/soc/qcom/zte/Makefile
obj-m += zte_misc/
obj-m += zte_power_supply/
obj-m += zte_imem_info/
obj-m += zte_stats_info/
obj-m += zte_sensor_sensitivity/
obj-m += zte_ir/
obj-m += zte_reboot_ext/
obj-m += zte_ramdisk_reboot/
obj-m += zte_led/
obj-m += zte_fingerprint/
obj-m += zte_charger_policy/
obj-m += zte_tpd/
```

The eleven non-TPD child Makefiles then declare only built-in objects:

```make
obj-y += zte_misc.o
```

The same `obj-y` pattern is present in all eleven non-TPD child directories.
Kbuild does not promote those child `obj-y` objects into the parent's `obj-m`
set. The module-directory descent requests each child's `modules.order`, while
that file is populated from child `obj-m` entries rather than child
`built-in.a` content. The repository's own Kbuild documentation describes this
orphaning combination as a likely Makefile or Kconfig bug in
`Documentation/kbuild/makefiles.rst`.

That combination does not create a loadable module for any of those
directories. `zte_tpd` is the exception because its child Makefile declares:

```make
obj-m += zte_tpd.o
zte_tpd-objs := globals.o $(obj-files)
```

The focused traversal produced direct filesystem evidence:

- `drivers/soc/qcom/zte/built-in.a` was 8 bytes and contained zero members.
- Every ordinary child `modules.order` file was zero bytes.
- The parent `modules.order` contained only
  `drivers/soc/qcom/zte/zte_tpd/zte_tpd.o`.
- The log contained 487 `CC [M]` entries for `zte_tpd` and zero compiler
  entries for the other eleven directories.

This is why a successful `super_build.sh` run can coexist with no current-build
output for eleven reconstructed drivers. Those sources never reached
compilation through the normal in-tree traversal. The script also does not
clean before building, so an old object on disk would not prove that the
current graph generated it.

### Forced source compilation

Each omitted source was then addressed directly through its Kbuild object
target. This bypassed only the broken directory-to-module declaration; it did
not change source or headers.

| Module | Direct object result | Clang warnings | Clang errors | Object size |
|---|---:|---:|---:|---:|
| `zte_misc` | pass | 0 | 0 | 243,148 bytes |
| `zte_power_supply` | pass | 0 | 0 | 274,468 bytes |
| `zte_imem_info` | pass | 0 | 0 | 156,568 bytes |
| `zte_stats_info` | pass | 0 | 0 | 351,796 bytes |
| `zte_sensor_sensitivity` | pass | 0 | 0 | 187,576 bytes |
| `zte_ir` | pass | 0 | 0 | 211,740 bytes |
| `zte_reboot_ext` | pass | 0 | 0 | 204,640 bytes |
| `zte_ramdisk_reboot` | pass | 0 | 0 | 192,844 bytes |
| `zte_led` | pass | 0 | 0 | 361,648 bytes |
| `zte_fingerprint` | pass | 0 | 0 | 389,012 bytes |
| `zte_charger_policy` | pass | 0 | 0 | 266,108 bytes |

Therefore, no missing include or raw C syntax failure was reproduced for these
eleven sources under this configuration. Their baseline blocker was Kbuild
integration.

### Validation-branch correction and result

The validation branch changes each of the eleven child declarations from
`obj-y` to `obj-m`, matching the parent's modular directory descent. No driver
source was changed.

After deleting the ZTE subtree from the temporary output directory, the same
focused in-tree target was run again. It exited 0 with:

- 498 ZTE compilation entries: 487 for `zte_tpd` plus the eleven restored
  single-file modules
- zero Clang warnings and zero Clang errors
- twelve entries in the parent `modules.order`
- one non-empty object and one `modules.order` entry in every child directory

| Restored module | Post-fix object size |
|---|---:|
| `zte_misc` | 186,784 bytes |
| `zte_power_supply` | 204,184 bytes |
| `zte_imem_info` | 117,464 bytes |
| `zte_stats_info` | 244,536 bytes |
| `zte_sensor_sensitivity` | 148,304 bytes |
| `zte_ir` | 153,032 bytes |
| `zte_reboot_ext` | 147,232 bytes |
| `zte_ramdisk_reboot` | 139,408 bytes |
| `zte_led` | 289,184 bytes |
| `zte_fingerprint` | 290,352 bytes |
| `zte_charger_policy` | 216,248 bytes |

The focused directory target stops before final module modpost/linking, so this
probe intentionally claims `.o` and `modules.order` restoration rather than
final `.ko` ABI validity. A direct single-module `.ko` target reached modpost
and then failed because the prepared output tree had neither `vmlinux.o` nor a
complete `Module.symvers`; that expected probe limitation is covered in F-4.

### Remaining linkage direction

The intended linkage is now consistently modular, but configuration still
needs to be made explicit:

- Replace unconditional `obj-m` entries with gated
  `obj-$(CONFIG_...) += <module>.o` once the missing Kconfig layer exists.
- If any driver is intentionally built in, make the parent/child graph
  consistently built-in and document why a stock `.ko` replacement is no
  longer expected.

The stock-module names and the existing deployment scripts indicate that
loadable modules are the intended mode used by this validation branch.

## Finding F-2: There Is No ZTE Kconfig Contract

Severity: high

No `Kconfig` exists at any of these integration boundaries:

- `kernel_platform/common/drivers/soc/qcom/zte/Kconfig`
- `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/Kconfig`
- `vendor/qcom/opensource/zte-drivers/Kconfig`

No kernel Kconfig file references these driver names. The parent Makefile is
unconditional, so it does not encode dependencies such as power supply, OF,
GPIO, SPI, LED, input, procfs, sysfs, or generic netlink support.

The audit configuration happens to enable the high-level facilities needed by
the eleven small sources, which is why the forced object probes pass. That is
not a portable dependency contract. A different defconfig can fail or produce
an invalid feature set without any Kconfig diagnostic.

## Finding F-3: `zte_tpd` Compiles by Suppressing Fatal Diagnostics

Severity: critical

The `zte_tpd` directory contains 488 C files. Its wildcard Makefile excludes
`offset_test.c` and excludes `globals.c` from the wildcard list before adding
`globals.o` explicitly. The production composite therefore compiles 487 C
translation units, not 488.

The normal build produced:

- 487 `CC [M]` entries
- `zte_tpd.o` at 12,658,888 bytes
- an external probe `zte_tpd.ko` at 11,582,160 bytes

The Makefile suppresses diagnostics including:

- implicit function declarations
- integer/pointer conversions
- incompatible pointer types
- discarded qualifiers
- uninitialized values
- missing return values
- non-prototype declarations
- self-assignment

A temporary copy was built with only the include path, forced `defs.h`, and
the existing frame-size threshold retained. No repository source was changed.
That strict probe exited with status 2 and emitted 1,251 errors. The largest
diagnostic classes were:

| Diagnostic class | Count |
|---|---:|
| strict prototypes | 519 |
| integer/pointer conversion | 341 |
| implicit function declaration | 190 |
| unused variable promoted to error | 51 |
| uninitialized use | 45 |
| incompatible pointer types | 33 |
| deprecated non-prototype call | 26 |
| discarded qualifiers | 13 |
| self-assignment | 12 |

One common error is declared in `defs.h`:

```c
extern __int64 syna_request_managed_device();
```

That declaration has no prototype and contaminates most translation units
because the Makefile force-includes `defs.h`. Other diagnostics are semantic,
not cosmetic: pointer values are stored in reconstructed integer types,
undeclared functions are called, and potentially uninitialized values are
consumed.

`offset_test.c` is an intentionally excluded compile-time layout probe:

```c
char (*__kaboom)[offsetof(struct device, release)] = 1;
```

It should not be counted as a production translation unit. Keeping it beside
wildcard-selected production sources is fragile and should be replaced by a
documented tooling or test location.

The permissive build proves that Clang can emit a composite object. It does not
prove type correctness, KCFI compatibility, or runtime safety.

## Finding F-4: The Build Script Can Accept Unresolved Module Symbols

Severity: high

`super_build.sh` invokes the complete build with:

```bash
make ... KBUILD_MODPOST_WARN=1 Image vmlinux modules dtbs
```

`KBUILD_MODPOST_WARN=1` downgrades unresolved module symbols from fatal modpost
errors to warnings. A zero exit status from this script is therefore not, by
itself, proof that every generated module can resolve against the built kernel.
This matters especially because `CONFIG_MODVERSIONS=y`.

The focused external probes in this audit used a prepared output tree without
a complete kernel `Module.symvers`. For `zte_tpd`, modpost displayed ten
undefined-symbol warnings and reported another 141 suppressed warnings. Those
warnings are classified as a probe limitation, not as 151 proven driver
defects, because the base symbol table was intentionally absent.

The release gate must run a clean complete build with a generated
`Module.symvers` and without `KBUILD_MODPOST_WARN=1`, then fail on every
unresolved or modversion mismatch.

## Finding F-5: Two Source Roots Have Already Diverged

Severity: high

The repository carries the eleven ordinary reconstructions twice:

- Standalone sources: `vendor/qcom/opensource/zte-drivers/<module>/`
- In-tree sources: `kernel_platform/common/drivers/soc/qcom/zte/<module>/`

Nine pairs are byte-identical. Two are not:

| Module | In-tree delta relative to standalone | Material differences |
|---|---:|---|
| `zte_imem_info` | +15 / -2 lines | `readl()` replaces `__raw_readl()`; module teardown and procfs cleanup are added |
| `zte_stats_info` | +64 / -55 lines | Generic-netlink family changes from `ZTE_STATS_CUSTOM` to `ZTE_STATS`; locking, allocation, reply construction, task accounting, and module teardown differ |

The standalone parent directory has no `Makefile`, `Kbuild`, or `Kconfig`, so
`super_build.sh` never traverses it. Its per-module Makefiles can be invoked
manually, but there is no authoritative sync or selection mechanism between
the two source roots.

This is already a protocol and behavior split, not harmless duplication. One
source root must be designated authoritative; generated mirrors must be
verified byte-for-byte in CI or removed.

The touchscreen reconstruction has a second, larger duplication boundary:

- `vendor/zte/zte_tpd/`
- `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/`

Of 490 paired C/header files, 472 are byte-identical and 18 differ. The in-tree
copy contains changes in work scheduling and panel notification, platform/input
lifecycle handling, SPI message construction, and touch-device registration
and removal. Examples include replacing a hard-coded CPU with
`WORK_CPU_UNBOUND`, using `sizeof(struct spi_transfer)` and standard SPI message
APIs, adding null/unregister guards, and adding module/platform lifecycle fixes.

This matters to artifact provenance because the packager prefers the vendor
`zte_tpd.ko` before falling back to the in-tree output. A successful in-tree
probe therefore does not prove which touchscreen source produced a packaged
artifact.

## Finding F-6: Module Names and Deployment Manifests Do Not Agree

Severity: high

Standalone Kbuild probes produced the following non-stock names:

- `zte_stats_info/Kbuild` emits `zte_stats_info_custom.ko`.
- `zte_charger_policy/Kbuild` emits `zte_charger_policy_custom.ko` and
  `zte_cleanup.ko`.

The deployment scripts instead request stock names:

- `vendor/zte/zte_tpd/post-fs-data.sh` expects `zte_stats_info.ko` and
  `zte_charger_policy.ko`.
- `vendor/qcom/opensource/zte-drivers/ksu_module/post-fs-data.sh` expects
  `zte_charger_policy.ko` even though the adjacent Kbuild emits
  `zte_charger_policy_custom.ko`.
- No inspected deployment script stages or loads `zte_cleanup.ko` together
  with `zte_charger_policy_custom.ko`.

The broad loader manifest lists only ten of the eleven ordinary modules plus
`zte_tpd`; it omits `zte_ramdisk_reboot` entirely.

`repack_perfect_sign.sh` does not close this gap. It checks
`vendor/zte/zte_tpd/zte_tpd.ko` before the in-tree fallback, copies only the
selected `zte_tpd.ko` and, when present, `msm_kgsl.ko`, and generates a new
two-module `post-fs-data.sh`. It does not copy the broader
`vendor/zte/zte_tpd/post-fs-data.sh` manifest. The separate overclock path
packages `adreno_overclock.ko`; none of the eleven ordinary ZTE module outputs
or `zte_cleanup.ko` is staged by the main repack path.

Consequently, even a corrected compile cannot deliver the advertised module
set through the current packaging flow.

## Finding F-7: Upstream Review Metadata Needs Cleanup

Severity: medium

This is not a compiler blocker, but it is an upstreaming blocker:

- `zte_power_supply.c` and `zte_charger_policy.c` have `MODULE_LICENSE(...)`
  declarations but no SPDX identifier.
- Some reconstructed files claim generated or organization authorship in
  `MODULE_AUTHOR(...)` fields. Those values should be replaced with factual,
  attributable reconstruction metadata or omitted; source provenance must not
  imply verified OEM authorship.
- `SOURCE_PROVENANCE.md` correctly identifies reconstruction classes, but its
  description of 488 `zte_tpd` C files as build-flow inputs should distinguish
  488 tracked C files from the 487 production translation units selected by
  the Makefile.

## Module Status Matrix

| Module | Baseline in-tree result | Validation-branch result | Standalone output | Current delivery status |
|---|---|---|---|---|
| `zte_misc` | orphaned | object + module-order entry | `zte_misc.ko` | not packaged |
| `zte_power_supply` | orphaned | object + module-order entry | `zte_power_supply.ko` | not packaged |
| `zte_imem_info` | orphaned | object + module-order entry | `zte_imem_info.ko` | divergent source; not packaged |
| `zte_stats_info` | orphaned | object + module-order entry | `zte_stats_info_custom.ko` | name mismatch; divergent source |
| `zte_sensor_sensitivity` | orphaned | object + module-order entry | `zte_sensor_sensitivity.ko` | not packaged |
| `zte_ir` | orphaned | object + module-order entry | `zte_ir.ko` | not packaged |
| `zte_reboot_ext` | orphaned | object + module-order entry | `zte_reboot_ext.ko` | not packaged |
| `zte_ramdisk_reboot` | orphaned | object + module-order entry | `zte_ramdisk_reboot.ko` | absent from loader manifest |
| `zte_led` | orphaned | object + module-order entry | `zte_led.ko` | not packaged |
| `zte_fingerprint` | orphaned | object + module-order entry | `zte_fingerprint.ko` | not packaged |
| `zte_charger_policy` | orphaned | object + module-order entry | `zte_charger_policy_custom.ko`, `zte_cleanup.ko` | name/dependency mismatch |
| `zte_tpd` | composite object generated | composite object generated | `zte_tpd.ko` | permissive compile only; sole ZTE module packaged by repack script |

The standalone `.ko` outputs in this matrix prove compile/link shape only. They
were generated against a prepared tree without the final base `Module.symvers`
and are not ABI or loadability evidence.

## Required Validation Stack

The corrective work should be split into reviewable code commits and must pass
these gates in order:

1. Choose one authoritative source root and remove or mechanically verify the
   duplicate mirror.
2. Add Kconfig symbols and explicit dependencies for all twelve modules.
3. Keep the corrected parent/child Kbuild mode and add CI assertions for the
   exact expected module filenames.
4. Repair `zte_tpd` declarations and types, then remove diagnostic suppressions
   incrementally. Treat implicit declarations, pointer/integer conversions,
   incompatible function pointers, and uninitialized use as hard failures.
5. Run a clean `Image vmlinux modules dtbs` build without
   `KBUILD_MODPOST_WARN=1` and preserve the generated `Module.symvers`.
6. Validate module version CRCs, vermagic, signing state, and KCFI configuration
   against the intended target kernel.
7. Make packaging consume the exact build outputs and fail if any required
   module, helper, or destination name is absent.
8. Perform device-side load, bind, functional, suspend/resume, and rollback
   validation before describing any reconstruction as production-ready.

## Confirmed vs. Unproven

Confirmed by this audit:

- At the baseline, all eleven non-TPD in-tree sources were orphaned by normal
  subtree module generation.
- After the eleven `obj-m` corrections, a clean focused in-tree build compiles
  all eleven with zero Clang diagnostics and records all twelve ZTE objects in
  `modules.order`.
- `zte_tpd` compiles only under an unusually broad suppression set and fails a
  strict probe with substantial semantic diagnostics.
- The source roots, output names, loader manifests, and packager disagree.

Not proven by this audit:

- Final module symbol or modversion resolution
- Stock-kernel ABI compatibility
- KCFI compatibility
- Module signature acceptance
- Successful load/unload or bind/unbind
- Touch, charging, LED, fingerprint, IR, reboot, or sensor functionality
- Boot, suspend/resume, thermal, or long-duration stability

Until those unproven gates are closed, the accurate status is: reconstructed
source present, Kbuild traversal repaired on the validation branch, broader
configuration and delivery integration incomplete, runtime validity unknown.
