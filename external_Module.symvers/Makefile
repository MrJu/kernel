# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the kernel modules.
#

# to use external Module.symvers
# make sure the vmlinux has been removed from the out directory

KERN_DIR := /lib/modules/$(shell uname -r)/build
KBUILD_EXTRA_SYMBOLS = absolute/path/to/Module.symvers

obj-m := foo.o

all:
	$(MAKE) -C $(KERN_DIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KERN_DIR) M=`pwd` modules clean
