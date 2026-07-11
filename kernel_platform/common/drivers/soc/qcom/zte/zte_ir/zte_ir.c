/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/spi/spi.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/build_bug.h>
#include <linux/spinlock.h>
#include <linux/overflow.h>

#define ZTE_IR_DRIVER_NAME             "zte_ir"
#define ZTE_IR_CLASS_NAME              "zte_ir_class"
#define ZTE_IR_OF_COMPATIBLE           "zte,zte_ir"

#define ZTE_IR_IOC_MAGIC               'i'
#define ZTE_IR_IOC_SET_CARRIER         _IOW(ZTE_IR_IOC_MAGIC, 0x31, int)

#define ZTE_IR_DEFAULT_CARRIER_HZ      38000U
#define ZTE_IR_DEFAULT_SPI_SPEED_HZ    608000U
#define ZTE_IR_SPI_WORDS_PER_CARRIER   16U
#define ZTE_IR_DT_MAX_SPI_HZ           5000000U

#define ZTE_IR_TX_WORD_CAPACITY        40000U
#define ZTE_IR_TX_BUFFER_BYTES         0x13880U
#define ZTE_IR_PRIVATE_DATA_SIZE       0x138e8U

struct zte_ir_recovered_device {
	dev_t devt;                              /* 0x00000, size 0x4 */
	u16 tx_buf[ZTE_IR_TX_WORD_CAPACITY];     /* 0x00004, size 0x13880 */
	u16 pattern_high;                        /* 0x13884, size 0x2 */
	u16 pattern_low;                         /* 0x13886, size 0x2 */
	u32 speed_hz;                            /* 0x13888, size 0x4 */
	u8 reserved_1388c[0x4];                 /* 0x1388c, pointer alignment */
	struct spi_device *spi;                  /* 0x13890, protected by spi_lock */
	u8 reserved_13898[0x8];                 /* 0x13898, semantics unknown */
	spinlock_t spi_lock;                     /* 0x138a0, size 0x4 */
	u8 reserved_138a4[0x4];                 /* 0x138a4, mutex alignment */
	struct mutex buf_lock;                   /* 0x138a8, size 0x30 */
	struct list_head device_entry;           /* 0x138d8, size 0x10 */
} __aligned(8);

struct zte_ir_runtime {
	struct zte_ir_recovered_device stock;     /* 0x00000 .. 0x138e7 */
	unsigned int users;                       /* 0x138e8 */
	bool removed;                             /* 0x138ec */
	u8 reserved_138ed[0x3];                  /* 0x138ed, alignment */
} __aligned(8);

static_assert(sizeof(struct zte_ir_recovered_device) == ZTE_IR_PRIVATE_DATA_SIZE);
static_assert(sizeof(struct zte_ir_runtime) == 0x138f0);

/* Global device administration */
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);
static int spidev_major;
static struct class *zte_ir_class;
static unsigned long device_in_use;

/* Forward declarations */
static struct spi_device *zte_ir_spi_get(struct zte_ir_runtime *runtime);
static int zte_ir_encode_pulses(struct zte_ir_runtime *runtime,
				const u32 *pulses,
				size_t pulse_count,
				u32 speed_hz,
				size_t *word_count);

static int zte_ir_open(struct inode *inode, struct file *file)
{
	struct zte_ir_runtime *runtime = NULL;
	struct zte_ir_runtime *pos;
	int ret = -ENXIO;

	mutex_lock(&device_list_lock);

	list_for_each_entry(pos, &device_list, stock.device_entry) {
		if (pos->stock.devt == inode->i_rdev) {
			runtime = pos;
			break;
		}
	}

	if (!runtime) {
		pr_debug("zte_ir: minor %d device not found\n", iminor(inode));
		mutex_unlock(&device_list_lock);
		return -ENXIO;
	}

	if (runtime->removed) {
		mutex_unlock(&device_list_lock);
		return -ENXIO;
	}

	runtime->users++;
	file->private_data = runtime;

	ret = nonseekable_open(inode, file);
	if (ret) {
		runtime->users--;
		file->private_data = NULL;
	}

	mutex_unlock(&device_list_lock);
	return ret;
}

static int zte_ir_release(struct inode *inode, struct file *file)
{
	struct zte_ir_runtime *runtime = file->private_data;
	bool free_structure = false;

	if (!runtime)
		return -ENODEV;

	mutex_lock(&device_list_lock);

	file->private_data = NULL;

	if (runtime->users > 0) {
		runtime->users--;
		if (runtime->removed && runtime->users == 0) {
			free_structure = true;
			pr_debug("zte_ir: release triggered deferred free\n");
		}
	} else {
		pr_debug("zte_ir: user counter underflow prevented\n");
	}

	mutex_unlock(&device_list_lock);

	if (free_structure)
		kfree(runtime);

	return 0;
}

static struct spi_device *zte_ir_spi_get(struct zte_ir_runtime *runtime)
{
	struct spi_device *spi;
	unsigned long flags;

	if (!runtime)
		return NULL;

	spin_lock_irqsave(&runtime->stock.spi_lock, flags);
	spi = spi_dev_get(runtime->stock.spi);
	spin_unlock_irqrestore(&runtime->stock.spi_lock, flags);

	if (!spi)
		pr_debug("zte_ir: SPI device unavailable\n");

	return spi;
}

static int zte_ir_encode_pulses(struct zte_ir_runtime *runtime,
				const u32 *pulses,
				size_t pulse_count,
				u32 speed_hz,
				size_t *word_count)
{
	size_t total_words = 0;
	size_t pulse_index;
	u32 carrier_hz;

	if (!word_count)
		return -EINVAL;

	*word_count = 0;
	if (!runtime || !pulses || !pulse_count)
		return -EINVAL;
	if (pulse_count > ZTE_IR_TX_WORD_CAPACITY)
		return -E2BIG;
	if (speed_hz < ZTE_IR_SPI_WORDS_PER_CARRIER ||
	    speed_hz > ZTE_IR_DT_MAX_SPI_HZ)
		return -EINVAL;

	lockdep_assert_held(&runtime->stock.buf_lock);
	carrier_hz = speed_hz >> 4;

	for (pulse_index = 0; pulse_index < pulse_count; pulse_index++) {
		u64 scaled;
		u64 pulse_words;
		size_t word_index;
		u16 pattern;

		if (check_mul_overflow((u64)pulses[pulse_index],
				       (u64)carrier_hz, &scaled) ||
		    check_add_overflow(scaled, 500000ULL, &scaled))
			return -EOVERFLOW;

		pulse_words = scaled / 1000000ULL;
		if (pulse_words >= ZTE_IR_TX_WORD_CAPACITY - total_words) {
			pr_debug("zte_ir: pulse buffer limit at index %zu\n",
				 pulse_index);
			return -E2BIG;
		}

		pattern = (pulse_index & 1) ? runtime->stock.pattern_low :
					      runtime->stock.pattern_high;
		for (word_index = 0; word_index < (size_t)pulse_words;
		     word_index++)
			runtime->stock.tx_buf[total_words + word_index] = pattern;

		total_words += (size_t)pulse_words;
	}

	*word_count = total_words;
	return 0;
}

static ssize_t zte_ir_write(struct file *file, const char __user *buffer,
			    size_t count, loff_t *position)
{
	struct zte_ir_runtime *runtime;
	u32 *pulses;
	size_t pulse_count;
	struct spi_device *spi;
	struct spi_transfer transfer;
	struct spi_message message;
	size_t word_count;
	ssize_t ret;

	if (!file || !file->private_data)
		return -ENODEV;

	runtime = file->private_data;

	if (!buffer) {
		pr_debug("zte_ir: write buffer is NULL\n");
		return -EINVAL;
	}

	if (count == 0 || (count % sizeof(u32)) != 0) {
		dev_dbg(&runtime->stock.spi->dev, "zte_ir: write size %zu unaligned\n", count);
		return -EINVAL;
	}

	pulse_count = count / sizeof(u32);
	if (pulse_count > ZTE_IR_TX_WORD_CAPACITY) {
		dev_dbg(&runtime->stock.spi->dev, "zte_ir: pulse count %zu exceeds capacity\n", pulse_count);
		return -E2BIG;
	}

	pulses = memdup_user(buffer, count);
	if (IS_ERR(pulses))
		return PTR_ERR(pulses);

	mutex_lock(&runtime->stock.buf_lock);

	if (runtime->removed) {
		ret = -ESHUTDOWN;
		goto unlock_buf;
	}

	spi = zte_ir_spi_get(runtime);
	if (!spi) {
		ret = -ENODEV;
		goto unlock_buf;
	}

	ret = zte_ir_encode_pulses(runtime, pulses, pulse_count, runtime->stock.speed_hz, &word_count);
	if (ret)
		goto put_spi;

	if (word_count > 0) {
		memset(&transfer, 0, sizeof(transfer));
		transfer.tx_buf = runtime->stock.tx_buf;
		transfer.len = word_count * sizeof(u16);
		transfer.speed_hz = runtime->stock.speed_hz;

		spi_message_init(&message);
		spi_message_add_tail(&transfer, &message);

		ret = spi_sync(spi, &message);
		if (ret) {
			dev_dbg(&spi->dev, "zte_ir: SPI sync failed (err=%d)\n", (int)ret);
			goto put_spi;
		}
	}

	dev_dbg(&spi->dev, "zte_ir: successfully transmitted %zu words\n", word_count);
	ret = count;

put_spi:
	spi_dev_put(spi);
unlock_buf:
	mutex_unlock(&runtime->stock.buf_lock);
	kfree(pulses);
	return ret;
}

static long zte_ir_ioctl(struct file *file, unsigned int command, unsigned long argument)
{
	struct zte_ir_runtime *runtime;
	int carrier_hz;
	u64 speed_hz_val;
	long ret = 0;

	if (!file || !file->private_data)
		return -ENODEV;

	runtime = file->private_data;

	if (command != ZTE_IR_IOC_SET_CARRIER) {
		pr_debug("zte_ir: unsupported ioctl command 0x%x\n", command);
		return -ENOTTY;
	}

	if (get_user(carrier_hz, (int __user *)argument))
		return -EFAULT;

	if (carrier_hz <= 0)
		return -EINVAL;

	speed_hz_val = (u64)carrier_hz * ZTE_IR_SPI_WORDS_PER_CARRIER;
	if (speed_hz_val > ZTE_IR_DT_MAX_SPI_HZ)
		return -EINVAL;

	mutex_lock(&runtime->stock.buf_lock);

	if (runtime->removed) {
		ret = -ESHUTDOWN;
		goto unlock_buf;
	}

	runtime->stock.speed_hz = (u32)speed_hz_val;
	dev_dbg(&runtime->stock.spi->dev, "zte_ir: carrier set to %d Hz (SPI speed %u Hz)\n",
		carrier_hz, runtime->stock.speed_hz);

unlock_buf:
	mutex_unlock(&runtime->stock.buf_lock);
	return ret;
}

static const struct file_operations zte_ir_fops = {
	.owner = THIS_MODULE,
	.open = zte_ir_open,
	.release = zte_ir_release,
	.write = zte_ir_write,
	.unlocked_ioctl = zte_ir_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.llseek = noop_llseek,
};

static int zte_ir_probe(struct spi_device *spi)
{
	struct zte_ir_runtime *runtime;
	struct device *dev;
	dev_t devt;
	int ret;

	dev_dbg(&spi->dev, "zte_ir: probe starting\n");

	runtime = kzalloc(sizeof(*runtime), GFP_KERNEL);
	if (!runtime)
		return -ENOMEM;

	spin_lock_init(&runtime->stock.spi_lock);
	mutex_init(&runtime->stock.buf_lock);
	INIT_LIST_HEAD(&runtime->stock.device_entry);

	runtime->stock.pattern_high = 0x001f;
	runtime->stock.pattern_low = 0;
	runtime->stock.speed_hz = ZTE_IR_DEFAULT_SPI_SPEED_HZ;
	runtime->stock.spi = spi;
	runtime->users = 0;
	runtime->removed = false;

	mutex_lock(&device_list_lock);

	if (device_in_use) {
		pr_debug("zte_ir: minor 0 is already in use\n");
		mutex_unlock(&device_list_lock);
		ret = -ENODEV;
		goto free_runtime;
	}
	device_in_use = 1;

	devt = MKDEV(spidev_major, 0);
	dev = device_create(zte_ir_class, &spi->dev, devt, runtime, "zte_ir");
	ret = PTR_ERR_OR_ZERO(dev);
	if (ret) {
		dev_dbg(&spi->dev, "zte_ir: device_create failed (err=%d)\n", ret);
		device_in_use = 0;
		mutex_unlock(&device_list_lock);
		goto free_runtime;
	}

	runtime->stock.devt = devt;
	list_add(&runtime->stock.device_entry, &device_list);

	mutex_unlock(&device_list_lock);

	spi_set_drvdata(spi, runtime);
	dev_dbg(&spi->dev, "zte_ir: probe successful, speed_hz=%u\n", runtime->stock.speed_hz);

	return 0;

free_runtime:
	kfree(runtime);
	return ret;
}

static void zte_ir_remove(struct spi_device *spi)
{
	struct zte_ir_runtime *runtime = spi_get_drvdata(spi);
	unsigned long flags;
	bool free_structure = false;

	if (!runtime)
		return;

	dev_dbg(&spi->dev, "zte_ir: remove starting\n");

	mutex_lock(&device_list_lock);

	/* Prevent new opens */
	runtime->removed = true;

	/* Invalidate SPI reference under spi_lock spinlock */
	spin_lock_irqsave(&runtime->stock.spi_lock, flags);
	runtime->stock.spi = NULL;
	spin_unlock_irqrestore(&runtime->stock.spi_lock, flags);

	/* Remove from tracking list and destroy device file node */
	list_del(&runtime->stock.device_entry);
	device_destroy(zte_ir_class, runtime->stock.devt);

	/* Release minor 0 registration */
	device_in_use = 0;

	spi_set_drvdata(spi, NULL);

	if (runtime->users == 0) {
		free_structure = true;
		pr_debug("zte_ir: immediate free on remove\n");
	} else {
		pr_debug("zte_ir: deferred free on remove (users=%u)\n", runtime->users);
	}

	mutex_unlock(&device_list_lock);

	if (free_structure)
		kfree(runtime);
}

static const struct of_device_id zte_ir_of_match[] = {
	{ .compatible = ZTE_IR_OF_COMPATIBLE },
	{ }
};
MODULE_DEVICE_TABLE(of, zte_ir_of_match);

static struct spi_driver zte_ir_spi_driver = {
	.driver = {
		.name = ZTE_IR_DRIVER_NAME,
		.of_match_table = zte_ir_of_match,
	},
	.probe = zte_ir_probe,
	.remove = zte_ir_remove,
};

static int __init zte_ir_init(void)
{
	int ret;

	ret = __register_chrdev(0, 0, 1, ZTE_IR_DRIVER_NAME, &zte_ir_fops);
	if (ret < 0) {
		pr_debug("zte_ir: failed to register char device\n");
		return ret;
	}
	spidev_major = ret;

	zte_ir_class = class_create(ZTE_IR_CLASS_NAME);
	if (IS_ERR(zte_ir_class)) {
		ret = PTR_ERR(zte_ir_class);
		pr_debug("zte_ir: failed to create device class (err=%d)\n", ret);
		goto unregister_chrdev;
	}

	ret = spi_register_driver(&zte_ir_spi_driver);
	if (ret) {
		pr_debug("zte_ir: failed to register SPI driver (err=%d)\n", ret);
		goto destroy_class;
	}

	pr_info("zte_ir: module loaded successfully (major %d)\n", spidev_major);
	return 0;

destroy_class:
	class_destroy(zte_ir_class);
unregister_chrdev:
	__unregister_chrdev(spidev_major, 0, 1, ZTE_IR_DRIVER_NAME);
	return ret;
}

static void __exit zte_ir_exit(void)
{
	spi_unregister_driver(&zte_ir_spi_driver);
	class_destroy(zte_ir_class);
	__unregister_chrdev(spidev_major, 0, 1, ZTE_IR_DRIVER_NAME);
	pr_debug("zte_ir: exit completed\n");
}

module_init(zte_ir_init);
module_exit(zte_ir_exit);

MODULE_AUTHOR("ZTE Curation Team");
MODULE_DESCRIPTION("Hardened GKI-compliant ZTE Infrared SPI Driver");
MODULE_LICENSE("GPL v2");
