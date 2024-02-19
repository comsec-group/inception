CC=gcc
KERNEL=/lib/modules/$(shell uname -r)/build

mod-name := inception_test_kmod
obj-m += $(mod-name).o
$(mod-name)-objs = main.o gadgets.o
ccflags-y := \
        -O1 \
        -DDEBUG \
        -std=gnu99 \
        -Werror \
	-Wno-address-of-packed-member \
	-Wno-frame-address \
	-Wno-unused-function \
	-I ${KERNEL}/arch/x86/kvm \
	$(CCFLAGS)
PWD=$(shell pwd)

all:
	make LLVM=1 -C ${KERNEL} M=$(PWD) modules

clean:
	make LLVM=1 -C ${KERNEL} M=$(PWD) clean

.PHONY:
install: all
	sudo insmod $(mod-name).ko

.PHONY:
remove:
	sudo rmmod $(mod-name).ko
