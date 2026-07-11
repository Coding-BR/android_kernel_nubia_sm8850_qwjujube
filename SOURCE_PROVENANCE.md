# NX809J Source Provenance Ledger

This repository is a hybrid source tree. It combines a published OEM/Qualcomm
baseline, project patches, clean C reconstructions, build-targeted Ghidra
reconstructions, decompiled evidence, and stock binary evidence. Presence in
the tree does not by itself prove OEM authorship or current buildability.

`SOURCE_PROVENANCE.json` is the machine-readable authority. CI verifies its
paths, evidence hashes, corpus counts, and explicitly absent source trees.

## Source Classes

| Class | Location | Meaning |
|---|---|---|
| Mixed published source | `kernel_platform/`, `vendor/qcom/opensource/`, `vendor/zte/` | OEM/Qualcomm baseline plus later project changes; file headers and Git history govern exact origin. |
| Project reconstructed C | `vendor/qcom/opensource/zte-drivers/` | Human-maintained C reconstructed from stock behavior; current branch build verification is tracked separately. |
| Build-targeted Ghidra reconstruction | `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/` | 499 tracked files and 488 C files used by the touchscreen module build flow; this is not original OEM source. |
| Ghidra evidence corpus | `decompiled/` | 104,002 tracked files across 302 module directories; retained as evidence and excluded as a direct build input. |
| Stock binary evidence | `evidence/stock-modules/`, `stock_rom_modules/` | Checksummed reference modules; not source and not direct kernel build inputs. |

## Current Reconstruction Layer

The project-maintained reconstruction directories are:

- `zte_adreno_overclock`
- `zte_charger_policy`
- `zte_fingerprint`
- `zte_imem_info`
- `zte_ir`
- `zte_led`
- `zte_misc`
- `zte_power_supply`
- `zte_ramdisk_reboot`
- `zte_reboot_ext`
- `zte_sensor_sensitivity`
- `zte_stats_info`

Their presence means reconstructed C exists. It does not replace a current
compile, module ABI, KCFI, boot, suspend/resume, or hardware validation result.

## Known Source-Form Gaps

Integrated source trees are still absent for:

- `vendor/qcom/opensource/wlan/qcacld-3.0`
- `vendor/qcom/opensource/wlan/qca-wifi-host-cmn`

The touchscreen reconstruction is present, but the original ZTE source-form
driver has not been published. Standalone or third-party repositories remain
external references until independently audited and imported with provenance.
