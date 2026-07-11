# RedMagic 11 Pro (NX809J) - Kernel Compilation and Boot Guide

> [!NOTE]
> Para a versão em português deste guia, consulte o arquivo [README_PT.md](README_PT.md).

This guide explains how to compile, pack, and boot the custom kernel with **KernelSU-Next** and **Binary Parity** support via RAM (without permanently flashing the phone's partitions), and outlines the next steps for project development.

---

## 🔴 CURRENT BLOCKER: Boot Hang on RedMagic Logo

> [!WARNING]
> **Build #8** boots past early-init but **hangs indefinitely on the RedMagic logo screen**.
> The root cause has been identified as an **SCMI timeout** on `arm-scmi.1.auto` that prevents the ADSP/CDSP remoteproc, keymint, and vold from starting — which blocks `/data` decryption (FBE).
>
> **Community help is needed!** If you have experience with Qualcomm SCMI, GKI KCFI, or Android FBE boot flow, please check the diagnostic documents below.

| Document | Language | Description |
|----------|----------|-------------|
| **[BOOT_HANG_DIAGNOSTIC_PT.md](BOOT_HANG_DIAGNOSTIC_PT.md)** | 🇧🇷 Português | Diagnóstico completo do hang de boot com dados de ramoops, comparação stock vs custom, e guia de investigação |
| **[BOOT_HANG_DIAGNOSTIC_EN.md](BOOT_HANG_DIAGNOSTIC_EN.md)** | 🇺🇸 English | Full boot hang diagnostic with ramoops data, stock vs custom comparison, and investigation guide |
| **[NOTIFICACAO_COMPLIANCE_GPL_ZTE.md](NOTIFICACAO_COMPLIANCE_GPL_ZTE.md)** | 🇧🇷 Português | Notificação formal de descumprimento da licença GPL v2 e lista de fontes em falta para enviar à ZTE |
| **[GPL_COMPLIANCE_NOTICE_ZTE.md](GPL_COMPLIANCE_NOTICE_ZTE.md)** | 🇺🇸 English | Formal GPL v2 compliance notice and missing components report to send to ZTE support |

---

## 📋 Prerequisites

To ensure compilation and packaging work correctly, you must supply the following components (which are configured in `.gitignore` to keep the repository clean):

1. **Clang Toolchain (r536225)**:
   - The exact AOSP revision and compiler hash are pinned in `toolchains.lock.json`.
   - Reversa resolves the default copy from `${XDG_CACHE_HOME:-$HOME/.cache}/reversa/toolchains/android-clang-linux-x86/clang-r536225`.
   - Set `CLANG_DIR` only when intentionally using another location; the resolver still enforces the pinned compiler identity and hash.

2. **Device Tree Blob (`dtb.img`)**:
   - Extract the official `dtb.img` file from the stock ZTE boot image (`boot.img`) corresponding to your target userdebug ROM.
   - Place the `dtb.img` file directly in the root of this repository. It is required to concatenate the Device Trees to the kernel image during packaging.

3. **Host Dependencies**:
   - Ensure your Linux machine has the essential build packages installed (`build-essential`, `libssl-dev`, `bison`, `flex`, `libelf-dev`, `python3`, etc.).

---

## 🔓 Fastboot Enablement Guide

To enable Fastboot flashing and commands on the RedMagic 11 Pro (NX809J) and other ZTE/Nubia devices, follow these steps:

### 1. Install Fastboot & ADB
* **Windows**: Download the official [Android SDK Platform-Tools](https://developer.android.com/tools/releases/platform-tools) and add them to your system's PATH.
* **Linux (Ubuntu/Debian)**: Install via apt:
  ```bash
  sudo apt update
  sudo apt install android-tools-adb android-tools-fastboot
  ```
* **macOS**: Install via Homebrew:
  ```bash
  brew install android-platform-tools
  ```

### 2. Flash Unlocked ABL via EDL Mode
To enable Fastboot, you must first swap your device's stock Bootloader image (`abl`) with the custom version. Since Fastboot is locked/disabled by default on stock ROMs, **you do not have Fastboot access yet**. You must write the file in **EDL Mode**:
1. Boot your device into EDL Mode (Emergency Download Mode) and open the **ZTE Family Toolbox** (ZTE Toolbox).
2. Use the toolbox to backup/dump your official **`abl_a`** and **`abl_b`** partitions (keep these backups safe!).
3. Write the custom unlocked ABL image provided in this repository (🔴 **`abl_unlock.elf`**) to both slots (`abl_a` and `abl_b`) by selecting **Option 12 (Write Partition)** in the ZTE Family Toolbox.
4. > [!IMPORTANT]
   > **ZTE Toolbox Option 19**: Immediately after writing the unlocked ABL via the ZTE Family Toolbox, you **MUST** run **Option 19** in the ZTE Family Toolbox to clear the device boot/temp flags. If you skip this, the device will trigger a boot lockout and boot into **Dumper Mode** (Crash Dump screen) on the next boot.
5. > [!WARNING]
   > **DO NOT use Option 18 (Fingerprint Fix)** in the ZTE Family Toolbox! Even though it is labeled to fix the fingerprint reader, executing Option 18 will corrupt the boot configuration, triggering a boot lockout and causing the device to boot loop back into **Dumper Mode**. Only use Option 19 to clear flags and avoid Option 18 entirely.

### 3. Disable vbmeta Verification
Once 🔴 **`abl_unlock.elf`** is successfully written and the boot state is cleared, reboot the device. It will now allow you to enter and run commands in **Fastboot Mode**:
1. Reboot the device into Fastboot Mode.
2. You **MUST** flash the stock `vbmeta` and `vbmeta_system` images with verification disabled to prevent boot loops or AVB verification issues:
   ```bash
   fastboot --disable-verity flash vbmeta_a vbmeta.img
   fastboot --disable-verity flash vbmeta_b vbmeta.img
   fastboot --disable-verity --disable-verification flash vbmeta_system_a vbmeta_system.img
   fastboot --disable-verity --disable-verification flash vbmeta_system_b vbmeta_system.img
   ```
   > [!CAUTION]
   > **DO NOT** use the `--disable-verification` flag when flashing `vbmeta_a` or `vbmeta_b` (only use `--disable-verity`). If you use `--disable-verification` on `vbmeta`, your device will get stuck in the bootloader and will not boot up, requiring a full recovery in **EDL Mode** to fix!
3. Finally, unlock the bootloader (if desired) by running:
   ```bash
   fastboot flashing unlock
   ```
   Confirm the unlock on the device screen using the volume keys and power button.

---

## 🚀 1. How to Compile and Boot the Kernel

### Step A: Compile the Main Kernel and Techpacks
We use the unified script [super_build.sh](super_build.sh) which sets up the environment, applies the platform defconfig (`nx809j_defconfig`), injects security flags (CFI Permissive, KernelSU-Next), and compiles the binaries with the appropriate Clang toolchain.

Run in your terminal:
```bash
./super_build.sh
```
* The kernel will be compiled and the resulting image will be located at: `kernel_platform/common/arch/arm64/boot/Image`.

---

### Step B: Pack and Sign the Boot Image (DTB + AVB)
Qualcomm's dynamic drivers require the physical concatenation of Device Trees (`dtb.img`) into the kernel header. The script [repack_perfect_sign.sh](repack_perfect_sign.sh) handles packaging without an internal ramdisk (exact size of 64MB) and performs the mandatory cryptographic signing via AVB to avoid bootloader lockouts.

Run:
```bash
./repack_perfect_sign.sh
```
* This will generate the signed boot image in the root of the project: **`dev_reverse_perfect.img`**.

---

### Step C: Boot in RAM (Temporary Boot)

> [!IMPORTANT]
> **Compiler and AnyKernel Compatibility:**
> To run this current kernel, the device **must already have** a working kernel compiled with **Clang 20** installed.
> Therefore, before attempting to perform a temporary boot in RAM (`fastboot boot`), you need to flash the boot image on the smartphone using **AnyKernel** so that the necessary patches like **SUSFS** and **Linux Droidspaces** are properly enabled.
> Otherwise, the temporary boot will not be stable or compatible for development, since the original stock kernel uses an older Clang version.

> [!WARNING]
> **NEVER run `fastboot flash boot` or flash this image permanently to your device during the testing phase.** The boot must always be loaded temporarily in RAM. If the system hangs or presents issues, simply holding the Power button for 10 seconds will restore the original official boot.

1. Reboot your smartphone into bootloader mode:
   ```bash
   adb reboot bootloader
   ```
2. Boot the image directly into RAM using fastboot:
   ```bash
   sudo fastboot boot dev_reverse_perfect.img
   ```
   *(Enter your sudo password if the terminal prompts for root privileges to establish USB communication with fastboot).*

3. The phone will boot into Android. Once booted, you can validate the active kernel by running:
   ```bash
   adb shell uname -a
   ```
   You should see the signature indicating your custom build with active **KernelSU-Next** support.

---

## 📦 2. ZTE Drivers Architecture (obj-y vs obj-m)

In the [drivers/soc/qcom/zte/Makefile](kernel_platform/common/drivers/soc/qcom/zte/Makefile), the drivers are configured as follows:

```makefile
obj-y += zte_misc/
obj-y += zte_power_supply/
obj-y += zte_imem_info/
obj-y += zte_stats_info/
obj-y += zte_sensor_sensitivity/
obj-y += zte_ir/
obj-y += zte_reboot_ext/
obj-y += zte_ramdisk_reboot/
obj-y += zte_led/
obj-y += zte_fingerprint/
obj-y += zte_charger_policy/
obj-m += zte_tpd/
```

### What does `y` and `m` mean?

*   **`obj-y` (Built-in / Statically Linked):**
    The driver code is compiled directly into the main kernel executable binary (`vmlinux` / `Image`).
    *   **How it loads:** It is loaded into RAM as part of the kernel boot process at the very early init stages.
    *   **Advantages:** It does not require any external `.ko` file to be loaded from the storage. It starts instantly and bypasses userspace loading signatures or late binding restrictions.

*   **`obj-m` (Modular / Loadable Kernel Module):**
    The driver code is compiled as an independent dynamic binary with a `.ko` (Kernel Object) extension (e.g., `zte_tpd.ko`).
    *   **How it loads:** The main kernel starts without this code. The operating system (Android) loads it at runtime using commands like `insmod` or `modprobe` (typically from `/vendor_dlkm/lib/modules/` or via a KernelSU boot script during `post-fs-data`).
    *   **Advantages:** Highly flexible; allows dynamic replacement, resolving of duplicate symbol collisions during build time, and hot-plug loading.

---

### Which drivers are fully open-source compiled vs. prebuilt phone dependencies?

Because this project is built for testing via `fastboot boot` (running a custom kernel temporarily in RAM while mounting the phone's physical partitions), the driver stack is split into two categories:

#### 1. Drivers Compiled from Open-Source in this Tree:
*   **Touchscreen (`zte_tpd.ko`):** Rebuilt dynamically as a modular driver (`obj-m`), optimized with our custom dynamic platform allocations and unbound workqueue fixes to prevent suspend watchdog bites.
    *   **How it loads and runs:** Since the phone's physical `/vendor_dlkm` partition is mounted as Read-Only, the custom `zte_tpd.ko` driver is loaded using **KernelSU-Next**. The compiled file is copied to `/data/adb/modules/zte_tpd_only/vendor_dlkm/lib/modules/zte_tpd.ko`. To ensure the overlay (bind-mount) applies correctly on Android 16, we create a script hook at `/data/adb/modules/zte_tpd_only/post-fs-data.sh` with the command (`mount -o bind /data/adb/modules/zte_tpd_only/vendor_dlkm/lib/modules/zte_tpd.ko /vendor_dlkm/lib/modules/zte_tpd.ko`). During boot, KernelSU executes this script to swap the active module seamlessly and safely.
*   **Core ZTE System Drivers:** All 11 drivers under `obj-y` (`zte_misc`, `zte_led`, `zte_fingerprint`, `zte_charger_policy`, etc.) are built-in directly to the compiled `Image`.
*   **KernelSU-Next:** Integrated and compiled statically inside the core kernel.
*   **Compatibility Stubs (`zte_parity.c`):** Establishes ABI parity, satisfying internal symbol links required by the closed-source parts.

#### 2. Prebuilt Drivers Loaded from the Smartphone (ROM dependencies):
When booting from RAM, Android mounts the phone's official physical `/vendor` and `/vendor_dlkm` partitions.
Therefore, the device loads the original Qualcomm closed-source modules from these partitions:
*   **Camera & Display:** `camera.ko`, `msm_drm.ko`, `msm_video.ko`.
*   **Audio & Power:** Audio codec drivers (`wcd939x_dlkm.ko`), haptics, and charge controller.
*   **Connectivity:** Mobile data modem drivers (`dataipa.ko`) and Wi-Fi host driver (`qca_cld3_peach_v2.ko`).

*Note: Because our custom kernel maintains perfect ABI parity with the stock ROM (e.g. keeping CONFIG_SCHED_CLASS_EXT and CONFIG_DEBUG_INFO_BTF options active), the prebuilt vendor modules load and run flawlessly without causing crash dumps.*

---

## 🛠️ 3. Project Next Steps

Now that we have achieved stable binary parity compatibility (where the custom kernel loads and runs the system normally), we can begin the specific development tasks described in [NEXT_STEPS.md](NEXT_STEPS.md):

### 1. Deploy and Test Rebuilt Modules (.ko)
Since booting via RAM uses the physical partitions and mounts `/vendor_dlkm` from the original system, the active modules at the moment are ZTE's original ones. To execute our rebuilt modules (reconstructed from Ghidra source code):
* **Built-in Method:** Modify the `Kbuild` of the drivers we rebuilt (such as `zte_charger_policy` and `zte_led`), changing the compilation to built-in (`obj-y` instead of `obj-m`). This way, they will be compiled directly inside the `Image` file and loaded before the system initializes the official ones.

### 2. GPU Overclock to 1200MHz+
* Extract and decompile the original Device Tree (`dtb.img` or `dtbo.img`) into text format (`.dts`).
* Locate the OPP (Operating Performance Points) table for the Adreno 830 GPU.
* Add the overclock frequency steps (with correct voltage bindings) to achieve a stable 1200MHz+.
* Recompile the tree and test via `fastboot boot`.

### 3. Full Integration of Other Techpacks
* Adapt and compile the touch modules (`touch-drivers`) to validate physical screen interaction using our tree.
* Validate mobile network connectivity (`dataipa`).

### 4. Audit of Proprietary Locks and CFI
* Monitor panic logs (`dmesg` and `console-ramoops-0`) to map any remaining CFI (Control Flow Integrity) failures in the loaded modules.

---

## 📬 Contact & Support

If you have questions, need support, or want to collaborate on this reverse engineering project, you can reach out via:
* 📧 **Email:** [idealcreativesuporte@idealcreative.com.br](mailto:idealcreativesuporte@idealcreative.com.br)
* 💬 **Telegram Group:** [Join Group](https://t.me/redmagic11PR0) (Or direct contact: [@idealcreative](https://t.me/idealcreative))
* 🌐 **XDA Thread:** [RedMagic 11 Pro Unlock & Guide](https://xdaforums.com/t/red-magic-11-pro-guide-bootloader-unlock-free-also-support-rm10-pad3pro-z70u-z80u-unlock-zte-family-toolbox.4780930/)

## Local GitHub Releases

This repository is intended to be built on the maintainer machine/container. A successful local build can publish the exact output to GitHub Releases with:

`ash
export GITHUB_TOKEN=<token with repo scope>
python3 scripts/publish_local_release.py /path/to/build/artifacts --build-log /path/to/build.log
`

The local release package includes the generated boot images, kernel Image, mlinux, modules, kernel.config, build log, DTBs archive, and an opensource-<commit>.tar.gz archive created from the exact Git commit used for that build.
