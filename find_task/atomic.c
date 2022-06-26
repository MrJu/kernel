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

#define VERSION_PREFIX atomic
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static struct task_struct *select_first_process_by_name(const char *name)
{

	struct task_struct *p;
	int found = 0;

	rcu_read_lock();
	for_each_process(p)
		if (!strcmp(p->comm, name)) {
			found = 1;
			break;
		}
	rcu_read_unlock();

	return found? p : NULL;
}

static int __init atomic_init(void)
{
	struct task_struct *p;

	p = select_first_process_by_name("insmod");
	if (p) {
		pr_info("%s: %d %s found\n",
				__func__, __LINE__, p->comm);
	}

	return 0;
}

static void __exit atomic_exit(void)
{
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_ALIAS("atomic-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
