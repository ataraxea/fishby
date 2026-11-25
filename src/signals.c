#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "signals.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <signal.h>

/* Windows console handlers run on a separate thread */
static volatile LONG stop_signal = 0;

static BOOL WINAPI handle_console_event(DWORD event)
{
    LONG signo;

    switch (event) {
        case CTRL_C_EVENT:
            signo = SIGINT;
            break;

        case CTRL_BREAK_EVENT:
            signo = SIGTERM;
            break;

        default:
            return FALSE;
    }

    InterlockedCompareExchange(
        &stop_signal,
        signo,
        0
    );

    return TRUE;
}

int signals_init(void)
{
    if (!SetConsoleCtrlHandler(
            handle_console_event,
            TRUE
        )) {

        return -1;
    }

    return 0;
}

int signals_requested(void)
{
    return InterlockedCompareExchange(
        &stop_signal, 0, 0
    ) != 0;
}

int signals_received(void)
{
    return (int)InterlockedCompareExchange(
        &stop_signal, 0, 0
    );
}

#else

#include <signal.h>
#include <stddef.h>

static volatile sig_atomic_t stop_signal = 0;

static void handle_signal(int signo)
{
    if (stop_signal == 0)
        stop_signal = signo;
}

int signals_init(void)
{
    struct sigaction action = {0};

    action.sa_handler = handle_signal;

    if (sigfillset(&action.sa_mask) < 0)
        return -1;

    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) < 0)
        return -1;

    if (sigaction(SIGTERM, &action, NULL) < 0)
        return -1;

    if (sigaction(SIGHUP, &action, NULL) < 0)
        return -1;

    return 0;
}

int signals_requested(void)
{
    return stop_signal != 0;
}

int signals_received(void)
{
    return (int)stop_signal;
}

#endif
