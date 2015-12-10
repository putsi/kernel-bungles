obj-m+=hardcryptor.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test

install:
	cp 99-hardcryptor.rules /etc/udev/rules.d/99-hardcryptor.rules; \
	insmod hardcryptor.ko; \
        depmod -a; \
        udevadm control --reload-rules; \
        service udev restart; \
        udevadm trigger; \

uninstall:
	rmmod hardcryptor
	rm /etc/udev/rules.d/99-hardcryptor.rules
