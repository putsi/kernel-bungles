obj-m+=rot.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test

install:
	cp 99-rot.rules /etc/udev/rules.d/99-rot.rules
	insmod rot.ko
uninstall:
	rmmod rot
	rm /etc/udev/rules.d/99-rot.rules
