#define MAX_NUMBER 10

struct my_pci_dev {
	char name[32];
	int vendor_id;
	int device_id;
	char revision_id;
	char interrupt_line;
	char latency_timer;
	int command;
};

struct my_inode {
	unsigned long i_ino;
	unsigned int i_nlink;
	unsigned short i_bytes;
};

struct result {
	int size;
	struct my_pci_dev devices[MAX_NUMBER];
	struct my_inode my_inode;
};
