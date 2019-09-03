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

#ifndef __RINGBUFFER
#define __RINGBUFFER

#include <linux/types.h>

enum {
	RBUF_RD,
	RBUF_WR,
};

struct ringbuffer {
	char *buf;
	size_t rd;
	size_t wr;
	size_t count;
	size_t size;
};

struct ringbuffer *ringbuffer_create(size_t bytes, gfp_t flags);
void ringbuffer_destroy(struct ringbuffer *ringbuf);
size_t ringbuffer_write(struct ringbuffer *ringbuf, const char *buf, size_t bytes);
size_t ringbuffer_read(struct ringbuffer *ringbuf, char *buf, size_t bytes);
size_t ringbuffer_avail(struct ringbuffer *ringbuf, int dir);

#endif
