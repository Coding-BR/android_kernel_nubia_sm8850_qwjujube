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

Additional live-touch capture files:

```text
getevent-event9-live-touch.txt
event9-live-touch-summary.txt
```

Status of live-touch capture:

- A bounded 10-second `adb shell getevent -lt /dev/input/event9` capture was attempted.
- Both live-touch output files were created, but no events were captured in that window.
- Treat this as an incomplete capture, not as evidence that `event9` is inactive.
- A manual rerun is still needed while physically touching/swiping the Android screen.

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

## Android Touchscreen Identity

Confirmed Android touchscreen identity:

```text
event node: /dev/input/event9
name:       synaptics_tcm_touch
location:   synaptics_tcm/touch_input
sysfs path: /sys/devices/platform/soc/1ac0000.qcom,qupv3_4_geni_se/1a80000.spi/spi_master/spi19/spi19.0
```

Confirmed capabilities:

```text
ABS_MT_POSITION_X max 12159
ABS_MT_POSITION_Y max 26879
ABS_MT_SLOT       max 9
INPUT_PROP_DIRECT
```

Android `dumpsys input` identity:

```text
Device 1: synaptics_tcm_touch
Path: /dev/input/event9
Sources: KEYBOARD | TOUCHSCREEN | TOUCH_MT
```

Important note:

`dumpsys input` also showed `Touch Input Mapper (mode - DISABLED)` in the captured state. Document this as a state clue only. It does not override the stronger evidence that Android identifies the node as a direct multitouch touchscreen.

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

Recovery-side comparison:

| Question | Current Evidence | Interpretation |
| --- | --- | --- |
| Does recovery/minui scan all `/dev/input/event*` nodes? | `system/etc/ueventd.rc` creates generic `/dev/input/*` nodes with `0660 root input`; `system/bin/recovery` has no obvious hardcoded Synaptics event node name. | Stock recovery is most likely relying on generic evdev/minui scanning rather than a fixed `/dev/input/event9` path. |
| Does recovery create permissions for Android's event node class? | `/dev/input/* 0660 root input` is present. | If the kernel publishes the node in recovery, ueventd should create a readable input node for recovery/minui, subject to process group/SELinux context. |
| Does recovery reference `synaptics_tcm_touch` directly? | No direct ramdisk or recovery-binary reference was found. Recovery contexts reference older/platform paths such as `synaptics_tcm.0`, `zte_touch`, `/dev/v4l-touch*`, and `/dev/hbtp_input`. | Recovery probably does not require the exact Android input name, but the mismatch is worth tracking if a future recovery image loses touch. |
| Could recovery fail even if the node exists? | Yes. Possible blockers include uevent timing, process permissions/group membership, SELinux labeling, minui filtering, or event classification. | Node existence alone will not prove recovery UI can consume touch. |
| Is this more likely kernel/DTB, recovery userspace, or missing ramdisk config? | Stock recovery touch works, and Android sees `synaptics_tcm_touch` as generic evdev. | For stock-kernel recovery work, userspace/minui timing/filtering is currently more likely than a missing touchscreen driver. Kernel/DTB stays a risk only when moving away from stock recovery/kernel behavior. |

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

Ranked hypotheses:

1. Recovery/minui expects a normal evdev touchscreen and should work if `/dev/input/event9` or an equivalent `synaptics_tcm_touch` node exists in recovery with usable permissions.
2. If touch fails in a modified recovery later, the first suspects should be event-node availability, uevent timing, minui filtering/classification, or permissions rather than the already-validated PNG/UI resource path.
3. The Android working input path is `synaptics_tcm_touch` at `synaptics_tcm/touch_input`, backed by the SPI `spi19.0` sysfs path.
4. The Android working axis range is raw panel scale:
   - X max: `12159`
   - Y max: `26879`
5. Kernel/DTB driver initialization remains a separate risk only when changing the kernel/DTB path. It is not the next recovery-only change target because stock recovery touch already works.

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

Manual live-touch command, run while touching/dragging on the Android screen for 5-10 seconds:

```powershell
adb shell getevent -lt /dev/input/event9 > C:\RM11-test\touch-input-evidence-001\getevent-event9-live-touch.txt
```

Stop the command with `Ctrl+C` after several touches.

Then create the filtered summary:

```powershell
Select-String -Path C:\RM11-test\touch-input-evidence-001\getevent-event9-live-touch.txt -Pattern "ABS_MT_POSITION_X|ABS_MT_POSITION_Y|ABS_MT_SLOT|SYN_REPORT|BTN_TOUCH|ABS_X|ABS_Y" | Select-Object -First 120 > C:\RM11-test\touch-input-evidence-001\event9-live-touch-summary.txt
```

Expected proof in the summary:

```text
EV_ABS ABS_MT_SLOT
EV_ABS ABS_MT_POSITION_X
EV_ABS ABS_MT_POSITION_Y
EV_SYN SYN_REPORT
```

Only after confirming `event9` emits sane live `ABS_MT` events should the next recovery-only plan be chosen.

Potential later recovery-only diagnostic options, each requiring separate review:

- add a non-invasive visual instruction page/resource for manual touch mapping
- add a recovery-side touch test UI only if it can remain inside the recovery partition and avoid wipe/data paths
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
