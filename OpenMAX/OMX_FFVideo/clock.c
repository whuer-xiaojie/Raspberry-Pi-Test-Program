/*
 * clock.c
 */

#include <stdio.h>
#include <sys/select.h>
#include <time.h>

#include "clock.h"

void __sleep(unsigned long ms)
{
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	select(0, NULL, NULL, NULL, &tv);
}
void __delay(unsigned long ms)
{
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	select(0, NULL, NULL, NULL, &tv);
}

unsigned long __clock(void)
{
	struct timespec ts;
	unsigned long n_ticks;

	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0)
		return 0;
	n_ticks = ts.tv_sec;
	n_ticks *= 1000;
	n_ticks += ((unsigned long)ts.tv_nsec + 500000L) / 1000000L;
	return n_ticks;
}
void get_local_time(struct tm *tm)
{
	struct timeval tv;
	/* get the local time */
	if (gettimeofday(&tv, NULL) != 0 || localtime_r(&tv.tv_sec, tm) == NULL)
		memset(tm, 0, sizeof(struct tm));
}