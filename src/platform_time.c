#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stddef.h>
#include <math.h>

#include "platform_time.h"
#include "signals.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int platform_time_now(double *seconds)
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    if (seconds == NULL)
        return -1;

    if (!QueryPerformanceFrequency(&frequency) ||
        frequency.QuadPart <= 0) {
        return -1;
    }

    if (!QueryPerformanceCounter(&counter))
        return -1;

    *seconds = (double)counter.QuadPart /
               (double)frequency.QuadPart;

    return 0;
}

int platform_sleep_until(double deadline)
{
    if (!isfinite(deadline))
        return -1;

    for (;;) {
        if (signals_requested())
            return 1;

        double now;

        if (platform_time_now(&now) < 0)
            return -1;

        double remaining = deadline - now;

        if (remaining <= 0.0)
            return 0;

        /*
         * Bound each sleep to keep shutdown responsive.
         * Avoid busy waiting.
         */
        double milliseconds = remaining * 1000.0;

        if (milliseconds > 10.0)
            milliseconds = 10.0;

        DWORD delay = (DWORD)milliseconds;

        if (delay == 0)
            delay = 1;

        Sleep(delay);
    }
}

#else

#include <errno.h>
#include <time.h>

int platform_time_now(double *seconds)
{
    struct timespec now;

    if (seconds == NULL)
        return -1;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return -1;

    *seconds = (double)now.tv_sec +
               (double)now.tv_nsec / 1e9;

    return 0;
}

int platform_sleep_until(double deadline)
{
    if (!isfinite(deadline))
        return -1;

    for (;;) {
        if (signals_requested())
            return 1;

        double now;

        if (platform_time_now(&now) < 0)
            return -1;

        double remaining = deadline - now;

        if (remaining <= 0.0)
            return 0;

        /* Bound sleep duration */
        if (remaining > 0.1)
            remaining = 0.1;

        struct timespec delay;

        delay.tv_sec = (time_t)remaining;

        delay.tv_nsec = (long)(
            (remaining - (double)delay.tv_sec) * 1e9
        );

        if (nanosleep(&delay, NULL) != 0)
            return errno == EINTR ? 1 : -1;
    }
}

#endif
