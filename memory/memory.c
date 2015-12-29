// based on
// http://www.freesoftwaremagazine.com/articles/drivers_linux
// https://coherentmusings.wordpress.com/2012/12/22/device-node-creation-without-using-mknod/
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/uaccess.h> /* copy_from/to_user */
#include <linux/device.h>  // for device_
#include <linux/cdev.h>  // for cdev

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
// # please: mknod /dev/memory c 60 0; chmod 666 /dev/memory
// static int memory_major = 60;

// or we can dynamically create /dev/memory
static dev_t first;         // Global variable for the first device number
static struct cdev c_dev;     // Global variable for the character device structure
static struct class *cl;     // Global variable for the device class
static int init_result;

// buffer size will be 10 bytes 
static int memory_buffer_size = 10;
/* Buffer to store data */
static char *memory_buffer;

 
static int memory_init(void) /* Constructor */
{

  int result;

  /* Registering device */
  /* 
  result = register_chrdev(memory_major, "memory", &memory_fops);
  if (result < 0) {
    printk(
      "<1>memory: cannot obtain major number %d\n", memory_major);
    return result;
  }
  */

    // automatically create /dev/memory 
    // get a major number and range of ninor numbers
    init_result = alloc_chrdev_region( &first, 0, 1, "memory" );
 
    if( 0 > init_result )
    {
        printk( KERN_ALERT "Device Registration failed\n" );
        return -1;
    }
    else
    {
        printk( KERN_ALERT "Major number is: %d\n",init_result );
    //    return 0;
    }

    // create device class 
    if ( (cl = class_create( THIS_MODULE, "chardev" ) ) == NULL )
    {
        printk( KERN_ALERT "Class creation failed\n" );
        unregister_chrdev_region( first, 1 );
        return -1;
    }

    // create device , udev will create device node /dev/memory
    // how can we set the correct permission on /dev/memory? 
    // /etc/udev/udev.conf : default_mode = "0666"
    // or udev rule? http://www.reactivated.net/writing_udev_rules.html
    if( device_create( cl, NULL, first, NULL, "memory" ) == NULL )
    {
        printk( KERN_ALERT "Device creation failed\n" );
        class_destroy(cl);
        unregister_chrdev_region( first, 1 );
        return -1;
    }

    // create device 
    cdev_init( &c_dev, &memory_fops );

    // add device to the system 
    if( cdev_add( &c_dev, first, 1 ) == -1)
    {
        printk( KERN_ALERT "Device addition failed\n" );
        device_destroy( cl, first );
        class_destroy( cl );
        unregister_chrdev_region( first, 1 );
        return -1;
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
  // unregister_chrdev(memory_major, "memory");

    cdev_del( &c_dev );
    device_destroy( cl, first );
    class_destroy( cl );
    unregister_chrdev_region( first, 1 );

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
