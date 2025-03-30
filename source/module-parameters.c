#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static char *whom = "World";
static int howmany = 1;

/*
module_param(name,type,perm);
Associates local variable with module parameters
Supported types: bool invbool charp int long short uint ulong ushort
permission values found in <linux/stat.h>.
S_IRUGO for a parameter that can be read by the world but cannot be changed;
S_IRUGO|S_IWUSR allows root to change the parameter.
*/
module_param(howmany, int,   S_IRUGO);
module_param(whom,    charp, S_IRUGO);

static
int __init m_init(void)
{
	pr_debug("parameters test module is loaded\n");

	for (int i = 0; i < howmany; ++i) {
		pr_info("#%d Hello, %s\n", i, whom);
	}
	return 0;
}

static
void __exit m_exit(void)
{
	pr_debug("parameters test module is unloaded\n");
    for (int i = 0; i < howmany; ++i) {
        pr_info("#%d Goodbye, %s\n", i, whom);
    }
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Spencer Gass");
MODULE_DESCRIPTION("Module parameters test program");