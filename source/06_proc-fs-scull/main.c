#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "main.h"
#include "scull-file-ops.h"
#include "proc-fs-ops.h"

static int scull_major = 0;
static int scull_minor = 0;

static struct scull_dev *scull_dev[SCULL_NDEVS];
static char *scull_proc_fnames[SCULL_NDEVS];

static struct file_operations scull_fops = {
	.owner   	= THIS_MODULE,
	.open    	= scull_open,
	.read    	= scull_read,
	.write   	= scull_write,
	.llseek		= scull_llseek,
	// .ioctl		= scull_ioctl,
	.release 	= scull_release,
};

static struct proc_dir_entry *parent = NULL;
static struct proc_ops proc_ops = {
	.proc_open 	    = proc_open,
	.proc_read 	    = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = proc_release,
};

static int scull_cdev_init(struct scull_dev *dev, int index)
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

static int scull_dev_init(struct scull_dev *dev, int index)
{
	int result = 0;

    // Initialize scull data structure
    dev->block_size = 4000;
    dev->block_list_size = 1000;
    dev->size = 0;
    dev->access_key = 0;
    sema_init(&dev->sem, 1);
	scull_dev_data_init(dev);

    // Register char device
    result = scull_cdev_init(dev, index);

    return result;
}

static int __init m_init(void)
{
	int result = 0;
	dev_t devno = MKDEV(scull_major, scull_minor);

	printk(KERN_NOTICE MODULE_NAME " loaded\n");

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
		printk(KERN_ERR "scull: can't get major %d\n", scull_major);
		return result;
	}

	// Create a directory in /proc for our proc files
	parent = proc_mkdir(SUB_DIR_NAME, NULL);

	for (int i=0; i<SCULL_NDEVS; i++) {

		// Allocate memory for scull data structure
		scull_dev[i] = kmalloc(sizeof(struct scull_dev), GFP_KERNEL);

		if (!scull_dev[i]){
			printk(KERN_WARNING "kmalloc failed on " MODULE_NAME "%d init.\n", i);
			return -ENOMEM;
		}

		// Initialize the scull data structure which in turn registers the character device
		result = scull_dev_init(scull_dev[i],i);

		if (result < 0) {
			// unregister character device and major number
			devno = MKDEV(scull_major, scull_minor + i);
			unregister_chrdev_region(devno, 1);
			// free memory
			kfree(scull_dev[i]);
			scull_dev[i] = NULL;
		} else {
            printk(KERN_NOTICE "Loaded " MODULE_NAME "%d\n", i);
        }

		// Create proc files
		scull_proc_fnames[i] = kasprintf(GFP_KERNEL, PROC_FS_NAME "%d", i);
		if (!proc_create_data(scull_proc_fnames[i], 0, parent, &proc_ops, scull_dev[i]))
			return -ENOMEM;
	}

	return 0;

}

static void __exit m_exit(void)
{
	dev_t devno;
	printk(KERN_NOTICE MODULE_NAME " unloaded\n");

	for (int i=0; i<SCULL_NDEVS; i++){
		if (scull_dev[i]){
			remove_proc_entry(scull_proc_fnames[i], parent);
			scull_trim(scull_dev[i]);
			cdev_del(&scull_dev[i]->cdev);
			kfree(scull_dev[i]);
            printk(KERN_NOTICE "Unloaded " MODULE_NAME "%d\n", i);
		}
	}

	remove_proc_entry(SUB_DIR_NAME, NULL);

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
