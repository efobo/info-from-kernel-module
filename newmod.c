#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include <linux/debugfs.h>
#include "user_struct.h"

#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/device.h>

#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <uapi/linux/stat.h>
#include <linux/namei.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Linux module for defining and printing kernal structure");
MODULE_VERSION("1.0");

static struct dentry *newmod_root;
static struct pci_dev *device_value;

static struct result *result;

static u32 n_pid = 1;
static char user_file_path[128];
static int vendor_id = 0;
static int device_id = 0;

/* Function prototypes */
static int __init newmod_init(void);
static void __exit newmod_exit(void);

/* Debugfs functions */
static loff_t llseek(struct file *flip, loff_t off, int whence);
static int open(struct inode *inode, struct file *flip);
static int release(struct inode *inode, struct file *flip);
static ssize_t read(struct file *flip, char __user *buf, size_t len, loff_t *off);
static ssize_t write(struct file *flip, const char __user *buf, size_t len, loff_t *off);

/* Function for getting structures */
int32_t fill_data(void);

/* Debugfs operation structure */
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = llseek,
	.open = open,
	.write = write,
	.read = read,
	.release = release,
};

static int open(struct inode *inode, struct file *flip) {
	printk(KERN_INFO "newmod: open file.\n");
	return 0;
}

static int release(struct inode *inode, struct file *flip) {
	printk(KERN_INFO "newmod: release file.\n");
	return 0;
}

static ssize_t read(struct file *flip, char __user *buf, size_t len, loff_t *off) {
	char buffer[2048];
	int32_t ret = 0;

	printk(KERN_INFO "newmod: reading in process.\n");

	if (len > 2048) {
		printk(KERN_INFO "newmod: len=%ld and *off=%lld", len, *off);
		return 0;
	}

	int32_t res = fill_data();
	if (res != 0) {
		printk(KERN_INFO "newmod: the data doesn't receive.\n");

			if (res == -1) {
			printk(KERN_ALERT "newmod: the PCI device with vendor ID = %d and device ID = %d wasn't found.\n", vendor_id, device_id);
		} else if (res == -2) {
			printk(KERN_ALERT "newmod: the file with path %s doesn't exist or you haven't proper permissions.\n", user_file_path);
		}

	}


	//write result (filled structure) to buffer
	char *c;
	int i;
	c = (char *)result;
	for (i = 0; i < sizeof(struct result); i++) {
		buffer[i] = *c++;
	}
	
	ret += i;

	if (copy_to_user(buf, buffer, ret)) {
		printk(KERN_ALERT "newmod: reading failed.\n");
		return -EFAULT;
	}
	*off = ret;
	return ret;
}


static ssize_t write(struct file *flip, const char __user *buf, size_t len, loff_t *off) {
	char buffer[2048];
	int32_t ret;

	printk(KERN_INFO "newmod: writing in process.\n");

	if (*off > 0 || len > 2048) {
		printk(KERN_ALERT "newmod: writing failed.\n");
		return -EFAULT;	
	}

	if (copy_from_user(buffer, buf, len)) { 
		printk(KERN_ALERT "newmod: writing failed.\n");
		return -EFAULT;
	}

	int32_t read_parameters = sscanf(buffer, "%d %d %s", &vendor_id, &device_id, user_file_path);

	if (read_parameters != 3) {
		printk(KERN_ALERT "newmod: the data in file is not correct.\n");
		return -EFAULT;	
	}

	printk(KERN_INFO "newmod: read file path = %s, vendor ID = %d, device ID = %d \n", user_file_path, vendor_id, device_id);

	ret = strlen(buffer);
	*off = ret;
	return ret;
}


static loff_t llseek(struct file *flip, loff_t off, int whence) {
	loff_t newpos;

	printk(KERN_INFO "newmod: llseek off = %lld and whence = %lld.\n", (long long)off, (long long)whence);
	switch(whence) {
		case SEEK_SET:
			newpos = off;
			break;
		case SEEK_CUR:
			newpos = flip->f_pos + off;
			break;
		case SEEK_END:
			newpos = sizeof(struct result) + off;
			break;
		default:
			return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	flip->f_pos = newpos;
	printk(KERN_INFO "newmod: llseek newpos = %lld.\n", (long long)newpos);
	return newpos;
}

int32_t fill_data() {
	int exit_code = 0;
	device_value = NULL;
	u16 dval;
	char byte;
	int i = 0;
	char name_buf[16];

	result = vmalloc(sizeof(struct result));
	struct my_pci_dev *mpd = vmalloc(10 * sizeof(struct my_pci_dev));

	
	while ((device_value = pci_get_device(vendor_id, device_id, device_value)) && i<10) {
	printk(KERN_INFO "Device %d:\n", i);
	strncpy(mpd->name, pci_name(device_value), 13);
	pci_read_config_word(device_value, PCI_VENDOR_ID, &dval);
	mpd->vendor_id = dval;
	printk(KERN_INFO "Vendor_id %d ", mpd->vendor_id);
	pci_read_config_word(device_value, PCI_DEVICE_ID, &dval);
	mpd->device_id = dval;
	printk(KERN_INFO "Device_id %d\n", mpd->device_id);
	pci_read_config_byte(device_value, PCI_REVISION_ID, &byte);
	mpd->revision_id = byte;
	pci_read_config_byte(device_value, PCI_INTERRUPT_LINE, &byte);
	mpd->interrupt_line = byte;
	pci_read_config_byte(device_value, PCI_LATENCY_TIMER, &byte);
	mpd->latency_timer = byte;
	pci_read_config_word(device_value, PCI_COMMAND, &dval);
	mpd->command = dval;
	pci_dev_put(device_value);
	result->devices[i] = *mpd;
	printk(KERN_INFO "New device %s, i = %d\n", mpd->name, i);
	i++;
	}
	result->size = i;

	if (i == 0) {
		exit_code = -1;
	}

	struct file *f;
	// struct dentry *nd = vmalloc(sizeof(struct dentry));
	f = filp_open(user_file_path, O_RDONLY, 0);

	struct my_inode *mi = vmalloc(sizeof(struct my_inode));

	if (IS_ERR(f)) {
		if (exit_code) {
			exit_code = -3;
		} else {
			exit_code = -2;
		}
	} else {
		mi->i_ino = f->f_inode->i_ino;
		mi->i_nlink = f->f_inode->i_nlink;
		mi->i_bytes = f->f_inode->i_bytes;
		printk("newmod: inode = %ld \n", f->f_inode->i_ino);
		filp_close(f, NULL);
	}

	result->my_inode = *mi;

	vfree(mpd);
	vfree(mi);

	return exit_code;
}


static int __init newmod_init(void) {
	printk(KERN_INFO "newmod: module's loading.\n");

	static struct dentry *pid_task_struct;

	
	newmod_root = debugfs_create_dir("newmod", NULL);
	if (!newmod_root) {
		printk(KERN_ALERT "newmod: failed to create the dir newmod.\n");
		return -1;
	}

	
	pid_task_struct = debugfs_create_file("data_structures", 0666, newmod_root, NULL, &fops);
	if (pid_task_struct == NULL) {
		printk(KERN_ALERT "newmod: Failed to create file data_structures");
		return -1;
	}

	return 0;
}


static void __exit newmod_exit(void) {
	debugfs_remove_recursive(newmod_root);
	printk(KERN_INFO "newmod: module unloaded.\n");
}

module_init(newmod_init);
module_exit(newmod_exit);
