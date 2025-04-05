#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "main.h"
#include "basic-scull-file-ops.h"

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;
    // use a macro from linux/kernel.h to get the scull_dev pointer
    // That contains the cdev pointer from the inode pointer.
    // container_of(pointer, container_type, container_field);
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    /* now trim to 0 the length of the device if open was write-only */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev); /* ignore errors */
    }

    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}

int scull_trim(struct scull_dev *dev){

    struct scull_qset *next, *dptr;
    int qset = dev->qset;
    int i;

    for (dptr = dev->data; dptr; dptr = next){
        if(dptr->data){
            for (i = 0; i < qset; i ++){
                kfree(dptr->data[i]);
            }
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }

    return 0;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int qset_idx)
{
    struct scull_qset *qsptr = dev->data;
    int i;
    for (i = 0; i < qset_idx; i++){
        qsptr = qsptr->next;
    }
    return qsptr;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

    /*TODO(sgass) rename:
        quantum -> quantum_size
        qset    -> qset_length
        itemsize-> qset_size
        item    -> qset_idx
        rest    -> qset_byte_ofs
        s_pos   -> quantum_idx
        q_pos   -> quantum_byte_ofs
    */

    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    int item;
    int s_pos;
    int q_pos;
    int rest;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;

    /* read only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)){
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
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    int item;
    int s_pos;
    int q_pos;
    int rest;
    ssize_t retval = -ENOMEM;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    // Convert byte offset to q_set index, quantum index, and byte offset in the quantum
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    // Traverse to the indicated quantum
    dptr = scull_follow(dev,item);
    if (dptr == NULL)
        goto out;
    if (!dptr->data) {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }

    // write only to the end of the quantum
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)){
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
    return 0;
}

int scull_ioctl(struct inode *inode, struct file *filp, unsigned int x, unsigned long y)
{
    return 0;
}