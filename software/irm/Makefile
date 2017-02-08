
ifeq ($(KERNELRELEASE),)


KERNELDIR ?= /C5S/linux_socfpga/github/SoCFPGA/linux-socfpga
PWD := $(shell pwd)
ARM ?=arm
CROSSS_COMPILE ?=arm-linux-gnueabihf-

modules:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *odule*

.PHONY: modules modules_install clean

else
    obj-m := terasic_ir.o
endif

