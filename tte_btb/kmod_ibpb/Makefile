CC=gcc
mod-name := ibpb
obj-m += $(mod-name).o
ccflags-y := \
	-O1 \
	-DDEBUG \
	-std=gnu99 \
	-Werror \
	$(CCFLAGS)
PWD := $(shell pwd)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY:
install: all
	cp $(mod-name).ko /tmp/ && sudo insmod /tmp/$(mod-name).ko

.PHONY:
remove:
	sudo rmmod $(mod-name)
