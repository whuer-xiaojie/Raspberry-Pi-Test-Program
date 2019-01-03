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
#include <assert.h>

#include "imageLayer.h"

static int init_image(IMAGE_T *image, int width, int height)
{
	assert(image != NULL && width > 0 && height > 0);

	image->width = width;
	image->height = height;
	image->patch = (ALIGN_TO_16(width)*BITS_PER_PIXEL) / 8;
	image->size = image->patch*image->height;

	image->colorBuf = (uint8_t *)malloc(image->size);
	assert(image->colorBuf != NULL);

	memset(image->colorBuf, 255, image->size);
}
static void set_image_pixel(IMAGE_T *image, unsigned char*buf, int32_t width, int32_t height)
{
	assert(image != NULL && buf != NULL);

	if (image->width != width || image->height != height)
		return;

	int i;
	int line_bytes = width * 3;
	int line_offset = 0;
	int image_offset = 0;
	for (i = 0; i < height; i++){
		memcpy(image->colorBuf + image_offset, buf + line_offset, line_bytes);
		image_offset += image->patch;
		line_offset += line_bytes;
	}
}

static void destory_image(IMAGE_T *image)
{
	assert(image != NULL);
	if (image->colorBuf != NULL){
		free(image->colorBuf);
		image->colorBuf = NULL;
	}
}

void init_image_layer(IMAGE_LAYER_T *il, VC_RECT_T *pos, int32_t layer)
{
	assert(il != NULL && pos != NULL && layer >= 0);

	uint32_t vc_image_ptr;
	int result = 0;

	il->layer = layer;

	il->resource = vc_dispmanx_resource_create(VC_IMAGE_RGB888, pos->width, pos->height, &vc_image_ptr);
	assert(il->resource != 0);

	//vc_dispmanx_rect_set(&(il->srcRect), 0 << 16, 0 << 16, pos->width << 16, pos->height << 16);
	vc_dispmanx_rect_set(&(il->srcRect), 0, 0, pos->width, pos->height);
	vc_dispmanx_rect_set(&(il->dstRect), pos->x, pos->y, pos->width, pos->height);

	init_image(&il->image, pos->width, pos->height);
	
	//here will set the real display area be white
	result = vc_dispmanx_resource_write_data(il->resource, VC_IMAGE_RGB888, il->image.patch, il->image.colorBuf, &il->srcRect);
	assert(result == 0);

}

void add_image_layer_element(IMAGE_LAYER_T *il, DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_UPDATE_HANDLE_T update)
{
	assert(il != NULL && display != 0 && update != 0);
	VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE, 255/*alpha 0->255*/, 0 };

	il->element = vc_dispmanx_element_add(update, display, il->layer,
		                                  &(il->dstRect), il->resource, &(il->srcRect), 
										  DISPMANX_PROTECTION_NONE, &alpha, 
										  NULL/*clamp*/, DISPMANX_NO_ROTATE);

	assert(il->element != 0);
}

void change_image_layer_source(IMAGE_LAYER_T *il, DISPMANX_UPDATE_HANDLE_T update, unsigned char *buf, int width, int height)
{
	assert(il != NULL && buf != NULL);

	int result = 0;

	set_image_pixel(&il->image, buf, width, height);

	result = vc_dispmanx_resource_write_data(il->resource, VC_IMAGE_RGB888, il->image.patch, il->image.colorBuf, &il->srcRect);
	assert(result == 0);

	result = vc_dispmanx_element_change_source(update, il->element, il->resource);
	assert(result == 0);
}

void destory_image_layer(IMAGE_LAYER_T *il)
{
	assert(il != NULL);

	int result = 0;

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	assert(update != 0);

	result = vc_dispmanx_element_remove(update, il->element);
	assert(result == 0);

	result = vc_dispmanx_update_submit_sync(update);
	assert(result == 0);

	result = vc_dispmanx_resource_delete(il->resource);
	assert(result == 0);

	destory_image(&il->image);

}