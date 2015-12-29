#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
 
static int __init hello_init(void) /* Constructor */
{
    printk(KERN_INFO "hello world!");
    return 0;
}
 
static void __exit hello_exit(void) /* Destructor */
{
    printk(KERN_INFO "bye from hello world ");
}
 
module_init(hello_init);
module_exit(hello_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Min Wang <mingewang@gmail.com>");
MODULE_DESCRIPTION("Hello World kernel module");
