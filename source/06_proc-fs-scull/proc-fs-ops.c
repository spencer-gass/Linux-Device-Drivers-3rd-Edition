#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "main.h"
#include "proc-fs-ops.h"

int proc_show(struct seq_file *m, void *v)
{

	struct scull_dev *scull_dev = m->private;
    seq_printf(m, "Device numbers %d:%d\n",
        MAJOR(scull_dev->cdev.dev),
        MINOR(scull_dev->cdev.dev));

    seq_printf(m, "Block Size:      %d\n", scull_dev->block_size);
    seq_printf(m, "Block List Size: %d\n", scull_dev->block_list_size);
    seq_printf(m, "Current Size:    %ld\n",scull_dev->size );
    seq_printf(m, "Semaphore count  %d\n", scull_dev->sem.count);

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