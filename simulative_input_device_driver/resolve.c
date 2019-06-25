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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define INPUT_FILE "/dev/input/event7"

int main(int argc, char **argv)
{
	int count, i;
	int fd;
	struct input_event ev_key;

	fd = open(INPUT_FILE, O_RDONLY);
	if (fd < 0) {
		perror("open device key!");
		return 1;
	}

	while (1) {
		count = read(fd, &ev_key, sizeof(struct input_event));
		if (EV_KEY == ev_key.type)
			printf("type: %d, code: %d, value: %d\n",
				ev_key.type, ev_key.code, ev_key.value);
		if (EV_SYN == ev_key.type)
			printf("syn event\n");
	}

	close(fd);
	return 0;
}
