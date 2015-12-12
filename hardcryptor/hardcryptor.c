/* Kernel headers, needed for e.g. KERN_ALERT. */
#include <linux/kernel.h>
/* Module headers, needed for various Kernel module-specific functions and globals. */
#include <linux/module.h>
/* Device headers, needed for managing a device. */
#include <linux/device.h>
/* File structure headers, needed for defining and creating a character device. */
#include <linux/fs.h>
/* Mutex-headers, needed for removing possibility for a race condition. */
#include <linux/mutex.h>
/* Uaccess-headers, needed for copying data between user space and Kernel space. */
#include <asm/uaccess.h>
/* Include ctype headers, so that we can validate the user input. */
#include <linux/ctype.h>

/* Set the licence, author, version, and description of the module. */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jarmo Puttonen");
MODULE_DESCRIPTION
    ("Character device that encrypts/decrypts given input by XORing with RC4-stream.");
MODULE_VERSION("1.2");

/* Maximum size of message in a single write- or read-operation. */
#define MESSAGE_MAX_SIZE 1024
/* Minimum length for the encryption key. */
#define KEY_MIN_SIZE 16
/* Maximum length for the encryption key. */
#define KEY_MAX_SIZE 1024
/* Device name which will be used in the file system (/dev/hcry). */
#define DEVICE_NAME "hcry"
/* Class name defines which class the module is specific to. */
#define CLASS_NAME "hardcryptor"

/* IOCTL-call values used for setting and getting the encryption key. */
#define IOCTL_SET_KEY 0
#define IOCTL_GET_KEY 1

/* Encryption-key will be stored here. */
static char encryptionKey[KEY_MAX_SIZE] = { 0 };

/* Device major number maps the device file to the corresponding driver. */
static int majorNum;
/* Initialize memory for the message. */
static char msg[MESSAGE_MAX_SIZE] = { 0 };

/* Variable for storing length of the string. */
static short msgSize;

/* The basic device class. */
static struct class *cryClass;
/* The basic device structure. */
static struct device *cryDevice;

/* Open is called when the user tries to open the character device file. */
static int cry_open(struct inode *, struct file *);
/* Release is called when a process closes the character device file. */
static int cry_release(struct inode *, struct file *);
/* Read is called when a process that has opened the character device file tries to read from it. */
static ssize_t cry_read(struct file *, char *, size_t, loff_t *);
/* Write is called when a process that has opened the character device file tries to write to it. */
static ssize_t cry_write(struct file *, const char *, size_t, loff_t *);
/* Ioctl is called when a process tries to do an ioctl call to the character device file. */
static long cry_ioctl(struct file *file, unsigned int cmd_in,
		      unsigned long arg);

/* Linux file structure operations which the character device will support. */
static struct file_operations fops = {
	.open = cry_open,
	.read = cry_read,
	.write = cry_write,
	.release = cry_release,
	.unlocked_ioctl = cry_ioctl,
};

/* Declare a mutex for making sure that multiple users don't use the device at same time. */
static DEFINE_MUTEX(cry_device_lock);
/* Declare a mutex for making sure that that writing and reading doesn't occur at same time. */
static DEFINE_MUTEX(cry_readwrite_lock);

/* Function prototype for the rc4 based encryption. */
void rc4(unsigned char *key, unsigned char *msg);

/* Function prototype for function that safely clears any buffers. */
void clear_buffer(unsigned char *buf, int bufsize);

/* This function will be executed at module initialization time. */
static int __init cry_init(void)
{
	printk(KERN_INFO "hardcryptor: Starting Crypto-module as LKM.\n");

	/* Register a character device and try to get a major number dynamically if possible. */
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0) {
		printk(KERN_ALERT
		       "hardcryptor: Could not register a major number!\n");
		return PTR_ERR(&majorNum);
	}
	printk(KERN_INFO "hardcryptor: Registered with major number %d.\n",
	       majorNum);

	/* Create a struct class structure which will be used in creating the device. */
	cryClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(cryClass)) {
		/* Unregister the character device as we could not create the device class. */
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT
		       "hardcryptor: Could not register the device class!\n");
		return PTR_ERR(cryClass);
	}
	printk(KERN_INFO "hardcryptor: Registered the device class.\n");

	/* Create a device and register it with sysfs. */
	cryDevice =
	    device_create(cryClass, NULL, MKDEV(majorNum, 0), NULL,
			  DEVICE_NAME);
	if (IS_ERR(cryDevice)) {
		/* Destroy the class and unregister the character device as we could not create the device driver. */
		class_destroy(cryClass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "hardcryptor: Could not create the device.\n");
		return PTR_ERR(cryDevice);
	}
	printk(KERN_INFO "hardcryptor: Created the device to /dev/%s.\n",
	       DEVICE_NAME);

	return 0;
}

/* This function which will be executed on the module cleanup time. */
static void __exit cry_exit(void)
{
	/* Destroy the device, destroy the class and unregister the character device. */
	device_destroy(cryClass, MKDEV(majorNum, 0));
	class_destroy(cryClass);
	unregister_chrdev(majorNum, DEVICE_NAME);
	printk(KERN_INFO "hardcryptor: LKM unloaded successfully.\n");
}

/* This is called when the user tries to open the character device file. */
static int cry_open(struct inode *inodep, struct file *filep)
{
	mutex_lock(&cry_device_lock);
	printk(KERN_INFO "hardcryptor: User opened the device.\n");
	return 0;
}

/* This is called when a process that has opened the character device file tries to read from it. */
static ssize_t
cry_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int errorCount = 0;

	/* Copy the saved message from the global variable to user space. */
	/* If there were any errors, return an I/O Error. */
	errorCount = copy_to_user(buffer, msg, msgSize);

	/* Avoid possible information leaks by clearing the buffer. */
	clear_buffer(msg, MESSAGE_MAX_SIZE);

	if (errorCount == 0) {
		printk(KERN_INFO "hardcryptor: Sent %d characters to user.\n",
		       msgSize);
		msgSize = 0;
		return (0);
	} else {
		printk(KERN_ALERT
		       "hardcryptor: Could not send %d characters to user!\n",
		       msgSize);
		return -EIO;
	}
}

/* This is called when a process that has opened the character device file tries to write to it. */
static ssize_t
cry_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	/* If there is no encryption key, return an invalid argument error. */
	if (strlen(encryptionKey) == 0) {
		printk(KERN_NOTICE
		       "hardcryptor: User tried to write in the device when there was no encryption key present.\n");
		return -EINVAL;
	}

	/* Write characters in input buffer to the message. */
	snprintf(msg, MESSAGE_MAX_SIZE+1, "%s", buffer);
	msgSize = strlen(msg);
	printk(KERN_INFO "hardcryptor: Received %d characters to device!\n",
	       msgSize);

	/* Lets encrypt/decrypt the message. */
	printk(KERN_INFO "hardcryptor: Encrypting/decrypting the message.\n");
	rc4(encryptionKey, msg);

	/* Return the amount of characters that were encrypted/decrypted. */
	return msgSize;
}

/* This is called when a process tries to do an ioctl call to the character device file. */
static long
cry_ioctl(struct file *file, unsigned int ioctl_cmd, unsigned long arg)
{
	int ret_val = 0;
	int i = 0;
        char buf[KEY_MAX_SIZE];

	/* Find out if the user wants to set or get the encryption key. */
	switch (ioctl_cmd) {
	case IOCTL_SET_KEY:
		/* Make sure that the encryption key is properly sized and formatted. */
		ret_val =
		    copy_from_user(buf, (char *)arg, KEY_MAX_SIZE);
		if (ret_val > 0) {
			printk(KERN_INFO "hardcryptor: Could not parse encryption key sent by the user.\n");
			ret_val = -EINVAL;
			break;
		}
		if (strlen(buf) < KEY_MIN_SIZE) {
			printk(KERN_ALERT "hardcryptor: User tried to enter too short encryption key.\n");
                        ret_val = -EINVAL;
			break;
		}
		if (strlen(buf) > KEY_MAX_SIZE) {
			printk(KERN_ALERT "hardcryptor: User tried to enter too long encryption key.\n");
                        ret_val = -EINVAL;
			break;
		}

		/* Make sure that user wrote only acceptable characters to the device. */
		/* For example, any control characters are not allowed. */
		for (i = 0; i < strlen(buf); i++) {
			if (isalnum(buf[i]) || isspace(buf[i]) || ispunct(buf[i])) {
				continue;
			}
			printk(KERN_INFO "hardcryptor: User tried to set invalid encryption key to the device.\n");
			return -EPERM;
		}

		/* Avoid possible information leaks by clearing the message buffer. */
		clear_buffer(msg, MESSAGE_MAX_SIZE);

		/* Finally, replace the old encryption key with the new one. */
		strncpy(encryptionKey, buf, KEY_MAX_SIZE);

		printk(KERN_INFO
		       "hardcryptor: User changed encryption key via IOCTL.\n");
		break;
	case IOCTL_GET_KEY:
		if (strlen(encryptionKey) == 0) {
			printk(KERN_INFO "hardcryptor: User tried to get encryption key when none was set.\n");
			ret_val = -EINVAL;
			break;
		}
		/* Copy data from the encryption key variable (Kernel space) to user space. */
		ret_val =
		    copy_to_user((char *)arg, encryptionKey,
				 sizeof(encryptionKey));
		printk(KERN_INFO
		       "hardcryptor: Encryption key sent to user via IOCTL.\n");
		break;
	default:
		/* If invalid ioctl call is given, log the operation and return. */
		ret_val = -EPERM;
		printk(KERN_WARNING
		       "hardcryptor: Received invalid IOCTL call (%d).\n",
		       ioctl_cmd);
		break;
	}
	return ret_val;
}

/* This is called when a process closes the character device file. */
static int cry_release(struct inode *inodep, struct file *filep)
{
	/* Avoid possible information leaks by clearing the buffer. */
        clear_buffer(msg, MESSAGE_MAX_SIZE);
        clear_buffer(encryptionKey, KEY_MAX_SIZE);

	mutex_unlock(&cry_device_lock);
	printk(KERN_INFO "hardcryptor: Device closed succesfully.\n");
	return 0;
}

/* Specify functions which will be called when the module is loaded and unloaded.. */
module_init(cry_init);
module_exit(cry_exit);

void clear_buffer(unsigned char *buf, int bufsize) {
	int i;
	for(i = 0; i < bufsize; i++) {
		buf[i] = 0;
	}
}

/*
    Following public domain RC4-implementation is from
    https://github.com/B-Con/crypto-algorithms
*/

void rc4_key_setup(unsigned char state[], const unsigned char key[], int len)
{
	int i;
	int j;

	for (i = 0; i < 256; ++i)
		state[i] = i;
	for (i = 0, j = 0; i < 256; ++i) {
		unsigned char t = state[i];

		j = (j + state[i] + key[i % len]) % 256;
		state[i] = state[j];
		state[j] = t;
	}
}

void rc4_generate_stream(unsigned char state[], unsigned char out[], size_t len)
{
	int i;
	int j;
	size_t idx;

	for (idx = 0, i = 0, j = 0; idx < len; ++idx) {
		unsigned char t = state[i];

		i = (i + 1) % 256;
		j = (j + state[i]) % 256;
		state[i] = state[j];
		state[j] = t;
		out[idx] = state[(state[i] + state[j]) % 256];
	}
}

void rc4(unsigned char *key, unsigned char *msg)
{
	unsigned char state[256];
	unsigned char buf[MESSAGE_MAX_SIZE];
	size_t i;

	rc4_key_setup(state, key, strlen(key));
	rc4_generate_stream(state, buf, MESSAGE_MAX_SIZE);
	for (i = 0; i < strlen(msg); i++) {
		msg[i] ^= buf[i];
	}
}
