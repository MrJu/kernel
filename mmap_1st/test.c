#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>

#define PAGE_SIZE 4096
#define NPAGES 32
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
		addr = mmap(NULL, PAGE_SIZE * NPAGES,
				PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (addr == MAP_FAILED) {
			perror("mmap failed");
			ret = -1;
			break;
		}

		printf("The original data in memory: %s", addr);

		munmap(addr, PAGE_SIZE * NPAGES);
	} while(0);

	close(fd);

	return ret;
}
