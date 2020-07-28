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
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched/clock.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define BUF_SIZE 4096
#define MSG_MAX_SIZE 256

static int __init foo_init(void)
{
	int i, wr = 0;
	char *buf = NULL;
	size_t size;
	unsigned long long ts;
	unsigned long rem_nsec;

	buf = (char *) vmalloc(BUF_SIZE);
	if (!buf)
		return -ENOMEM;

	for (i = 0; i < 16; i++) {
		ts = sched_clock();
		rem_nsec = do_div(ts, 1000000000);

		size = snprintf(buf + wr, MSG_MAX_SIZE,
			"[%5lu.%06lu] %s: circle %d\n",
			(unsigned long)ts, rem_nsec / 1000, __func__, i);
		if (size > 0)
			wr += size;

		if (wr > BUF_SIZE - MSG_MAX_SIZE)
			wr = 0;

		msleep(500);
	}

	printk("%s", buf);

	vfree(buf);

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
