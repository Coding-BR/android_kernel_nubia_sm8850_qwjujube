#ifndef _DEFS_H
#define _DEFS_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/input/mt.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/pm_wakeup.h>



// IDA / Ghidra Decompilation Types
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8 char


#define __fastcall

typedef unsigned long long _QWORD;
typedef unsigned int _DWORD;
typedef unsigned short _WORD;
typedef unsigned char _BYTE;

#define _BOOL8 unsigned char
#define _BOOL4 int

#define BYTEn(x, n)   (*((unsigned char*)&(x)+(n)))
#define BYTE0(x)   BYTEn(x,  0)
#define BYTE1(x)   BYTEn(x,  1)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)
#define BYTE4(x)   BYTEn(x,  4)
#define BYTE5(x)   BYTEn(x,  5)
#define BYTE6(x)   BYTEn(x,  6)
#define BYTE7(x)   BYTEn(x,  7)





// Map Ghidra's __break calls to a safe kernel warning instead of panicking
#define __break(x) pr_warn("zte_tpd CFI/assert warning: 0x%x at %s:%d\n", (unsigned int)(x), __FILE__, __LINE__)

// Cast wrappers for standard kernel functions called with __int64 / long long arguments
#undef simple_read_from_buffer
#define simple_read_from_buffer(to, count, ppos, from, available) \
    simple_read_from_buffer((void __user *)(to), (size_t)(count), (loff_t *)(ppos), (const void *)(from), (size_t)(available))

#undef cancel_delayed_work_sync
#define cancel_delayed_work_sync(dwork) cancel_delayed_work_sync((struct delayed_work *)(dwork))

#undef cancel_work_sync
#define cancel_work_sync(work) cancel_work_sync((struct work_struct *)(work))

#undef destroy_workqueue
#define destroy_workqueue(wq) destroy_workqueue((struct workqueue_struct *)(wq))

#undef queue_delayed_work
#define queue_delayed_work(wq, dwork, delay) queue_delayed_work((struct workqueue_struct *)(wq), (struct delayed_work *)(dwork), delay)

#undef queue_work
#define queue_work(wq, work) queue_work((struct workqueue_struct *)(wq), (struct work_struct *)(work))

#undef flush_workqueue
#define flush_workqueue(wq) __flush_workqueue((struct workqueue_struct *)(wq))

// kstrto and check_object_size wrappers
#undef kstrtouint_from_user
#define kstrtouint_from_user(s, count, base, res) kstrtouint_from_user((const char __user *)(s), (size_t)(count), (unsigned int)(base), (unsigned int *)(res))

#undef kstrtouint
#define kstrtouint(s, base, res) kstrtouint((const char *)(s), (unsigned int)(base), (unsigned int *)(res))

#undef _check_object_size
#define _check_object_size(ptr, n, to_user) ((void)0)

// WORD and HIWORD/LOWORD extraction macros
#define WORDn(x, n)   (*((unsigned short*)&(x)+(n)))
#define WORD0(x)   WORDn(x,  0)
#define WORD1(x)   WORDn(x,  1)
#define WORD2(x)   WORDn(x,  2)
#define WORD3(x)   WORDn(x,  3)
#define WORD4(x)   WORDn(x,  4)
#define WORD5(x)   WORDn(x,  5)
#define WORD6(x)   WORDn(x,  6)
#define WORD7(x)   WORDn(x,  7)

#define HIWORD(x)  (*((unsigned short*)&(x)+1))
#define LOWORD(x)  (*((unsigned short*)&(x)+0))

#define HIBYTE(x)   (*((unsigned char*)&(x)+1))
#define LOBYTE(x)   (*((unsigned char*)&(x)+0))

#define LODWORD(x)  (*((unsigned int*)&(x)+0))
#define HIDWORD(x)  (*((unsigned int*)&(x)+1))


// kmalloc decompiler mapping
#undef _kmalloc_cache_noprof
#define _kmalloc_cache_noprof(cache, flags, size) \
    ((__int64)(((flags) == 3520) ? kzalloc(size, GFP_KERNEL) : kmalloc(size, GFP_KERNEL)))

// Decompiler helper macros
#define __PAIR64__(high, low) (((unsigned long long)(high) << 32) | (unsigned int)(low))
#define __rev16(x) __builtin_bswap16(x)
#define __dmb(x) __asm__ __volatile__("dmb ish" : : : "memory")

// Kernel symbol mappings
#define _init_swait_queue_head(q, name, key) __init_swait_queue_head(q, name, key)
#define _kmalloc_large_noprof(size, flags) kmalloc(size, flags)
#define _of_parse_phandle_with_args(np, list_name, cells_name, cell_count, index, out_args) \
    __of_parse_phandle_with_args(np, list_name, cells_name, cell_count, index, out_args)

// Mutex lock wrappers
#undef mutex_lock
#define mutex_lock(lock) mutex_lock((struct mutex *)(lock))

#undef mutex_unlock
#define mutex_unlock(lock) mutex_unlock((struct mutex *)(lock))

#undef mutex_init
#define mutex_init(lock) mutex_init((struct mutex *)(lock))

#define _mutex_init(lock, name, key) __mutex_init((struct mutex *)(lock), name, key)

#undef mutex_lock_interruptible
#define mutex_lock_interruptible(lock) mutex_lock_interruptible((struct mutex *)(lock))

// Input device event reporting wrappers
#undef input_event
#define input_event(dev, type, code, value) input_event((struct input_dev *)(dev), type, code, value)

#undef input_mt_report_slot_state
#define input_mt_report_slot_state(dev, tool_type, active) input_mt_report_slot_state((struct input_dev *)(dev), tool_type, active)

#undef input_report_key
#define input_report_key(dev, code, value) input_report_key((struct input_dev *)(dev), code, value)

#undef input_report_abs
#define input_report_abs(dev, code, value) input_report_abs((struct input_dev *)(dev), code, value)

#undef input_sync
#define input_sync(dev) input_sync((struct input_dev *)(dev))

#undef input_mt_sync_frame
#define input_mt_sync_frame(dev) input_mt_sync_frame((struct input_dev *)(dev))

// Custom copy_from_user wrapper declaration
extern size_t inline_copy_from_user(__int64 a1, unsigned __int64 a2, size_t n);

// kfree and vfree casting wrappers
#undef kfree
#define kfree(obj) kfree((const void *)(obj))

#undef vfree
#define vfree(obj) vfree((const void *)(obj))




// Forward declarations of platform drivers used in init/cleanup
extern struct platform_driver zte_touch_device_driver;
extern struct platform_driver syna_dev_driver;
extern struct spi_driver syna_spi_driver;
extern struct platform_device syna_spi_device;
extern void report_ufp_uevent(unsigned int val);

#define syna_pal_mutex_alloc___key dummy_lock_key
#define syna_pal_mutex_alloc___key_0 dummy_lock_key
#define syna_pal_mutex_alloc___key_1 dummy_lock_key
#define syna_pal_mutex_alloc___key_2 dummy_lock_key
#define syna_pal_mutex_alloc___key_3 dummy_lock_key
#define syna_pal_mutex_alloc___key_4 dummy_lock_key
#define syna_pal_mutex_alloc___key_5 dummy_lock_key
#define syna_pal_mutex_alloc___key_6 dummy_lock_key
#define syna_pal_mutex_alloc___key_7 dummy_lock_key
#define syna_pal_mutex_alloc___key_8 dummy_lock_key
#define syna_cdev_create___key dummy_lock_key
#define zte_touch_probe___key dummy_lock_key
#define zte_touch_probe___key_92 dummy_lock_key
#define zte_touch_probe___key_94 dummy_lock_key
#define init_completion___key dummy_lock_key
#define init_completion___key_1 dummy_lock_key
#define init_completion___key_2 dummy_lock_key
#define syna_dev_probe___key dummy_lock_key
#define syna_dev_probe___key_53 dummy_lock_key

extern __int64 syna_cdev_ioctl_raw_read(__int64 a1, unsigned __int64 a2, __int64 a3, unsigned int a4);
extern __int64 syna_cdev_ioctl_raw_write(__int64 a1, unsigned __int64 a2, __int64 a3, unsigned int a4);
extern __int64 syna_tcm_clear_data_duplicator(__int64 a1);
extern __int64 syna_request_managed_device();
extern __int64 syna_tcm_reset(__int64 a1, unsigned int a2, __int64 a3);
extern __int64 syna_dev_free_input_events(__int64 a1);
extern __int64 syna_tcm_set_report_dispatcher(__int64 a1, int a2, void *a3, __int64 a4);
extern __int64 syna_dev_process_touch_report(unsigned __int8 a1, const void *a2, __int64 a3, __int64 a4);
extern void syna_dev_reflash_startup_work(struct work_struct *work);
extern __int64 syna_tcm_set_dynamic_config(__int64 a1, int a2, int a3, int a4);
extern __int64 syna_tcm_do_fw_update(__int64 a1, __int64 a2, __int64 a3, unsigned int a4, char a5);
extern __int64 syna_tcm_detect_device(__int64 a1, char a2, __int64 a3);
extern __int64 syna_tcm_parse_fw_image(__int64 a1, _QWORD a2, _QWORD *a3);
extern __int64 syna_dev_set_up_app_fw(_QWORD *a1);
extern __int64 syna_tcm_switch_fw_mode(__int64 a1, int a2, unsigned int a3);
extern __int64 syna_dev_set_up_input_device(__int64 a1);
extern __int64 syna_tcm_get_boot_info(__int64 a1, void *a2, __int64 a3);
extern __int64 syna_dev_process_unexpected_reset(__int64 a1, __int64 a2, __int64 a3, __int64 a4);
extern __int64 syna_dev_isr(__int64 a1, __int64 *a2);

extern int syna_usb_detect_flag;
struct drm_panel;
extern struct drm_panel *active_panel;
extern struct file_operations zte_fops;
extern void syna_dev_helper_work(struct work_struct *work);
extern __int64 tpd_goodix_ts_resume(__int64 a1, __int64 a2, __int64 a3);
extern __int64 tpd_goodix_ts_suspend(__int64 a1, __int64 a2, __int64 a3);

// _flush_workqueue -> flush_workqueue
#define _flush_workqueue(wq) flush_workqueue((struct workqueue_struct *)(wq))

// Decompiler register stubs - nop on GKI
#define SP_EL0 0
#define DAIF 0
#define TTBR1_EL1 0
#define TTBR0_EL1 0
#define _ReadStatusReg(x) 0LL
#define _WriteStatusReg(x, y) ((void)0)
#define __isb(x) ((void)0)

// Standard kernel copy wrappers
#define _arch_copy_from_user(to, from, n) copy_from_user(to, (const void __user *)(from), n)
#define _arch_copy_to_user(to, from, n) copy_to_user((void __user *)(to), (const void *)(from), n)

// Ghidra C++ identifier -> C
#define nullptr NULL

// Signed HIDWORD extraction (used by process_reports)
#define SHIDWORD(x) ((int)(*((int*)&(x)+1)))

// Decompiler internal function stubs (must return value for expression context)
#define _fortify_panic(a, b, c) ({ WARN_ONCE(1, "fortify panic"); 0LL; })
#define _copy_overflow(a, b) ({ WARN_ONCE(1, "copy overflow"); 0LL; })

// Kernel function name mapping (single vs double underscore)
#define _init_waitqueue_head __init_waitqueue_head
#define _this_module __this_module
#define _spi_register_driver __spi_register_driver
#define _platform_driver_register __platform_driver_register
#define _list_del_entry_valid_or_report(entry) list_del((struct list_head *)(entry))
#define _list_add_valid_or_report(new, head) ((void)0)
#define _kmalloc_noprof(size, flags) kmalloc(size, GFP_KERNEL)
#define _wake_up(wq, mode, nr, key) wake_up((struct wait_queue_head *)(wq))

// Ghidra condition flags (zero flag etc.) - must be assignable
extern int _ZF;

// Void-return function wrappers (decompiler assigns void returns to __int64)
// Use GCC statement expressions so they can be used in expression context
static inline __int64 zte_devm_kfree_wrap(void *dev, void *ptr) {
    devm_kfree((struct device *)dev, (const void *)ptr);
    return 0;
}
#undef devm_kfree
#define devm_kfree(dev, ptr) zte_devm_kfree_wrap((void*)(dev), (void*)(ptr))



#undef gpio_free
static inline int wrap_gpio_free(unsigned int gpio) {
    gpio_free(gpio);
    return 0;
}
#define gpio_free(gpio, ...) wrap_gpio_free(gpio)

#undef mutex_unlock
#define mutex_unlock(lock) ({ mutex_unlock((struct mutex *)(lock)); 0LL; })

#undef unregister_chrdev_region
#define unregister_chrdev_region(start, count) ({ unregister_chrdev_region(start, count); 0LL; })

// Missing global variables
extern int syna_cdev_push_data_to_fifo_pre_remaining_frames;
extern __int64 one_key_report;
extern __int64 syna_get_charger_status_batt_psy;
extern char DEVICE_NODE_NAME[100];

// Missing function forward declarations
extern __int64 syna_tcm_set_data_duplicator(__int64 a1, int a2, void *a3, __int64 a4);
extern __int64 syna_cdev_process_reports(__int64 a1, _QWORD *a2, __int64 a3, __int64 *a4);
extern __int64 syna_cdev_open(void);
extern __int64 syna_tcm_get_event_data(__int64 a1, unsigned __int8 *a2, __int64 a3);
extern __int64 syna_tcm_enable_predict_reading(__int64 a1, char a2, __int64 a3);
extern __int64 syna_tcm_sleep(__int64 a1, char a2, __int64 a3);
extern __int64 syna_dev_enable_lowpwr_gesture(_QWORD *a1, char a2, unsigned int a3);
extern __int64 tpd_touch_release(__int64 result, unsigned __int16 a2, int a3);
struct ufp_tp_ops_struct {
    union {
        struct {
            struct platform_device *pdev;
            int field_8;
            int field_c;
            struct delayed_work single_tap_work;
        };
        struct {
            char pad_to_128[128];
            struct completion gesture_complete;
            char field_a0;
            char field_a1;
            char field_a2;
        };
        char pad[168];
    };
};
extern struct ufp_tp_ops_struct ufp_tp_ops;

extern int current_lcd_state;
extern char panel_enter_low_power;
extern void change_tp_state(int state);
extern void syna_ts_panel_notifier_callback(__int64 a1, __int64 a2, __int64 a3);
extern int syna_work_charger_detect_work_status;

struct point_info_struct {
    int x;                     // offset 0
    int y;                     // offset 4
    unsigned char touch_major; // offset 8
    unsigned char touch_minor; // offset 9
    char pad1[70];             // pad to 80
    unsigned char field_80;    // offset 80
    char pad2[2];              // pad to 83
    unsigned char field_83;    // offset 83
    unsigned char field_84;    // offset 84
    char pad3[11];             // pad to 96
    unsigned long last_stamp;  // offset 96
    unsigned long pad4;        // offset 104
    unsigned long timestamp;   // offset 112
    unsigned long duration;    // offset 120
    struct input_dev *input;   // offset 128
};
extern __int64 edge_long_press_up(struct input_dev *input, int index);

extern int large_area_ignore_count;
extern int large_area_uevent_count;
extern __int64 syna_dev_connect(__int64 *a1, __int64 a2, __int64 a3);
extern __int64 syna_dev_disconnect(__int64 a1, __int64 a2, __int64 a3);
extern __int64 syna_dev_resume(__int64 a1, __int64 a2, __int64 a3);
extern __int64 syna_dev_suspend(__int64 a1, __int64 a2, __int64 a3);
extern __int64 syna_tcm_v1_read_message(__int64 a1, _BYTE *a2, __int64 a3);
extern __int64 syna_tcm_v1_write_message(__int64 a1, unsigned __int8 a2, unsigned __int8 *a3, unsigned int a4, _BYTE *a5, int a6);
extern __int64 syna_tcm_v1_set_up_max_rw_size(__int64 a1, unsigned int a2, __int64 a3);
extern __int64 syna_tcm_v1_check_max_rw_size(__int64 a1);
extern __int64 syna_tcm_v1_terminate(__int64 result, __int64 a2, __int64 a3);

// Charger & TPD callback functions
extern __int64 syna_work_charger_detect_work(__int64 a1, __int64 a2, __int64 a3);
extern __int64 syna_charger_notify_call(__int64 a1, __int64 a2, __int64 **a3);
extern __int64 tpd_init_tpinfo(__int64 a1, __int64 a2, __int64 a3);
extern __int64 tpd_get_wakegesture(__int64 a1);
extern __int64 tpd_enable_wakegesture(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_singlegamegesture(__int64 a1);
extern __int64 tpd_set_singlegamegesture(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_singleaodgesture(__int64 a1);
extern __int64 tpd_set_singleaodgesture(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_singlefpgesture(__int64 a1);
extern __int64 tpd_set_singlefpgesture(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_set_one_key(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_one_key(__int64 a1);
extern __int64 tpd_test_cmd_store(__int64 a1, __int64 a2, __int64 a3);
extern __int64 tpd_test_cmd_show(__int64 a1, char *a2, __int64 a3);
extern __int64 tpd_get_tp_report_rate(__int64 a1);
extern __int64 tpd_set_tp_report_rate(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_sensibility_level(__int64 a1);
extern __int64 tpd_set_sensibility_level(__int64 a1, unsigned __int8 a2, __int64 a3);
extern __int64 tpd_get_follow_hand_level(__int64 a1);
extern __int64 tpd_set_follow_hand_level(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_stability_level(__int64 a1);
extern __int64 tpd_set_stability_level(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_rotation_limit_level(__int64 a1);
extern __int64 tpd_set_rotation_limit_level(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_set_display_rotation(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_play_game(__int64 a1);
extern __int64 tpd_set_play_game(__int64 a1, unsigned int a2, __int64 a3);
extern __int64 tpd_set_game_partition(void);
extern __int64 tpd_get_frame_data(__int64 a1);
extern __int64 tpd_set_frame_data(__int64 a1, unsigned int a2, __int64 a3);
extern __int64 tpd_set_fake_sleep(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_fake_sleep(__int64 a1);
extern __int64 tpd_set_screen_off_awake(__int64 a1, int a2, __int64 a3);
extern __int64 tpd_get_screen_off_awake(__int64 a1);
extern __int64 tpd_get_palm_mode(__int64 a1);
extern __int64 tpd_set_palm_mode(__int64 a1, unsigned int a2);
extern __int64 syna_ghost_check_reset(__int64 a1);

// Testing check and helper functions
bool syna_tcm_testing_0500_check_upper_bound(unsigned short *a1, unsigned short *a2, __int64 a3, unsigned int a4);
bool syna_tcm_testing_0500_check_lower_bound(unsigned short *a1, unsigned short *a2, __int64 a3, unsigned int a4);
bool syna_tcm_testing_0A00_check_upper_bound(short *a1, short *a2, __int64 a3, unsigned int a4);
bool syna_tcm_testing_0A00_check_lower_bound(short *a1, short *a2, __int64 a3, unsigned int a4);
__int64 syna_tcm_testing_check_frame_data(__int64 a1, unsigned __int64 a2, __int64 a3, int a4, _DWORD *a5, __int64 a6, unsigned __int64 a7);
__int64 syna_tcm_testing_check_frame_data_0(__int64 a1, unsigned __int64 a2, __int64 a3, int a4, _DWORD *a5, __int64 a6, unsigned __int64 a7);

extern __int64 tpd_id0_report_work(void);
extern __int64 tpd_id1_report_work(void);
extern __int64 tpd_id2_report_work(void);
extern __int64 tpd_id3_report_work(void);
extern __int64 tpd_id4_report_work(void);
extern __int64 tpd_id5_report_work(void);
extern __int64 tpd_id6_report_work(void);
extern __int64 tpd_id7_report_work(void);
extern __int64 tpd_id8_report_work(void);
extern __int64 tpd_id9_report_work(void);
extern __int64 ztp_probe_work(__int64 a1, __int64 a2, __int64 a3);
extern void tpd_resume_work(void);
extern void tpd_suspend_work(void);
extern __int64 ufp_report_lcd_state_work(void);
extern __int64 tp_ghost_check_work(void);
extern void ufp_single_tap_work(void);
extern __int64 __fastcall tpd_report_uevent(unsigned __int8 a1, __int64 a2, __int64 a3);

// Injected globals
#include "globals.h"

#endif // _DEFS_H
