#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "main.h"

static int proc_open(struct inode *inode, struct file *filp);
static int proc_release(struct inode *inode, struct file *filp);
static int print_nr = PRINT_NR;

static struct proc_dir_entry *parent = NULL;
static struct proc_ops proc_ops = {
	.proc_open 	    = proc_open,
	.proc_read 	    = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = proc_release,
};

static
int proc_show(struct seq_file *sf, void *v)
{
	int c = (sf->private) ? *(int*)(sf->private) : 1;

	for (int i = 0; i < c; ++i)
		seq_printf(sf, "Hello proc fs!\n");

	return 0;
}

static
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

static
int proc_release(struct inode *inode, struct file *filp){
    return single_release(inode, filp);
}


// Called when module is loaded
static int __init m_init(void)
{
	printk(KERN_WARNING "Loaded " MODULE_NAME " module.\n");

    // Create a directory in /proc for our proc files
	parent = proc_mkdir(SUB_DIR_NAME, NULL);

    // Create hello proc files
	if (!proc_create(PROC_FS_NAME, 0, parent, &proc_ops))
		return -ENOMEM;

    if (!proc_create_data(PROC_FS_NAME_MUL, 0, parent, &proc_ops, (void*)&print_nr))
        return -ENOMEM;
    else
	    return 0;
}

// Called when module is unloaded
static void __exit m_exit(void)
{
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	remove_proc_entry(PROC_FS_NAME, parent);
	remove_proc_entry(PROC_FS_NAME_MUL, parent);
	remove_proc_entry(SUB_DIR_NAME, NULL);
}

// Registers the init and exit functions
module_init(m_init);
module_exit(m_exit);

// Module Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Hello proc fs module.");
