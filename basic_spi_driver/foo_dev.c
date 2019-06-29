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
#include <linux/spi/spi.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"
#define SPI_BUS 0

static struct spi_device *foo_dev;

static int __init foo_init(void)
{
	struct spi_master *master;
	struct spi_board_info chip = {
		.modalias     = DEVICE_NAME, /* to match with driver */
		.mode         = 0x00,
		.bus_num      = 0,
		.chip_select  = 0,
		.max_speed_hz = 2000000,
	};

	master = spi_busnum_to_master(SPI_BUS);
	if (!master)
		return -ENODEV;

	foo_dev = spi_new_device(master, &chip);
	if (!foo_dev)
		return -EBUSY;

	return 0;
}

static void __exit foo_exit(void)
{
	spi_unregister_device(foo_dev);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
