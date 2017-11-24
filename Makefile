#PLATFORM		=s2l
#BOARD_VERSION	=s2lm_kiwi

#TOOLCHAIN_PATH	=/usr/local/linaro-multilib-2014.06-gcc4.9/bin/

#ARCH			= arm
#CROSS_COMPILE	= $(TOOLCHAIN_PATH)/arm-linux-gnueabihf-


#KERNEL_PATH=/home/lqy/S2L_IPCAM/SDK1.5/Package/s2l_linux_sdk/ambarella/out/s2lm_kiwi/kernel/linux_kiwi

PWD:=$(shell pwd)


#######################################
#motor modules
#######################################

obj-m+= motor.o
motor-objs := motor_dev.o motor_drv.o



drv_mod:

	make -C $(KERNEL_PATH)  M=$(PWD)	modules 	ARCH=arm  CROSS_COMPILE=$(CROSS_TOOL)
#	make -C $(PWD)/../../../../../../../sdk/arm-linux-3.3/linux-3.3-fa  M=$(PWD)	modules 	ARCH=arm  CROSS_COMPILE=/opt/grain-media/toolchain_gnueabi-4.4.0_ARMv5TE/usr/bin/arm-unknown-linux-uclibcgnueabi-
clean:

#	make -C $(KERNEL_BUILD_PATH)  M=$(PWD)	clean    	ARCH=arm  CROSS_COMPILE=$(CROSS_TOOL)
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order

tftp_tx:

	cp  motor.ko   $(TFTP_DIR)


release:
	chmod 755 motor.ko
	cp motor.ko	$(RELEASE_MODULE_DIR)
