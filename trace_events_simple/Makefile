# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the kernel modules.
#

NAME := atomic
KERN_DIR := ~/Documents/virt/linux
TARGET := ~/Documents/virt/rootfs/busybox/home/andrew
CFLAGS_atomic.o := -I$(src)

obj-m += $(NAME).o

all:
	$(MAKE) -C $(KERN_DIR) ARCH=arm64 CC=clang-12 CROSS_COMPILE=aarch64-linux-gnu- M=`pwd` modules

clean:
	$(MAKE) -C $(KERN_DIR) ARCH=arm64 CC=clang-12 CROSS_COMPILE=aarch64-linux-gnu- M=`pwd` modules clean

install:
	cp $(NAME).ko $(TARGET)
