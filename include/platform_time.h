#ifndef FISHBY_PLATFORM_TIME_H
#define FISHBY_PLATFORM_TIME_H

int platform_time_now(double *seconds);
int platform_sleep_until(double deadline);

#endif