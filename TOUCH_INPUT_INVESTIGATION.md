# RM11 Pro Touch Input Investigation

Device: RedMagic 11 Pro / NX809J / canoe

Date: 2026-05-24

Purpose: collect Android-side touchscreen/input evidence and compare it with recovery ramdisk assumptions before making any recovery or kernel changes.

---

## Capture Package

Windows capture folder:

```text
C:\RM11-test\touch-input-evidence-001
```

Captured files:

```text
adb-devices.txt
dev-input-list.txt
dmesg.txt
dumpsys-input.txt
getevent-lp.txt
getprop.txt
logcat-all.txt
proc-bus-input-devices.txt
slot_suffix.txt
sys-class-input-find.txt
sys-class-input.txt
sys_boot_completed.txt
```

Safe state during capture:

```text
ro.boot.slot_suffix = _a
sys.boot_completed  = 1
```

Permission-limited captures:

- `adb shell dmesg` returned `klogctl: Permission denied`
- `adb shell cat /proc/bus/input/devices` returned `Permission denied`
- `sys-class-input-find.txt` was empty from the unprivileged command

No flashing, recovery reboot, boot image modification, kernel change, or recovery image build was performed for this capture.

---

## Android Working Input Evidence

Android exposes the touchscreen as a normal multitouch evdev input device.

Primary touchscreen device from `getevent -lp`:

```text
event node: /dev/input/event9
name:       synaptics_tcm_touch
props:      INPUT_PROP_DIRECT
```

Capabilities:

```text
KEY_WAKEUP
BTN_TOOL_FINGER
BTN_TOUCH
ABS_X
ABS_Y
ABS_MT_SLOT           min 0 max 9
ABS_MT_TOUCH_MAJOR    min 0 max 255
ABS_MT_TOUCH_MINOR    min 0 max 255
ABS_MT_POSITION_X     min 0 max 12159
ABS_MT_POSITION_Y     min 0 max 26879
ABS_MT_TRACKING_ID    min 0 max 65535
```

Android `dumpsys input` confirms:

```text
Device: synaptics_tcm_touch
Path: /dev/input/event9
Location: synaptics_tcm/touch_input
Classes: KEYBOARD | TOUCH | TOUCH_MT
Sources: KEYBOARD | TOUCHSCREEN
Identifier: bus=0x0000, vendor=0x0000, product=0x0001, version=0x0001
Touch Input Mapper mode: DISABLED
GestureMode: MULTI_TOUCH
DeviceType: TOUCH_SCREEN
InputDeviceOrientation: Rotation0
```

Observed device nodes from `/dev/input`:

```text
event0  gpio-keys
event1  pmic_pwrkey
event2  pmic_resin
event3  gpio-keys_nubia
event4  nubia_tgk_aw_sar0_ch0
event5  nubia_tgk_aw_sar1_ch0
event6  goodix_fp
event7  canoe-mtp-wsa884x-snd-card Headset Jack
event8  canoe-mtp-wsa884x-snd-card Button Jack
event9  synaptics_tcm_touch
```

The touchscreen node permissions in Android are:

```text
crw-rw---- root input /dev/input/event9
```

Relevant Android logcat clues:

- kernel boot log includes `input device check on`
- logcat is dominated by unrelated boot/platform noise, but it does include kernel and init output
- no clear fatal touchscreen error was identified in the first keyword pass

Important interpretation:

- Android does not need a special user-visible app service for the basic input node to exist.
- The working input path is compatible with generic evdev-style handling.
- The main question for recovery is whether `synaptics_tcm_touch` exists and can be opened by recovery/minui during recovery boot.

---

## Recovery Ramdisk Input Assumptions

Static recovery ramdisk inspection shows generic input support rather than a device-specific touchscreen userspace stack.

Relevant files and assumptions:

```text
system/etc/ueventd.rc
plat_file_contexts
plat_property_contexts
prop.default
default.prop
vendor_file_contexts
system/bin/recovery
```

`system/etc/ueventd.rc`:

```text
subsystem input
    devname uevent_devpath
    dirname /dev/input

/dev/input/*              0660   root       input
/dev/v4l-touch*           0660   root       input
/sys/devices/virtual/input/input*   enable      0660  root   input
/sys/devices/virtual/input/input*   poll_delay  0660  root   input
```

SELinux/file context assumptions:

```text
/dev/input(/.*)?          input_device
/dev/v4l-touch[0-9]*      input_device
```

Recovery/minui properties:

```text
ro.minui.pixel_format=RGBX_8888
ro.minui.default_rotation
ro.minui.overscan_percent
ro.minui.pixel_format
```

Touch-related vendor context paths present in the recovery ramdisk:

```text
/sys/devices/platform/synaptics_tcm.0/sysfs(/.*)?
/sys/bus/platform/devices/synaptics_tcm.0/uevent
/sys/devices/virtual/tsp_fw/touchscreen(/.*)?
/sys/bus/platform/devices/zte_touch/uevent
/data/vendor/touchscreen(/.*)?
/dev/v4l-touch[0-9]*
/dev/hbtp_input
```

`system/bin/recovery` string inspection:

- no obvious `syna`, `synaptics`, `zte_tpd`, or touchscreen firmware filenames were found
- menu/UI strings are compiled into the binary
- input handling is likely generic recovery/minui evdev behavior plus kernel input driver availability

Important interpretation:

- Recovery appears prepared to create and label `/dev/input` nodes.
- Recovery does not appear to carry a visible Synaptics-specific userspace input helper.
- If touch fails in custom recovery later, the likely split is kernel/DTB/driver availability versus minui opening/mapping the event device.

---

## Hypotheses

Most likely:

1. Recovery/minui expects a normal evdev touchscreen and should work if `/dev/input/event9` or equivalent exists in recovery.
2. The kernel/input driver path is `synaptics_tcm_touch`, location `synaptics_tcm/touch_input`.
3. The Android working axis range is raw panel scale:
   - X max: `12159`
   - Y max: `26879`
4. Recovery touch problems are more likely to come from event-device availability/timing/mapping than from a missing recovery UI resource.

Possible issue classes to separate:

- kernel driver does not initialize touch in recovery path
- DTB/input node mismatch
- recovery/minui generic input handling mismatch
- missing firmware/config from recovery-visible paths
- panel/touch orientation or coordinate mapping mismatch
- input device exists but permissions/uevent timing prevent recovery from opening it
- Android userspace transforms touch in a way recovery does not reproduce

Lower-confidence but worth tracking:

- `/dev/v4l-touch*` or `/dev/hbtp_input` may be relevant to auxiliary touch paths, but Android's primary working device is `/dev/input/event9`
- `/data/vendor/touchscreen` may contain calibration/config data not available in recovery depending on mount state

---

## Next Safe Test Recommendation

Do not build or flash a new image yet.

Next safe step is a second Android-side evidence capture with active touch movement, so the event stream can be tied directly to `event9`.

Suggested command, run while touching/dragging on the Android screen for a few seconds:

```powershell
adb shell getevent -lt /dev/input/event9 > C:\RM11-test\touch-input-evidence-001\getevent-event9-touch-sample.txt
```

Stop the command with `Ctrl+C` after several touches.

Then collect:

```powershell
adb shell dumpsys input > C:\RM11-test\touch-input-evidence-001\dumpsys-input-after-touch.txt
adb shell logcat -b all -d > C:\RM11-test\touch-input-evidence-001\logcat-all-after-touch.txt
```

Only after confirming event9 emits sane ABS_MT events should the next recovery-only plan be chosen.

Potential later recovery-only diagnostic options, each requiring separate review:

- add a non-invasive visual instruction page/resource for manual touch mapping
- add a minimal recovery/minui property change only if rotation/mapping evidence supports it
- add a recovery ramdisk config file only if stock recovery expects one and it is missing
- avoid init services/actions until a specific missing runtime event or path is identified

Hard restrictions:

- no `fastboot boot` recovery path
- no force-recovery cmdline images
- no `boot`, `vendor_boot`, `init_boot`, or `vbmeta` changes
- no wipe/data changes
- no risky init services/actions
- recovery partition only remains the safe lane
