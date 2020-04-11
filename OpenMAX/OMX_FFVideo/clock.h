/*
 * clock.h
 */

#ifndef CLOCK_H
#define CLOCK_H

#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW	4
#endif

void __sleep(unsigned long ms);
void __delay(unsigned long ms);
unsigned long __clock(void);
void get_local_time(struct tm *tm);

#endif	/* CLOCK_H */
