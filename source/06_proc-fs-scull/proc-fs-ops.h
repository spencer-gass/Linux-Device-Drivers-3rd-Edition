#ifndef _PROC_FS_OPS
#define _PROC_FS_OPS

#define SUB_DIR_NAME        "scull"
#define PROC_FS_NAME        "scull"

int proc_open(struct inode *inode, struct file *filp);
int proc_release(struct inode *inode, struct file *filp);
int proc_show(struct seq_file *sf, void *v);


#endif