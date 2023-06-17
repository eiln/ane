
ifneq ($(KERNELRELEASE),)
	obj-m := ane.o
	ane-objs := ./src/ane_drv.o ./src/ane_tm.o
else
	KERNELDIR := /lib/modules/$(shell uname -r)/build
 	PWD := $(shell pwd)

default:
	make -C $(KERNELDIR) M=$(PWD) modules

install:
	sudo make -C $(KERNELDIR) M=$(PWD) modules_install
	# sudo depmod -a
	sudo modprobe ane
	sudo chmod 666 /dev/accel/accel0

uninstall:
	sudo modprobe -r ane

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
