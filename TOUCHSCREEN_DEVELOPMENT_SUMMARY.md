# Touchscreen Driver Porting and Kernel Safety Patches - Development Summary

This document details the modifications made to the AArch64 Linux Kernel (6.12.23 / Android 16 GKI) for the **RedMagic 11 Pro (NX809J)** to support the reverse-engineered Synaptics touchscreen driver (`zte_tpd`) and prevent early-boot kernel crashes.

---

## 1. Low-Level Pointer Validation in Assembly (strcmp, strncmp, strlen)

### The Problem
* The stock ROM binaries (such as proprietary display and network modules) were compiled with relocations that resulted in 32-bit truncation inside the GKI kernel (e.g., `0xffffffc0ffffxxxx` was truncated to `0x00000000ffffxxxx`). 
* When these truncated pointers were passed to string helper functions (`strcmp`, `strlen`, `strncmp`), the CPU triggered an immediate Page Translation Fault / Data Abort during dereferencing, causing the phone to bootloop into Qualcomm CrashDump mode.
* To solve this, guards were previously added in the assembly functions (`strcmp.S`, `strncmp.S`, `strlen.S`) using `lsr x2, x0, #52` and `cmp x2, #0xfff` to ensure pointers resided in the upper kernel space.
* **The Bug**: Android 16 kernel uses Tag-based KASAN / MTE (Memory Tagging Extension). This means valid kernel pointers can have arbitrary tags in their top 8 bits (bits 56-63). The previous `lsr #52` check flagged these valid tagged pointers as invalid because the top bits were not `0xfff`, causing false-positive comparisons and breaking boot.

### The Solution (Patches Applied)
We modified the assembly files to mask out the top 8-bit memory tags and validate only bits 48-55, ensuring that tagged kernel pointers are accepted while truncated and invalid userspace pointers are still caught.

#### Modified Files:
* `kernel_platform/common/arch/arm64/lib/strcmp.S`
* `kernel_platform/common/arch/arm64/lib/strlen.S`
* `kernel_platform/common/arch/arm64/lib/strncmp.S`

#### Diffs:

**`strcmp.S`:**
```diff
 SYM_FUNC_START(__pi_strcmp)
 	/* Safety check for truncated/invalid pointers */
-	lsr	x2, x0, #52
-	cmp	x2, #0xfff
+	lsr	x2, x0, #48
+	and	x2, x2, #0xff
+	cmp	x2, #0xff
 	b.eq	.L_check_src2
 	cbz	x0, .L_check_src2	/* If NULL, let it proceed to check src2 */
 	/* src1 is invalid! */
 	cmp	x0, x1
 	cset	x0, ne
 	ret
 
 .L_check_src2:
-	lsr	x2, x1, #52
-	cmp	x2, #0xfff
+	lsr	x2, x1, #48
+	and	x2, x2, #0xff
+	cmp	x2, #0xff
 	b.eq	.L_strcmp_go
 	cbz	x1, .L_strcmp_go	/* If NULL, let standard comparison run or handle it */
```

**`strlen.S`:**
```diff
 SYM_FUNC_START(__pi_strlen)
 	/* Safety check for truncated/invalid pointers */
-	lsr	x1, x0, #52
-	cmp	x1, #0xfff
+	lsr	x1, x0, #48
+	and	x1, x1, #0xff
+	cmp	x1, #0xff
 	b.eq	.L_strlen_go
 	cbz	x0, .L_strlen_go
 	/* srcin is invalid! Return 0 */
 	mov	x0, #0
 	ret
```

**`strncmp.S`:**
```diff
 SYM_FUNC_START(__pi_strncmp)
 	cbz	limit, L(ret0)
 	/* Safety check for truncated/invalid pointers */
-	lsr	x3, x0, #52
-	cmp	x3, #0xfff
+	lsr	x3, x0, #48
+	and	x3, x3, #0xff
+	cmp	x3, #0xff
 	b.eq	.L_check_src2
 	cbz	x0, .L_check_src2
 	/* src1 is invalid! */
 	cmp	x0, x1
 	cset	x0, ne
 	ret
 
 .L_check_src2:
-	lsr	x3, x1, #52
-	cmp	x3, #0xfff
+	lsr	x3, x1, #48
+	and	x3, x3, #0xff
+	cmp	x3, #0xff
 	b.eq	.L_strncmp_go
 	cbz	x1, .L_strncmp_go
```

---

## 2. Refactoring input_dev Structure Member Offsets

### The Problem
* The reverse-engineered touchscreen driver contained compiled writes to hardcoded offsets of the allocated `struct input_dev` object (e.g. `*(device + 656) = parent`, `*(device + 32) |= 2` for capabilities).
* These offsets changed drastically in Kernel 6.12 due to structure additions and Kernel ABI modifications. Writing to these hardcoded offsets corrupted unrelated input device fields, generating a kernel panic during device registration.

### The Solution (Patches Applied)
We refactored `syna_dev_set_up_input_device.c` to use safe, type-safe API helper functions and proper struct fields instead of pointer arithmetic.

#### Modified Files:
* `vendor/zte/zte_tpd/syna_dev_set_up_input_device.c`
* `kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/syna_dev_set_up_input_device.c`

#### Key Code Replacements:
Instead of writing to raw offsets, we cast the allocated structure pointer to `struct input_dev *` and set it cleanly:
```c
struct input_dev *input_dev = (struct input_dev *)device;
struct platform_device *pdev = *(struct platform_device **)(a1 + 8);
struct device *parent = pdev ? &pdev->dev : NULL;

input_dev->name = "synaptics_tcm_touch";
input_dev->phys = "synaptics_tcm/touch_input";
input_dev->id.bustype = 1;
input_dev->id.vendor = 1;

input_dev->dev.parent = parent;
input_set_drvdata(input_dev, (void *)a1);

// Setting Capabilities
__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
__set_bit(EV_SYN, input_dev->evbit);
__set_bit(EV_KEY, input_dev->evbit);
__set_bit(EV_ABS, input_dev->evbit);
__set_bit(KEY_WAKEUP, input_dev->keybit);
__set_bit(BTN_TOUCH, input_dev->keybit);
__set_bit(BTN_TOOL_FINGER, input_dev->keybit);

input_set_capability(device, 1, 143);
input_set_abs_params(input_dev, 53, 0, v7[4], 0, 0);
input_set_abs_params(input_dev, 54, 0, v7[5], 0, 0);
input_mt_init_slots(input_dev, v7[6], 2);
input_set_abs_params(input_dev, 48, 0, 255, 0, 0);
input_set_abs_params(input_dev, 49, 0, 255, 0, 0);
```

---

## 3. Kernel Formatter Safety Checks (vsprintf)

#### Modified File:
* `kernel_platform/common/lib/vsprintf.c`

#### Logic Added:
We enforce a check inside `check_pointer_msg` to intercept and safely mark any pointer below `0xfff0000000000000UL` as `(efault)` when formatting format strings via `%s`:
```c
#ifdef CONFIG_64BIT
	if ((unsigned long)ptr < 0xfff0000000000000UL)
		return "(efault)";
#endif
```

---

## 4. Build and Signature Repack Workflow

1. **Compilation**: Run `./super_build.sh` at the workspace root to compile the GKI kernel image (`Image`) and modules.
2. **Repackaging and Signing**: Run `./repack_perfect_sign.sh` to package `Image` + `dtb.img` and apply an AVB hash footer:
   * Algorithm: `NONE`
   * Partition size: `67108864` (64MB)
3. **Execution**: Boot temporarily via Fastboot to test the image:
   ```bash
   fastboot boot dev_reverse_perfect.img
   ```
4. **Log Collection**: If the device panics or succeeds, boot back to stock kernel to query the pstore ramoops:
   ```bash
   ./scratch/get_ramoops.sh
   ```
   Logs will be dumped to `scratch/ramoops.log`.
