# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the HID driver
#

KERN_DIR := /lib/modules/$(shell uname -r)/build

bar-objs	:= foo.o kernel_io.o
obj-m		+= bar.o

all:
	$(MAKE) -C $(KERN_DIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KERN_DIR) M=`pwd` modules clean
