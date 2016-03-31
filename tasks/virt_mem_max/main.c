#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main()
{
	size_t bytesCount = 0;
	size_t unit = -1;

	while (unit > 0) {
		if (MAP_FAILED == mmap(NULL, unit, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0))
			unit /= 2;
		else
			bytesCount += unit;
	}

	printf("Max mem size = %ld bytes.\n", bytesCount);
    return 0;
}
