#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "main.h"
#include "basic-scull-file-ops.h"

static int scull_major = 0;
static int scull_minor = 0;

static struct scull_dev *scull_dev;

static struct file_operations scull_fops = {
	.owner   	= THIS_MODULE,
	.open    	= scull_open,
	.read    	= scull_read,
	.write   	= scull_write,
	.llseek		= scull_llseek,
	// .ioctl		= scull_ioctl,
	.release 	= scull_release,
};

static int scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err;
	int devno = MKDEV(scull_major, scull_minor + index);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	// Once this function returns, the module is live and must be ready to accpet requests.
	err = cdev_add(&dev->cdev, devno, 1);
	return err;
}

static int __init m_init(void)
{

	int result = 0;
	dev_t devno = MKDEV(scull_major, scull_minor);

	printk(KERN_WARNING MODULE_NAME " loaded\n");

	// If scull major is set to a non-zero value by the user, then use the supplied values
	if (scull_major){
		// int register_chrdev_region(dev_t first, unsigned int count, char *name);
		result = register_chrdev_region(devno, 1, MODULE_NAME);
	} else { // else get an assigned major number
		// int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);
		result = alloc_chrdev_region(&devno, scull_minor, 1, MODULE_NAME);
		scull_major = MAJOR(devno);
		scull_minor = MINOR(devno);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	// Create scull data structure
	scull_dev = kmalloc(sizeof(struct scull_dev), GFP_KERNEL);

	if (!scull_dev){
		printk(KERN_WARNING "kmalloc failed on basic-scull init.\n");
		return -ENOMEM;
	}

	// Register the character device
	result = scull_setup_cdev(scull_dev, 0);
	if (result < 0) {
		printk(KERN_NOTICE "Error %d adding scull\n", result);
		kfree(scull_dev);
		scull_dev = NULL;
	}

	return result;
}

static
void __exit m_exit(void)
{

	dev_t devno;
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	cdev_del(&scull_dev->cdev);
	scull_trim(scull_dev);
	kfree(scull_dev);

	devno = MKDEV(scull_major, scull_minor);
	// void unregister_chrdev_region(dev_t first, unsigned int count);
	unregister_chrdev_region(devno, 1);

}

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("simple Character Utility for Loading Localities (SCULL).");
