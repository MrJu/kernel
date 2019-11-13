/*
 * Copyright (C) 2019 Andrew <mrju.email@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/string.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Bar
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static int __init bar_init(void)
{
	char *name = "printk";
	unsigned long addr;

	addr = kallsyms_lookup_name(name);
	if (!addr) {
		printk("%s: symbol %s not found\n", __func__, name);
		return -EAGAIN;
	}

	printk("%s: %s 0x%lx\n", __func__, name, addr);

	return 0;
}

static void __exit bar_exit(void)
{
}

module_init(bar_init);
module_exit(bar_exit);

MODULE_ALIAS("bar-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
