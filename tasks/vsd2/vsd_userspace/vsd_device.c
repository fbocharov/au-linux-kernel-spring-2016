#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <vsd_ioctl.h>

#include "vsd_device.h"

#define PAGE_ALIGNED(addr, page_size) ((size_t)(addr) % (page_size) == 0)

#define VSD_PATH "/dev/vsd"

int vsd = -1;

int vsd_init()
{
    vsd = open(VSD_PATH, O_RDWR);
    return vsd >= 0 ? 0 : -1;
}

int vsd_deinit()
{
    return close(vsd);
}

int vsd_get_size(size_t *out_size)
{
    vsd_ioctl_get_size_arg_t arg;

    if (ioctl(vsd, VSD_IOCTL_GET_SIZE, &arg))
        return -1;

    *out_size = arg.size;

    return 0;
}

int vsd_set_size(size_t size)
{
    vsd_ioctl_set_size_arg_t arg = {
        .size = size
    };

    if (ioctl(vsd, VSD_IOCTL_SET_SIZE, &arg))
        return -1;

    return 0;
}

ssize_t vsd_read(char* dst, off_t offset, size_t size)
{
    if (-1 == lseek(vsd, offset, SEEK_SET))
        return -1;

    return read(vsd, dst, size);
}

ssize_t vsd_write(const char* src, off_t offset, size_t size)
{
    if (-1 == lseek(vsd, offset, SEEK_SET))
        return -1;

    return write(vsd, src, size);
}

static size_t get_mmap_length(size_t offset)
{
    size_t vsd_size;

    if (vsd_get_size(&vsd_size))
        return -1;

    return vsd_size - offset;
}

void* vsd_mmap(size_t offset)
{
    size_t length;
    int page_size = getpagesize();

    length = get_mmap_length(offset);
    if (-1 == length)
        return MAP_FAILED;

    return mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, vsd, offset);
}

int vsd_munmap(void* addr, size_t offset)
{
    size_t length;
    int page_size = getpagesize();

    if (!PAGE_ALIGNED(addr, page_size) ||
            !PAGE_ALIGNED(offset, page_size)) {
        errno = EINVAL;
        return -1;
    }

    length = get_mmap_length(offset);
    if (-1 == length)
        return -1;

    return munmap(addr, length);
}
