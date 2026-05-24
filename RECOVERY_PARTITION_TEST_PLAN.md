# RM11 Pro Recovery Partition Test Plan

Device: RedMagic 11 Pro / NX809J / canoe

Date: 2026-05-24

Purpose: define the safest controlled path for validating recovery partition writes after proving that `fastboot boot` does not reproduce recovery-mode routing on this device.

---

## Current Model

- `fastboot boot` is available after the ABL unlock/restoration work.
- `fastboot boot` is not a valid recovery-mode test path on RM11 Pro.
- `fastboot boot` routes Android even with stock kernel + stock DTB + stock recovery ramdisk.
- Stock recovery boots correctly through `adb reboot recovery`.
- Stock recovery display and touch both work.
- `recovery.img` is ramdisk-only:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20458914]
RAMDISK_FMT     [lz4_legacy]
```

- The repacked stock recovery baseline is byte-identical to the E: ROM stock `recovery.img`, so recovery unpack/repack tooling is clean.

---

## Current Safe State

Last verified from Android/ADB:

```text
adb devices -l                         -> 912607710184 device product:NX809J-UN model:NX809J device:NX809J
adb shell getprop ro.boot.slot_suffix  -> _a
adb shell getprop sys.boot_completed   -> 1
adb shell getprop ro.boot.verifiedbootstate -> orange
adb shell getprop ro.boot.flash.locked -> 0
```

Known slot state from captured fastboot output:

```text
(bootloader) current-slot:a
(bootloader) slot-unbootable:a:no
(bootloader) slot-successful:a:yes
(bootloader) slot-unbootable:b:no
(bootloader) slot-successful:b:no
```

Do not continue if these values change unexpectedly.

---

## Recovery Partition Facts

Captured from:

```text
C:\RM11-test\after-abl-fastboot-getvar-all.txt
```

Relevant lines:

```text
(bootloader) partition-type:recovery_a:raw
(bootloader) partition-size:recovery_a: 0x6400000
(bootloader) partition-type:recovery_b:raw
(bootloader) partition-size:recovery_b: 0x6400000
(bootloader) current-slot:a
(bootloader) has-slot:boot:yes
```

No `has-slot:recovery` line was found in the captured output, but `recovery_a` and `recovery_b` exist as explicit raw partitions.

---

## Baseline Images

Stock E: ROM recovery:

```text
E:\Android\RM-11-Pro\BOOT\02-UL-Rom-16\images\recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Byte-identical repacked baseline:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
/home/richtofen/android/output/recovery/rm11-repacked-stock-recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Meaning: flashing the repacked baseline should be a no-op content write relative to stock recovery.

---

## Hard Stops

Do not proceed if any of these are true:

- ADB does not show the phone as `device`.
- Current slot is not `_a`.
- `sys.boot_completed` is not `1`.
- `ro.boot.flash.locked` is not `0`.
- Stock `adb reboot recovery` no longer reaches recovery UI.
- Recovery touch does not work in stock recovery.
- Stock recovery backup under E: is missing.
- The repacked baseline hash differs from stock recovery.
- You cannot access fastboot after rebooting bootloader.

Never use:

```text
C:\RM11-test\recovery\force-tests\force-recovery-mode.CRASHDUMP_DO_NOT_BOOT.img
```

Do not wipe data.

Do not modify or flash:

- `boot_a`
- `boot_b`
- `vendor_boot_a`
- `vendor_boot_b`
- `init_boot_a`
- `init_boot_b`
- `vbmeta`
- active slot metadata

No slot-B boot tests are needed for this loop.

---

## Preflight Verification Commands

Run from Windows PowerShell before any recovery partition write:

```powershell
mkdir C:\RM11-test\recovery-partition-plan -Force

adb devices -l
adb shell getprop ro.boot.slot_suffix
adb shell getprop sys.boot_completed
adb shell getprop ro.boot.verifiedbootstate
adb shell getprop ro.boot.flash.locked

Get-FileHash -Algorithm SHA256 `
  E:\Android\RM-11-Pro\BOOT\02-UL-Rom-16\images\recovery.img,`
  C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
```

Confirm stock recovery UI before writing:

```powershell
adb reboot recovery
```

Manual checks:

- recovery UI appears
- display works
- touch works
- do not wipe data
- return to Android with `Reboot system`

Then confirm fastboot state:

```powershell
adb reboot bootloader
fastboot devices
fastboot getvar all 2>&1 | Out-File -Encoding utf8 C:\RM11-test\recovery-partition-plan\fastboot-getvar-all.txt

Select-String -Path C:\RM11-test\recovery-partition-plan\fastboot-getvar-all.txt `
  -Pattern 'partition-size:recovery|partition-size:recovery_a|partition-size:recovery_b|has-slot:recovery|has-slot:boot|current-slot|slot-unbootable|slot-successful'
```

Expected:

```text
current-slot:a
partition-size:recovery_a: 0x6400000
partition-size:recovery_b: 0x6400000
slot-unbootable:a:no
slot-unbootable:b:no
```

---

## No-Op Recovery Partition Write

Purpose: validate the recovery partition writing workflow without changing recovery contents.

Candidate commands:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
```

Do not run these commands until all preflight checks pass.

Rationale for writing both:

- captured fastboot output exposes `recovery_a` and `recovery_b`
- both partitions are raw and the same size
- writing the byte-identical stock baseline to both slots avoids slot ambiguity
- no boot, vendor_boot, init_boot, vbmeta, or active-slot metadata is touched

Rollback command is the same content, using the original E: stock recovery:

```powershell
fastboot flash recovery_a E:\Android\RM-11-Pro\BOOT\02-UL-Rom-16\images\recovery.img
fastboot flash recovery_b E:\Android\RM-11-Pro\BOOT\02-UL-Rom-16\images\recovery.img
```

Because the baseline is byte-identical, rollback should be content-identical as well.

---

## Post-Write Validation

After the no-op write:

```powershell
fastboot reboot
```

Wait for Android, then:

```powershell
adb devices -l
adb shell getprop ro.boot.slot_suffix
adb shell getprop sys.boot_completed
adb shell getprop ro.boot.verifiedbootstate
adb shell getprop ro.boot.flash.locked
adb reboot recovery
```

Manual recovery checks:

- recovery UI appears
- display works
- touch works
- `View recovery logs` remains accessible
- no data wipe is performed

Pass criteria:

- Android boots normally after the write
- current slot remains `_a`
- recovery still boots through `adb reboot recovery`
- recovery display/touch still work

Fail criteria:

- black screen
- CrashDump
- bootloader loop
- recovery UI missing
- touch failure in stock recovery after byte-identical no-op write

If fail occurs:

- do not flash boot/vendor_boot/init_boot
- return to bootloader if possible
- reflash original E: stock recovery to `recovery_a` and `recovery_b`
- restore known-good boot path only if explicitly needed and already verified

---

## Later Minimal Recovery Ramdisk Test

Only after the byte-identical baseline write passes:

1. unpack stock `recovery.img`
2. add a harmless marker to the recovery ramdisk
3. repack as a ramdisk-only recovery image
4. flash only `recovery_a` and `recovery_b`
5. boot recovery through `adb reboot recovery`

Allowed marker ideas:

- add `/etc/rm11_custom_recovery_marker.txt`
- add a harmless comment/marker file under a non-executed path

Avoid:

- changing kernel or DTB
- changing `init_boot`
- changing `vendor_boot`
- changing `boot`
- changing `vbmeta`
- changing active slots
- changing recovery UI behavior before marker-only validation

Expected marker validation may require recovery ADB authorization or recovery logs. If recovery ADB remains unauthorized, use only visible/loggable changes that do not affect boot.

---

## Minimal Marker Recovery Image Result

Built a minimal recovery ramdisk marker image from the known-good byte-identical stock recovery baseline.

Image:

```text
C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
/home/richtofen/android/output/recovery/rm11-recovery-marker-ramdisk.img
SHA-256: FF1DE80E20EF2CBEA3391351D6F184EBF2023DE30888B1D524F3854D57C01367
```

Only ramdisk change:

```text
rm11_recovery_marker.txt
```

Marker content:

```text
RM11 recovery marker test
created=2026-05-24
source=rm11-repacked-stock-recovery.img
purpose=ramdisk-only marker validation
```

Header check:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20459088]
PAGESIZE        [4096]
CMDLINE         []
RAMDISK_FMT     [lz4_legacy]
VBMETA
```

Flashed partitions:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-ramdisk.img
```

Fastboot result:

```text
Sending 'recovery_a' (102400 KB) OKAY
Writing 'recovery_a' OKAY
Sending 'recovery_b' (102400 KB) OKAY
Writing 'recovery_b' OKAY
```

Post-flash Android state:

```text
adb devices      -> 912607710184 device
ro.boot.slot_suffix -> _a
sys.boot_completed -> 1
ro.boot.verifiedbootstate -> orange
ro.boot.flash.locked -> 0
```

No boot, vendor_boot, init_boot, vbmeta, or slot metadata was modified.

Next validation:

```powershell
adb reboot recovery
```

Manual checks:

- recovery UI appears
- display works
- touch works
- do not wipe data
- if recovery ADB becomes authorized, check for `/rm11_recovery_marker.txt`
- otherwise validate only that recovery still boots normally after marker-only ramdisk change

---

## Marker 001 Recovery Image Result

Built a second minimal marker image using the preferred marker text and preserving the existing `/etc -> /system/etc` symlink.

Note: in the stock recovery ramdisk, `/etc` is a symlink to `/system/etc`. To avoid replacing or disturbing that symlink, the marker was placed at:

```text
system/etc/rm11_recovery_marker.txt
```

At runtime, this should correspond to:

```text
/etc/rm11_recovery_marker.txt
```

Marker content:

```text
RM11 recovery ramdisk marker test 001
```

Image:

```text
C:\RM11-test\recovery\rm11-recovery-marker-001.img
/home/richtofen/android/output/recovery/rm11-recovery-marker-001.img
SHA-256: D14DA83E240888B711853E24196C60B647EE5166398F7E2FC27EDEECF535C61E
```

Header check:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20458935]
PAGESIZE        [4096]
CMDLINE         []
RAMDISK_FMT     [lz4_legacy]
VBMETA
```

Image size:

```text
104857600 bytes
```

Partition size:

```text
0x6400000 = 104857600 bytes
```

Flash commands executed:

```powershell
fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-001.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-001.img
fastboot --set-active=a
fastboot reboot
```

Fastboot result:

```text
Sending 'recovery_a' (102400 KB) OKAY
Writing 'recovery_a' OKAY
Sending 'recovery_b' (102400 KB) OKAY
Writing 'recovery_b' OKAY
Setting current slot to 'a' OKAY
```

Post-flash Android state:

```text
adb devices      -> 912607710184 device
ro.boot.slot_suffix -> _a
sys.boot_completed -> 1
ro.boot.verifiedbootstate -> orange
ro.boot.flash.locked -> 0
```

Recovery boot command sent:

```powershell
adb reboot recovery
```

Manual validation result: PASS.

- recovery UI appeared
- display worked
- touch worked
- no CrashDump
- no FTM
- no black screen
- no wipe
- recovery ADB appeared unauthorized, matching stock recovery behavior

Artifacts/logs:

```text
C:\RM11-test\recovery-marker-001-flash
```

Runtime marker visibility status:

- the marker is present in the built image at `system/etc/rm11_recovery_marker.txt`
- runtime path should be `/etc/rm11_recovery_marker.txt` because stock recovery has `/etc -> /system/etc`
- recovery ADB remains unauthorized, so direct runtime file verification is not available yet
- marker-001 still passes as a recovery partition safety test because the modified ramdisk boots stock recovery behavior cleanly

---

## Marker 002 Recommendation

Inspection of the stock recovery ramdisk found a very small `init.recovery.qcom.rc`:

```text
on init
    write /sys/class/backlight/panel0-backlight/brightness 200
    setprop sys.usb.configfs 1

on property:ro.boot.usbcontroller=*
    setprop sys.usb.controller ${ro.boot.usbcontroller}
    wait /sys/bus/platform/devices/${ro.boot.usb.dwc3_msm:-a600000.ssusb}/mode
    write /sys/bus/platform/devices/${ro.boot.usb.dwc3_msm:-a600000.ssusb}/mode peripheral
    wait /sys/class/udc/${ro.boot.usbcontroller} 1

on fs
    wait /dev/block/platform/soc/${ro.boot.bootdevice}
    symlink /dev/block/platform/soc/${ro.boot.bootdevice} /dev/block/bootdevice
```

Useful stock recovery facts:

- `/etc` is a symlink to `/system/etc`
- `system/bin/recovery`, `system/bin/minadbd`, `system/bin/logcat`, `system/bin/sh`, `toybox`, and `toolbox` exist
- recovery file contexts include `/tmp`, `/cache/recovery`, and `/data/misc/recovery`
- there is no obvious existing script that prints arbitrary marker files into visible recovery logs

Recommended marker-002 method:

1. Keep the marker static and harmless.
2. Add `system/etc/rm11_recovery_marker_002.txt`.
3. Add only a comment to `init.recovery.qcom.rc`, for static unpacked-image verification.
4. Do not add init actions, services, property changes, UI changes, or log-writing behavior yet.

Suggested marker-002 content:

```text
RM11 recovery ramdisk marker test 002
purpose=static marker only
source=rm11-repacked-stock-recovery.img
```

Suggested `init.recovery.qcom.rc` comment:

```text
# RM11 recovery marker 002: static ramdisk verification only.
```

Reasoning:

- marker-001 already proves recovery partition replacement can boot safely with a ramdisk-only change
- recovery ADB authorization is still the blocker for direct runtime file reads
- adding an init `exec`, `write`, property, service, or UI-visible mutation would increase behavioral risk before it gives a reliable verification path
- the next useful test should stay static unless recovery logs or authorized recovery ADB become available

Optional later runtime visibility path, after marker-002 static validation:

- solve recovery ADB authorization, then read `/etc/rm11_recovery_marker_002.txt`
- or use stock recovery `View recovery logs` if it can expose a controlled marker without changing init behavior
- only after that consider a low-risk runtime marker such as writing a file under `/tmp`; do not do this in marker-002

---

## Marker 002 Static Image Build Result

Built from the known-good byte-identical stock recovery baseline:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
SHA-256: 1158594EB464748CD5E9313CC10B6505CDD58FE03CE09A9E7DA0B3F7A1E4187D
```

Output image:

```text
C:\RM11-test\recovery\rm11-recovery-marker-002.img
/home/richtofen/android/output/recovery/rm11-recovery-marker-002.img
SHA-256: D8BF44E93C54EC61AB3D6810B01E21EE36E74A90CB0B458F7BBBFCF4928703C5
```

Static ramdisk changes only:

```text
system/etc/rm11_recovery_marker_002.txt
init.recovery.qcom.rc comment only
```

Marker content:

```text
RM11 recovery ramdisk marker test 002
static file only
no init actions
no services
```

Comment added to `init.recovery.qcom.rc`:

```text
# RM11 recovery marker 002 static validation
```

Header verification:

```text
HEADER_VER      [4]
KERNEL_SZ       [0]
RAMDISK_SZ      [20459092]
PAGESIZE        [4096]
CMDLINE         []
RAMDISK_FMT     [lz4_legacy]
VBMETA
```

Size verification:

```text
image size:      104857600 bytes
partition size:  0x6400000 = 104857600 bytes
```

Note: the image is exactly the same size as the stock/repacked recovery image and exactly matches the recovery partition size. This matches the already validated baseline and marker-001 artifacts.

Repacked ramdisk verification:

- `system/etc/rm11_recovery_marker_002.txt` exists after unpacking the final image
- marker content matches the requested static text
- `init.recovery.qcom.rc` contains only the marker comment; no init action, service, property, or UI change was added

Test commands, recovery partitions only:

```powershell
adb devices -l
adb shell getprop ro.boot.slot_suffix
adb shell getprop sys.boot_completed

adb reboot bootloader

fastboot flash recovery_a C:\RM11-test\recovery\rm11-recovery-marker-002.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-recovery-marker-002.img
fastboot --set-active=a
fastboot reboot
```

Post-flash validation:

```powershell
adb devices -l
adb shell getprop ro.boot.slot_suffix
adb shell getprop sys.boot_completed
adb reboot recovery
```

Manual recovery checks:

- recovery UI appears
- display works
- touch works
- no CrashDump
- no FTM
- no black screen
- do not wipe data

Rollback image:

```text
C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
```

Rollback commands:

```powershell
adb reboot bootloader
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot --set-active=a
fastboot reboot
```

---

## Next Technical Investigation

Compare stock recovery boot environment versus Android after `fastboot boot`:

- bootconfig
- `ro.bootmode`
- `ro.boot.bootreason`
- `ro.boot.slot_suffix`
- `init_boot` digest
- `vendor_boot` digest
- `misc` / BCB command
- `/proc/cmdline`
- `/proc/bootconfig`

Current limitation: `/proc/cmdline`, `/proc/bootconfig`, raw `misc`, dmesg, and pstore are restricted without root or authorized recovery ADB.
