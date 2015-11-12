#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("MIT");
MODULE_AUTHOR("putsi");
MODULE_DESCRIPTION("Hello world module with simple user parameter");
MODULE_VERSION("1.0");

// Default value if user didn't give any parameters while loading module.
static char *user = "world";
// user is char pointer and can be read but cannot be modified.
module_param(user, charp, S_IRUGO);
// user parameter description.
MODULE_PARM_DESC(user, "User's name which will be used in helloing.");

// Function which will be executed at module initialization time.
static int __init hello_init(void) {
	// Lets print a string to /var/log/kern.log.
	printk(KERN_INFO "Hello %s\n", user);
	return 0;
}

// Function which will be executed on module cleanup time.
static void __exit hello_exit(void) {
	printk(KERN_INFO "Bye %s\n", user);
}

// Specify module initialization and cleanup functions.
module_init(hello_init);
module_exit(hello_exit);
