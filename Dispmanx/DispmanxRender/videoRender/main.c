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
#include "../common/keyboard.h"
#include "../common/clock.h"

#include "video.h"

static int showWidth = 800;
static int showHeight = 600;

static void usage(char *program)
{
	printf("Usage: %s   <-w/W width> <-h/H height > <-t/T 0/1 show decode and render time > <video file path>", program);
}

int main(int argc, char **argv)
{
	int opt = 0;
	int showTime = 0;

	while ((opt = getopt(argc, argv, "W:w:H:h:t:T:")) != -1){
		switch (opt){
		case 'W':
		case 'w':
			showWidth = atoi(optarg);
			break;
		case 'H':
		case 'h':
			showHeight = atoi(optarg);
			break;
		case 'T':
		case 't':
			showTime = atoi(optarg);
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
	/*******************************************************************/

	unsigned char *buf = NULL;
	if ((buf = (unsigned char *)malloc(showWidth*showHeight * 3)) == NULL){
		return 0;
	}
	const char *filePath = argv[optind];

	DISPRENDER_T render;
	VIDEO_DECODER_T decoder;
	char c;
	int quit = 0;

	/*******************************************************************/
	init_video_decoder(&decoder, filePath, showWidth, showHeight);

	init_dispmanx_render(&render, showWidth, showHeight);
    
	if (init_keyboard() == 0){
		printf("Press 'q/Q' or 'Esc' to quit!\n");
	}

	while (!quit){

		//the keyboard input q or Q to quit the program
		if (((c = __getch()) >= 0) && (c == 'q' || c == 'Q' || c == 27))
			break;
		
		unsigned long t1 = get_clock_ms();
		if (decode_video_next_frame(&decoder, buf) != 0)
			break;

		unsigned long t2 = get_clock_ms();
		//render_image_rgb888(&render, buf);
		render_image_rgb888(&render, decoder.pFrameRGB->data[0]);

		unsigned long t3 = get_clock_ms();

		if (showTime == 1)
			fprintf(stderr, "%s():decode time=%lu ms  render time=%lu ms\n", __FUNCTION__, t2 - t1, t3 - t2);
		//sleep_ms(decoder.delay_ms);
	}
	
	close_keyboard();
	destory_dispmanx_render(&render);
	if (buf != NULL) free(buf);
	return 0;
}