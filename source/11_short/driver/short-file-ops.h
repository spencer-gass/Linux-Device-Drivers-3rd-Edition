#ifndef _SHORT_FILE_OPS_H
#define _SHORT_FILE_OPS_H

int short_open(struct inode *inode, struct file *filp);
int short_release(struct inode *inode, struct file *filp);
ssize_t short_io_port_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t short_io_port_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
ssize_t short_mmio_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t short_mmio_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

#endif
