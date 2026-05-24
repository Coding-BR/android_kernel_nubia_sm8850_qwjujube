# Current Project Note

This report remains relevant as a GPL/source-compliance inventory, but some project-local recovery work has progressed since it was written.

Current local updates:
- `zte_tpd` reconstruction/refactor work exists under `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/`.
- The touchscreen refactor branch is `codex/zte-tpd-input-device-refactor`.
- Physical validation is still pending.
- This does not replace a complete vendor GPL source release.

---

# ZTE RedMagic 11 Pro (NX809J) — GPL Kernel Source Compliance Report

**Subject:** Formal Request for Missing Source Code, Headers, and Configuration Files in the Android 16 Kernel Source Release  
**Device:** RedMagic 11 Pro (NX809J)  
**SoC:** Qualcomm Snapdragon 8 Gen 5 (SM8850, codename `canoe` / board `qwjujube`)  
**Kernel Version:** 6.12.23-android16  
**Contact Email:** [idealcreativesuporte@idealcreative.com.br](mailto:idealcreativesuporte@idealcreative.com.br)  
**Date:** 2026-05-19  

---

## 1. Executive Summary

A complete compilation audit of the GPL kernel source release for the NX809J has been performed using the AOSP Clang r536225 toolchain targeting `arm64`. While the base GKI kernel image (`Image`) compiles successfully, the compilation of out-of-tree vendor driver modules (`vendor/qcom/opensource/`) reveals that **multiple required components are missing from the delivered source package**.

This document enumerates all missing files required to achieve a complete, native, unmodified compilation of all kernel modules present in the official firmware.

Under the terms of the GNU General Public License v2 (GPLv2), the complete and corresponding source code for all GPL-licensed kernel modules shipped in the firmware must be provided.

---

## 2. Missing Components

### 2.1 Platform Defconfig Fragment

* **Missing File:** `canoe.fragment` (or equivalent platform configuration fragment for SM8850/canoe)
* **Expected Location:** `kernel_platform/common/arch/arm64/configs/` or equivalent
* **Impact:** Compiling with the default `gki_defconfig` leaves essential Qualcomm SoC-level drivers disabled (e.g., `CONFIG_QCOM_SCM`, `CONFIG_ARM_SMMU`, `CONFIG_QCOM_RPMH`, `CONFIG_INTERCONNECT_QCOM`, regulators, clock controllers). This prevents the compilation of approximately **194 platform-level kernel modules** that are present and loaded in the production firmware.
* **Evidence:** The running device loads modules such as `clk_rpmh`, `gcc_canoe`, `gpucc_canoe`, `dispcc_canoe`, `arm_smmu`, `qcom_rpmh`, etc., but the corresponding Kconfig options are not enabled in the delivered defconfig.

---

### 2.2 Missing WLAN Host Driver Source Code

* **Missing Repositories:**
  * `vendor/qcom/opensource/wlan/qcacld-3.0/`
  * `vendor/qcom/opensource/wlan/qca-wifi-host-cmn/`
* **What Was Delivered:** Only `fw-api/` (firmware API headers) and `wlan-devicetree/` (DT bindings)
* **Impact:** The actual WLAN host driver source code is entirely absent. The production firmware loads `qca_cld3_peach_v2.ko` (3.8MB) plus supporting modules (`cnss2.ko`, `cnss_nl.ko`, `cnss_prealloc.ko`, `cnss_utils.ko`, `cnss_plat_ipc_qmi_svc.ko`, `wlan_firmware_service.ko`) — a total of **7 WLAN modules** that cannot be compiled.

---

### 2.3 Missing Multi-Media Resource Manager (MMRM) Source Code

* **Missing Path:** `vendor/qcom/opensource/mmrm-driver/`
* **Missing Header:** `<linux/soc/qcom/msm_mmrm.h>`
* **Impact:** The display drivers (`sde_io_util.c`, `dp_display.c`), camera-kernel, and video-driver all reference MMRM structures and functions. The production firmware loads `msm_mmrm.ko`. The complete driver source and header are absent from the release.

---

### 2.4 Missing Synx (Synchronization Framework) Source Code

* **Missing Path:** `vendor/qcom/opensource/synx-kernel/`
* **Missing Headers:** `<synx_api.h>`, `<synx_interop.h>`
* **Impact:** The multimedia hardware fence module (`mm-drivers/hw_fence/`) fails to compile:
  ```
  hw_fence_drv_priv.c:9:10: fatal error: 'synx_api.h' file not found
  ```
  The production firmware loads `synx_driver.ko`. Both the driver source and API headers are absent.

---

### 2.5 Missing Platform Kernel Headers

The following platform header files are referenced by the delivered techpack source code but are absent from the kernel tree (`kernel_platform/common/include/`):

| # | Missing Header | Referenced By | Compilation Error |
|---|---------------|---------------|-------------------|
| 1 | `<linux/mem-buf.h>` | `graphics-kernel/kgsl.c`, `securemsm-kernel/`, `video-driver/` | `fatal error: 'linux/mem-buf.h' file not found` |
| 2 | `<linux/msm_ion.h>` | `securemsm-kernel/qseecom/qseecom.c` | `fatal error: 'linux/msm_ion.h' file not found` |
| 3 | `<linux/msm_dma_iommu_mapping.h>` | `video-driver/driver/vidc/src/msm_vidc_probe.c` | `fatal error: 'linux/msm_dma_iommu_mapping.h' file not found` |
| 4 | `<linux/qcom-iommu-util.h>` | `display-drivers/msm/msm_smmu.c` | Undefined references to `qcom_iommu_sid_switch`, `qcom_iommu_enable_s1_translation` |
| 5 | `<soc/qcom/minidump.h>` | `display-drivers/msm/sde_dbg.c`, `graphics-kernel/`, `camera-kernel/` | Undefined references to `msm_minidump_add_region`, `qcom_va_md_register` |
| 6 | `<linux/hdcp_qseecom.h>` | `display-drivers/msm/dp/dp_hdcp2p2.c`, `securemsm-kernel/` | Undefined structs `hdcp2_app_data`, `hdcp1_topology`, `hdcp2_buffer` |
| 7 | `<linux/soc/qcom/battery_charger.h>` | `audio-kernel/asoc/codecs/swr-haptics.c` | Undefined `VMAX_CLAMP`, `register_hboost_event_notifier` |
| 8 | `<linux/qti-regmap-debugfs.h>` | `audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc.c` | `fatal error: 'linux/qti-regmap-debugfs.h' file not found` |
| 9 | `<linux/soc/qcom/msm_mmrm.h>` | `display-drivers/`, `camera-kernel/`, `video-driver/` | See section 2.3 |

---

### 2.6 Missing/Incomplete API Declarations in Delivered Kernel Headers

The following functions are called by the delivered techpack source code but are **not declared** in the corresponding kernel headers:

| # | Missing Function | Header File | Referenced By |
|---|-----------------|-------------|---------------|
| 1 | `rproc_set_state()` | `<linux/remoteproc/qcom_rproc.h>` | `mm-drivers/hfi_core/src/hfi_transport/hfi_smmu.c` |
| 2 | `msm_gpio_mpm_wake_set()` | `<linux/pinctrl/qcom-pinctrl.h>` | `audio-kernel/asoc/codecs/msm-cdc-pinctrl.c`, `bt-kernel/pwr/btpower.c` |
| 3 | `msm_gpio_get_pin_address()` | `<linux/pinctrl/qcom-pinctrl.h>` | `display-drivers/msm/dsi/dsi_display.c` |
| 4 | `socinfo_get_part_info()` | `<soc/qcom/socinfo.h>` | `display-drivers/msm/sde/sde_hw_catalog.c` |
| 5 | `socinfo_get_part_count()` | `<soc/qcom/socinfo.h>` | `display-drivers/msm/sde/sde_hw_catalog.c` |
| 6 | `socinfo_get_subpart_info()` | `<soc/qcom/socinfo.h>` | `display-drivers/msm/sde/sde_hw_catalog.c` |

---

### 2.7 Missing ZTE Proprietary Module Source Code

The following **12 ZTE-branded kernel modules** are loaded in the production firmware but their source code is absent from the delivered package:

| # | Module | Function |
|---|--------|----------|
| 1 | `zte_charger_policy.ko` | Battery charging policy control |
| 2 | `zte_power_supply.ko` | Power supply management |
| 3 | `zte_led.ko` | RGB LED driver |
| 4 | `zte_fingerprint.ko` | Fingerprint sensor driver |
| 5 | `zte_misc.ko` | Miscellaneous ZTE utilities |
| 6 | `zte_ir.ko` | Infrared (IR blaster) driver |
| 7 | `zte_tpd.ko` | Touch Protection Driver |
| 8 | `zte_imem_info.ko` | IMEM information driver |
| 9 | `zte_sensor_sensitivity.ko` | Sensor calibration driver |
| 10 | `zte_stats_info.ko` | System statistics driver |
| 11 | `zte_reboot_ext.ko` | Extended reboot driver |
| 12 | `zte_ramdisk_reboot.ko` | Ramdisk reboot driver |

---

### 2.8 Missing Nubia/Third-Party Module Source Code

The following modules are loaded in the production firmware but their source code is not included:

| # | Module | Function |
|---|--------|----------|
| 1 | `gpio_keys_nubia.ko` | Nubia custom GPIO keys driver |
| 2 | `nubia_hw_version.ko` | Hardware version identification |
| 3 | `haptic_86938.ko` | Awinic haptic motor driver (AW86938) |
| 4 | `aw86320.ko` | Awinic LED driver (AW86320) |
| 5 | `aw9620x.ko` | Awinic SAR sensor driver |
| 6 | `awp1921.ko` | Awinic power driver |

---

### 2.9 Build System Errors in Audio Techpack

* **File:** `vendor/qcom/opensource/audio-kernel/dsp/aw882xx/Kbuild`, line 89
* **Error 1 (Syntax):** Missing `-I` compiler flag prefix in include directive:
  ```
  $(ROOT_DIR)/msm-kernel/include #add by zte QC 20220901
  ```
  This causes Clang to treat the directory path as a source file input.
* **Error 2 (Path):** References `/msm-kernel/include` but the kernel tree is delivered under `kernel_platform/common/`.
* **Error 3 (Recursion):** The `audio-kernel/Makefile` lacks `MODNAME=audio` in `KBUILD_OPTIONS`, causing an infinite build recursion loop in `asoc/codecs/Kbuild`.

---

## 3. Formal Request

Under the GNU General Public License v2 (GPLv2), Section 3, we formally request the following items to achieve complete and corresponding source code for the NX809J production firmware:

1. **Platform configuration fragment** (`canoe.fragment` or equivalent defconfig) for SM8850/canoe enabling all Qualcomm platform drivers
2. **WLAN host driver source code** (`qcacld-3.0` and `qca-wifi-host-cmn` repositories)
3. **MMRM driver source code** (`mmrm-driver/`) and header (`msm_mmrm.h`)
4. **Synx driver source code** (`synx-kernel/`) and headers (`synx_api.h`, `synx_interop.h`)
5. **All 9 missing platform kernel headers** listed in Section 2.5
6. **All 6 missing function declarations** listed in Section 2.6
7. **Source code for all 12 ZTE proprietary modules** listed in Section 2.7
8. **Source code for all 6 Nubia/third-party modules** listed in Section 2.8
9. **Corrections to the 3 build system errors** in the audio techpack listed in Section 2.9

---

*This report was generated through exhaustive compilation testing of the delivered source package. All referenced modules, headers, and functions were verified against the production firmware running on a physical NX809J device (kernel `6.12.23-android16-OP-WILD`).*
