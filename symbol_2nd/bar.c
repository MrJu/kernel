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

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Bar
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static int s_symbol_variable;

int b_symbol_variable;

int B_symbol_variable1;
EXPORT_SYMBOL(B_symbol_variable1);

static int s_symbol_func0(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}

static int s_symbol_func1(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}

int t_symbol_func0(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}

int t_symbol_func1(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}

int T_symbol_func2(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}
EXPORT_SYMBOL(T_symbol_func2);

int T_symbol_func3(void)
{
	printk("%s: %d\n", __func__, __LINE__);
	return 0;
}
EXPORT_SYMBOL(T_symbol_func3);

static int __init bar_init(void)
{
	printk("%s: %d T_symbol_func2 0x%lx\n",
			__func__, __LINE__, (unsigned long)T_symbol_func2);
	printk("%s: %d T_symbol_func3 0x%lx\n",
			__func__, __LINE__, (unsigned long)T_symbol_func3);
	printk("%s: %d t_symbol_func0 0x%lx\n",
			__func__, __LINE__, (unsigned long)t_symbol_func0);
	printk("%s: %d t_symbol_func1 0x%lx\n",
			__func__, __LINE__, (unsigned long)t_symbol_func1);
	printk("%s: %d s_symbol_func0 0x%lx\n",
			__func__, __LINE__, (unsigned long)s_symbol_func0);
	printk("%s: %d s_symbol_func1 0x%lx\n",
			__func__, __LINE__, (unsigned long)s_symbol_func1);
	printk("%s: %d B_symbol_variable1 0x%lx\n",
			__func__, __LINE__, (unsigned long)&B_symbol_variable1);
	printk("%s: %d b_symbol_variable 0x%lx\n",
			__func__, __LINE__, (unsigned long)&b_symbol_variable);
	printk("%s: %d s_symbol_variable 0x%lx\n",
			__func__, __LINE__, (unsigned long)&s_symbol_variable);

	return 0;
}

static void __exit bar_exit(void)
{
	return;
}

module_init(bar_init);
module_exit(bar_exit);

MODULE_ALIAS("bar-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
