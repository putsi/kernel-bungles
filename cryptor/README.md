# Loadable kernel module that provides RC4-cryptography

Note: The implementation is very unsafe and contains multiple vulnerabilities.  A hardened version will be provided by the [hardcryptor-module](https://github.com/putsi/kernel-bungles/tree/master/hardcryptor).

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

## Example
### Standard output
```
$ insmod cryptor
$ ./test

String that will be encrypted&decrypted: salainen lause

73 61 6C 61 69 6E 65 6E 20 6C 61 75 73 65  <-- Original  message (HEX)
CE 7B DB E4 66 AC 5D 9E 25 97 C2 DD 24 92  <-- Encrypted message (HEX)
73 61 6C 61 69 6E 65 6E 20 6C 61 75 73 65  <-- Decrypted message (HEX)
salainen lause <-- Original  message
�{��f�]�%���$� <-- Encrypted message
salainen lause <-- Decrypted message

Current encryption key: alkukey
Setting new encryption key via IOCTL.
Current encryption key: uusiavai

73 61 6C 61 69 6E 65 6E 20 6C 61 75 73 65  <-- Original  message (HEX)
A1 A1 5B 28 0D 66 A2 16 1C 7B 03 07 56 2A  <-- Encrypted message (HEX)
73 61 6C 61 69 6E 65 6E 20 6C 61 75 73 65  <-- Decrypted message (HEX)
salainen lause <-- Original  message
f�V* <-- Encrypted message
salainen lause <-- Decrypted message

$ rmmod cryptor
```
### Kernel log
```
Dec  2 12:05:18 kali kernel: [42327.506632] cryptor: Starting Crypto-module as LKM.
Dec  2 12:05:18 kali kernel: [42327.506635] cryptor: Registered with major number 250.
Dec  2 12:05:18 kali kernel: [42327.506686] cryptor: Registered the device class.
Dec  2 12:05:18 kali kernel: [42327.506737] cryptor: Created the device to /dev/cry.
Dec  2 12:05:26 kali kernel: [42335.133778] cryptor: User opened the device.
Dec  2 12:05:30 kali kernel: [42338.805126] cryptor: Received 14 characters to device!
Dec  2 12:05:30 kali kernel: [42338.805127] cryptor: Encrypting/decrypting the message.
Dec  2 12:05:30 kali kernel: [42338.805133] cryptor: Sent 14 characters to user.
Dec  2 12:05:30 kali kernel: [42338.805133] cryptor: Received 14 characters to device!
Dec  2 12:05:30 kali kernel: [42338.805134] cryptor: Encrypting/decrypting the message.
Dec  2 12:05:30 kali kernel: [42338.805138] cryptor: Sent 14 characters to user.
Dec  2 12:05:30 kali kernel: [42338.805152] cryptor: Encryption key sent to user via IOCTL.
Dec  2 12:05:30 kali kernel: [42338.805154] cryptor: User changed encryption key via IOCTL.
Dec  2 12:05:30 kali kernel: [42338.805155] cryptor: Encryption key sent to user via IOCTL.
Dec  2 12:05:30 kali kernel: [42338.805156] cryptor: Received 14 characters to device!
Dec  2 12:05:30 kali kernel: [42338.805156] cryptor: Encrypting/decrypting the message.
Dec  2 12:05:30 kali kernel: [42338.805160] cryptor: Sent 14 characters to user.
Dec  2 12:05:30 kali kernel: [42338.805161] cryptor: Received 14 characters to device!
Dec  2 12:05:30 kali kernel: [42338.805161] cryptor: Encrypting/decrypting the message.
Dec  2 12:05:30 kali kernel: [42338.805165] cryptor: Sent 14 characters to user.
Dec  2 12:05:30 kali kernel: [42338.805173] cryptor: Received invalid IOCTL call (6).
Dec  2 12:05:30 kali kernel: [42338.805176] cryptor: Device closed succesfully.
Dec  2 12:10:01 kali kernel: [42610.234653] cryptor: LKM unloaded successfully.
```
