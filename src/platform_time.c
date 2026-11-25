#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <time.h>

#include "platform_time.h"

int platform_time_now(double *seconds)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return -1;

    *seconds = (double)now.tv_sec
        + (double)now.tv_nsec / 1e9;

    return 0;
}

int platform_sleep_until(double deadline)
{
    double now;

    if (platform_time_now(&now) < 0)
        return -1;

    double remaining = deadline - now;

    if (remaining <= 0.0)
        return 0;

    struct timespec delay;

    delay.tv_sec = (time_t)remaining;

    delay.tv_nsec = (long)(
        (remaining - (double)delay.tv_sec) * 1e9
    );

    if (nanosleep(&delay, NULL) == 0)
        return 0;

    return errno == EINTR ? 1 : -1;
}
