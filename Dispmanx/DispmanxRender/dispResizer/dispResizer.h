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

#ifndef DISPRESIZER_H_
#define DISPRESIZER_H_

#include <bcm_host.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct DISPRESIZER_T_{
	int32_t srcWidth;
	int32_t srcHeight;
	int32_t destWidth;
	int32_t destHeight;
	int32_t xRatio;
	int32_t yRatio;
	VC_IMAGE_TYPE_T srcType;
	VC_IMAGE_TYPE_T destType;
	VC_RECT_T srcRect;
	VC_RECT_T destRect;
	VC_RECT_T tempRect;
	DISPMANX_RESOURCE_HANDLE_T destResource;
	DISPMANX_RESOURCE_HANDLE_T srcResource;
	DISPMANX_DISPLAY_HANDLE_T display;
}DISPRESIZER_T;

int init_disp_resizer(DISPRESIZER_T *resizer, int32_t srcW, int32_t srcH, VC_IMAGE_TYPE_T srcType,
	                  int32_t destW, int32_t destH, VC_IMAGE_TYPE_T destType, bool keepSrcRatio);

int disp_resize_image(DISPRESIZER_T *resizer, uint8_t *srcPixel, uint8_t *destPixel);

int destory_disp_resizer(DISPRESIZER_T *resizer);

int dispmanx_resize_RGB24(uint8_t *src_image, int32_t src_width, int32_t src_height,
	                      uint8_t *dest_image, int32_t dest_width, int32_t dest_height);
#endif // DISPRESIZER_H_