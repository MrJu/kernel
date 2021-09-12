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
#include <linux/platform_device.h>
#include <linux/of.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX consumer
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "consumer"

static int consumer_probe(struct platform_device *pdev)
{
	pr_info("%s is executed\n", __func__);
	return 0;
}

static int consumer_remove(struct platform_device *pdev)
{
	pr_info("%s is executed\n", __func__);
	return 0;
}

static const struct of_device_id consumer_of_match[] = {
	{
		.compatible = "artech,consumer",
	},
	{
		/* end */
	},
};
MODULE_DEVICE_TABLE(of, consumer_of_match);

static struct platform_driver consumer_drv = {
	.probe	= consumer_probe,
	.remove	= consumer_remove,
	.driver	= {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
		.of_match_table = of_match_ptr(consumer_of_match),
	}
};

static int __init consumer_init(void)
{
	return platform_driver_register(&consumer_drv);
}

static void __exit consumer_exit(void)
{
	platform_driver_unregister(&consumer_drv);
}

module_init(consumer_init);
module_exit(consumer_exit);

MODULE_ALIAS("consumer-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
