#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "state.h"

#define STATE_FILE "pet.state"
#define LOCK_FILE "pet.lock"

/* Active storage session */
static int lock_fd = -1;
static int active_dir = -1;

static int make_parents(char *path)
{
    for (char *p = path + 1; *p; ++p) {
        if (*p != '/')
            continue;

        *p = '\0';

        int rc = mkdir(path, 0700);

        *p = '/';

        if (rc < 0 && errno != EEXIST)
            return -1;
    }

    return 0;
}

static int acquire_lock(int dir)
{
    if (lock_fd >= 0) {
        errno = EALREADY;
        return -1;
    }

    int fd = openat(
        dir,
        LOCK_FILE,
        O_RDWR | O_CREAT |
        O_NOFOLLOW | O_NONBLOCK |
        O_CLOEXEC,
        0600
    );

    if (fd < 0)
        return -1;

    struct stat st;

    /* Validate lock file metadata */
    if (fstat(fd, &st) < 0) {
        int error = errno;
        close(fd);
        errno = error;
        return -1;
    }

    if (!S_ISREG(st.st_mode) ||
        st.st_uid != geteuid() ||
        (st.st_mode & 0777) != 0600 ||
        st.st_nlink != 1) {

        close(fd);
        errno = EPERM;
        return -1;
    }

    /* Never wait for another instance */
    int rc;

    do {
        rc = flock(fd, LOCK_EX | LOCK_NB);
    } while (rc < 0 && errno == EINTR);

    if (rc < 0) {
        int error = errno;

        close(fd);
        errno = error;

        return -1;
    }

    lock_fd = fd;

    return 0;
}

int state_open(void)
{
    const char *base = getenv("XDG_DATA_HOME");
    const char *suffix = "/fishby";

    if (!base || base[0] != '/') {
        base = getenv("HOME");
        suffix = "/.local/share/fishby";
    }

    if (!base || base[0] != '/') {
        errno = EINVAL;
        return -1;
    }

    char path[4096];

    int n = snprintf(
        path,
        sizeof(path),
        "%s%s",
        base,
        suffix
    );

    if (n < 0 || (size_t)n >= sizeof(path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    if (make_parents(path) < 0)
        return -1;

    if (mkdir(path, 0700) < 0 && errno != EEXIST)
        return -1;

    int fd = open(
        path,
        O_RDONLY | O_DIRECTORY |
        O_NOFOLLOW | O_CLOEXEC
    );

    if (fd < 0)
        return -1;

    struct stat st;

    if (fstat(fd, &st) < 0 ||
        st.st_uid != geteuid() ||
        (st.st_mode & 077) != 0) {

        close(fd);
        return -1;
    }

    /* Lock storage before loading pet state */
    if (acquire_lock(fd) < 0) {
        int error = errno;

        close(fd);
        errno = error;

        return -1;
    }

    active_dir = fd;

    return fd;
}

void state_close(int dir)
{
    if (dir < 0 || dir != active_dir)
        return;

    /* Close storage while the lock is still held */
    close(active_dir);
    active_dir = -1;

    /* Closing the descriptor releases the lock */
    if (lock_fd >= 0) {
        close(lock_fd);
        lock_fd = -1;
    }
}

static int parse_number(
    char **p,
    unsigned long *value
)
{
    if (**p < '0' || **p > '9')
        return -1;

    char *end;

    errno = 0;

    unsigned long n = strtoul(*p, &end, 10);

    if (errno == ERANGE || *end != '\n')
        return -1;

    *p = end + 1;
    *value = n;

    return 0;
}

int state_load(int dir, Pet *pet)
{
    if (lock_fd < 0 ||
        dir < 0 ||
        dir != active_dir) {

        errno = EPERM;
        return -1;
    }

    int fd = openat(
        dir,
        STATE_FILE,
        O_RDONLY | O_NOFOLLOW |
        O_NONBLOCK | O_CLOEXEC
    );

    if (fd < 0)
        return errno == ENOENT ? 0 : -1;

    struct stat st;
    char buf[128];
    size_t used = 0;

    if (fstat(fd, &st) < 0 ||
        !S_ISREG(st.st_mode) ||
        st.st_uid != geteuid() ||
        (st.st_mode & 077) != 0) {
        goto fail;
    }

    /* Read a bounded amount of data */
    for (;;) {
        if (used == sizeof(buf) - 1)
            goto fail;

        ssize_t n = read(
            fd,
            buf + used,
            sizeof(buf) - 1 - used
        );

        if (n < 0 && errno == EINTR)
            continue;

        if (n < 0)
            goto fail;

        if (n == 0)
            break;

        used += (size_t)n;
    }

    /* Reject embedded NUL bytes */
    if (memchr(buf, '\0', used))
        goto fail;

    buf[used] = '\0';

    /* Validate the format version */
    if (strncmp(buf, "FISHBY1\n", 8) != 0)
        goto fail;

    char *p = buf + 8;

    unsigned long hunger;
    unsigned long eaten;

    if (parse_number(&p, &hunger) < 0 ||
        hunger > 100000 ||
        parse_number(&p, &eaten) < 0 ||
        *p != '\0') {
        goto fail;
    }

    close(fd);

    /* Apply state only after full validation */
    pet->hunger = (float)hunger / 1000.0f;
    pet->eaten = eaten;
    pet->happy_timer = 0.0f;

    pet_update(pet, 0.0f);

    return 1;

fail:
    close(fd);
    return -1;
}

int state_save(int dir, const Pet *pet)
{
    if (lock_fd < 0 ||
        dir < 0 ||
        dir != active_dir) {

        errno = EPERM;
        return -1;
    }

    if (!isfinite(pet->hunger) ||
        pet->hunger < 0 ||
        pet->hunger > 100) {
        return -1;
    }

    char buf[128];
    char tmp[64];

    int n = snprintf(
        buf,
        sizeof(buf),
        "FISHBY1\n%lu\n%lu\n",
        (unsigned long)lroundf(
            pet->hunger * 1000.0f
        ),
        pet->eaten
    );

    if (n < 0 || (size_t)n >= sizeof(buf))
        return -1;

    int fd = -1;

    /* Create an exclusive temporary file */
    for (unsigned i = 0; i < 128; ++i) {
        int len = snprintf(
            tmp,
            sizeof(tmp),
            ".pet.%ld.%u.tmp",
            (long)getpid(),
            i
        );

        if (len < 0 || (size_t)len >= sizeof(tmp))
            return -1;

        fd = openat(
            dir,
            tmp,
            O_WRONLY | O_CREAT | O_EXCL |
            O_NOFOLLOW | O_CLOEXEC,
            0600
        );

        if (fd >= 0)
            break;

        if (errno != EEXIST)
            return -1;
    }

    if (fd < 0)
        return -1;

    size_t used = 0;
    int ok = 1;

    /* Handle partial writes and interruptions */
    while (used < (size_t)n) {
        ssize_t w = write(
            fd,
            buf + used,
            (size_t)n - used
        );

        if (w < 0 && errno == EINTR)
            continue;

        if (w <= 0) {
            ok = 0;
            break;
        }

        used += (size_t)w;
    }

    if (ok && fsync(fd) < 0)
        ok = 0;

    if (close(fd) < 0)
        ok = 0;

    /* Atomically replace the previous state */
    if (!ok ||
        renameat(dir, tmp, dir, STATE_FILE) < 0) {

        unlinkat(dir, tmp, 0);
        return -1;
    }

    return fsync(dir);
}
