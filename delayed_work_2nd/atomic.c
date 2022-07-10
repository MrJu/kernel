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
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define INTERVAL_IN_MSECS 3000

static struct delayed_work atomic_delayed_work;
static int count = 0;

static void atomic_delayed_func(struct work_struct *work)
{
	if (count++ > 4)
		return;

	pr_info("%s: %d count: %d before\n",
			__func__, __LINE__, count);

	mod_delayed_work(system_wq, &atomic_delayed_work, 0);
	msleep(INTERVAL_IN_MSECS);

	pr_info("%s: %d count: %d after\n",
			__func__, __LINE__, count);
}

static int __init atomic_init(void)
{
	INIT_DELAYED_WORK(&atomic_delayed_work, atomic_delayed_func);
	mod_delayed_work(system_wq, &atomic_delayed_work, 0);
	msleep(INTERVAL_IN_MSECS);

	pr_info("%s: %d\n", __func__, __LINE__);

	return 0;
}

static void __exit atomic_exit(void)
{
	cancel_delayed_work_sync(&atomic_delayed_work);
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_ALIAS("atomic-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
