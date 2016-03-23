#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vsd_ioctl.h"

void usage(char * prog) {
	printf("Usage: %s size_get\n"
		   "       %s size_set SIZE_IN_BYTES\n", prog, prog);
}

int print_size(int vsd) {
	vsd_ioctl_get_size_arg_t arg;
	if (ioctl(vsd, VSD_IOCTL_GET_SIZE, &arg)) {
		perror("ioctl");
		return EXIT_FAILURE;
	}
	printf("vsd size: %lu\n", arg.size);

	return EXIT_SUCCESS;
}

int set_size(int vsd, int size) {
	vsd_ioctl_set_size_arg_t arg = { 
		.size = size
	};

	if (ioctl(vsd, VSD_IOCTL_SET_SIZE, &arg)) {
		perror("ioctl");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int main(int argc, char **argv) {
	int vsd, ret = EXIT_FAILURE;

	if (argc > 3 || argc < 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	vsd = open("/dev/vsd", O_RDONLY);
	if (vsd == -1) {
		perror("open");
		goto exit;
	}

	if (!strcmp("size_get", argv[1]))
		ret = print_size(vsd);
	else if (!strcmp("size_set", argv[1])) {
		if (argc != 3) {
			usage(argv[0]);
			goto err;
		}

		int size = atoi(argv[2]);
		if (size <= 0) {
			usage(argv[0]);
			goto err;
		}

		ret = set_size(vsd, size);
	} else {
		usage(argv[0]);
		goto err;
	}

err:
	close(vsd);
exit:
    return ret;
}
