#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#include "main.h"
#include "short-file-ops.h"

int short_open(struct inode *inode, struct file *filp)
{
    PDEBUG( "open, inode:%p, file:%p\n", inode, filp);
    nonseekable_open(inode, filp);
    return 0;
}

int short_release(struct inode *inode, struct file *filp)
{
    PDEBUG( "release, inode:%p, file:%p\n", inode, filp);
    return 0;
}

ssize_t short_io_port_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

    unsigned long port = short_base;
    char value;


    if (*f_pos > 0)
    return 0;

    value = inb(port);
    rmb();

    PDEBUG( "read: filp:                 %p\n",  filp);
    PDEBUG( "read: count:                %ld\n", count);
    PDEBUG( "read: port:                 %ld\n", port);
    PDEBUG( "read: f_pos:                %lld\n", *f_pos);
    PDEBUG( "read: value:                %c\n",  value);

    (*f_pos)++;

    if (copy_to_user(buf, &value, 1))
        return -EFAULT;

    return 1;
}

ssize_t short_io_port_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{

    unsigned long port = short_base;
    unsigned char *kbuf, *ptr;
    size_t cnt = count;

    kbuf = kmalloc(cnt, GFP_KERNEL);

    if (!kbuf)
        return -ENOMEM;
    if (copy_from_user(kbuf, buf, cnt))
        return -EFAULT;

    PDEBUG( "wrtie: filp:                 %p\n",  filp);
    PDEBUG( "wrtie: count:                %ld\n", count);
    PDEBUG( "write: port:                 %ld\n", port);
    PDEBUG( "write: f_pos:                %lld\n", *f_pos);
    PDEBUG( "write: value:                %c\n",  *kbuf);

    ptr = kbuf;

    while (cnt--) {
		pr_debug("port=%lu(%#lx), val=%d(%#x)\n", port, port, *ptr, *ptr);
        outb(*(ptr++), port);
        wmb();
    }

    kfree(kbuf);

    return count;
}

ssize_t short_mmio_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	char v;

	if (*f_pos > 0) {
		return 0;
	}

	v = ioread8(short_iomem);
	rmb();

	(*f_pos)++;

	if (copy_to_user(buf, &v, 1)) {
		return -EFAULT;
	}

	return 1;
}

ssize_t short_mmio_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned char *kbuf, *ptr;
	size_t cnt = count;

	kbuf = kmalloc(cnt, GFP_KERNEL);

	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user(kbuf, buf, cnt))
		return -EFAULT;

	ptr = kbuf;

	while (cnt--) {
		pr_debug("mem=%p, val=%d(%#x)\n", short_iomem, *ptr, *ptr);
		iowrite8(*ptr++, short_iomem);
		wmb();
	}

	kfree(kbuf);

	return count;
}
