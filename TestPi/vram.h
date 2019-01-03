/*
 * pc/vram.h
 */

#ifndef PC_VRAM_H
#define PC_VRAM_H

enum {
	PC_VRAM_REGION_STAT_0,
	PC_VRAM_REGION_STAT_1,	/* last frame sent */
	PC_VRAM_REGION_STAT_2	/* gone */
};

typedef struct pc_vram_region_data_s {
	unsigned long addr;
	unsigned long size;
	unsigned long time;
	int last;
	int sent;
	struct pc_vram_region_data_s *next;
} pc_vram_region_data_t;

typedef struct pc_vram_region_s {
	unsigned short id;
	int state;
	pc_vram_region_data_t *region_data_list;
} pc_vram_region_t;

typedef struct pc_vram_s {
	unsigned long addr;
	unsigned long size;
	struct pc_vram_s *next;
} pc_vram_t;

struct FOLD_LINE
{
	int x;
	int y;
	int width;
	int height;
};

struct FOLD_DISPLAY
{
	int showed;
	int n_lines;
	struct FOLD_LINE *fold_lines;

};
void *pc_vram_main(void *arg);

#endif	/* PC_VRAM_H */
