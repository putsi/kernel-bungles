#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define DEVICE_NAME "cry"
#define CLASS_NAME "cryptor"
#define MESSAGE_SIZE 2048

MODULE_LICENSE("GPL");
MODULE_AUTHOR("putsi");
MODULE_DESCRIPTION("Simple character device module which performs symmetric encryptions/decryptions.");
MODULE_VERSION("1.1");

// Encryption key..
static char *encryptionKey = NULL;
// Encryption key is char pointer and ... TODO: Fix permissions
module_param(encryptionKey, charp, 0600);
// Encryption key parameter description.
MODULE_PARM_DESC(encryptionKey, "Encryption key that will be used in cryptography operations.");


// Device number will be stored here.
static int majorNum;
// Initialize memory for the message given by user.
static char msg[MESSAGE_SIZE] = {0};
// Variable for storing length of the string.
static short msgSize;

static struct class* cryClass = NULL;
static struct device* cryDevice = NULL;

// Function prototypes for the character driver.
static int cry_open(struct inode*, struct file*);
static int cry_release(struct inode*, struct file*);
static ssize_t cry_read(struct file*, char*, size_t, loff_t*);
static ssize_t cry_write(struct file*, const char*, size_t, loff_t*);

// Linux file structure operations which the character device will support.
static struct file_operations fops =
{
	.open = cry_open,
	.read = cry_read,
	.write = cry_write,
	.release = cry_release,
};

// TODO Encrypt/decrypt
static void encrypt(void) {
	// Loop through each character.
	int i = 0;
	for(i = 0; i < msgSize; i++) {
		char c = msg[i];
		// If currect character is end of string, return.
		if (c == '\0') {
			return;
		}
		// Rotate current character by specified amount of characters.
		if (isalpha(c)) {
			char alpha = islower(c) ? 'a' : 'A';
			c = (c - alpha + 13) % 26 + alpha;
		}
		msg[i] = c;
	}
}

// Function which will be executed at module initialization time.
static int __init cry_init(void) {
	printk(KERN_INFO "CRYPTOR: Starting Crypto-module as LKM.\n");

	// Try to get the major number dynamically if possible.
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0) {
		printk(KERN_ALERT "CRYPTOR: Could not register a major number!\n");
		return majorNum;
	}
	printk(KERN_INFO "CRYPTOR: Registered with major number %d.\n", majorNum);

	// Register the device class.
	cryClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(cryClass)) {
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "CRYPTOR: Could not register the device class!\n");
		return PTR_ERR(cryDevice);
	}
	printk(KERN_INFO "CRYPTOR: Registered the device class.\n");

	// Register the device driver.
	cryDevice = device_create(cryClass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
	if (IS_ERR(cryDevice)) {
		class_destroy(cryClass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "CRYPTOR: Could not create the device.\n");
		return PTR_ERR(cryDevice);
	}
	printk(KERN_INFO "CRYPTOR: Created the device to /dev/%s.\n", DEVICE_NAME);

	return 0;
}

// Function which will be executed on module cleanup time.
static void __exit cry_exit(void) {
	device_destroy(cryClass, MKDEV(majorNum, 0));
	class_destroy(cryClass);
	unregister_chrdev(majorNum, DEVICE_NAME);
	printk(KERN_INFO "CRYPTOR: CRYPTOR LKM unloaded successfully.\n");
}

// Function which will be executed on device open.
static int cry_open(struct inode* inodep, struct file* filep) {
	printk(KERN_INFO "CRYTOR: User opened the device.\n");
	return 0;
}

// Function which will be used when data is read from the character device.
static ssize_t cry_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
	int errorCount = 0;
	errorCount = copy_to_user(buffer, msg, msgSize);
	if (errorCount == 0) {
		printk(KERN_INFO "CRYPTOR: Sent %d characters to user.\n", msgSize);
		msgSize = 0;
		return(0);
	} else {
		printk(KERN_INFO "CRYPTOR: Could not send %d characters to user!\n", msgSize);
		return -EFAULT;
	}
}

// Function which will be used when data is written to the character device.
// filep is a pointer to a file object.
// buffer is a pointer to the string which is to be written.
// len is length of the string buffer.
static ssize_t cry_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
	// Write characters in buffer to the message.
	sprintf(msg, "%s", buffer);
	msgSize = strlen(msg);
	printk(KERN_INFO "CRYPTOR: Received %d characters to device!\n", msgSize);

	// Lets encrypt/decrypt the message. TODO encryption&decryption check and different methods?
	printk(KERN_INFO "CRYPTOR: Encrypting/decrypting the message.");
	encrypt();

	return msgSize;
}

// Function which will be used when the device is closed by the userspace user.
// inodep is a pointer to an inode object (see linux/fs.h).
// filep is a pointer to a file objec (see linux/fs.h).
static int cry_release(struct inode* inodep, struct file* filep) {
	printk(KERN_INFO "CRYPTOR: Device closed succesfully.\n");
	return 0;
}

// Specify module initialization and cleanup functions.
module_init(cry_init);
module_exit(cry_exit);
