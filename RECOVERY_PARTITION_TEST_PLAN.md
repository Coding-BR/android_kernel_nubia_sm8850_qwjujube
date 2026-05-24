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
