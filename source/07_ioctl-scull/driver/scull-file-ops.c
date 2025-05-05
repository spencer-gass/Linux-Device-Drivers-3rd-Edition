#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "main.h"
#include "scull-file-ops.h"
#include "scull-ioctl.h"

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;
    // use a macro from linux/kernel.h to get the scull_dev pointer
    // That contains the cdev pointer from the inode pointer.
    // container_of(pointer, container_type, container_field);
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;
    PDEBUG( "open, inode:%p, file:%p\n", inode, filp);

    /* now trim to 0 the length of the device if open was write-only */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        PDEBUG( "open in write only mode.\n");
        scull_trim(dev); /* ignore errors */
        scull_dev_data_init(dev);
    }

    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    PDEBUG( "release, inode:%p, file:%p\n", inode, filp);
    return 0;
}

int scull_dev_data_init(struct scull_dev *dev)
{
    dev->data = kmalloc(sizeof(struct scull_list_node), GFP_KERNEL);
    if (!dev->data){
        printk(KERN_WARNING "Failed to kmalloc scull_dev data\n");
        return -ENOMEM;
    }
    dev->data->data = NULL;
    dev->data->next = NULL;
    PDEBUG( "Initializing scull data\n");

    return 0;
}

int scull_trim(struct scull_dev *dev){

    struct scull_list_node *next, *dptr;
    int block_list_size = dev->block_list_size;
    int i;

    PDEBUG( "trim: block list size:%d.\n", block_list_size);

    for (dptr = dev->data; dptr; dptr = next){
        if(dptr->data){
            for (i = 0; i < block_list_size; i ++){
                kfree(dptr->data[i]);
            }
            PDEBUG( "trim: free node:%pp\n", dptr->data);
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        PDEBUG( "trim: free head:%p\n", dptr);
        kfree(dptr);
    }

    dev->size = 0;

    return 0;
}

struct scull_list_node *scull_follow(struct scull_dev *dev, int list_node_idx)
{
    struct scull_list_node *qsptr = dev->data;
    int i;
    for (i = 0; i < list_node_idx; i++){
        PDEBUG( "follow: node:%d, pointer:%p\n", i, qsptr);
        qsptr = qsptr->next;
    }
    return qsptr;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

    struct scull_dev *dev = filp->private_data;
    struct scull_list_node *dptr;
    int block_size = dev->block_size;
    int block_list_size = dev->block_list_size;
    int list_node_data_size = block_size * block_list_size;
    int list_node_idx;
    int block_idx;
    int byte_idx;
    int block_list_idx;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    list_node_idx = (long)*f_pos / list_node_data_size;
    block_list_idx = (long)*f_pos % list_node_data_size;
    block_idx = block_list_idx / block_size;
    byte_idx = block_list_idx % block_size;

    dptr = scull_follow(dev, list_node_idx);

    PDEBUG( "read: filp:                 %p\n", filp);
    PDEBUG( "read: count:                %ld\n", count);
    PDEBUG( "read: f_pos:                %p\n", f_pos);
    PDEBUG( "read: scull dev:            %p\n", dev);
    PDEBUG( "read: scull size:           %ld\n", dev->size);
    PDEBUG( "read: node ptr:             %p\n", dptr);
    PDEBUG( "read: block size:           %d\n", block_size);
    PDEBUG( "read: block list size:      %d\n", block_list_size);
    PDEBUG( "read: list node data size:  %d\n", list_node_data_size);
    PDEBUG( "read: list node idx:        %d\n", list_node_idx);
    PDEBUG( "read: block list idx:       %d\n", block_list_idx);
    PDEBUG( "read: block idx:            %d\n", block_idx);
    PDEBUG( "read: byte idx:             %d\n", byte_idx);

    if (dptr == NULL || !dptr->data || !dptr->data[block_idx])
        goto out;

    /* read only up to the end of this block */
    if (count > block_size - byte_idx)
        count = block_size - byte_idx;

    if (copy_to_user(buf, dptr->data[block_idx] + byte_idx, count)){
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
}

static ssize_t scull_write_common(struct file *filp, const char *buf, size_t count, loff_t *f_pos, int is_user_buf)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_list_node *dptr;
    int block_size = dev->block_size;
    int block_list_size = dev->block_list_size;
    int list_node_data_size = block_size * block_list_size;
    int list_node_idx;
    int block_idx;
    int byte_idx;
    int block_list_idx;
    ssize_t retval = -ENOMEM;
    const char *operation = is_user_buf ? "write" : "append";

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    // Convert byte offset to q_set index, block index, and byte offset in the block
    list_node_idx = (long)*f_pos / list_node_data_size;
    block_list_idx = (long)*f_pos % list_node_data_size;
    block_idx = block_list_idx / block_size;
    byte_idx = block_list_idx % block_size;

    // Traverse to the indicated block
    dptr = scull_follow(dev, list_node_idx);

    PDEBUG("%s: filp:                 %p\n", operation, filp);
    PDEBUG("%s: count:                %ld\n", operation, count);
    PDEBUG("%s: f_pos:                %lld\n", operation, (long long)*f_pos);
    PDEBUG("%s: scull dev:            %p\n", operation, dev);
    PDEBUG("%s: scull size:           %ld\n", operation, dev->size);
    PDEBUG("%s: node ptr:             %p\n", operation, dptr);
    PDEBUG("%s: block size:           %d\n", operation, block_size);
    PDEBUG("%s: block list size:      %d\n", operation, block_list_size);
    PDEBUG("%s: list node data size:  %d\n", operation, list_node_data_size);
    PDEBUG("%s: list node idx:        %d\n", operation, list_node_idx);
    PDEBUG("%s: block list idx:       %d\n", operation, block_list_idx);
    PDEBUG("%s: block idx:            %d\n", operation, block_idx);
    PDEBUG("%s: byte idx:             %d\n", operation, byte_idx);

    if (!dptr){
        printk(KERN_WARNING "scull_dev isn't initialized.\n");
        goto out;
    }
    if (!dptr->data) {
        dptr->data = kmalloc(block_list_size * sizeof(char *), GFP_KERNEL);
        if (!dptr->data){
            goto out;
        }
        memset(dptr->data, 0, block_list_size * sizeof(char *));
    }
    if (!dptr->data[block_idx]) {
        dptr->data[block_idx] = kmalloc(block_size, GFP_KERNEL);
        if (!dptr->data[block_idx]){
            goto out;
        }
    }

    // write only to the end of the block
    if (count > block_size - byte_idx)
        count = block_size - byte_idx;

    if (is_user_buf) {
        if (copy_from_user(dptr->data[block_idx] + byte_idx, buf, count)) {
            retval = -EFAULT;
            goto out;
        }
    } else {
        /* For non-user buffers (like in scull_append), use direct copy */
        memcpy(dptr->data[block_idx] + byte_idx, buf, count);
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos)
        dev->size = *f_pos;

out:
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    return scull_write_common(filp, buf, count, f_pos, IS_USER_BUF);
}

loff_t scull_llseek(struct file *filp, loff_t offset, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;

    PDEBUG( "llseek\n");

    switch(whence){
        case 0: // SEEK_SET
            newpos = offset;
            break;
        case 1: // SEEK_CUR
            newpos = filp->f_pos + offset;
            break;
        case 2: // SEEK_END
            newpos = dev->size + offset;
            break;
        default:
            return -EINVAL;
    }
    if (newpos < 0) return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}

static int scull_reset(struct file *filp)
{
    struct scull_dev *dev = filp->private_data;
    scull_trim(dev);
    return scull_dev_data_init(dev);
}

static int scull_status(struct file *filp, unsigned long argu)
{
    struct scull_dev *dev = filp->private_data;
    struct ioctl_arg argk = {
        .len = 0,
        .msg = ""
    };

    memset(argk.msg, 0, MAX_IOCTL_MSG_SIZE * sizeof(char));

    argk.len += sprintf(argk.msg+argk.len, "Device numbers %d:%d\n",
        MAJOR(dev->cdev.dev),
        MINOR(dev->cdev.dev));

    argk.len += sprintf(argk.msg+argk.len, "Block Size:      %d\n", dev->block_size);
    argk.len += sprintf(argk.msg+argk.len, "Block List Size: %d\n", dev->block_list_size);
    argk.len += sprintf(argk.msg+argk.len, "Current Size:    %ld\n",dev->size );
    argk.len += sprintf(argk.msg+argk.len, "Semaphore State: %d\n", dev->sem.count);

    PDEBUG("Status Size: %d\n", argk.len);

    if (argk.len > MAX_IOCTL_MSG_SIZE){
        PDEBUG("Status string exceeded buffer and was truncated.\n");
        argk.len = MAX_IOCTL_MSG_SIZE;
    }

    if (copy_to_user((void __user *)argu, &argk, sizeof(struct ioctl_arg))) {
        printk(KERN_WARNING "Failed to copy to user space.\n");
        return -EFAULT;
    }

    return 0;

}

static int scull_ioctl_write(struct file *filp, unsigned long argu)
{
    struct ioctl_arg argk;
    int result;

    if (!argu) {
        printk(KERN_WARNING "ioctl_write message pointer is NULL\n");
        return -EFAULT;
    }
    if (copy_from_user(&argk, (struct ioctl_arg __user *)argu, sizeof(struct ioctl_arg))) {
        printk(KERN_WARNING "Failed to copy ioctl_write message to kernel space.\n");
        return -EFAULT;
    }

    if (argk.len < 1) {
        printk(KERN_WARNING "ioctl_write message length is less than 1.\n");
        return -EINVAL;
    }

    PDEBUG("ioctl_write message: %s\n", argk.msg);
    PDEBUG("ioctl_write length:  %d\n", argk.len);

    result = scull_write_common(filp, argk.msg, argk.len, &filp->f_pos, IS_KERNEL_BUF);

    return result;
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
    case IOCTL_WRITE:
        PDEBUG("ioctl cmd: ioctl_write\n");
        retval = scull_ioctl_write(filp, arg);
        break;
    }

    return retval;
}
