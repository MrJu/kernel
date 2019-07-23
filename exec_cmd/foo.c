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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/kmod.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static int cmd_ls(void)
{
	char *cmd = "/bin/bash";
	char *argv[] = {
		cmd,
		"-c",
		"/bin/cat /proc/version >> /home/pi/Documents/cmd-ls.txt",
		NULL
	};

	char *envp[] = {
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL
	};

	return call_usermodehelper(cmd, argv, envp, UMH_WAIT_PROC);
}

static int cmd_chmod(void)
{
	char *cmd = "/bin/bash";
	char *argv[] = {
		cmd,
		"-c",
		"/bin/chmod 777 /dev/*",
		NULL
	};

	char *envp[] = {
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL
	};

	return call_usermodehelper(cmd, argv, envp, UMH_WAIT_PROC);
}

static int __init foo_init(void)
{
	int ret;

	ret = cmd_ls();
	if (ret < 0)
		printk("cmd_ls() return with %d", ret);

	ret = cmd_chmod();
	if (ret < 0)
		printk("cmd_chmod() return with %d", ret);

	return 0;
}

static void __exit foo_exit(void)
{
	printk("%s(): %d\n", __func__, __LINE__);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
