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
#include <linux/i2c.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"

struct foo_device_data {
	char *name;
	int dummy;
};

static int foo_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

static int foo_remove(struct i2c_client *client)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

static const struct foo_device_data foo_dev_data = {
	.name = DEVICE_NAME,
	.dummy = 0,
};

static const struct i2c_device_id foo_i2c_id[] = {
	{
		.name = DEVICE_NAME,
		.driver_data = (unsigned long)&foo_dev_data,
	},
	{
		/* end */
	}
};
MODULE_DEVICE_TABLE(i2c, foo_i2c_id);

static const struct of_device_id foo_of_match[] = {
	{
		.compatible = "artech, foo",
		.data = &foo_dev_data,
	},
	{
		.compatible = "artech, foo-dev",
		.data = &foo_dev_data,
	},
	{
		/* end */
	}
};
MODULE_DEVICE_TABLE(of, foo_of_match);

static struct i2c_driver foo_driver = {
	.driver = {
		.name	= DEVICE_NAME,
		.of_match_table = of_match_ptr(foo_of_match),
	},
	.probe		= foo_probe,
	.remove		= foo_remove,
	.id_table	= foo_i2c_id,
};

static int __init foo_init(void)
{
	return i2c_add_driver(&foo_driver);
}

static void __exit foo_exit(void)
{
	i2c_del_driver(&foo_driver);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
