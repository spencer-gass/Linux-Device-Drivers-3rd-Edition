#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "main.h"
#include "scull-file-ops.h"
#include "scull-ioctl.h"

static inline int get_occupancy(struct scull_fifo_dev *dev){
    return (dev->wptr - dev->rptr) % dev->mem_size;
}

static inline int get_free_bytes(struct scull_fifo_dev *dev){
    return dev->mem_size - get_occupancy(dev);
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_fifo_dev *dev;
    // use a macro from linux/kernel.h to get the scull_fifo_dev pointer
    // That contains the cdev pointer from the inode pointer.
    // container_of(pointer, container_type, container_field);
    dev = container_of(inode->i_cdev, struct scull_fifo_dev, cdev);
    filp->private_data = dev;
    PDEBUG( "open, inode:%p, file:%p\n", inode, filp);

    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    PDEBUG( "release, inode:%p, file:%p\n", inode, filp);
    return 0;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

    struct scull_fifo_dev *dev = filp->private_data;
    int occupancy = get_occupancy(dev);
    ssize_t retval = 0;

    if (!occupancy)
        goto out;

    if (count > occupancy)
        count = occupancy;

    if (down_interruptible(&dev->rsem))
    return -ERESTARTSYS;

    if (copy_to_user(buf, dev->rptr, count)){
        retval = -EFAULT;
        goto out;
    }

    dev->rptr = dev->rptr + count;
    if (dev->rptr > dev->data + dev->mem_size)
    dev->rptr -= dev->mem_size;
    retval = count;

out:
    up(&dev->rsem);

    PDEBUG( "read: filp:                 %p\n",  filp);
    PDEBUG( "read: count:                %ld\n", count);
    PDEBUG( "read: scull dev:            %p\n",  dev);
    PDEBUG( "read: scull occupancy:      %d\n",  occupancy);
    PDEBUG( "read: scull read pointer:   %d\n",  (int)(dev->rptr - dev->data));
    PDEBUG( "read: scull write pointer:  %d\n",  (int)(dev->wptr - dev->data));

    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_fifo_dev *dev = filp->private_data;
    int free_bytes = get_free_bytes(dev);
    ssize_t retval = -ENOMEM;

    if (count > free_bytes)
        count = free_bytes;

    if (down_interruptible(&dev->wsem))
        return -ERESTARTSYS;

    if (copy_from_user(dev->wptr, buf, count)){
        retval = -EFAULT;
        goto out;
    }

    dev->wptr = dev->wptr + count;
    if (dev->wptr > dev->data + dev->mem_size)
        dev->wptr -= dev->mem_size;
    retval = count;

out:
    up(&dev->wsem);

    PDEBUG( "wrtie: filp:                 %p\n",  filp);
    PDEBUG( "wrtie: count:                %ld\n", count);
    PDEBUG( "wrtie: scull dev:            %p\n",  dev);
    PDEBUG( "wrtie: scull free bytes:     %d\n",  free_bytes);
    PDEBUG( "wrtie: scull read pointer:   %d\n",  (int)(dev->rptr - dev->data));
    PDEBUG( "wrtie: scull write pointer:  %d\n",  (int)(dev->wptr - dev->data));

    return retval;
}

loff_t scull_llseek(struct file *filp, loff_t fpos, int x)
{
    PDEBUG( "llseek\n");
    return 0;
}

static int scull_reset(struct file *filp){

    struct scull_fifo_dev *dev = filp->private_data;

    if (down_interruptible(&dev->wsem))
        return -ERESTARTSYS;
    if (down_interruptible(&dev->rsem))
        return -ERESTARTSYS;

    dev->wptr = dev->data;
    dev->rptr = dev->data;

    up(&dev->wsem);
    up(&dev->rsem);

    return 0;
}

static int scull_status(struct file *filp, unsigned long status_u)
{
    struct scull_fifo_dev *dev = filp->private_data;
    struct ioctl_status status_k = {
        .len = 0,
        .msg = ""
    };

    memset(status_k.msg, 0, MAX_IOCTL_MSG_SIZE * sizeof(char));

    status_k.len += sprintf(status_k.msg+status_k.len, "Device numbers %d:%d\n",
        MAJOR(dev->cdev.dev),
        MINOR(dev->cdev.dev));

    status_k.len += sprintf(status_k.msg+status_k.len, "FIFO memory size:       %d\n", dev->mem_size);
    status_k.len += sprintf(status_k.msg+status_k.len, "Write Pointer:          %d\n", (int) (dev->wptr - dev->data));
    status_k.len += sprintf(status_k.msg+status_k.len, "Read Pointer:           %d\n", (int) (dev->rptr - dev->data));
    status_k.len += sprintf(status_k.msg+status_k.len, "Occupancy:              %d\n", get_occupancy(dev));
    status_k.len += sprintf(status_k.msg+status_k.len, "Free Bytes:             %d\n", get_free_bytes(dev));
    PDEBUG("Pre lock prints\n");
    if (dev->wr_mutex_type == SPINLOCK){
        status_k.len += sprintf(status_k.msg+status_k.len, "Write Mutex:            Spin Lock\n");
        status_k.len += sprintf(status_k.msg+status_k.len, "Write Spin Lock Locked: %d\n", spin_is_locked(&dev->wspinlock));
    } else {
        status_k.len += sprintf(status_k.msg+status_k.len, "Write Mutex:            Semaphore\n");
        status_k.len += sprintf(status_k.msg+status_k.len, "Write Semaphore State:  %d\n", dev->wsem.count);
    }
    if (dev->rd_mutex_type == SPINLOCK){
        status_k.len += sprintf(status_k.msg+status_k.len, "Read Mutex:             Spin Lock\n");
        status_k.len += sprintf(status_k.msg+status_k.len, "Read Spin Lock Locked:  %d\n", spin_is_locked(&dev->rspinlock));
    } else {
        status_k.len += sprintf(status_k.msg+status_k.len, "Read Mutex:             Semaphore\n");
        status_k.len += sprintf(status_k.msg+status_k.len, "Read Semaphore State:   %d\n", dev->rsem.count);
    }

    PDEBUG("Status Size: %d", status_k.len);

    if (status_k.len > MAX_IOCTL_MSG_SIZE)
        status_k.len = MAX_IOCTL_MSG_SIZE;

    if (copy_to_user((void __user *)status_u, &status_k, sizeof(struct ioctl_status))) {
        printk(KERN_WARNING "Failed to copy to user space.");
        return -EFAULT;
    }

    return 0;
}

static int scull_set_mutex_type(struct file *filp, unsigned long mutex_config_u)
{
    struct scull_fifo_dev *dev = filp->private_data;
    struct ioctl_mutex_config mutex_config_k;

    if (copy_from_user(&mutex_config_k, (void __user *) mutex_config_u, sizeof(struct ioctl_mutex_config))){
        PDEBUG("Failed to copy from user on mutex config set.\n");
        return -EFAULT;
    }

    // Ignoring cases where the mutex type changes while the mutex is in use
    // For this example, only change the mutex type when the module isn't in use.
    dev->wr_mutex_type = mutex_config_k.wr_mutex_type;
    dev->rd_mutex_type = mutex_config_k.rd_mutex_type;

    return 0;
}

long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

    int err = 0;
    int retval = 0;

    PDEBUG( "%s() is invoked\n", __FUNCTION__);

    if (_IOC_TYPE(cmd) != IOCTL_IOC_MAGIC) {
        PDEBUG("ioctl command error\n");
        return -ENOTTY;
    }

    if (_IOC_NR(cmd) > IOCTL_MAXNR) {
        PDEBUG("Number of ioctl parameters error\n");
        return -ENOTTY;
    }

    err = access_ok((void *__user)arg, _IOC_SIZE(cmd));
    if (!err)
        return -EFAULT;

    switch (cmd) {
    case IOCTL_RESET:
        PDEBUG("ioctl cmd: reset\n");
        retval = scull_reset(filp);
        break;
    case IOCTL_STATUS:
        PDEBUG("ioctl cmd: status\n");
        retval = scull_status(filp, arg);
        break;
    case IOCTL_SET_MUTEX_T:
        PDEBUG("ioctl cmd: set mutex type\n");
        retval = scull_set_mutex_type(filp, arg);
        break;
    }

    return retval;
}