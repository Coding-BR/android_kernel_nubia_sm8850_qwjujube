# RedMagic 11 Pro (NX809J) - Kernel Compilation and Boot Guide

> [!NOTE]
> Para a versão em português deste guia, consulte o arquivo [README_PT.md](README_PT.md).

This guide explains how to compile, pack, and boot the custom kernel with **KernelSU-Next** and **Binary Parity** support via RAM (without permanently flashing the phone's partitions), and outlines the next steps for project development.

---

## 📋 Prerequisites

To ensure compilation and packaging work correctly, you must supply the following components (which are configured in `.gitignore` to keep the repository clean):

1. **Clang Toolchain (r536225)**:
   - Download the AOSP Clang toolchain (revision `r536225`).
   - Extract it to the root of this repository in a folder named `clang-r536225/` (such that the main binary is located at `clang-r536225/bin/clang`).
   - Alternatively, you can set the `CLANG_DIR` environment variable pointing to your Clang directory before compiling.

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

## 🧪 2. Recovery Touchscreen Test Image

The current custom recovery work does **not** start by flashing `recovery.img`. On the NX809J, the stock `recovery.img` is a ramdisk-only Android boot image:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_FMT     [lz4_legacy]
```

For the first safe test, use a boot-header based image that combines:

* stock E: ROM `boot.img` as the boot container
* stock E: ROM `recovery.img` ramdisk
* rebuilt kernel `Image`
* rebuilt `dtb.img`
* built-in `zte_tpd` touchscreen refactor from `codex/zte-tpd-input-device-refactor`

Current first-test artifact:

```text
/home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
SHA-256: da0cc68e4d814927d8232dadafd27a4c0741b8e547f2f457e1325113cf15788f
```

Boot it temporarily:

```bash
fastboot boot /home/richtofen/android/output/recovery/rm11-e-rom-recovery-fastboot-fixed-kernel-bootbase.img
```

> [!WARNING]
> Do **not** use `fastboot flash` for this image. It is only a temporary boot test.

Decision path:

| Result | Next action |
| :--- | :--- |
| Recovery boots and touch works | Begin custom recovery ramdisk adaptation using this kernel base |
| Recovery boots but touch does not work | Collect logs and continue `zte_tpd` repair |
| CrashDump/reboot | Boot stock again and collect `scratch/ramoops.log` |

The full manifest is stored at:

```text
/home/richtofen/android/output/recovery/RM11_RECOVERY_TEST_MANIFEST.md
```

---

## 🛠️ 3. Project Next Steps

Now that we have achieved stable binary parity compatibility (where the custom kernel loads and runs the system normally), we can begin the specific development tasks described in [NEXT_STEPS.md](NEXT_STEPS.md):

### 1. Validate `zte_tpd` in Recovery
The active touchscreen path for this device is the reconstructed ZTE/Synaptics driver under:

```text
kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/
```

The current branch `codex/zte-tpd-input-device-refactor` fixes the obvious `struct input_dev` corruption risk by replacing raw offset writes such as `device + 656` and `device + 712` with named fields and kernel input helpers. The next proof point is physical testing through the recovery image above.

### 2. GPU Overclock to 1200MHz+
* Extract and decompile the original Device Tree (`dtb.img` or `dtbo.img`) into text format (`.dts`).
* Locate the OPP (Operating Performance Points) table for the Adreno 830 GPU.
* Add the overclock frequency steps (with correct voltage bindings) to achieve a stable 1200MHz+.
* Recompile the tree and test via `fastboot boot`.

### 3. Full Integration of Other Techpacks
* Continue `zte_tpd` validation from real recovery logs instead of starting from generic `goodix` or `focaltech` touch modules.
* Validate mobile network connectivity (`dataipa`).

### 4. Audit of Proprietary Locks and CFI
* Monitor panic logs (`dmesg` and `console-ramoops-0`) to map any remaining CFI (Control Flow Integrity) failures in the loaded modules.

---

## 📬 Contact & Support

If you have questions, need support, or want to collaborate on this reverse engineering project, you can reach out via:
* 📧 **Email:** [idealcreativesuporte@idealcreative.com.br](mailto:idealcreativesuporte@idealcreative.com.br)
* 💬 **Telegram Group:** [Join Group](https://web.telegram.org/a/#-1003471835008_307)
* 🌐 **XDA Thread:** [RedMagic 11 Pro Unlock & Guide](https://xdaforums.com/t/red-magic-11-pro-guide-bootloader-unlock-free-also-support-rm10-pad3pro-z70u-z80u-unlock-zte-family-toolbox.4780930/)
