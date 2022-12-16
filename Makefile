obj-m += newmod.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc user_prog.c -o user_prog
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user_prog
test:
	sudo dmesg -C
	sudo insmod newmod.ko
	dmesg
close:
	sudo rmmod newmod.ko
	dmesg