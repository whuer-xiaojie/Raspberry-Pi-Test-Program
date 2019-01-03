/*
 * pc/vram.c
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../keyboard.h"
#include "../clock.h"
#include "../config.h"
#include "../debug.h"
#include "../libunix.h"
#include "../region.h"
#include "../vram.h"
#include "graphics.h"
#include "vram.h"


/* ../box.c */
extern int sign_width;
extern int sign_height;

/* ../display.c */
extern size_t pixel_size;

/* ../main.c */
extern int graphics_mode;
extern int quit;

int vram_error = 0;

unsigned char *vram_base = NULL;

static size_t pc_vram_bufsize;
static unsigned char *pc_vram_buf = NULL;

static pc_vram_region_t *pc_vram_region_list = NULL;
static int pc_vram_region_list_size = 0;
static pthread_mutex_t pc_vram_region_list_mutex = PTHREAD_MUTEX_INITIALIZER;

static pc_vram_t *pc_vram_list = NULL;

/* options */
int vram_error_action;

/* fold displayed */
struct FOLD_DISPLAY *fold_display = NULL;
int view_width;
int view_height;
static unsigned char *vram_fold_buf = NULL;
static size_t vram_fold_buf_size;


void vram_read_config(void)
{
	vram_error_action = query_signed_value("vram_error_action",
		ACTION_NONE, 0, ACTION_MAX - 1);

	int showed; int lines;
	showed = query_signed_value("fold_display", 0, 0, 1);
	//fprintf(stderr, "%s():showed = %d \n", __FUNCTION__, showed);
	if (showed!=0)
	{
		fold_display = (struct FOLD_DISPLAY *)malloc(sizeof(struct FOLD_DISPLAY));
		if (fold_display==NULL)
		{
			fprintf(stderr, "%s():malloc for fold_display failed \n", __FUNCTION__);
			return;
		}
		fold_display->fold_lines = NULL;
		fold_display->showed = showed;
		lines = query_signed_value("fold_lines", 1, 1, 100);/* read fold lines */
		//fprintf(stderr, "%s(): lines = %d \n", __FUNCTION__, lines);
		fold_display->n_lines = lines;
		if (lines>1)
		{
			fold_display->fold_lines = (struct FOLD_LINE *)malloc(sizeof(struct FOLD_LINE)*lines);
			if (fold_display->fold_lines==NULL)
			{
				fprintf(stderr, "%s():malloc for fold lines failed \n", __FUNCTION__);
				free(fold_display); fold_display == NULL;
				return;
			}
			int i;
			for (i = 0; i < lines;i++)
			{
				char line_name[MAXSTRLEN]; char line_info[MAXSTRLEN]; char * lines;
				sprintf(line_name, "%s%d", "fold_line", i);

				lines = query_string_value(line_name);

				tokenize(line_info, lines, ',', 0);
				trim(line_info);
				fold_display->fold_lines[i].x = atoi(line_info);

				tokenize(line_info, lines, ',', 1);
				trim(line_info);
				fold_display->fold_lines[i].y = atoi(line_info);

				tokenize(line_info, lines, ',', 2);
				trim(line_info);
				fold_display->fold_lines[i].width = atoi(line_info);

				tokenize(line_info, lines, ',',3);
				trim(line_info);
				fold_display->fold_lines[i].height = atoi(line_info);	

				//fprintf(stderr, "%s():line%d  : x=%d  y=%d  w=%d   h=%d \n,", __FUNCTION__, i, fold_display->fold_lines[i].x,
				//	fold_display->fold_lines[i].y, fold_display->fold_lines[i].width, fold_display->fold_lines[i].height);
			}
		}
		else
		{
			free(fold_display);
			fold_display = NULL;
		}
	}
}

void vram_save_config(void)
{
	save_string_value("#vram", "=========");

	save_signed_value("vram_error_action", vram_error_action);

	/* save the fold display information */
	save_string_value("#fold_display", "=========");

	if (fold_display!=NULL)
	{
		save_signed_value("fold_display", fold_display->showed);
		save_signed_value("fold_lines", fold_display->n_lines);
		if (fold_display->n_lines>0)
		{
			int i;
			for (i = 0; i < fold_display->n_lines;i++)
			{
				char line_info[MAXSTRLEN]; char line_name[MAXSTRLEN];
				sprintf(line_name, "%s%d", "fold_line", i);
				sprintf(line_info, "%d,%d,%d,%d", fold_display->fold_lines[i].x, fold_display->fold_lines[i].y,
					fold_display->fold_lines[i].width, fold_display->fold_lines[i].height);
				save_string_value(line_name, line_info);
			}	
		}
	}
}

/************************************************************************/
static void vram_get_fold_viewport_size(struct FOLD_DISPLAY *fold_display)
{
	if (fold_display==NULL)
	{
		//fprintf(stderr, "%s():the fold display struct is NULL \n", __FUNCTION__);
		view_width = sign_width;
		view_height = sign_height;
		return;
	}
	if (fold_display->n_lines<=1 || fold_display->showed==0)
	{
		view_width = sign_width;
		view_height = sign_height;
		return;
	}
	int i; int min_x, max_y, max_width, max_height;
	min_x      = fold_display->fold_lines[0].x;
	max_y      = fold_display->fold_lines[fold_display->n_lines - 1].y;/* the last line start y */
	max_width  = fold_display->fold_lines[0].width;
	max_height = fold_display->fold_lines[fold_display->n_lines - 1].height; /* the last line height */
	view_width = min_x + max_width;
	for (i = 0; i < (fold_display->n_lines); i++)
	{
		if ((fold_display->fold_lines[i].x + fold_display->fold_lines[i].width) > view_width)
		{
			view_width = fold_display->fold_lines[i].x + fold_display->fold_lines[i].width;
		}
		if ((fold_display->fold_lines[i].width) > max_width)
		{
			max_width = fold_display->fold_lines[i].width;
		}
	}

	view_height = max_y + max_height;
	//fprintf(stderr, "%s(): view_width=%d  view_height=%d sign_width=%d sign_height=%d\n", __FUNCTION__,view_width, view_height, sign_width, sign_height);

	if (view_width > 800 || view_height > 600)
	{
		fprintf(stderr, "%s():the view port out of the window range \n", __FUNCTION__);

	}
}
static int fold_display_init(void)
{
	if ((fold_display!=NULL ) && (fold_display->showed!=0))
	{
		//fprintf(stderr, "%s(): need fold display \n", __FUNCTION__);
		vram_get_fold_viewport_size(fold_display);
		vram_fold_buf_size = view_width*view_height*pixel_size;
		if ((vram_fold_buf = (unsigned char *)malloc(vram_fold_buf_size)) == NULL)
		{
			DPRINT("%s(): malloc for fold buffer failed", __FUNCTION__);
			set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
			return -1;
		}
	}
	else
	{
		view_width  = sign_width;
		view_height = sign_height;
		vram_fold_buf_size = view_width*view_height*pixel_size;
		if ((vram_fold_buf = (unsigned char *)malloc(vram_fold_buf_size)) == NULL)
		{
			DPRINT("%s(): malloc for fold buffer failed", __FUNCTION__);
			set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
			return -1;
		}
	}
	return 0;
}
static void fold_display_close(void)
{
	if (fold_display!=NULL)
	{
		if (fold_display->fold_lines != NULL)
		{
			free(fold_display->fold_lines);
			fold_display->fold_lines = NULL;
		}
		free(fold_display);
		fold_display = NULL;
	}
	if (vram_fold_buf!=NULL)
	{
		free(vram_fold_buf);
		vram_fold_buf = NULL;
	}
}
static void vram_cal_fold_buf(unsigned char *src_buf)
{
       memset(vram_fold_buf, 0, vram_fold_buf_size);
	int src_len = strlen(src_buf);
	if ((view_width == sign_width && view_height == sign_height && src_len==vram_fold_buf_size)
		|| fold_display==NULL  || (fold_display->showed==0) || (fold_display->n_lines<=0))/* not need fold display */
	{
		memcpy(vram_fold_buf, src_buf, src_len);
		return;
	}
	int i; int x; int y;
	int fold_offset;int fold_start_offset; int fold_line_size;
	int src_offset_x = 0; int src_offset_x_byte; int src_offset;

	int src_line_byte = sign_width*pixel_size;/* the source screen one line pixel bytes*/
	int fold_line_byte = view_width*pixel_size; /* the folded screen one line pixel bytes */
	//fprintf(stderr, "%s():src_line_byte=%d  fold_line_byte=%d\n", __FUNCTION__, src_line_byte, fold_line_byte);

	for (i = 0; i < fold_display->n_lines; i++) /* calculate one folded part pixel info */
	{
		fold_start_offset = fold_line_byte*(fold_display->fold_lines[i].y) + (fold_display->fold_lines[i].x)*pixel_size;
		fold_line_size = fold_display->fold_lines[i].width*pixel_size;

		src_offset_x_byte =src_offset_x*pixel_size;
		src_offset_x += fold_display->fold_lines[i].width;
		//fprintf(stderr, "%s(): fold_start_offset=%d  fold_line_size=%d  src_offset_x_byte=%d src_offset_x=%d \n", __FUNCTION__,fold_start_offset, fold_line_size, src_offset_x_byte, src_offset_x);

		for (y = 0; y < fold_display->fold_lines[i].height;y++)
		{
			fold_offset = ( y * fold_line_byte )+fold_start_offset;
			src_offset = ( src_offset_x_byte  +  y * src_line_byte );
			//fprintf(stderr, "%s(): fold_offset=%d  src_offset=%d \n", __FUNCTION__, fold_offset, src_offset);

			memcpy(vram_fold_buf+fold_offset, src_buf+src_offset, fold_line_size);
		}
	}

}
/****************************************************************************/

int vram_init(void)
{
	pc_vram_bufsize = pixel_size * sign_width * sign_height;
	if ((pc_vram_buf = (unsigned char *)malloc(pc_vram_bufsize)) == NULL) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		return -1;
	}
	if ((fold_display_init()) < 0)
		return -1;
	return 0;
}

int vram_close(void)
{
	if (pc_vram_buf)
		free(pc_vram_buf);
	fold_display_close();
	return 0;
}

/****************************************************************************/

static void pc_vram_blit_region_buffer(const unsigned char *buf,
				       unsigned long size)
{
	unsigned char blend_mode = buf[11];

	if (!pc_vram_buf)
		return;

	if ((blend_mode & BLEND_ALPHA) && (blend_mode & BLEND_COLORKEY))
		blit_region_buffer_alpha_colorkey(pc_vram_buf, buf, size);
	else if (blend_mode & BLEND_ALPHA)
		blit_region_buffer_alpha(pc_vram_buf, buf, size);
	else if (blend_mode & BLEND_COLORKEY)
		blit_region_buffer_colorkey(pc_vram_buf, buf, size);
	else
		blit_region_buffer_normal(pc_vram_buf, buf, size);
}

static void pc_vram_check_regions(unsigned long n_ticks)
{
	int i;
	int all_sent = 1;
	pc_vram_region_data_t *head;
	pc_vram_region_t *region;

	pthread_mutex_lock(&pc_vram_region_list_mutex);

	if (pc_vram_region_list_size == 0)
		goto out;

	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].state != PC_VRAM_REGION_STAT_1)
			break;
	if (i >= pc_vram_region_list_size)	/* stopped */
		goto out;

	for (i = 0; i < pc_vram_region_list_size; i++) 
	{
		head = pc_vram_region_list[i].region_data_list;
		if (!head || time_after(head->time, n_ticks))/*now time < data time*/
			goto out;
		if (head->sent/*current data had showed*/
			&& head->next/*next data not NULL*/
			&& time_before_eq(head->next->time, n_ticks)) /*head next time >= now time*/
		{
			pc_vram_region_list[i].region_data_list = head->next;/*update current data*/
			free(head);
		}
		if (!pc_vram_region_list[i].region_data_list->sent)
			all_sent = 0;
	}

	if (all_sent)
		goto out;

	if (pc_vram_buf)
		memset(pc_vram_buf, 0, pc_vram_bufsize);
	/*build all region current show info buffer*/
	for (i = 0; i < pc_vram_region_list_size; i++) {
		region = &pc_vram_region_list[i];
		head = region->region_data_list;
		pc_vram_blit_region_buffer((unsigned char *)head->addr,
					   head->size);
		if (region->state == PC_VRAM_REGION_STAT_0 && head->last)
			region->state = PC_VRAM_REGION_STAT_1;
		head->sent = 1;/*current region data showed*/
	}

	pthread_mutex_unlock(&pc_vram_region_list_mutex);
   
	vram_cal_fold_buf(pc_vram_buf);
      //fprintf(stderr,"%s():finish blit region buffer!\n",__FUNCTION__);
	if (graphics_mode)
		if (pc_vram_buf)
			render_buffer(vram_fold_buf);
	return;

out:
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
}

void *pc_vram_main(void *arg)
{
	unsigned long n_ticks;
	unsigned long pc_vram_check_regions_ticks;
#ifdef __X9__
	if(graphics_mode)
	{
		if (init_graphics()==0)
		{
			x9_create_texture();
		}
	}
		
#endif
	n_ticks = __clock();
	pc_vram_check_regions_ticks = n_ticks;
char c;
	while (!quit) 
	{
		n_ticks = __clock();

		if (n_ticks - pc_vram_check_regions_ticks >= 2L) 
		{
			//fprintf(stderr,"%s():start pc_vram_check_regions!\n",__FUNCTION__);
			pc_vram_check_regions(n_ticks);
			pc_vram_check_regions_ticks = n_ticks;
		}
	}

#ifdef __X9__
	if (graphics_mode)
	{
		x9_close_texture();
		close_graphics();
	}
		
#endif

	fprintf(stderr, "%s() terminated\n", __FUNCTION__);
	return NULL;
}

/****************************************************************************/

int vram_set_mode(int mode)
{
	return 0;
}

int vram_set_frequency(int frequency)
{
	return 0;
}

static void pc_vram_free_regions(void)
{
	int i;
	pc_vram_region_data_t *head;
	pc_vram_region_data_t *node;

	if (!pc_vram_region_list)
		return;

	for (i = 0; i < pc_vram_region_list_size; i++) {
		head = pc_vram_region_list[i].region_data_list;
		while (head) {/*free current region all data*/
			node = head;
			head = node->next;
			free(node);
		}
	}

	free(pc_vram_region_list);
	pc_vram_region_list = NULL;
	pc_vram_region_list_size = 0;
}

int vram_replace_regions(const unsigned char *buf)
{
	int ret = -1;
	size_t size;
	int i;

	if (buf == NULL)
		return -1;

	pthread_mutex_lock(&pc_vram_region_list_mutex);

	pc_vram_free_regions();

	pc_vram_region_list_size = GET_BE16(buf);
	if (pc_vram_region_list_size == 0)
		goto out_succeeded;

	size = sizeof(pc_vram_region_t) * pc_vram_region_list_size;
	if ((pc_vram_region_list = (pc_vram_region_t *)malloc(size)) == NULL) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		pc_vram_region_list_size = 0;
		goto out_failed;
	}

	buf += 2;
	for (i = 0; i < pc_vram_region_list_size; i++) {
		pc_vram_region_list[i].id = GET_BE16(buf);
		pc_vram_region_list[i].state = PC_VRAM_REGION_STAT_0;
		pc_vram_region_list[i].region_data_list = NULL;
		buf += 2;
	}

out_succeeded:
	ret = 0;

out_failed:
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return ret;
}

int vram_update_regions(const unsigned char *buf)
{
	int ret = -1;
	int reset_state;
	int n_regions, i, j;
	size_t size;
	pc_vram_region_t *region_list;

	if (buf == NULL)
		return -1;

	pthread_mutex_lock(&pc_vram_region_list_mutex);

	reset_state = buf[0];

	n_regions = GET_BE16(buf + 1);
	if (n_regions == 0) {
		pc_vram_free_regions();
		goto out_succeeded;
	}

	size = sizeof(pc_vram_region_t) * n_regions;
	if ((region_list = (pc_vram_region_t *)malloc(size)) == NULL) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		goto out_failed;
	}

	buf += 3;
	for (i = 0; i < n_regions; i++) {
		region_list[i].id = GET_BE16(buf);
		for (j = 0; j < pc_vram_region_list_size; j++)
			if (pc_vram_region_list[j].id == region_list[i].id)
				break;
		if (j >= pc_vram_region_list_size) {	/* new region */
			region_list[i].state = PC_VRAM_REGION_STAT_0;
			region_list[i].region_data_list = NULL;
		} else {	/* old region */
			region_list[i] = pc_vram_region_list[j];
			if (reset_state)
				region_list[i].state = PC_VRAM_REGION_STAT_0;
			else if (region_list[i].state > PC_VRAM_REGION_STAT_1)
				region_list[i].state = PC_VRAM_REGION_STAT_1;
			pc_vram_region_list[j].region_data_list = NULL;
		}
		buf += 2;
	}

	pc_vram_free_regions();
	pc_vram_region_list = region_list;
	pc_vram_region_list_size = n_regions;

out_succeeded:
	ret = 0;

out_failed:
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return ret;
}

int vram_write(unsigned char *buf)
{
	int ret = -1;
	unsigned short id;
	unsigned long addr;
	unsigned long size;
	unsigned long time;
	int last;
	pc_vram_region_data_t *tail;
	int i;
	pc_vram_region_data_t *node;

	if (buf == NULL)
		return -1;

	id   = GET_BE16(buf);
	addr = GET_BE32(buf + 2);
	size = GET_BE32(buf + 6);
	time = GET_BE32(buf + 10);
	last = buf[14];

	tail = (pc_vram_region_data_t *)malloc(sizeof(pc_vram_region_data_t));
	if (!tail) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		return -1;
	}

	tail->addr = addr;
	tail->size = size;
	tail->time = time;
	tail->last = last;
	tail->sent = 0;
	tail->next = NULL;

	pthread_mutex_lock(&pc_vram_region_list_mutex);

	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].id == id)
			break;
	if (i >= pc_vram_region_list_size)
		goto out;

	node = pc_vram_region_list[i].region_data_list;
	if (!node) {
		PUT_BE32(buf, time);

		pc_vram_region_list[i].region_data_list = tail;
	} else {
		time = node->time;	/* head */
		PUT_BE32(buf, time);

		while (node->next)
			node = node->next;
		node->next = tail;
	}

	ret = 0;

out:
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	if (ret)
		free(tail);
	return ret;
}

int vram_set_last_frame(unsigned short id)
{
	int ret = -1;
	int i;
	pc_vram_region_t *region;
	pc_vram_region_data_t *tail;

	pthread_mutex_lock(&pc_vram_region_list_mutex);

	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].id == id)
			break;
	if (i >= pc_vram_region_list_size)
		goto out;

	region = &pc_vram_region_list[i];
	tail = region->region_data_list;
	if (!tail)
		goto out;

	while (tail->next)
		tail = tail->next;
	if (!tail->last) {
		tail->last = 1;
		if (region->state == PC_VRAM_REGION_STAT_0 && tail->sent)
			/* tail is head */
			region->state = PC_VRAM_REGION_STAT_1;
	}

	ret = 0;

out:
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return ret;
}

int vram_get_head_frame(unsigned short id, unsigned long *time)
{
	int ret = -1;
	int i;
	pc_vram_region_data_t *head;

	if (time == NULL)
		return -1;

	pthread_mutex_lock(&pc_vram_region_list_mutex);
	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].id == id) {
			head = pc_vram_region_list[i].region_data_list;
			if (head) {
				*time = head->time;
				ret = 0;
			}
			break;
		}
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return ret;
}

int vram_get_frame_count(unsigned short id)
{
	int ret = 0;
	int i;
	pc_vram_region_data_t *node;

	pthread_mutex_lock(&pc_vram_region_list_mutex);
	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].id == id) {
			node = pc_vram_region_list[i].region_data_list;
			while (node) {
				ret++;
				node = node->next;
			}
			break;
		}
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return ret;
}

int vram_stopped(void)
{
	int stopped = 1;
	int i;

	pthread_mutex_lock(&pc_vram_region_list_mutex);
	for (i = 0; i < pc_vram_region_list_size; i++)
		if (pc_vram_region_list[i].state != PC_VRAM_REGION_STAT_1) {
			stopped = 0;
			break;
		}
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return stopped;
}

int vram_go(void)
{
	int i;

	pthread_mutex_lock(&pc_vram_region_list_mutex);
	for (i = 0; i < pc_vram_region_list_size; i++)
		pc_vram_region_list[i].state = PC_VRAM_REGION_STAT_2;
	pthread_mutex_unlock(&pc_vram_region_list_mutex);
	return 0;
}

/****************************************************************************/

unsigned long vram_alloc(unsigned long size)
{
	void *addr;
	pc_vram_t *node;

	if (size == 0)
		return (unsigned long)-1;

	if ((addr = malloc((size_t)size)) == NULL) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		return (unsigned long)-1;
	}

	if ((node = (pc_vram_t *)malloc(sizeof(pc_vram_t))) == NULL) {
		DPRINT("%s(): not enough memory", __FUNCTION__);
		set_error(ERROR_SYSTEM, NOT_ENOUGH_MEMORY);
		free(addr);
		return (unsigned long)-1;
	}

	node->addr = (unsigned long)addr;
	node->size = size;
	node->next = pc_vram_list;

	pc_vram_list = node;
	return (unsigned long)addr;
}

int vram_free(unsigned long addr, unsigned long size)
{
	pc_vram_t *node;
	pc_vram_t *prev;

	if (!pc_vram_list)
		return -1;

	if (pc_vram_list->addr == addr) {
		node = pc_vram_list;
		pc_vram_list = node->next;
		free(node);
		free((void *)addr);
		return 0;
	}

	prev = pc_vram_list;
	node = prev->next;
	while (node) {
		if (node->addr == addr) {
			prev->next = node->next;
			free(node);
			free((void *)addr);
			return 0;
		}
		prev = node;
		node = prev->next;
	}

	return -1;
}

unsigned long used_vram_size(void)
{
	unsigned long size = 0;
	pc_vram_t *node = pc_vram_list;

	while (node) {
		size += node->size;
		node = node->next;
	}

	return size;
}
