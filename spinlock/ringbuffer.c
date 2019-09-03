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

#include <linux/types.h>
#include <linux/slab.h>
#include <asm/string.h>
#include "ringbuffer.h"

struct __must_check ringbuffer *ringbuffer_create(size_t bytes, gfp_t flags)
{
	struct ringbuffer *ringbuf;

	ringbuf = kzalloc(sizeof(*ringbuf), flags);
	if (!ringbuf)
		return ERR_PTR(-ENOMEM);

	ringbuf->buf = kzalloc(bytes, flags);
	if (!ringbuf->buf)
		return ERR_PTR(-ENOMEM);

	ringbuf->rd = 0;
	ringbuf->wr = 0;
	ringbuf->count = 0;
	ringbuf->size = bytes;

	return ringbuf;
}

void ringbuffer_destroy(struct ringbuffer *ringbuf)
{
	kfree(ringbuf->buf);
	kfree(ringbuf);
}

size_t ringbuffer_write(struct ringbuffer *ringbuf,
			const char *buf, size_t bytes)
{
	size_t avail, bytes1;

	avail = ringbuf->size - ringbuf->count;
	if (avail < bytes)
		return 0;

	if (ringbuf->wr + bytes > ringbuf->size) {
		bytes1 = ringbuf->size - ringbuf->wr;
		memcpy(ringbuf->buf + ringbuf->wr, buf, bytes1);
		memcpy(ringbuf->buf, buf + bytes1, bytes - bytes1);
	} else {
		memcpy(ringbuf->buf + ringbuf->wr, buf, bytes);
	}

	ringbuf->wr = (ringbuf->wr + bytes) % ringbuf->size;

	ringbuf->count += bytes;

	return bytes;
}

size_t ringbuffer_read(struct ringbuffer *ringbuf, char *buf, size_t bytes)
{
	size_t avail, bytes1;

	avail = ringbuf->count;
	if (avail < bytes)
		return 0;

	if (ringbuf->rd + bytes > ringbuf->size) {
		bytes1 = ringbuf->size - ringbuf->rd;
		memcpy(buf, ringbuf->buf + ringbuf->rd, bytes1);
		memcpy(buf + bytes1, ringbuf->buf, bytes - bytes1);
	} else {
		memcpy(buf, ringbuf->buf + ringbuf->rd, bytes);
	}

	ringbuf->rd = (ringbuf->rd + bytes) % ringbuf->size;

	ringbuf->count -= bytes;

	return bytes;
}

size_t ringbuffer_avail(struct ringbuffer *ringbuf, int dir)
{
	size_t avail;

	if (dir) /* write */
		avail = ringbuf->size - ringbuf->count;
	else /* read */
		avail = ringbuf->count;

	return avail;
}
