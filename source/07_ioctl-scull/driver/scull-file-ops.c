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
    PDEBUG( "release\n");
    return 0;
}

int scull_dev_data_init(struct scull_dev *dev)
{
    dev->data = kmalloc(sizeof(struct scull_list_node), GFP_KERNEL);
    if (!dev->data){
        printk(KERN_WARNING "Failed to kmalloc scull_dev data");
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
                PDEBUG( "trim: free block:%p\n", dptr->data[i]);
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

    PDEBUG( "write: filp:                 %p\n", filp);
    PDEBUG( "write: count:                %ld\n", count);
    PDEBUG( "write: f_pos:                %p\n", f_pos);
    PDEBUG( "write: scull dev:            %p\n", dev);
    PDEBUG( "write: scull size:           %ld\n", dev->size);
    PDEBUG( "write: node ptr:             %p\n", dptr);
    PDEBUG( "write: block size:           %d\n", block_size);
    PDEBUG( "write: block list size:      %d\n", block_list_size);
    PDEBUG( "write: list node data size:  %d\n", list_node_data_size);
    PDEBUG( "write: list node idx:        %d\n", list_node_idx);
    PDEBUG( "write: block list idx:       %d\n", block_list_idx);
    PDEBUG( "write: block idx:            %d\n", block_idx);
    PDEBUG( "write: byte idx:             %d\n", byte_idx);

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

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
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

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    // Convert byte offset to q_set index, block index, and byte offset in the block
    list_node_idx = (long)*f_pos / list_node_data_size;
    block_list_idx = (long)*f_pos % list_node_data_size;
    block_idx = block_list_idx / block_size;
    byte_idx = block_list_idx % block_size;

    // Traverse to the indicated block
    dptr = scull_follow(dev,list_node_idx);

    PDEBUG( "write: filp:                 %p\n", filp);
    PDEBUG( "write: count:                %ld\n", count);
    PDEBUG( "write: f_pos:                %p\n", f_pos);
    PDEBUG( "write: scull dev:            %p\n", dev);
    PDEBUG( "write: scull size:           %ld\n", dev->size);
    PDEBUG( "write: node ptr:             %p\n", dptr);
    PDEBUG( "write: block size:           %d\n", block_size);
    PDEBUG( "write: block list size:      %d\n", block_list_size);
    PDEBUG( "write: list node data size:  %d\n", list_node_data_size);
    PDEBUG( "write: list node idx:        %d\n", list_node_idx);
    PDEBUG( "write: block list idx:       %d\n", block_list_idx);
    PDEBUG( "write: block idx:            %d\n", block_idx);
    PDEBUG( "write: byte idx:             %d\n", byte_idx);

    if (!dptr){
        printk(KERN_WARNING "scull_dev isn't initialized.");
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

    if (copy_from_user(dptr->data[block_idx]+byte_idx, buf, count)){
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos)
        dev->size = *f_pos;

    out:
        up(&dev->sem);
        return retval;

    return 0;
}

loff_t scull_llseek(struct file *filp, loff_t fpos, int x)
{
    PDEBUG( "llseek\n");
    return 0;
}

int scull_reset(struct scull_dev *dev)
{
    return 0;
}

int scull_status(struct scull_dev *dev, unsigned long argu)
{
    char status[23] = "String from the Kernel";
    struct ioctl_arg argk = {
        .len = 23,
        .msg = ""
    };

    strcpy(argk.msg, status);

    if (copy_to_user((void __user *)argu, &argk, sizeof(struct ioctl_arg))) {
        printk(KERN_WARNING "Failed to copy to user space.");
        return -EFAULT;
    }

    return 0;
}

int scull_append(struct scull_dev *dev, unsigned long argu)
{

    struct ioctl_arg argk;

    if (!argu) {
        printk(KERN_WARNING "append message pointer is NULL");
        return -EFAULT;
    }
    if (copy_from_user(&argk, (struct ioctl_arg __user *)argu, sizeof(struct ioctl_arg))){
        printk(KERN_WARNING "Failed to copy append message to kernel space.");
        return -EFAULT;
    }

    if (argk.len < 1){
        printk(KERN_WARNING "Append message length is less than 1.");
        return -EINVAL;
    }

    PDEBUG("Append Message: %s", argk.msg);

    return 0;
}

long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

    int err = 0;
    int retval = 0;
    struct scull_dev *dev = filp->private_data;

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
        PDEBUG("ioctl cmd: reset");
        retval = scull_reset(dev);
        break;
    case IOCTL_STATUS:
        PDEBUG("ioctl cmd: status");
        retval = scull_status(dev, arg);
        break;
    case IOCTL_APPEND:
        PDEBUG("ioctl cmd: append");
        retval = scull_append(dev, arg);
        break;
    }

    return retval;
}
