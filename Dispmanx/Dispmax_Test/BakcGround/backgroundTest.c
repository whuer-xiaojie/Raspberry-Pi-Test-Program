#include<stdio.h>
#include<stdlib.h>

#include"../Common/backLayer.h"
#include"../Common/dispmanx_graphics.h"

static int32_t width = 1920;
static int32_t height = 1080;

unsigned char *get_test_buf(int w, int h)
{
	unsigned char *buf = (unsigned char *)malloc(w*h * 3);
	if (buf == NULL) {
		fprintf(stderr, "%s():malloc failed\n", __FUNCTION__);
		return NULL;
	}

	int i = 0;
	for (i = 0; i < w*h * 3; i++)
		buf[i] = i % 255;

	return buf;
}
int main(int argc, int **argv)
{
	fprintf(stderr, "%s():000000000000000\n", __FUNCTION__);
	dispmanx_init_graphics(width, height);
	fprintf(stderr, "%s():111111111111111111\n", __FUNCTION__);
	unsigned char *buf = get_test_buf(width, height);
	dispmanx_render_buffer_RGB888(buf);
	fprintf(stderr, "%s():2222222222222222\n", __FUNCTION__);
	sleep(10);

	dispmanx_close_graphics();

	return 0;
}