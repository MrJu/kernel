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
#include <linux/mempool.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX atomic
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "atomic"
#define NR_MIN_ELEMENTS 8
#define NR_ELEMENT_ORDER 0

static int atomic_probe(struct platform_device *pdev)
{
	int ret;
	mempool_t *pool;
	void *element;

	pool = mempool_create_page_pool(NR_MIN_ELEMENTS, NR_ELEMENT_ORDER);
	if (!pool) {
		ret = -ENOMEM;
		goto err0;
	}

	element = mempool_alloc(pool, GFP_KERNEL);
	if (!element) {
		ret = -ENOMEM;
		goto err1;
	}

	memset(page_address(element), 0xff, (1 << NR_ELEMENT_ORDER) * PAGE_SIZE);

	mempool_free(element, pool);
	mempool_destroy(pool);

	return 0;
err1:
	mempool_destroy(pool);

err0:
	return ret;
}

static int atomic_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver atomic_drv = {
	.probe	= atomic_probe,
	.remove	= atomic_remove,
	.driver	= {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
	}
};

static int __init atomic_init(void)
{
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&atomic_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit atomic_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(atomic_drv.driver.bus, NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&atomic_drv);
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_ALIAS("atomic-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
