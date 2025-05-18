#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "main.h"
#include "short-file-ops.h"

static int short_major = 0;
static int short_minor = 0;
unsigned long short_base = 0;
void __iomem *short_iomem;
static int short_type = IO_PORT_TYPE;

static struct file_operations io_port_fops = {
	.owner   		= THIS_MODULE,
	.open    		= short_open,
	.read    		= short_io_port_read,
	.write   		= short_io_port_write,
	.llseek			= no_llseek,
	.release 		= short_release,
};

static struct file_operations mmio_fops = {
	.owner   		= THIS_MODULE,
	.open    		= short_open,
	.read    		= short_mmio_read,
	.write   		= short_mmio_write,
	.llseek			= no_llseek,
	.release 		= short_release,
};

static void short_unregister_region(void)
{
	switch (short_type) {
		case IO_PORT_TYPE:
			release_region(short_base, SHORT_NR_PORTS);
			break;
		case REMAP_TYPE:
			ioport_unmap(short_iomem);
			release_region(short_base, SHORT_NR_PORTS);
			break;
		case MMIO_TYPE:
			iounmap(short_iomem);
			release_region(short_base, SHORT_MMIO_LENGTH);
			break;

	}
}

static int __init m_init(void)
{
	int result = 0;
	struct file_operations *fops = NULL;

	if (short_type >= NUM_TYPES || short_type < 0) {
		printk(KERN_ERR "Invalid short type: %d. 0-2 are allowed\n", short_type);
		return -EINVAL;
	}

	switch (short_type) {
		case IO_PORT_TYPE:

			short_base = SHORT_IO_PORT_BASE;
			PDEBUG("base = %#lx\n", short_base);

			if (!request_region(short_base, SHORT_NR_PORTS, MODULE_NAME)) {
				printk(KERN_ERR "short could not get I/O port address %#lx\n", short_base);
				return -ENODEV;
			}

			fops = &io_port_fops;

			break;

		case REMAP_TYPE:

			short_base = SHORT_IO_PORT_BASE;
			PDEBUG("base = %#lx\n", short_base);

			if (!request_region(short_base, SHORT_NR_PORTS, MODULE_NAME)) {
				printk(KERN_ERR "short could not get I/O port address %#lx\n", short_base);
				return -ENODEV;
			}

			short_iomem = ioremap(short_base, SHORT_NR_PORTS);
			PDEBUG("ioremap %#lx\n", (unsigned long) short_iomem);

			fops = &mmio_fops;

			break;

		case MMIO_TYPE:

			short_base = SHORT_MMIO_BASE;
			PDEBUG("base = %#lx\n", short_base);

			if (!request_mem_region(short_base, SHORT_MMIO_LENGTH, MODULE_NAME)) {
				printk(KERN_ERR "short could not get I/O mem address %#lx\n", short_base);
				return -ENODEV;
			}

			short_iomem = ioremap(short_base, SHORT_MMIO_LENGTH);
			PDEBUG("ioremap %#lx\n", (unsigned long) short_iomem);

			fops = &mmio_fops;

			break;
	}

	result = register_chrdev(short_major, MODULE_NAME, fops);
	if (result < 0) {
		printk(KERN_ERR "cannot get major number\n");
		short_unregister_region();
		return result;
	}
	short_major = (short_major == 0) ? result : short_major;

	return 0;
}

static void __exit m_exit(void)
{
	unregister_chrdev(short_major, MODULE_NAME);
	short_unregister_region();
}

module_param(short_major, int, S_IRUGO);
module_param(short_minor, int, S_IRUGO);
module_param(short_type,  int, S_IRUGO);

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Simple Hardware Operations and Raw Tests (SHORT).");
