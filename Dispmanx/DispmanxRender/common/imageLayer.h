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

#ifndef IMAGELAYER_H_
#define IMAGELAYER_H_

#include <bcm_host.h>
#include <stdint.h>

#ifndef ALIGN_TO_16
#define ALIGN_TO_16(x)  ((x + 15) & ~15)
#endif

#define BITS_PER_PIXEL 24

typedef struct IMAGE_T_{
	int32_t width;
	int32_t height;
	int32_t patch;
	int32_t size;
	uint8_t *colorBuf;
}IMAGE_T;

typedef struct IMAGE_LAYER_T_{
	IMAGE_T image;
	int32_t layer;
	VC_RECT_T srcRect;
	VC_RECT_T dstRect;
	DISPMANX_RESOURCE_HANDLE_T resource;
	DISPMANX_ELEMENT_HANDLE_T element;
}IMAGE_LAYER_T;

void init_image_layer(IMAGE_LAYER_T *il, VC_RECT_T *pos, int32_t layer);

void add_image_layer_element(IMAGE_LAYER_T *il, DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_UPDATE_HANDLE_T update);

void change_image_layer_source(IMAGE_LAYER_T *il, DISPMANX_UPDATE_HANDLE_T update, 
	                           unsigned char *buf, int width, int height);

void destory_image_layer(IMAGE_LAYER_T *il);


#endif // IMAGELAYER_H_
