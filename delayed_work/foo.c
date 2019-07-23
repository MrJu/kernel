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

#define INTERVAL_IN_MSEC 500

unsigned long last_jiffies;
unsigned long interval;

static struct delayed_work foo_delayed_work;

static void foo_delayed_func(struct work_struct *work)
{
	interval = jiffies - last_jiffies;
	last_jiffies = jiffies;

	printk("%s, %d, %ld, %d\n", __func__, __LINE__,
			jiffies, jiffies_to_msecs(interval));

	mod_delayed_work(system_wq, &foo_delayed_work,
			round_jiffies(msecs_to_jiffies(INTERVAL_IN_MSEC)));
}

static int __init foo_init(void)
{
	INIT_DELAYED_WORK(&foo_delayed_work, foo_delayed_func);
	schedule_delayed_work(&foo_delayed_work,
			round_jiffies(msecs_to_jiffies(INTERVAL_IN_MSEC)));
	last_jiffies = jiffies;

	return 0;
}

static void __exit foo_exit(void)
{
	cancel_delayed_work_sync(&foo_delayed_work);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
