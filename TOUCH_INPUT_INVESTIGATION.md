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
touch-input-summary.txt
```

Additional live-touch capture files:

```text
getevent-event9-live-touch.txt
event9-live-touch-summary.txt
```

Status of live-touch capture:

- CONFIRMED/PASS: live touch was captured successfully from `/dev/input/event9`.
- The saved stream proves `event9` emits real touchscreen movement events while the Android UI is touched.

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

Fresh Android-side package refresh:

```text
folder: C:\RM11-test\touch-input-evidence-001
time:   2026-05-24 12:35 America/Chicago
```

The refreshed package includes `touch-input-summary.txt`, a local summary of the captured Android input state.

---

## Android Touchscreen Identity

Current state: Android-side touchscreen evidence is confirmed.

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
Classes: KEYBOARD | TOUCH | TOUCH_MT
Sources: KEYBOARD | TOUCHSCREEN | TOUCH_MT
Touch mapping: TOUCH_SCREEN / MULTI_TOUCH
Android display size: 1216x2688
```

Important note:

`dumpsys input` also showed `Touch Input Mapper (mode - DISABLED)` in the captured state. Document this as a state clue only. It does not override the stronger evidence that Android identifies the node as a direct multitouch touchscreen.

---

## Live Android Event9 Proof

Command used:

```powershell
adb shell getevent -lt /dev/input/event9 > C:\RM11-test\touch-input-evidence-001\getevent-event9-live-touch.txt
```

Filtered summary command:

```powershell
Select-String -Path C:\RM11-test\touch-input-evidence-001\getevent-event9-live-touch.txt -Pattern "ABS_MT_POSITION_X|ABS_MT_POSITION_Y|ABS_MT_SLOT|SYN_REPORT|BTN_TOUCH|ABS_X|ABS_Y" | Select-Object -First 120 > C:\RM11-test\touch-input-evidence-001\event9-live-touch-summary.txt
```

Files:

```text
C:\RM11-test\touch-input-evidence-001\getevent-event9-live-touch.txt
C:\RM11-test\touch-input-evidence-001\event9-live-touch-summary.txt
```

Observed live event types/codes:

```text
EV_KEY BTN_TOUCH DOWN
EV_ABS ABS_MT_TRACKING_ID
EV_ABS ABS_MT_SLOT
EV_ABS ABS_MT_POSITION_X
EV_ABS ABS_MT_POSITION_Y
EV_ABS ABS_MT_TOUCH_MAJOR
EV_SYN SYN_REPORT
```

Sample observed coordinate range from the filtered capture:

```text
ABS_MT_POSITION_X observed min 5257 max 10926
ABS_MT_POSITION_Y observed min 11427 max 14482
SYN_REPORT observed repeatedly
```

Correction note:

- The live event9 touch capture is confirmed, not incomplete.
- Manual summary confirmed `BTN_TOUCH DOWN`, `ABS_MT_POSITION_X`, `ABS_MT_POSITION_Y`, `ABS_MT_SLOT`, and repeated `SYN_REPORT`.
- `ABS_MT_SLOT` is also confirmed as a device capability from `getevent -lp` with max slot `9`.
- Android input stack maps the device as `TOUCH_SCREEN` with `MULTI_TOUCH`.
- Android display viewport reports `1216x2688`.

Conclusion:

`/dev/input/event9` is confirmed as the real live Android touchscreen event stream for `synaptics_tcm_touch`.

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

Fresh capture summary file:

```text
C:\RM11-test\touch-input-evidence-001\touch-input-summary.txt
```

Summary conclusion:

```text
Android working touch is synaptics_tcm_touch on /dev/input/event9 with direct multitouch ABS_MT capabilities.
Recovery comparison should focus on whether recovery runtime publishes and minui opens the equivalent event node.
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
init.recovery.qcom.rc
system/etc/ueventd.rc
system/etc/recovery.fstab
plat_file_contexts
plat_property_contexts
prop.default
default.prop
vendor_file_contexts
system/bin/recovery
```

Static scan inputs:

```text
stock recovery ramdisk: /home/richtofen/android/output/recovery/inspect/unpack-stock-recovery/ramdisk.cpio
marker-005 ramdisk:     /home/richtofen/android/output/recovery/rm11-recovery-marker-005-verify/ramdisk.cpio
```

Marker-005 static comparison matched the stock recovery input-related references. Its only intended functional difference remains the already-tested UI PNG resource change.

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

Focused static search results:

| Search Term | Stock Recovery Static Result |
| --- | --- |
| `synaptics_tcm_touch` | No direct hit in ramdisk text files or `system/bin/recovery` strings. |
| `synaptics_tcm` | Present as `synaptics_tcm.0` sysfs/vendor file context paths only. |
| `touch_input` | No direct hit found. |
| `event9` | No direct hit found. |
| `/dev/input` | Present in `system/etc/ueventd.rc` and `plat_file_contexts`. |
| `/dev/input/event` | No event-number-specific hit found. |
| `zte_touch` | Present as `/sys/bus/platform/devices/zte_touch/uevent` vendor file context only. |
| `/dev/v41-touch` | No hit found. |
| `/dev/v4l-touch` | Present in ueventd and file contexts. |
| `/dev/hbtp_input` | Present as a vendor file context. |
| `ro.minui` | Present for display-related recovery properties. |
| `minui` | No useful touch-specific static selector found. |
| `evdev` | No direct hit found. |
| `ueventd` | Present through init startup and ueventd rc imports. |
| `input` | Present broadly in input node permissions, file contexts, and generic Android service/property context strings. |

Recovery-side comparison:

| Question | Current Evidence | Interpretation |
| --- | --- | --- |
| Does recovery/minui scan all `/dev/input/event*` nodes? | `system/etc/ueventd.rc` creates generic `/dev/input/*` nodes with `0660 root input`; `system/bin/recovery` has no obvious hardcoded Synaptics event node name. | Stock recovery is most likely relying on generic evdev/minui scanning rather than a fixed `/dev/input/event9` path. |
| Does recovery create permissions for Android's event node class? | `/dev/input/* 0660 root input` is present. | If the kernel publishes the node in recovery, ueventd should create a readable input node for recovery/minui, subject to process group/SELinux context. |
| Does recovery reference `synaptics_tcm_touch` directly? | No direct ramdisk or recovery-binary reference was found. Recovery contexts reference older/platform paths such as `synaptics_tcm.0`, `zte_touch`, `/dev/v4l-touch*`, and `/dev/hbtp_input`. | Recovery probably does not require the exact Android input name, but the mismatch is worth tracking if a future recovery image loses touch. |
| Could recovery fail even if the node exists? | Yes. Possible blockers include uevent timing, process permissions/group membership, SELinux labeling, minui filtering, or event classification. | Node existence alone will not prove recovery UI can consume touch. |
| Is this more likely kernel/DTB, recovery userspace, or missing ramdisk config? | Stock recovery touch works, and Android sees `synaptics_tcm_touch` as generic evdev. | For stock-kernel recovery work, userspace/minui timing/filtering is currently more likely than a missing touchscreen driver. Kernel/DTB stays a risk only when moving away from stock recovery/kernel behavior. |

Android working touch evidence vs recovery static references:

| Android Working Evidence | Recovery Static Reference | Current Read |
| --- | --- | --- |
| `/dev/input/event9` is live touch input. | `/dev/input/* 0660 root input` in `system/etc/ueventd.rc`; `/dev/input(/.*)?` in `plat_file_contexts`. | Recovery has generic input-node creation and labeling, not an `event9`-specific rule. |
| `synaptics_tcm_touch` is the Android input name. | No direct `synaptics_tcm_touch` string found in the stock recovery ramdisk or `system/bin/recovery` strings. | Recovery is probably not matching this exact input name in userspace. |
| Android location is `synaptics_tcm/touch_input`. | No direct `touch_input` string found in stock recovery userspace files. | The location string is an Android input identity clue, not a recovery userspace dependency found so far. |
| Sysfs path uses SPI `spi19.0`. | Recovery vendor contexts reference `synaptics_tcm.0` and `zte_touch` platform paths. | Static contexts may be older or generic vendor labels; they do not prove the live recovery node path. |
| `ABS_MT_POSITION_X/Y`, `BTN_TOUCH`, `SYN_REPORT` observed live. | `system/bin/recovery` has no clear hardcoded touch-driver strings; input handling appears generic/minui. | If the node exists during recovery, minui should have a plausible event stream to consume. |
| Android device has `ABS_MT_SLOT max 9` and `INPUT_PROP_DIRECT`. | Recovery has `ro.minui.*` display properties, but no discovered touch-axis config file. | Any future issue may be event classification, orientation, or scaling rather than missing static ramdisk config. |
| Android primary path is generic evdev. | Recovery also contains alternate references to `/dev/v4l-touch*` and `/dev/hbtp_input`. | These alternate paths are worth tracking, but Android's confirmed primary touchscreen stream is `/dev/input/event9`. |

Direct answers from static recovery comparison:

1. Stock recovery contains no direct reference to `synaptics_tcm_touch`.
2. Stock recovery contains generic `/dev/input` setup, but no direct `event9` or `/dev/input/event9` reference.
3. `system/etc/ueventd.rc` defines `/dev/input/* 0660 root input` and `subsystem input` with `dirname /dev/input`.
4. Alternate vendor touch references exist for `synaptics_tcm.0`, `zte_touch`, `/dev/v4l-touch*`, and `/dev/hbtp_input`; no `/dev/v41-touch` hit was found.
5. The next high-value question is runtime, not static: does recovery userspace/minui actually see and open `/dev/input/event9` or its recovery-time equivalent for `synaptics_tcm_touch`?

`system/bin/recovery` string inspection:

- no obvious `syna`, `synaptics`, `zte_tpd`, or touchscreen firmware filenames were found
- no direct `synaptics_tcm_touch`, `touch_input`, `event9`, or `/dev/input/event9` string was found
- no focused `minui` or `evdev` string hit explained event selection behavior
- menu/UI strings are compiled into the binary
- input handling is likely generic recovery/minui evdev behavior plus kernel input driver availability

Important interpretation:

- Recovery appears prepared to create and label `/dev/input` nodes.
- Recovery does not appear to carry a visible Synaptics-specific userspace input helper.
- If touch fails in custom recovery later, the likely split is kernel/DTB/driver availability versus minui opening/mapping the event device.

---

## Hypotheses

Ranked hypotheses:

1. Recovery/minui ignores or fails to select `event9` or the recovery-time equivalent node. This remains possible because no static selector is visible in `system/bin/recovery`, and runtime node selection is not observable yet.
2. Recovery ueventd/permissions/timing issue. Generic `/dev/input/* 0660 root input` exists, but runtime timing, process group membership, SELinux domain behavior, or late node creation could still prevent minui from consuming the touch stream.
3. Recovery expects alternate vendor node names. Static references exist for `synaptics_tcm.0`, `zte_touch`, `/dev/v4l-touch*`, and `/dev/hbtp_input`, but Android's confirmed primary stream is `/dev/input/event9`.
4. Recovery boot path initializes touch differently. This is the biggest runtime unknown: stock recovery touch works, but any custom recovery path must still publish a usable `synaptics_tcm_touch`/evdev node early enough for recovery UI input.
5. Stock recovery/minui likely uses generic evdev scanning, not a hardcoded `synaptics_tcm_touch` or `event9` path.
6. The Android working input path is `synaptics_tcm_touch` at `synaptics_tcm/touch_input`, backed by the SPI `spi19.0` sysfs path.
7. The Android working axis range is raw panel scale:
   - X max: `12159`
   - Y max: `26879`
8. Kernel/DTB driver initialization remains a separate risk only when changing the kernel/DTB path. It is not the next recovery-only change target because stock recovery touch already works.

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

Android-side live event proof is now complete enough to proceed with design work, but still not enough to justify a risky image.

Known limitations:

- `dmesg` and `/proc/bus/input/devices` are permission-limited from unprivileged Android.
- Recovery ADB remains unauthorized, so runtime recovery shell access is not available.

Current narrowed problem:

Stock Android proves the Synaptics hardware, driver, and input node work. The remaining unknown is recovery runtime behavior:

1. whether recovery creates the same event node
2. whether minui scans and opens it
3. whether the node appears too late for recovery input initialization
4. whether recovery permissions/classification differ from Android
5. whether recovery logs expose input/minui/evdev discovery

Next safe step:

Path A only:

1. Boot stock or current marker recovery through the normal recovery path.
2. Manually open `Advanced options -> View recovery logs`.
3. Photograph any pages that mention:
   - `input`
   - `touch`
   - `evdev`
   - `minui`
   - `event`
   - `synaptics`
   - `tcm`
   - `spi19`
   - `/dev/input`
   - `ueventd`
   - `hbtp`
   - `v41`
   - `zte_touch`
4. Record manual behavior:
   - touch works or not
   - volume navigation works or not
   - power select works or not
   - `View recovery logs` accessible or not

Recommended next test design target:

- Do not propose or build a Path B logger until Path A is recorded.
- A passive recovery logger may be considered only if `View recovery logs` gives no usable input/runtime evidence.

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

---

## Recovery Runtime Observation Plan

Main runtime question:

```text
When booted into recovery, does the Synaptics touch input node exist at all?
If yes, what event number/name does it get, and does recovery/minui open it?
```

This is now the highest-value question. Android has already proven that the live touchscreen stream is `/dev/input/event9` with name `synaptics_tcm_touch`, but static recovery inspection cannot prove the recovery runtime node list.

### Path A: Stock Recovery UI/Log Observation

Goal: gather any stock recovery logs without modifying recovery.

Test outline:

1. Boot the currently validated stock or marker recovery through the normal recovery path.
2. Use the recovery UI only.
3. Open `Advanced options`.
4. Open `View recovery logs`.
5. Search visually for input/minui/evdev/touch/open errors.
6. Photograph the log screen if export is unavailable.

Search terms to look for in the UI/log view:

```text
minui
evdev
input
touch
synaptics
tcm
event
/dev/input
open
permission
```

Expected value:

- If stock recovery logs show minui input enumeration, this may answer whether recovery sees the touch node without any image change.
- If logs cannot be exported because recovery ADB is unauthorized, a clear photo is still useful.

Limitations:

- The built-in log viewer may not include early input enumeration.
- The log viewer may not expose `/tmp/recovery.log` or kernel logs in enough detail.
- This path may prove only that touch works, not which event node minui opened.

Risk:

- Very low. No flashing, no image modification, no wipe/data operation.
- Hard warning: do not choose wipe/data options.

### Path B: Passive Recovery Ramdisk Input Logger

Goal: after review only, add a minimal passive diagnostic that records recovery-runtime input discovery without changing UI behavior or touch behavior.

Candidate diagnostic contents:

```sh
date
ls -l /dev/input
getevent -lp
cat /proc/bus/input/devices
dmesg | grep -Ei "synaptics|touch|input|tcm|spi19|v4l|v41|hbtp|zte"
logcat -b all -d | grep -Ei "minui|evdev|input|touch|synaptics|tcm"
```

Static availability in the stock recovery ramdisk:

- `sh` exists.
- `toybox` exists with common utilities such as `cat`, `ls`, `grep`, `find`, `dmesg`, `sleep`, `tee`, and `date`.
- `getevent` exists as a toolbox applet link.
- `logcat` was not confirmed as a present binary in the extracted stock recovery ramdisk, so `logcat` must be treated as optional unless verified at build time.

Potential writable targets:

| Target | Static Evidence | Usefulness | Risk/Limit |
| --- | --- | --- | --- |
| `/tmp/` | `init.rc` mounts tmpfs at `/tmp`, sets owner `root:shell`, mode `0775`. | Safest runtime scratch target. | Volatile; lost after reboot unless recovery UI/log collection copies it. |
| `/cache/recovery/` | File contexts exist for `/cache/recovery`; `init.rc` creates `/cache`, but no stock recovery fstab cache mount was identified. | Traditional recovery log location if mounted/used by recovery. | Runtime writability is unproven; writing here must not be assumed safe until tested carefully. |
| `/dev/kmsg` | File context exists; health recovery service has `file /dev/kmsg w`. | A short diagnostic line may enter kernel log. | Android-side retrieval is not guaranteed without root; pstore should not be treated as available unless a crash/panic preserves it. |
| `/sys/fs/pstore` | No safe write path; Android-side pstore was permission-limited earlier. | Not suitable as a planned output target. | Do not write here. |

Existing init/service pattern:

- `system/etc/init/hw/init.rc` starts `ueventd` in `early-init`, mounts `/tmp` during `on init`, and starts the normal `recovery` service as root with `seclabel u:r:recovery:s0`.
- `init.recovery.qcom.rc` uses simple `on init`, property, and `on fs` actions.
- Existing recovery services are long-running platform services (`recovery`, `adbd`, `fastbootd`, health HAL), not small custom oneshot shell diagnostics.

Conservative conclusion:

- There is no existing tiny diagnostic shell service pattern to copy directly.
- If Path B is approved later, the safest form is likely a disabled or oneshot root shell service/action that writes only to `/tmp/rm11-input-diag.txt` and optionally one short `/dev/kmsg` marker.
- Do not write to `/cache/recovery` until the recovery log destination and mount behavior are better understood.
- Do not add init actions/services without a separate reviewed plan.

Questions answered before building:

1. Existing service pattern: recovery has service definitions and init actions, but no tiny passive shell diagnostic pattern. A oneshot diagnostic would be new and must be reviewed.
2. `/cache/recovery` writability: not proven statically. Contexts exist, but the extracted recovery fstab does not show a dedicated cache mount. Treat as unproven.
3. Passive log file safety: writing to `/tmp` is safest and should not trigger wipe/update behavior. Writing to `/cache/recovery` could interact with recovery log handling and needs more caution.
4. Commands to log: `ls -l /dev/input`, `getevent -lp`, `cat /proc/bus/input/devices`, and filtered `dmesg` are plausible from available recovery tools. `logcat` is optional/unproven.
5. `/dev/kmsg`: possible for a short marker line, but Android retrieval after reboot is not guaranteed without root/pstore access.

Least invasive recommendation:

1. Run Path A first using stock/marker recovery UI logs and photos.
2. If Path A does not reveal runtime input enumeration, design Path B as a reviewed recovery-only diagnostic image.
3. Path B should initially write only to `/tmp/rm11-input-diag.txt` and a short `/dev/kmsg` marker, then rely on the recovery UI/log viewer or a photograph if ADB remains unauthorized.
4. Do not build Path B until the exact init hook, output target, and rollback steps are reviewed.

Rollback for any future Path B image:

```powershell
adb reboot bootloader
fastboot flash recovery_a C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot flash recovery_b C:\RM11-test\recovery\rm11-repacked-stock-recovery.img
fastboot --set-active=a
fastboot reboot
```

Hard restrictions for this phase:

- no image build yet
- no flashing
- no recovery modification
- no kernel/DTB work
- no boot/vendor_boot/init_boot/vbmeta work
- no wipe/data changes
- no fastboot boot recovery tests
