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

static struct scull_dev *scull_dev[SCULL_NDEVS];

static struct file_operations scull_fops = {
	.owner   	= THIS_MODULE,
	.open    	= scull_open,
	.read    	= scull_read,
	.write   	= scull_write,
	.llseek		= scull_llseek,
	// .ioctl		= scull_ioctl,
	.release 	= scull_release,
};

static int scull_cdev_init(struct scull_dev *dev, int index)
{
	int result;
	dev_t devno = MKDEV(scull_major, scull_minor + index);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	// Once this function returns, the module is live and must be ready to accpet requests.
	result = cdev_add(&dev->cdev, devno, 1);

    if (result < 0) {
		printk(KERN_NOTICE "Error %d adding basic-scull%d\n", result, index);
		cdev_del(&dev->cdev);
    }

	return result;
}

static int scull_dev_init(struct scull_dev *dev, int index)
{
	int result = 0;

    // Initialize scull data structure
    dev = kmalloc(sizeof(struct scull_dev), GFP_KERNEL);

	if (!dev){
		printk(KERN_WARNING "kmalloc failed on basic-scull%d init.\n", index);
		return -ENOMEM;
	}

    dev->data       = NULL;
    dev->quantum    = 0;
    dev->qset	    = 0;
    dev->size	    = 0;
    dev->access_key = 0;
    sema_init(&dev->sem, 1);

    // Register char device
    result = scull_cdev_init(dev, index);

    if (result < 0){
		printk(KERN_NOTICE "Error %d adding basic-scull%d\n", result, index);
        kfree(dev);
        dev = NULL;
    }

    return result;
}

static int __init m_init(void)
{
	int result = 0;
	dev_t devno = MKDEV(scull_major, scull_minor);

	printk(KERN_WARNING MODULE_NAME " loaded\n");

	// If scull major is set to a non-zero value by the user, then use the supplied values
	if (scull_major){
		// int register_chrdev_region(dev_t first, unsigned int count, char *name);
		result = register_chrdev_region(devno, SCULL_NDEVS, MODULE_NAME);
	} else { // else get a dynamically assigned major number
		// int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);
		result = alloc_chrdev_region(&devno, scull_minor, SCULL_NDEVS, MODULE_NAME);
		scull_major = MAJOR(devno);
		scull_minor = MINOR(devno);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	for (int i=0; i<SCULL_NDEVS; i++) {

		// Initialize the scull data structure which in turn registers the character device
		result = scull_dev_init(scull_dev[i],i);

		if (result < 0) {
			printk(KERN_NOTICE "Error %d adding basic-scull%d\n", result, i);
			// unregister character device and major number
			devno = MKDEV(scull_major, scull_minor + i);
			unregister_chrdev_region(devno, 1);
		} else {
            printk(KERN_NOTICE "Loaded basic-scull%d\n", i);
        }
	}

	return 0;
}

static void __exit m_exit(void)
{
	dev_t devno;
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	for (int i=0; i<SCULL_NDEVS; i++){
		if (scull_dev[i]){
			scull_trim(scull_dev[i]);
			cdev_del(&scull_dev[i]->cdev);
			kfree(scull_dev[i]);
		}
	}

	devno = MKDEV(scull_major, scull_minor);
	// void unregister_chrdev_region(dev_t first, unsigned int count);
	unregister_chrdev_region(devno, SCULL_NDEVS);
}

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Simple Character Utility for Loading Localities (SCULL).");
