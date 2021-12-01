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

#define VERSION_PREFIX dummy
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "dummy"

static void dummy_bus_release(struct device * dev)
{
}

struct device dummy_bus = {
	.init_name = "dummy",
	.release = dummy_bus_release,
};

static int dummy_drv_probe(struct device *device)
{
	struct dummy_driver *drv = to_dummy_driver(device->driver);
	struct dummy_device *dev = to_dummy_device(device);

	if (!drv->probe)
		return -EINVAL;

	return drv->probe(dev);
}

static void dummy_dev_release(struct device * dev)
{
}

int dummy_device_register(struct dummy_device *dummy_dev)
{
	struct device *dev = &dummy_dev->device;

	if (!dev_name(dev))
		return -EINVAL;

	if (!dev->parent)
		dev->parent = &dummy_bus;

	if (!dev->release)
		dev->release = dummy_dev_release;

	dev->bus = &dummy_bus_type;

	return device_register(dev);
}
EXPORT_SYMBOL(dummy_device_register);

void dummy_device_unregister(struct dummy_device *dummy_dev)
{
	device_del(&dummy_dev->device);
	put_device(&dummy_dev->device);
}
EXPORT_SYMBOL(dummy_device_unregister);

int __dummy_driver_register(struct dummy_driver *dummy_drv,
					struct module *module)
{
	struct device_driver *drv = &dummy_drv->driver;

	if (!drv->name)
		return -EINVAL;

	drv->owner = module;
	drv->bus = &dummy_bus_type;
	drv->probe = dummy_drv_probe;

	return driver_register(drv);
}
EXPORT_SYMBOL(__dummy_driver_register);

void dummy_driver_unregister(struct dummy_driver *drv)
{
	driver_unregister(&drv->driver);
}
EXPORT_SYMBOL_GPL(dummy_driver_unregister);

static int dummy_match(struct device *dev,
			struct device_driver *drv)
{
	return (!strcmp(dev_name(dev), drv->name));
}

struct bus_type dummy_bus_type = {
    .name        = "dummy",
    .match        = dummy_match,
};
EXPORT_SYMBOL_GPL(dummy_bus_type);

static int __init dummy_init(void)
{
	int ret;

	ret = device_register(&dummy_bus);
	if (ret) {
		put_device(&dummy_bus);
		return ret;
	}

	ret = bus_register(&dummy_bus_type);
	if (ret)
		device_unregister(&dummy_bus);

	return ret;
}

static void __exit dummy_exit(void)
{
	bus_unregister(&dummy_bus_type);
	device_unregister(&dummy_bus);
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_ALIAS("dummy-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
