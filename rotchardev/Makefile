obj-m+=rot.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test.c -o testrot
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm testrot
