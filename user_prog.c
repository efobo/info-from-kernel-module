#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "user_struct.h"
#include <fcntl.h>

#define BUF_SIZE 128

int main (int argc, char *argv[]) {
	if (argc != 4) { // 3
		printf("Wrong number of args. Must be three - file path, vendor ID and device ID.\n");
		return 0;
	}

	char user_file_path[BUF_SIZE];
	sprintf(user_file_path, "%s", argv[1]);
	int32_t vendor_id = atoi(argv[2]);
	int32_t device_id = atoi(argv[3]);

	struct result *res = (struct result *) malloc(sizeof(struct result));
	char buffer_wr[256];
	char buffer_rd[1024];
	

	int fd = open("/sys/kernel/debug/newmod/data_structures", O_RDWR);
	if (fd < 0) {
		printf("Can't open file.\n");
		return 0;
	}

	printf("Writing path to file.\n");
	int buf_wr_length = sprintf(buffer_wr, "%d %d %s", vendor_id, device_id, user_file_path);
	write(fd, buffer_wr, sizeof(char) * buf_wr_length);

	printf("Getting data from file.\n");
	lseek(fd, 0, SEEK_SET);
	ssize_t read_char = read(fd, buffer_rd, sizeof(struct result));
	printf("Read chars count = %ld.\n", read_char);
	if (read_char == 0) {
		printf("The inode for file %s wasn't found and PCI Device with vendor ID = %d and device ID = %d doesn't exist.\n", user_file_path, vendor_id, device_id);
		return 0;
	}

	int i;
	char *c;
	c = (char *)res;
	for (i = 0; i < sizeof(struct result); i++) {
		*c = buffer_rd[i];
		c++;
	}

	printf("\nInode: ");
	if (res->my_inode.i_ino == 0) {
		printf("none");
	} else {
		printf("\nInode number = %ld", res->my_inode.i_ino);
		printf("\nHard link count = %d", res->my_inode.i_nlink);
		printf("\nSize = %d bytes", res->my_inode.i_bytes);
	}

	printf("\n\nPCI_device: ");
	if (!res->size) {
		printf("none");
	} printf("\n");

	for (i = 0; i < res->size; i++) {
	printf("\nDevice #%d:\n", i+1);
	printf(" Name %s\n", res->devices[i].name);
	printf(" Vendor ID = %d, Device ID = %d\n", res->devices[i].vendor_id, res->devices[i].device_id);
	printf(" Revision ID = %d\n", res->devices[i].revision_id);
	printf(" Interrupt Line = %d\n", res->devices[i].interrupt_line);
	printf(" Latency Timer = %d\n", res->devices[i].latency_timer);
	printf(" Command = %d\n", res->devices[i].command);
	}

	close(fd);
	return 0;
}
