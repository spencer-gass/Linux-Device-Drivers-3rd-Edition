#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "proc-fs-ops.h"

int proc_show(struct seq_file *sf, void *v)
{
	int c = (sf->private) ? *(int*)(sf->private) : 1;

	for (int i = 0; i < c; ++i)
		seq_printf(sf, "Hello proc fs!\n");

	return 0;
}

int proc_open(struct inode *inode, struct file *filp)
{
    /*
	 * the variable passed from proc_create_data() can be extracted
	 * from inode by PED_DATA() macro.
	 * Here, we simply pass it to single_open which will be store as
	 * the `private` field of struct seq_file struct.
	 */
	return single_open(filp, proc_show, pde_data(inode));
}

int proc_release(struct inode *inode, struct file *filp){
    return single_release(inode, filp);
}