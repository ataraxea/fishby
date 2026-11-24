#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stddef.h>

#include "signals.h"

static volatile sig_atomic_t stop_signal = 0;

static void handle_signal(int signo)
{
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
