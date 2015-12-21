# Loadable kernel module that provides RC4-cryptography

#### Note: This is a hardened version of the [cryptor-module](https://github.com/putsi/kernel-bungles/tree/master/cryptor).

##### Following security fixes have been implemented in this version:
 * UDEV rules modified so that only members of group "crypto" can use the device.
 * The encryption key module parameter was removed as it is shown as cleartext when used.
 * Improper handling of encryption key was fixed and error handling was added for cases where the key has not been set.
 * To avoid information leaks, message buffer is filled with zeroes after it is read or when the device is released.
 * To avoid information leaks, encryption key buffer is filled with zeroes after the device is released.
 * Added mutex so that no race conditions will happen when multiple users try to use the device at the same time.
 * Added mutex so that no race conditions will happen when user tries to read, write and/or set/get encryption key at the same time.
 * Replaced sprintf with snprintf so that no overflow will occur when user writes too long message.
 * Only alphanumeric, whitespace and punctuation characters are allowed in the encryption key.
 * Added null check and minimum length check for the ioctl-call which sets encryption key. 
 * Will now use the length-parameter on read/write-operations if given and if smaller than max size.
 * Variables will be initialized to null when defined.
 * Added some operation mode restrictions to IOCTL macro definitions.
 * Added null check to logic which uses IOCTL parameter.
 * Refactored printk error levels so that less non-debug log will be printed.

#### [Source code modifications related to the fixes can be seen here](https://github.com/putsi/kernel-bungles/compare/29421c2fb418d9761c200f15f0e9f94d8902d85f...master).

## Installing
```
sudo -s
cd /tmp
git clone https://github.com/putsi/kernel-bungles.git
cd kernel-bungles/cryptor
make && make install
```

## Using
Module creates a character device to /dev/cry, which encrypts or decrypts any data written into it.
Encryption key can be changed with IOCTL-call 0 and retrieved with IOCTL-call 1.

Usage example is provided by test-program which can be used with (must be run with root-user or with user that belongs to crypto-group):
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

## Example
### Console input
```
putsi@kali:~/kernel-bungles/hardcryptor$ sudo make install
putsi@kali:~/kernel-bungles/hardcryptor$ groups
putsi sudo crypto
putsi@kali:~/kernel-bungles/hardcryptor$ ./test
Current encryption key: thisIsOldKeyAndNowLongEnough


String that will be encrypted&decrypted: moro testiviesti

6D 6F 72 6F 20 74 65 73 74 69 76 69 65 73 74 69  <-- Original  message (HEX)
6F 2D 09 14 8D E6 05 63 E7 41 05 D7 70 DA 5D 0A  <-- Encrypted message (HEX)
6D 6F 00 00 00 00 00 00 00 00 00 00 00 00 00 00  <-- Decrypted message (HEX)
moro testiviesti <-- Original  message
o-	��c�A�p�]
 <-- Encrypted message
mo <-- Decrypted message

Current encryption key: thisIsOldKeyAndNowLongEnough
Setting new encryption key via IOCTL.
Current encryption key: newKeyHereAndThisIsAlsoLongEnough

6D 6F 72 6F 20 74 65 73 74 69 76 69 65 73 74 69  <-- Original  message (HEX)
87 A2 B7 4B 60 7C 2C 27 F5 9C 1B FF C3 35 68 88  <-- Encrypted message (HEX)
6D 6F 72 6F 20 74 65 73 74 69 76 69 65 73 74 69  <-- Decrypted message (HEX)
moro testiviesti <-- Original  message
���K`|,'��5h� <-- Encrypted message
moro testiviesti <-- Decrypted message

putsi@kali:~/kernel-bungles/hardcryptor$ sudo make uninstall
```
### Kernel log
```
Dec 21 13:14:32 kali kernel: [ 7249.464389] hardcryptor: Starting Crypto-module as LKM.
Dec 21 13:14:32 kali kernel: [ 7249.464392] hardcryptor: Registered with major number 250.
Dec 21 13:14:32 kali kernel: [ 7249.464399] hardcryptor: Registered the device class.
Dec 21 13:14:32 kali kernel: [ 7249.465679] hardcryptor: Created the device to /dev/hcry.
Dec 21 13:15:36 kali kernel: [ 7313.531638] hardcryptor: User opened the device.
Dec 21 13:15:36 kali kernel: [ 7313.531648] hardcryptor: User changed encryption key via IOCTL.
Dec 21 13:15:36 kali kernel: [ 7313.531649] hardcryptor: Encryption key sent to user via IOCTL.
Dec 21 13:15:41 kali kernel: [ 7318.289631] hardcryptor: Received 16 characters to device!
Dec 21 13:15:41 kali kernel: [ 7318.289633] hardcryptor: Encrypting/decrypting the message.
Dec 21 13:15:41 kali kernel: [ 7318.289639] hardcryptor: Sent 16 characters to user.
Dec 21 13:15:41 kali kernel: [ 7318.289639] hardcryptor: Received 2 characters to device!
Dec 21 13:15:41 kali kernel: [ 7318.289640] hardcryptor: Encrypting/decrypting the message.
Dec 21 13:15:41 kali kernel: [ 7318.289644] hardcryptor: Sent 2 characters to user.
Dec 21 13:15:41 kali kernel: [ 7318.289666] hardcryptor: Encryption key sent to user via IOCTL.
Dec 21 13:15:41 kali kernel: [ 7318.289670] hardcryptor: User changed encryption key via IOCTL.
Dec 21 13:15:41 kali kernel: [ 7318.289671] hardcryptor: Encryption key sent to user via IOCTL.
Dec 21 13:15:41 kali kernel: [ 7318.289672] hardcryptor: Received 16 characters to device!
Dec 21 13:15:41 kali kernel: [ 7318.289673] hardcryptor: Encrypting/decrypting the message.
Dec 21 13:15:41 kali kernel: [ 7318.289677] hardcryptor: Sent 16 characters to user.
Dec 21 13:15:41 kali kernel: [ 7318.289678] hardcryptor: Received 16 characters to device!
Dec 21 13:15:41 kali kernel: [ 7318.289678] hardcryptor: Encrypting/decrypting the message.
Dec 21 13:15:41 kali kernel: [ 7318.289682] hardcryptor: Sent 16 characters to user.
Dec 21 13:15:41 kali kernel: [ 7318.289692] hardcryptor: Received invalid IOCTL call (6).
Dec 21 13:15:41 kali kernel: [ 7318.289695] hardcryptor: Device closed succesfully.
Dec 21 13:15:55 kali kernel: [ 7331.984939] hardcryptor: LKM unloaded successfully.

```
