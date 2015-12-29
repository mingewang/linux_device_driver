#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/uaccess.h> /* copy_from/to_user */

/* Declaration of memory.c functions */
static int memory_open(struct inode *inode, struct file *filp);
static int memory_release(struct inode *inode, struct file *filp);
static ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t memory_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void memory_exit(void);
static int memory_init(void);

/* Structure that declares the usual file */
/* access functions */
static struct file_operations memory_fops = {
  .read = memory_read,
  .write = memory_write,
  .open = memory_open,
  .release = memory_release
};


/* Global variables of the driver */
/* Major number */
// # please: mknod /dev/memory c 60 0
static int memory_major = 60;
static int memory_buffer_size = 10;
/* Buffer to store data */
static char *memory_buffer;

 
static int memory_init(void) /* Constructor */
{

  int result;

  /* Registering device */
  result = register_chrdev(memory_major, "memory", &memory_fops);
  if (result < 0) {
    printk(
      "<1>memory: cannot obtain major number %d\n", memory_major);
    return result;
  }

  /* Allocating memory for the buffer */
  memory_buffer = kmalloc(memory_buffer_size, GFP_KERNEL); 
  if (!memory_buffer) { 
    result = -ENOMEM;
    goto fail; 
  } 
  memset(memory_buffer, 0, memory_buffer_size);

  printk("<1>Inserting memory module\n"); 
  return 0;

  fail: 
    memory_exit(); 
    return result;
}
 
static void memory_exit(void) /* Destructor */
{
  /* Freeing the major number */
  unregister_chrdev(memory_major, "memory");

  /* Freeing buffer memory */
  if (memory_buffer) {
    kfree(memory_buffer);
  }

  printk("<1>Removing memory module\n");

}

static int memory_open(struct inode *inode, struct file *filp) {
  /* Success */
  return 0;
}

static int memory_release(struct inode *inode, struct file *filp) {
 
  /* Success */
  return 0;
}

static ssize_t memory_read(struct file *filp, char *buf, 
                    size_t count, loff_t *f_pos) { 

  int read_size = count; 
  long ret;
  printk("<1>read count is %d: \n", (int)count); 

  /* Transfering data to user space */ 
  if ( count <= memory_buffer_size ) {
  	ret = copy_to_user(buf,memory_buffer,count);
  } else {
  	ret = copy_to_user(buf,memory_buffer,memory_buffer_size);
	read_size = memory_buffer_size;
  	printk("<1>read too bigger, only read part of   \n"); 

  }

  /* Changing reading position as best suits */ 
  if (*f_pos == 0) { 
   *f_pos+=read_size; 
    return read_size; 
  } else {
    return 0; 
  }
 
}


static ssize_t memory_write( struct file *filp, const char *buf,
                      size_t count, loff_t *f_pos) {

  int write_size = count; 
  long ret;

  printk("<1>write count is %d: \n", (int)count); 
  if( count <= memory_buffer_size ) {
	  //tmp=buf+count-1;
	  ret = copy_from_user(memory_buffer,buf,count);
  } else {
	// count is bigger, only copy part of
	  ret = copy_from_user(memory_buffer,buf,memory_buffer_size);
	  write_size = memory_buffer_size;
  	  printk("<1>too bigger, only write part of input  \n"); 
  }

  /* Changing write position as best suits */ 
//  if (*f_pos == 0) { 
  //  *f_pos+=write_size; 
    return write_size; 
 // } else { 
  //  return 1; 
  //}

}


/* Declaration of the init and exit functions */
module_init(memory_init);
module_exit(memory_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Min Wang <mingewang@gmail.com>");
MODULE_DESCRIPTION("Memroy kernel module");
