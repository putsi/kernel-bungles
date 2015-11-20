obj-m+=cryptor.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test

install:
	cp 99-cryptor.rules /etc/udev/rules.d/99-cryptor.rules
	insmod cryptor.ko
uninstall:
	rmmod cryptor
	rm /etc/udev/rules.d/99-cryptor.rules
