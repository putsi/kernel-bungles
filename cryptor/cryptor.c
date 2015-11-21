#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("putsi");
MODULE_DESCRIPTION
  ("Character device that encrypts/decrypts given input by XORing with RC4-stream.");
MODULE_VERSION ("1.1");

#define MESSAGE_MAX_SIZE 1024
#define DEVICE_NAME "cry"
#define CLASS_NAME "cryptor"

#define IOCTL_SET_KEY 0
#define IOCTL_GET_KEY 1

// Encryption key.
static char *encryptionKey = NULL;
// Encryption key is char pointer that can be read and write by root.
module_param (encryptionKey, charp, S_IRWXU);
// Encryption key parameter description.
MODULE_PARM_DESC (encryptionKey,
		  "Encryption key that will be used in cryptography operations.");

// Device number will be stored here.
static int majorNum;
// Initialize memory for the message given by user.
static char msg[MESSAGE_MAX_SIZE] = { 0 };

// Variable for storing length of the string.
static short msgSize;

static struct class *cryClass = NULL;
static struct device *cryDevice = NULL;

// Function prototypes for the character driver.
static int cry_open (struct inode *, struct file *);
static int cry_release (struct inode *, struct file *);
static ssize_t cry_read (struct file *, char *, size_t, loff_t *);
static ssize_t cry_write (struct file *, const char *, size_t, loff_t *);
static long cry_ioctl (struct file *file, unsigned int cmd_in,
		       unsigned long arg);

// Function prototype for the rc4 based encryption.
void rc4 (unsigned char *key, unsigned char *msg);

// Linux file structure operations which the character device will support.
static struct file_operations fops = {
  .open = cry_open,
  .read = cry_read,
  .write = cry_write,
  .release = cry_release,
  .unlocked_ioctl = cry_ioctl,
};

// Function which will be executed at module initialization time.
static int __init
cry_init (void)
{
  printk (KERN_INFO "cryptor: Starting Crypto-module as LKM.\n");

  // Try to get the major number dynamically if possible.
  majorNum = register_chrdev (0, DEVICE_NAME, &fops);
  if (majorNum < 0)
    {
      printk (KERN_ALERT "cryptor: Could not register a major number!\n");
      return majorNum;
    }
  printk (KERN_INFO "cryptor: Registered with major number %d.\n", majorNum);

  // Register the device class.
  cryClass = class_create (THIS_MODULE, CLASS_NAME);
  if (IS_ERR (cryClass))
    {
      unregister_chrdev (majorNum, DEVICE_NAME);
      printk (KERN_ALERT "cryptor: Could not register the device class!\n");
      return PTR_ERR (cryDevice);
    }
  printk (KERN_INFO "cryptor: Registered the device class.\n");

  // Register the device driver.
  cryDevice =
    device_create (cryClass, NULL, MKDEV (majorNum, 0), NULL, DEVICE_NAME);
  if (IS_ERR (cryDevice))
    {
      class_destroy (cryClass);
      unregister_chrdev (majorNum, DEVICE_NAME);
      printk (KERN_ALERT "cryptor: Could not create the device.\n");
      return PTR_ERR (cryDevice);
    }
  printk (KERN_INFO "cryptor: Created the device to /dev/%s.\n", DEVICE_NAME);

  return 0;
}

// Function which will be executed on module cleanup time.
static void __exit
cry_exit (void)
{
  device_destroy (cryClass, MKDEV (majorNum, 0));
  class_destroy (cryClass);
  unregister_chrdev (majorNum, DEVICE_NAME);
  printk (KERN_INFO "cryptor: LKM unloaded successfully.\n");
}

// Function which will be executed on device open.
static int
cry_open (struct inode *inodep, struct file *filep)
{
  if (encryptionKey == NULL)
    {
      printk (KERN_NOTICE
	      "cryptor: User tried to use the device when there was no encryption key present.");
      return -EINVAL;
    }
  printk (KERN_INFO "cryptor: User opened the device.\n");
  return 0;
}

// Function which will be used when data is read from the character device.
static ssize_t
cry_read (struct file *filep, char *buffer, size_t len, loff_t * offset)
{
  int errorCount = 0;
  errorCount = copy_to_user (buffer, msg, msgSize);
  if (errorCount == 0)
    {
      printk (KERN_INFO "cryptor: Sent %d characters to user.\n", msgSize);
      msgSize = 0;
      return (0);
    }
  else
    {
      printk (KERN_ALERT "cryptor: Could not send %d characters to user!\n",
	      msgSize);
      return -EIO;
    }
}

// Function which will be used when data is written to the character device.
// filep is a pointer to a file object.
// buffer is a pointer to the string which is to be written.
// len is length of the string buffer.
static ssize_t
cry_write (struct file *filep, const char *buffer, size_t len,
	   loff_t * offset)
{
  // Write characters in buffer to the message.
  sprintf (msg, "%s", buffer);
  msgSize = strlen (msg);
  printk (KERN_INFO "cryptor: Received %d characters to device!\n", msgSize);

  // Lets encrypt/decrypt the message.
  printk (KERN_INFO "cryptor: Encrypting/decrypting the message.");
  rc4 (encryptionKey, msg);

  return msgSize;
}

static long
cry_ioctl (struct file *file, unsigned int ioctl_cmd, unsigned long arg)
{
  int ret_val = 0;
  switch (ioctl_cmd)
    {
    case IOCTL_SET_KEY:
      ret_val =
	copy_from_user (encryptionKey, (char *) arg, sizeof (encryptionKey));
      printk (KERN_INFO "cryptor: User changed encryption key via IOCTL.\n");
      break;
    case IOCTL_GET_KEY:
      ret_val =
	copy_to_user ((char *) arg, encryptionKey, sizeof (encryptionKey));
      printk (KERN_INFO "cryptor: Encryption key sent to user via IOCTL.\n");
      break;
    default:
      printk (KERN_WARNING "cryptor: Received invalid IOCTL call (%d).\n",
	      ioctl_cmd);
      break;
    }
  return ret_val;
}

// Function which will be used when the device is closed by the userspace user.
// inodep is a pointer to an inode object (see linux/fs.h).
// filep is a pointer to a file objec (see linux/fs.h).
static int
cry_release (struct inode *inodep, struct file *filep)
{
  printk (KERN_INFO "cryptor: Device closed succesfully.\n");
  return 0;
}

// Specify module initialization and cleanup functions.
module_init (cry_init);
module_exit (cry_exit);

/*
    Following public domain RC4-implementation is from
    https://github.com/B-Con/crypto-algorithms
*/

void
rc4_key_setup (unsigned char state[], const unsigned char key[], int len)
{
  int i;
  int j;
  for (i = 0; i < 256; ++i)
    state[i] = i;
  for (i = 0, j = 0; i < 256; ++i)
    {
      unsigned char t = state[i];
      j = (j + state[i] + key[i % len]) % 256;
      state[i] = state[j];
      state[j] = t;
    }
}

void
rc4_generate_stream (unsigned char state[], unsigned char out[], size_t len)
{
  int i;
  int j;
  size_t idx;
  for (idx = 0, i = 0, j = 0; idx < len; ++idx)
    {
      unsigned char t = state[i];
      i = (i + 1) % 256;
      j = (j + state[i]) % 256;
      state[i] = state[j];
      state[j] = t;
      out[idx] = state[(state[i] + state[j]) % 256];
    }
}

void
rc4 (unsigned char *key, unsigned char *msg)
{
  unsigned char state[256];
  unsigned char buf[MESSAGE_MAX_SIZE];
  size_t i;

  rc4_key_setup (state, key, strlen (key));
  rc4_generate_stream (state, buf, MESSAGE_MAX_SIZE);
  for (i = 0; i < strlen (msg); i++)
    {
      msg[i] ^= buf[i];
    }
}
