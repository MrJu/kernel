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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>

#define PAGE_SHIFT 12
#define NPAGES 32
#define SIZE NPAGES << PAGE_SHIFT
#define OFFSET (1 << PAGE_SHIFT) * 0
#define DEV_PATH "/dev/foo"

int main(int argc, char **argv) {
	int fd, ret = 0;
	void *addr;

	fd = open(DEV_PATH, O_RDWR);
	if(fd < 0) {
		perror("Open device failed");
		return -1;
	}

	do{
		addr = mmap(NULL, SIZE,
				PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, OFFSET);
		if (addr == MAP_FAILED) {
			perror("mmap failed");
			ret = -1;
			break;
		}

		printf("The original data in memory: %s", addr);

		munmap(addr, SIZE);
	} while(0);

	close(fd);

	return ret;
}
