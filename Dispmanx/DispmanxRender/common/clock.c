/*
 * MIT License
 *
 * Copyright (c) 2018 Whuer_XiaoJie <1939346428@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <stdio.h>
#include <sys/select.h>
#include <time.h>

#include "clock.h"

void sleep_us(unsigned long us)
{
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = us;
	select(0, NULL, NULL, NULL, &tv);
}

void sleep_ms(unsigned long ms)
{
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	select(0, NULL, NULL, NULL, &tv);
}

void delay_ms(unsigned long ms)
{
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	select(0, NULL, NULL, NULL, &tv);
}

unsigned long get_clock_ms(void)
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

int get_local_time(struct tm *tm)
{
	int ret = 0;
	struct timeval tv;

	if (tm == NULL)
		return -1;

	/* get the local time */
	if (gettimeofday(&tv, NULL) != 0 || localtime_r(&tv.tv_sec, tm) == NULL){
		ret = -1;
		memset(tm, 0, sizeof(struct tm));
	}
	//fprintf(stderr, "Local time :%04d-%02d-%02d %02d:%02d:%02d \n",
	//	tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour,
	//	tm->tm_min, tm->tm_sec);
	return ret;
}

int get_utc_time(struct tm *tm)
{
	int ret = 0;
	struct timeval tv;

	if (tm == NULL)
		return -1;

	if (gettimeofday(&tv, NULL) != 0 || gmtime_r(&tv.tv_sec, tm) == NULL){
		ret = -1;
		memset(tm, 0, sizeof(struct tm));
	}
	//fprintf(stderr, "UTC time :%04d-%02d-%02d %02d:%02d:%02d \n",
	//	tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour,
	//	tm->tm_min, tm->tm_sec);
	return ret;
}
