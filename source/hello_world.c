#include <linux/module.h>

// Called when module is loaded
static int __init m_init(void)
{
	printk(KERN_ALERT "Hello, world!\n");
	return 0;
}

// Called when module is unloaded
static void __exit m_exit(void)
{
	printk(KERN_ALERT "Bye, world!\n");
}

// Registers the init and exit functions
module_init(m_init);
module_exit(m_exit);

// Module Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Hello World program");
