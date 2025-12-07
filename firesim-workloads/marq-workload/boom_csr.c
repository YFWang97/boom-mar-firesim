#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "boom_csr.h"

#define DRV_NAME "boom_csr"

#define CSR_MAR_ENABLE 	  "0xBC0"
#define CSR_MAR_READ_FIFO "0xBD0"
#define CSR_MODE          "0xBC1"

static int boom_csr_read_one(enum boom_csr_id id, uint64_t* val) {

	uint64_t v;
  
	switch (id) {
		case BOOM_READ_FIFO:
			asm volatile("csrr %0, 0xBD0" : "=r"(v));
			break;
		default:
			return -EINVAL;
	}


	*val = v;
   	return 0;
}

static int boom_csr_write_one(enum boom_csr_id id) {
	
	switch (id) {
		case BOOM_ENABLE:
			asm volatile("csrw 0xBC0, %0" :: "r"(1));
			break;
		case BOOM_DISABLE:
			asm volatile("csrw 0xBC0, %0" :: "r"(0));
			break;
		default:
			return -EINVAL;
	}
			
	return 0;
}

static long boom_csr_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
	boom_csr_op_t op;

	int ret = 0;

	if (_IOC_TYPE(cmd) != BOOM_CSR_MAGIC) return -ENOTTY;

	switch (cmd) {
		case BOOM_CSR_READ:
			if (copy_from_user(&op, (boom_csr_op_t *)arg, sizeof(op)))
				return -EFAULT;
			ret = boom_csr_read_one(op.id, &op.data);
			if (ret) return ret;
			if (copy_to_user((boom_csr_op_t *)arg, &op, sizeof(op)))
				return -EFAULT;
			break;

		case BOOM_CSR_WRITE:
			if (copy_from_user(&op, (boom_csr_op_t *)arg, sizeof(op)))
				return -EFAULT;
			ret = boom_csr_write_one(op.id);
			if (ret) return ret;
			break;

		default:
			return -ENOTTY;
	}

	return 0;
}

static const struct file_operations boom_csr_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = boom_csr_ioctl,
};

static struct miscdevice boom_csr_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRV_NAME,
	.fops		= &boom_csr_fops,
};

static int __init boom_csr_init(void) {
	int ret = misc_register(&boom_csr_miscdev);
	if (ret) {
		pr_err(DRV_NAME ": misc_register failed %d\n", ret);
		return ret;
	}

	pr_info(DRV_NAME ": loaded, device /dev/%s\n", boom_csr_miscdev.name);
	return 0;
}

static void __exit boom_csr_exit(void) {
	misc_deregister(&boom_csr_miscdev);
	pr_info(DRV_NAME ": unloaded\n");
}
		
module_init(boom_csr_init);
module_exit(boom_csr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yangfan Wang");
MODULE_DESCRIPTION("BOOM CSR control via ioctl");
