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
#include <stdlib.h>

#include "../common/dispRender.h"
#include "mypng.h"
#include "resize.h"

#include "../dispResizer/dispResizer.h"
#include "../common/clock.h"

static int showWidth = 800;
static int showHeight = 600;

static void usage(char *program)
{
	printf("Usage: %s   <-w/W width> <-h/H height > <png file path>", program);
}

int main(int argc, char **argv)
{
	int opt = 0;

	while ((opt = getopt(argc, argv, "W:w:H:h:")) != -1){
		switch (opt){
		case 'W':
		case 'w':
			showWidth = atoi(optarg);
			break;
		case 'H':
		case 'h':
			showHeight = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (optind >= argc){
		usage(argv[0]);
		return -1;
	}

	const char *filePath = argv[optind];

	/*******************************************************************/

	unsigned char *buf = NULL;
	if ((buf = (unsigned char *)malloc(showWidth*showHeight * 3)) == NULL){
		return 0;
	}
	DISPRENDER_T render;

	init_dispmanx_render(&render, showWidth, showHeight);

	unsigned char *src_buf = NULL;
	int src_width, src_height;

	if (decode_png_rgb888(filePath, &src_buf, &src_width, &src_height) != 0){
		destory_dispmanx_render(&render);
		if (src_buf) free(src_buf);
		if (buf) free(buf);
		return 0;
	}

	//resize_image(src_buf, src_width * 3, src_width, src_height, buf, showWidth, showHeight);
	unsigned long t1 = get_clock_ms();
	dispmanx_resize_RGB24(src_buf, src_width, src_height, buf, showWidth, showHeight);
	fprintf(stderr, "%s():resize time=%lu ms\n", __FUNCTION__, get_clock_ms() - t1);

	render_image_rgb888(&render, buf);

	sleep(10);

	destory_dispmanx_render(&render);
	if (src_buf) free(src_buf);
	if (buf) free(buf);
	return 0;
}