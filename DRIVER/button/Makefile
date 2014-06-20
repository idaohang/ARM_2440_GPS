ifneq ($(KERNELRELEASE),)
obj-m:=my_button_test.o
else
#KERNELDIR?=/lib/modules/$(shell uname -r)/build
KERNELDIR ?= /home/whq/mini2440/linux-2.6.32.2
PWD:=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *.symvers
endif
