#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <mutex.h>
#include <shared_spinlock.h>
#include <mutex_ioctl.h>

#define MUTEX_DRIVER_PATH "/dev/mutex"

int mutex_driver_fd = -1;

mutex_err_t mutex_init(mutex_t *m)
{
	mutex_ioctl_lock_create_arg_t karg;

	shared_spinlock_init(&m->spinlock);
	m->kwaiters_cnt = 0;

	if (0 > ioctl(mutex_driver_fd, MUTEX_IOCTL_LOCK_CREATE, &karg))
		return MUTEX_INTERNAL_ERR;
	m->kid = karg.id;

	return MUTEX_OK;
}

mutex_err_t mutex_deinit(mutex_t *m)
{
	mutex_ioctl_lock_destroy_arg_t karg;

	karg.id = m->kid;
	if (0 > ioctl(mutex_driver_fd, MUTEX_IOCTL_LOCK_DESTROY, &karg))
		return MUTEX_INTERNAL_ERR;

	return MUTEX_OK;
}

mutex_err_t mutex_lock(mutex_t *m)
{
	mutex_ioctl_lock_wait_arg_t karg;

	if (shared_spin_trylock(&m->spinlock))
		return MUTEX_OK;

	karg.spinlock = &m->spinlock;
	karg.id = m->kid;

	__sync_add_and_fetch(&m->kwaiters_cnt, 1);
	if (0 > ioctl(mutex_driver_fd, MUTEX_IOCTL_LOCK_WAIT, &karg)) {
		__sync_sub_and_fetch(&m->kwaiters_cnt, 1);
		return MUTEX_INTERNAL_ERR;
	}
	__sync_sub_and_fetch(&m->kwaiters_cnt, 1);

	// NOTE: spinlock is locked in kernel

    return MUTEX_OK;
}

mutex_err_t mutex_unlock(mutex_t *m)
{
	mutex_ioctl_lock_wake_arg_t karg;

	if (!m->kwaiters_cnt) {
		if (!shared_spin_unlock(&m->spinlock))
			return MUTEX_INTERNAL_ERR;

		return MUTEX_OK;
	}

	karg.spinlock = &m->spinlock;
	karg.id = m->kid;
	if (0 > ioctl(mutex_driver_fd, MUTEX_IOCTL_LOCK_WAKE, &karg))
	    return MUTEX_INTERNAL_ERR;

	// NOTE: spinlock is unlocked in kernel

	return MUTEX_OK;
}

mutex_err_t mutex_lib_init()
{
	if (mutex_driver_fd >= 0)
		return MUTEX_INTERNAL_ERR;

	mutex_driver_fd = open(MUTEX_DRIVER_PATH, O_RDWR);

	return mutex_driver_fd < 0 ? MUTEX_INTERNAL_ERR : MUTEX_OK;
}

mutex_err_t mutex_lib_deinit()
{
	if (-1 == mutex_driver_fd)
		return MUTEX_INTERNAL_ERR;

	if (0 > close(mutex_driver_fd))
		return MUTEX_INTERNAL_ERR;
	mutex_driver_fd = -1;

    return MUTEX_OK;
}
