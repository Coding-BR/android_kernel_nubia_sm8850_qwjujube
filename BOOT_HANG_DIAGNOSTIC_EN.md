# 🔴 Boot Hang Diagnostic — RedMagic 11 Pro (NX809J) Custom GKI Kernel

> **Status:** Under active investigation — Community help welcome!
>
> **Device:** Nubia RedMagic 11 Pro (NX809J) — Snapdragon 8 Elite (SM8850/Canoe)
>
> **Kernel Base:** GKI 6.12.23-android16 with KernelSU-Next
>
> **Target Firmware:** ZTE official userdebug

---

## Problem Summary

The custom kernel compiles successfully (Build #8), boots past early-init, but **hangs indefinitely on the RedMagic logo screen**. The user must long-press power to reboot.

### What works:
- ✅ Kernel boots (no NULL pointer dereference)
- ✅ UFS Storage detects all LUNs (`sda`, `sde`)
- ✅ `/metadata` (sda10) mounts successfully via F2FS
- ✅ `/system`, `/vendor`, `/product` mount via dm-verity/erofs
- ✅ SHM Bridge enabled and registered (4MB, paddr: 0xaa400000)
- ✅ Display initializes (RedMagic logo screen appears)
- ✅ KernelSU-Next v33129 loads

### What does NOT work:
- ❌ `/data` (sda12) **does not mount** — FBE (File-Based Encryption) requires decryption
- ❌ `vold` does not appear in logs
- ❌ `keymint` does not appear in logs
- ❌ `adsp-loader` fails in loop: "fail to get rproc" (~20 times)
- ❌ SCMI `arm-scmi.1.auto` **times out**
- ❌ BRD daemon infinite loop trying to read `/sys/class/power_supply/battery/status`

---

## Root Cause Identified

### Failure Chain (Root Cause Analysis)

```
SCMI arm-scmi.1.auto TIMEOUT (do_xfer)
    ↓
cpuss_telemetry_probe FAILS ("Not able to find shared memory")
    ↓
qcom_q6v5_pas DOES NOT START (remoteproc unavailable)
    ↓
adsp-loader: "fail to get rproc" (loops 20+ times)
    ↓
ADSP/CDSP remoteproc OFFLINE
    ↓
smcinvoke/qseecom NOT AVAILABLE
    ↓
keymint DOES NOT START
    ↓
vold CANNOT decrypt /data
    ↓
/data DOES NOT MOUNT (FBE requires keymint)
    ↓
HANG on RedMagic logo (init stalls waiting for /data)
```

### Critical Evidence: Stock vs Custom Comparison

The `scm_mem_protection_init_do: SCM call failed 0, resp 7` and `qcom_scm_cfg_pddr_protected_region: resp 4 ret -22` happen **identically** on the stock kernel and are **non-fatal**. These are NOT the cause of the hang.

| Event | Stock Kernel ✅ | Custom Kernel ❌ |
|-------|:-:|:-:|
| SCM mem_protection resp 7 | Happens (non-fatal) | Happens (non-fatal) |
| SHM Bridge enabled | Yes | Yes |
| SCMI FC Failed ret:-22 | Happens (fallback) | Happens |
| TBU failed probe (4x) | Happens (non-fatal) | Happens |
| `/metadata` mounted | ✅ | ✅ |
| `/system`, `/vendor` mounted | ✅ | ✅ |
| `vendor/vm-system` invalid magic | Happens (normal) | Happens (normal) |
| `/data` "invalid magic" | Happens (normal, it's FBE) | Happens |
| **SCMI arm-scmi.1.auto** | **Connects OK** | **TIMEOUT** ⚠️ |
| **`keymint` starts** | ✅ PID 1023, @1.27s | ❌ NEVER APPEARS |
| **`vold` starts** | ✅ PID 1027, @1.28s | ❌ NEVER APPEARS |
| **`vdc cryptfs mountFstab`** | ✅ Called | ❌ NEVER CALLED |
| **`adsp-loader`** | ✅ Works | ❌ "fail to get rproc" |
| **Boot completes** | ✅ | ❌ Hang |

---

## Extracted Diagnostic Data

### Stock Kernel (working — currently running on device)

```
[0.245881] arm-scmi arm-scmi.1.auto: SCMI Protocol v2.0 'Qualcomm:PDP0' Firmware version 0x0
[0.261743] qcom-scm: __get_convention: qcom_scm: convention: smc arm 64
[0.262910] shmbridge: [130]: shmbridge is enabled
[0.262992] shmbridge: [565]: qtee shmbridge registered default bridge with size: 4194304 bytes
[0.265188] qcom-scm: scm_mem_protection_init_do: SCM call failed 0, resp 7
[0.265191] shmbridge: [570]: MEM protection Not enabled, 7
[0.275645] qcom-scm: qcom_scm_cfg_pddr_protected_region: resp 4 ret -22
[1.277863] init: starting service 'vendor.keymint'...
[1.282921] init: starting service 'vold'...
[1.550030] init: [libfs_mgr] mount_with_alternatives(): skipping mount (invalid magic) mountpoint=/data
[1.550078] init: Calling: /system/bin/vdc cryptfs mountFstab /dev/block/.../userdata /data false
[1.745983] F2FS-fs (dm-57): recover_data: ...   ← /data mounted successfully via dm-crypt!
```

### Custom Kernel (failing) — Extracted from console-ramoops-0

```
[0.181496] arm-scmi arm-scmi.1.auto: Failed to get FC for protocol 13 - ret:-22
[0.203758] qcom-scm: scm_mem_protection_init_do: SCM call failed, resp 7
[0.214551] qcom-scm: qcom_scm_cfg_pddr_protected_region: resp 4 ret -22
[1.112xx] arm-scmi arm-scmi.1.auto: timed out in resp (caller: do_xfer+0x140/0x4ec)  ← TIMEOUT!
[1.116211] scmi_cpuss_telemetry_probe: Not able to find shared memory location
[1.774-1.875] adsp-loader soc:qcom,msm-adsp-loader: fail to get rproc  ← (loops 20x)
[1.200813] init: Failed to mount an un-encryptable, interrupted, or wiped partition
[1.379476] init: mount_with_alternatives(): skipping mount (invalid magic) mountpoint=/data
  ← vdc cryptfs mountFstab IS NEVER CALLED!
  ← keymint NEVER STARTS!
  ← vold NEVER STARTS!
[22.995404] init: process 'vendor.per_mgr' exited 4 times before boot completed
[229.530209] reboot: Restarting system with command 'LONGPRESS'  ← User long-press power
```

### Module Taint Flags (from crash dump)

In the custom kernel crash dump, modules show suspicious flags:
- `clk_scmi(ME)` — Flag **M** = module init failed? Flag **E** = unsigned
- `qcom_scmi_vendor(NE)` — Flag **N** = uncertain meaning
- `qcom_scm(OA)` — Flags **O** = out-of-tree, **A** = uncertain

On stock, all modules show clean `(O)` flags with no extra error flags.

---

## Loaded Modules (Stock vs Custom)

### Critical boot modules (vendor_boot ramdisk):

On stock, these are loaded by `init` from ramdisk in exact order:
1. `qcom_scmi_vendor.ko`
2. `qcom_scmi_client.ko`
3. `clk-scmi.ko`
4. `qcom-scm.ko`
5. `arm_smmu.ko`
6. `spmi_pmic_arb.ko`
7. `sched_walt.ko`
8. `mem_prot.ko`
9. `secure_buffer.ko`
10. `qcom_ice.ko`
...and ~50+ more early boot modules

### On custom kernel:

The above modules are **compiled from source** and inserted into boot.img via ramdisk. The problem may be:
1. **Different loading order**
2. **Modules with init errors** (M/N flags in taint flags)
3. **CFI warnings** causing silent probe failures
4. **SCMI transport/mailbox** not initializing correctly

---

## Investigation Guide (For Contributors)

### 1. CFI Warning Analysis

The ramoops shows several "CFI failure ... - WARNING ONLY" for modules like `spmi_pmic_arb`, `qcom_cpucp`, `qcom_llcc_pmu`. On stock, these modules have **no** CFI warnings.

**Action:** Verify if CFI type hashes of compiled modules match those expected by the GKI kernel. Modules may be loaded but with probe callbacks **not executed** due to KCFI mismatch.

```bash
# Check CFI hashes:
objdump -d module.ko | grep "kcfi"
```

### 2. Investigate the SCMI Timeout

The `arm-scmi.1.auto` timeout is the most critical divergence. On stock, it connects at ~0.245s. On custom, timeout at ~1.1s.

**Action:** Compare custom vs stock DTB for `arm-scmi` nodes:
```bash
# Extract and compare DTBs
dtc -I dtb -O dts custom_dtb.img > custom.dts
dtc -I dtb -O dts stock_dtb.img > stock.dts
diff custom.dts stock.dts | grep -A5 -B5 "scmi"
```

### 3. Verify modules.load and loading order

**Action:** Extract the custom boot.img ramdisk and verify `modules.load`:
```bash
# Unpack boot.img
unbootimg dev_reverse_perfect.img
# Check ramdisk
ls -la ramdisk/lib/modules/
cat ramdisk/lib/modules/modules.load
```

### 4. Compare lsmod between stock and custom

**Action:** On next custom boot, before it hangs, try to capture via adb:
```bash
adb shell "lsmod" > lsmod_custom.txt
adb shell "dmesg" > dmesg_custom.txt
```

### 5. Test with CONFIG_CFI_PERMISSIVE=y

If CFI warnings are blocking probes, test with permissive CFI:
```
CONFIG_CFI_PERMISSIVE=y
```

### 6. Check if spmi_pmic_arb probe actually succeeds

The SPMI PMIC arbiter has a CFI warning AND a transaction failure in the ramoops. If `spmi_pmic_arb` doesn't probe correctly, the PMIC infrastructure won't work, which cascades to power/regulator failures.

```
[0.174846] CFI failure at do_one_initcall (target: init_module [spmi_pmic_arb]) - WARNING ONLY
[0.281208] spmi spmi-1: pmic_arb_wait_for_done 0x4 0x104: transaction failed (0x3)
[0.281393] pmic-spmi 1-04: probe with driver pmic-spmi failed with error -4
```

---

## Relevant Files

| File | Description |
|------|-------------|
| `console-ramoops-0.log` | RAM dump from last custom boot (binary, 274KB) |
| `console-ramoops-0-clean.txt` | Text version extracted via `strings` |
| `dmesg_stock.txt` | Full stock kernel dmesg |
| `zte_parity.c` | Proprietary symbol bridge |
| `super_build.sh` | Build script |
| `REVERSE_ENGINEERING_MASTER_PLAN.md` | Global project plan |

---

## Project Context

This kernel is part of a reverse engineering effort for the RedMagic 11 Pro (NX809J) aiming to:
- Achieve binary parity with ZTE official firmware
- Native KernelSU-Next support
- GPU overclock to 1200MHz+
- Removal of ZTE proprietary locks

The original ZTE source release is **incomplete** and omits many modules,
headers, integrated WLAN source trees, and the platform configuration fragment
(`canoe.fragment`). This repository preserves a comprehensive Ghidra-derived
evidence corpus and build-targeted reconstructions for the 12 tracked ZTE
modules. Those reconstructions are not original OEM source, and their current
compile, ABI/KCFI, boot, suspend/resume, and hardware status must be proven by
separate gates. See `SOURCE_PROVENANCE.md` for the authoritative classification.

---

## How to Contribute

If you have experience with:
- Qualcomm SCMI/SCM subsystem
- GKI module loading and KCFI
- ARM SMMU v500 TBU probing
- File-Based Encryption (FBE) boot flow on Android 16
- Remoteproc/ADSP/CDSP boot sequence

Please open an issue or PR with your findings. All help is welcome!

---

*Last updated: 2026-05-27*
*Build: #8 (6.12.23-android16-OP-WILD)*
