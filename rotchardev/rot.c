#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define DEVICE_NAME "rot"
#define CLASS_NAME "rot"
#define MESSAGE_SIZE 2048

MODULE_LICENSE("GPL");
MODULE_AUTHOR("putsi");
MODULE_DESCRIPTION("Simple character device module which rotates given message by n characters.");
MODULE_VERSION("1.1");

// How many times a character will be rotated for.
static int *rotations = 13;
// rotations is int pointer and can be read but cannot be modified.
module_param(rotations, int, S_IRUGO);
// rotations parameter description.
MODULE_PARM_DESC(rotations, "How many times a character will be rotated (default is 13 as in ROT13).");

// Device number will be stored here.
static int majorNum;
// Initialize memory for the message given by user.
static char msg[MESSAGE_SIZE] = {0};
// Variable for storing length of the string.
static short msgSize;
// How many times the device has been opened.
static int openCount;
static struct class* rotClass = NULL;
static struct device* rotDevice = NULL;

// Function prototypes for the character driver.
static int rot_open(struct inode*, struct file*);
static int rot_release(struct inode*, struct file*);
static ssize_t rot_read(struct file*, char*, size_t, loff_t*);
static ssize_t rot_write(struct file*, const char*, size_t, loff_t*);

// Linux file structure operations which the character device will support.
static struct file_operations fops =
{
	.open = rot_open,
	.read = rot_read,
	.write = rot_write,
	.release = rot_release,
};

// Rotation function.
static void rotate(void) {
	// Loop through each character.
	int i = 0;
	for(i = 0; i < msgSize; i++) {
		char c = msg[i];
		// If currect character is end of string, return.
		if (c == '\0') {
			return;
		}
		if (isalpha(c)) {
			char alpha = islower(c) ? 'a' : 'A';
			c = (c - alpha + 13) % 26 + alpha;
		}
		msg[i] = c;
	}
}

// Function which will be executed at module initialization time.
static int __init rot_init(void) {
	printk(KERN_INFO "ROT: Starting ROT-module as LKM.\n");

	// Try to get the major number dynamically if possible.
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0) {
		printk(KERN_ALERT "ROT: Could not register a major number!\n");
		return majorNum;
	}
	printk(KERN_INFO "ROT: Succesfully registered with major number %d.\n", majorNum);

	// Register the device class.
	rotClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(rotClass)) {
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "ROT: Could not register the device class!\n");
		return PTR_ERR(rotDevice);
	}
	printk(KERN_INFO "ROT: Succesfully registered the device class.\n");

	// Register the device driver.
	rotDevice = device_create(rotClass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
	if (IS_ERR(rotDevice)) {
		class_destroy(rotClass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "ROT: Could not create the device!\n");
		return PTR_ERR(rotDevice);
	}
	printk(KERN_INFO "ROT: Succesfully created the device to /dev/%s.\n", DEVICE_NAME);
	return 0;
}

// Function which will be executed on module cleanup time.
static void __exit rot_exit(void) {
	device_destroy(rotClass, MKDEV(majorNum, 0));
	class_destroy(rotClass);
	unregister_chrdev(majorNum, DEVICE_NAME);
	printk(KERN_INFO "ROT: ROT LKM unloaded successfully.\n");
}

// Function which will be executed on device open.
static int rot_open(struct inode* inodep, struct file* filep) {
	openCount++;
	printk(KERN_INFO "ROT: Succesfully opened the device for the %dth time.\n", openCount);
	return 0;
}

static ssize_t rot_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
	int errorCount = 0;
	errorCount = copy_to_user(buffer, msg, msgSize);
	if (errorCount == 0) {
		printk(KERN_INFO "ROT: Succesfully sent %d characters to user.\n", msgSize);
		msgSize = 0;
		return(0);
	} else {
		printk(KERN_INFO "ROT: Could not send %d characters to user!\n", msgSize);
		return -EFAULT;
	}
}

// Function which will be used when data is written to the character device.
// filep is a pointer to a file object.
// buffer is a pointer to the string which is to be written.
// len is length of the string buffer.
static ssize_t rot_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
	// Write characters in buffer to the message.
	sprintf(msg, "%s", buffer);
	msgSize = strlen(msg);

	// Lets rotate the message.
	printk(KERN_INFO "ROT: Rotating message by %d characters.", rotations);
	rotate();

	printk(KERN_INFO "ROT: Received %d characters to device!\n", len);
	return len;
}

// Function which will be used when the device is closed by the userspace user.
// inodep is a pointer to an inode object (see linux/fs.h).
// filep is a pointer to a file objec (see linux/fs.h).
static int rot_release(struct inode* inodep, struct file* filep) {
	printk(KERN_INFO "ROT: Device closed succesfully!\n");
	return 0;
} 

// Specify module initialization and cleanup functions.
module_init(rot_init);
module_exit(rot_exit);
