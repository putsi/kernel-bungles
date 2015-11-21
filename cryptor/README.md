# Loadable kernel module that provides RC4-cryptography

Note: The implementation is very unsafe and contains multiple vulnerabilities.  A hardened version will be provided by the hardcryptor-module.

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
