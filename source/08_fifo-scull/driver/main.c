#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "main.h"
#include "scull-file-ops.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_fifo_size = 4096;

static struct scull_fifo_dev *scull_fifo_dev[SCULL_NDEVS];

static struct file_operations scull_fops = {
	.owner   		= THIS_MODULE,
	.open    		= scull_open,
	.read    		= scull_read,
	.write   		= scull_write,
	.llseek			= no_llseek,
	.unlocked_ioctl	= scull_ioctl,
	.release 		= scull_release,
};

static int scull_cdev_init(struct scull_fifo_dev *dev, int index)
{
	int result;
	dev_t devno = MKDEV(scull_major, scull_minor + index);
	char devno_buf[20];
	PDEBUG("device number %s\n", format_dev_t(devno_buf, devno));

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	// Once this function returns, the module is live and must be ready to accpet requests.
	result = cdev_add(&dev->cdev, devno, 1);

    if (result < 0) {
		printk(KERN_WARNING "Error %d adding " MODULE_NAME "%d\n", result, index);
		cdev_del(&dev->cdev);
    }

	return result;
}

static int scull_fifo_dev_init(struct scull_fifo_dev *dev, int index)
{
	int result = 0;

    // Initialize scull data structure
    dev->data = kmalloc(scull_fifo_size * sizeof(char), GFP_KERNEL);
    if (!dev->data){
        printk(KERN_WARNING "Failed to kmalloc scull_fifo_dev data.\n");
        return -ENOMEM;
    }
    dev->wptr = dev->data;
    dev->rptr = dev->data;
    dev->mem_size = scull_fifo_size;
    sema_init(&dev->wsem, 1);
    sema_init(&dev->rsem, 1);

    // Register char device
    result = scull_cdev_init(dev, index);

    return result;
}

static int __init m_init(void)
{
	int result = 0;
	dev_t devno = MKDEV(scull_major, scull_minor);

	printk(KERN_NOTICE MODULE_NAME " loaded\n");

	result = alloc_chrdev_region(&devno, scull_minor, SCULL_NDEVS, MODULE_NAME);
	scull_major = MAJOR(devno);
	scull_minor = MINOR(devno);

	if (result < 0) {
		printk(KERN_ERR "scull: can't get major %d\n", scull_major);
		return result;
	}

	for (int i=0; i<SCULL_NDEVS; i++) {

		// Allocate memory for scull data structure
		scull_fifo_dev[i] = kmalloc(sizeof(struct scull_fifo_dev), GFP_KERNEL);

		if (!scull_fifo_dev[i]){
			printk(KERN_WARNING "kmalloc failed on " MODULE_NAME "%d init.\n", i);
			return -ENOMEM;
		}

		// Initialize the scull data structure which in turn registers the character device
		result = scull_fifo_dev_init(scull_fifo_dev[i],i);

		if (result < 0) {
			// unregister character device and major number
			devno = MKDEV(scull_major, scull_minor + i);
			unregister_chrdev_region(devno, 1);
			// free memory
			kfree(scull_fifo_dev[i]);
			scull_fifo_dev[i] = NULL;
		} else {
            printk(KERN_NOTICE "Loaded " MODULE_NAME "%d\n", i);
        }
	}

	return 0;
}

static void __exit m_exit(void)
{
	dev_t devno;
	printk(KERN_NOTICE MODULE_NAME " unloaded\n");

	for (int i=0; i<SCULL_NDEVS; i++){
		if (scull_fifo_dev[i]){
			cdev_del(&scull_fifo_dev[i]->cdev);
			kfree(scull_fifo_dev[i]->data);
			kfree(scull_fifo_dev[i]);
            printk(KERN_NOTICE "Unloaded " MODULE_NAME "%d\n", i);
		}
	}

	devno = MKDEV(scull_major, scull_minor);
	unregister_chrdev_region(devno, SCULL_NDEVS);
}

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_fifo_size, int, S_IRUGO);

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Simple Character Utility for Loading Localities (SCULL) circular buffer.");
