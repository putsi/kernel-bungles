# Loadable kernel module that provides [TODO]-cryptography

Note: This is a hardened version of the [cryptor-module](https://github.com/putsi/kernel-bungles/tree/master/cryptor).

Following security fixes have been implemented in this version:
 * UDEV rules modified so that only members of group "crypto" can use the device.
 * The encryption key module parameter was removed as it is shown as cleartext when used.
 * Improper handling of encryption key was fixed and error handling was added for cases where the key has not been set.
 * To avoid information leaks, message buffer is filled with zeroes after it is read or when the device is released.
 * To avoid information leaks, encryption key buffer is filled with zeroes after the device is released.
 * Added mutex so that no race conditions will happen when multiple users try to use the device at the same time.
 * Replaced sprintf with snprintf so that no overflow will occur when user writes too long message.
 * Only alphanumeric, whitespace and punctuation characters are allowed in the encryption key.
 * Added null check and minimum length check for the ioctl-call which sets encryption key. 

## Installing
```
sudo -s
cd /tmp
git clone https://github.com/putsi/kernel-bungles.git
cd kernel-bungles/cryptor
make && make install
```

## Using
Initial encryption key is set by the Makefile.
Module creates a character device to /dev/cry, which encrypts or decrypts any data written into it.
Encryption key can be changed with IOCTL-call 0 and retrieved with IOCTL-call 1.

Usage example is provided by test-program which can be used with:
```
chmod +x test
./test
```

## Uninstalling
```
make uninstall
```

## Logs
Logs can be examined for example with:
```
tail -n25 /var/log/kern.log
```
