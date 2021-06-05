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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/blk-mq.h>
#include <linux/hdreg.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX "Gendisk"
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

static int dev_major = 0;

struct block_dev {
	sector_t capacity;
	u8 *data;
	struct blk_mq_tag_set tag_set;
	struct request_queue *queue;
	struct gendisk *gdisk;
};

static struct block_dev *gendisk = NULL;

static int gendisk_open(struct block_device *dev, fmode_t mode)
{
	printk("%s %d\n", __func__, __LINE__);
	return 0;
}

static void gendisk_release(struct gendisk *gdisk, fmode_t mode)
{
	printk("%s %d\n", __func__, __LINE__);
}

int gendisk_ioctl(struct block_device *bdev, fmode_t mode,
		unsigned cmd, unsigned long arg)
{
	printk("%s %d cmd 0x%08x\n", __func__, __LINE__, cmd);
	return -ENOTTY;
}

static struct block_device_operations gendisk_ops = {
	.owner = THIS_MODULE,
	.open = gendisk_open,
	.release = gendisk_release,
	.ioctl = gendisk_ioctl
};

static int do_request(struct request *rq, unsigned int *nr_bytes)
{
	int ret = 0;
	struct bio_vec bvec;
	struct req_iterator iter;
	struct block_dev *dev = rq->q->queuedata;
	loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
	loff_t dev_size = (loff_t)(dev->capacity << SECTOR_SHIFT);

	printk("%s %d request start from sector %lld pos = %lld dev_size = %lld\n",
		__func__, __LINE__, blk_rq_pos(rq), pos, dev_size);

	rq_for_each_segment(bvec, rq, iter) {
		unsigned long b_len = bvec.bv_len;
		void* b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

		if ((pos + b_len) > dev_size)
			b_len = (unsigned long)(dev_size - pos);

		if (rq_data_dir(rq) == WRITE)
			memcpy(dev->data + pos, b_buf, b_len);
		else
			memcpy(b_buf, dev->data + pos, b_len);

		pos += b_len;
		*nr_bytes += b_len;
	}

	return ret;
}

static blk_status_t queue_rq(struct blk_mq_hw_ctx *hctx,
		const struct blk_mq_queue_data* bd)
{
	unsigned int nr_bytes = 0;
	blk_status_t status = BLK_STS_OK;
	struct request *rq = bd->rq;

	blk_mq_start_request(rq);

	if (do_request(rq, &nr_bytes) != 0)
		status = BLK_STS_IOERR;

	if (blk_update_request(rq, status, nr_bytes))
		BUG();

	__blk_mq_end_request(rq, status);

	return status;
}

static struct blk_mq_ops mq_ops = {
	.queue_rq = queue_rq,
};

static int __init gendisk_init(void)
{
	dev_major = register_blkdev(dev_major, "gendisk");

	gendisk = kmalloc(sizeof (struct block_dev), GFP_KERNEL);
	if (!gendisk) {
		printk("%s %d failed to alloc gendisk\n", __func__, __LINE__);
		unregister_blkdev(dev_major, "gendisk");

		return -ENOMEM;
	}

	/* nsectors * SECTOR_SIZE; */
	gendisk->capacity = (256 * 1024 * 1024UL) >> 9;

	gendisk->data = vmalloc(gendisk->capacity << 9);
	if (!gendisk->data) {
		printk("%s %d failed to alloc data buffer\n", __func__, __LINE__);

		unregister_blkdev(dev_major, "gendisk");
		kfree(gendisk);

		return -ENOMEM;
	}

	gendisk->queue = blk_mq_init_sq_queue(&gendisk->tag_set, &mq_ops, 128, BLK_MQ_F_SHOULD_MERGE);
	if (!gendisk->queue) {
		printk("%s %d failed to alloc request queue\n", __func__, __LINE__);

		vfree(gendisk->data);
		unregister_blkdev(dev_major, "gendisk");
		kfree(gendisk);

		return -ENOMEM;
	}

	gendisk->queue->queuedata = gendisk;
	gendisk->gdisk = alloc_disk(1);

	gendisk->gdisk->flags = GENHD_FL_NO_PART_SCAN;
	gendisk->gdisk->major = dev_major;
	gendisk->gdisk->first_minor = 0;

	gendisk->gdisk->fops = &gendisk_ops;
	gendisk->gdisk->queue = gendisk->queue;
	gendisk->gdisk->private_data = gendisk;

	strcpy(gendisk->gdisk->disk_name, "gendisk");
	set_capacity(gendisk->gdisk, gendisk->capacity);
	add_disk(gendisk->gdisk);

	return 0;
}

static void __exit gendisk_exit(void)
{
	if (gendisk->gdisk) {
		del_gendisk(gendisk->gdisk);
		put_disk(gendisk->gdisk);
	}

	if (gendisk->queue)
		blk_cleanup_queue(gendisk->queue);

	vfree(gendisk->data);

	unregister_blkdev(dev_major, "gendisk");
	kfree(gendisk);
}

module_init(gendisk_init);
module_exit(gendisk_exit);

MODULE_ALIAS("gendisk-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
