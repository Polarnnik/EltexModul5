obj-m +=proc.o

all:
	make ARCH=x86_64 -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make ARCH=x86_64 -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
