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
#include <linux/device.h>
#include "dummy.h"

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX atomic
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "atomic"

static int atomic_probe(struct dummy_device *dummy_dev)
{
	dump_stack();
	return 0;
};

struct dummy_device atomic_dev = {
	.device = {
		.init_name = "atomic",
	},
};

struct dummy_driver atomic_drv = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "atomic",
	},
	.probe = atomic_probe,
};

static int __init atomic_init(void)
{
	int ret;

	ret = dummy_driver_register(&atomic_drv);
	if (ret)
		goto err0;

	ret = dummy_device_register(&atomic_dev);
	if (ret)
		goto err1;

	return 0;

err1:
	dummy_driver_unregister(&atomic_drv);

err0:
	return ret;
}

static void __exit atomic_exit(void)
{

	dummy_device_unregister(&atomic_dev);
	dummy_driver_unregister(&atomic_drv);
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_ALIAS("atomic-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
