/*
 * Copyright (C) 2019 Andrew <mrju.email@gail.com>
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
#include <linux/of.h>
#include <linux/spi/spi.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"

static int foo_probe(struct spi_device *spi)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

static int foo_remove(struct spi_device *spi)
{
	return 0;
}

static const struct spi_device_id foo_spi_id[] = {
	{
		DEVICE_NAME, 0
	},
	{
		/* end */
	}
};
MODULE_DEVICE_TABLE(spi, foo_spi_id);

static const struct of_device_id foo_of_match[] = {
	{
		.compatible = "artech, foo",
	},
	{
		/* end */
	},
};
MODULE_DEVICE_TABLE(of, foo_of_match);

static struct spi_driver foo_drv = {
	.driver = {
		.name = DEVICE_NAME,
		.of_match_table = of_match_ptr(foo_of_match),
	},
	.probe        = foo_probe,
	.remove       = foo_remove,
	.id_table     = foo_spi_id,
};

static int __init foo_init(void)
{
	return spi_register_driver(&foo_drv);
}

static void __exit foo_exit(void)
{
	spi_unregister_driver(&foo_drv);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
