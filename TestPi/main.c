/*
 * main.c
 */

#include <fcntl.h>
#include <pthread.h>
#ifndef __X6B__
#include <SDL2/SDL.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "arch.h"
#include "clock.h"
#include "config.h"
#include "debug.h"
#include "display.h"
#include "graphics.h"
#include "keyboard.h"
#include "libunix.h"
#include "main.h"
#include "sensor-and-control-panel.h"
#include "vram.h"


const int major_version = 7;
const int minor_version = 1;
const int build_year = 2015;
const int build_month = 7;
const int build_day = 22;

int graphics_mode = 0;
struct timeval last_reset_time;
char *sys_path = NULL;
char *sys_dir = NULL;
int quit = 0;

int expired = 0;
static const int days_in_mon[] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

//define lcd test global variable
int lcd_start = 0;
/* options */

static int exp_year, exp_month, exp_day;

void main_read_config(void)
{
	char *value;
	char token[MAXSTRLEN];

	value = query_string_value("exp_date");
	tokenize(token, value, ',', 0);
	trim(token);
	exp_year = atoi(token);
	tokenize(token, value, ',', 1);
	trim(token);
	exp_month = atoi(token);
	tokenize(token, value, ',', 2);
	trim(token);
	exp_day = atoi(token);
}

void main_save_config(void)
{
	char value[MAXSTRLEN];

	save_string_value("#main", "=========");

	sprintf(value, "%d,%d,%d", exp_year, exp_month, exp_day);
	save_string_value("exp_date", value);
}

/****************************************************************************/

static initcall_t initcalls[INITCALL_COUNT];
static int initcall_cnt = 0;
static initcall_t exitcalls[INITCALL_COUNT];
static int exitcall_cnt = 0;

void add_initcall(initcall_t initcall)
{
	if (initcall_cnt >= INITCALL_COUNT)
		return;
	initcalls[initcall_cnt++] = initcall;
}

void add_exitcall(initcall_t exitcall)
{
	if (exitcall_cnt >= INITCALL_COUNT)
		return;
	exitcalls[exitcall_cnt++] = exitcall;
}

/****************************************************************************/

static thread_t threads[THREAD_COUNT];
static int thread_cnt = 0;

void add_thread(void *(*start_routine)(void *), void *arg, int blocked)
{
	if (thread_cnt >= THREAD_COUNT)
		return;
	threads[thread_cnt].start_routine = start_routine;
	threads[thread_cnt].arg           = arg;
	threads[thread_cnt].blocked       = blocked;
	threads[thread_cnt].started       = 0;
	thread_cnt++;
}

/****************************************************************************/

int IsValidDate(int year, int month, int day)
{
	return (year >= 2000 && year <= 2037
		&& month >= 1 && month <= 12
		&& day >= 1
		&& day <= days_in_mon[month] + (month == 2 && is_leap(year)));
}

static void check_exp(void)
{
	struct stat stat;
	struct timeval tv;
	struct tm tm;
	int year, month, day;
	int fd;

	if (lstat(EXP_FILE, &stat) == 0) {	/* file exists */
		expired = 1;
		return;
	}

	if (gettimeofday(&tv, NULL) != 0
	    || localtime_r(&tv.tv_sec, &tm) == NULL) {
		DPRINT("%s(): cannot get time", __FUNCTION__);
		set_error(ERROR_SYSTEM, GET_TIME);
		return;
	}

	year  = tm.tm_year + 1900;
	month = tm.tm_mon + 1;
	day   = tm.tm_mday;

	if (year > exp_year
	    || (year == exp_year && month > exp_month)
	    || (year == exp_year && month == exp_month && day >= exp_day)) {
		if ((fd = creat(EXP_FILE, S_IRWXUGO)) == -1) {
			DPRINT("%s(): cannot create file \"%s\"", __FUNCTION__,
			       EXP_FILE);
			set_error(ERROR_SYSTEM, CREATE_FILE);
		} else
			close(fd);
		expired = 1;
	}
}

static char *progname = NULL;

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-g]\n", progname);
}

int main(int argc, char *argv[])
{
	char *p;
#ifndef __X6B__
	struct timespec ts;
#endif
	int i;
	int c;
	unsigned long n_ticks;
	unsigned long debugging_info_ticks;
	unsigned long check_exp_ticks;

	fprintf(stderr, "beacon %d.%02d build %d%02d%02d\n\
(C) 1998-2017 Shanghai Sansi Electronic Engineering Co., Ltd.\n",
		major_version, minor_version,
		build_year - 2000, build_month, build_day);

	progname = argv[0];
	if ((p = strrchr(progname, '/')) != NULL)
		progname = p + 1;

	if (argc > 2)
		usage();
	else if (argc == 2) {
		if (strcmp(argv[1], "-g") != 0)
			usage();
		else
			graphics_mode = 1;
	}

#ifndef __X6B__
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
		fprintf(stderr, "%s(): cannot get time\n", __FUNCTION__);
		SYSLOG("%s(): cannot get time", __FUNCTION__);
		exit(1);
	}
#endif

	if (gettimeofday(&last_reset_time, NULL) != 0) {
		fprintf(stderr, "%s(): cannot get time\n", __FUNCTION__);
		SYSLOG("%s(): cannot get time", __FUNCTION__);
		exit(1);
	}

	if ((sys_path = xreadlink("/proc/self/exe")) == NULL) {
		fprintf(stderr, "%s(): xreadlink() failed\n", __FUNCTION__);
		SYSLOG("%s(): xreadlink() failed", __FUNCTION__);
		exit(1);
	}

	if (strlen(sys_path) >= PATH_MAX) {
		fprintf(stderr, "%s(): sys_path too long\n", __FUNCTION__);
		SYSLOG("%s(): sys_path too long", __FUNCTION__);
		exit(1);
	}

	if ((sys_dir = strdup(sys_path)) == NULL) {
		fprintf(stderr, "%s(): strdup() failed\n", __FUNCTION__);
		SYSLOG("%s(): strdup() failed", __FUNCTION__);
		exit(1);
	}

	if ((p = strrchr(sys_dir, '/')) == NULL) {
		fprintf(stderr, "%s(): strrchr() failed\n", __FUNCTION__);
		SYSLOG("%s(): strrchr() failed", __FUNCTION__);
		exit(1);
	}
	p[1] = '\0';
	//通过Makefile指定编译（X6B,PC,X9）其中一个目录下的文件
	add_config(arch_read_config, arch_save_config);	/* do it first */
	add_config(debug_read_config, debug_save_config);
	add_config(main_read_config, main_save_config);
	add_config(vram_read_config, vram_save_config);
	read_config(NULL, NULL);
	read_config(SIGN_CONFIG_FILE, arch_read_sign_config);

	create_subdirs();
      fprintf(stderr, "graphics_mode=%d\n",graphics_mode);

	if (init_keyboard() != 0)
		fprintf(stderr, "%s(): cannot initialize keyboard\n",__FUNCTION__);
	else
		fprintf(stderr, "Press Q to exit\n");
	
	add_initcall(init_brightness_table);
	add_initcall(init_brightness_timetable);

	for (i = 0; i < initcall_cnt; i++)
		(initcalls[i])();

	for (i = 0; i < thread_cnt; i++) {
		if (pthread_create(&threads[i].thread, NULL,
				   threads[i].start_routine,
				   threads[i].arg) == 0)
			threads[i].started = 1;
		else {
			DPRINT("%s(): cannot create thread %d",__FUNCTION__,i);
			SYSLOG("%s(): cannot create thread %d",__FUNCTION__,i);
		}
	}

	n_ticks = __clock();
	debugging_info_ticks = n_ticks;
	check_exp_ticks = n_ticks;

	while (!quit) {
		while ((c = __getch()) >= 0) {
			if (c == 'Q' || c == 'q') {
				quit = 1;
				break;
			}
	}
	if (quit) break;

	n_ticks = __clock();

	if (n_ticks - debugging_info_ticks >= 6000L) {
		debugging_info();
		debugging_info_ticks = n_ticks;
	}

	if (IsValidDate(exp_year, exp_month, exp_day)) {
		if (n_ticks - check_exp_ticks >= 60000L) {
			check_exp();
			check_exp_ticks = n_ticks;
		}
	}
	__sleep(300L);
	}

	for (i = 0; i < thread_cnt; i++)
		if (threads[i].started) {
			if (threads[i].blocked)
				pthread_kill(threads[i].thread, SIGHUP);
			pthread_join(threads[i].thread, NULL);
		}

	for (i = 0; i < exitcall_cnt; i++)
		(exitcalls[i])();

	close_keyboard();

	free(sys_dir);
	free(sys_path);
	return 0;
}
