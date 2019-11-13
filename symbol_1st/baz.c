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

#define VERSION_PREFIX Baz
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static int __init baz_init(void)
{
	int i;
	int (*func)(void);
	char *name[] = {
		"T_symbol_func0",
		"t_symbol_func0",
		"s_symbol_func0",
	};

	for (i = 0; i < 3; i++) {
		func = kallsyms_lookup_name(name[i]);
		if (!func) {
			printk("%s: symbol %s not found\n", __func__, name[0]);
			continue;
		}

		func();

		printk("%s: %s 0x%lx\n", __func__, name[i], func);
	}

	printk("%s: %s module loaded.\n",
			__func__, __this_module.name);

	return 0;
}

static void __exit baz_exit(void)
{
	printk("%s: %s module unloaded.\n",
			__func__, __this_module.name);
}

module_init(baz_init);
module_exit(baz_exit);

MODULE_ALIAS("baz-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
