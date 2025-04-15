#ifndef _SCULL_FILE_OPS_H
#define _SCULL_FILE_OPS_H

int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);
int scull_dev_data_init(struct scull_dev *dev);
int scull_trim(struct scull_dev *dev);
struct scull_list_node *scull_follow(struct scull_dev *dev, int list_node_idx);
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t scull_llseek(struct file *filp, loff_t fpos, int x);
int scull_ioctl(struct inode *inode, struct file *filp, unsigned int x, unsigned long y);

#endif