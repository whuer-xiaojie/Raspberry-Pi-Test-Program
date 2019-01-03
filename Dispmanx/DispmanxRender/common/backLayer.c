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
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"backLayer.h"

void init_back_layer(BACK_LAYER_T *bg, int32_t layer)
{
	assert(bg != NULL && layer >= 0);

	VC_IMAGE_TYPE_T type = VC_IMAGE_RGB888;
	uint8_t backColor[3] = { 0xFF,0xFF,0xFF };//red,green,blue
	uint32_t vc_image_ptr;
	int result = 0;

	bg->layer = layer;

	bg->resource = vc_dispmanx_resource_create(type, 1, 1, &vc_image_ptr);
	assert(bg->resource != 0);
		
	
	VC_RECT_T rect;
	vc_dispmanx_rect_set(&rect, 0, 0, 1, 1);

	result = vc_dispmanx_resource_write_data(bg->resource, type, 3, backColor, &rect);
	assert(result==0);
}

void add_back_layer_element(BACK_LAYER_T *bg, DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_UPDATE_HANDLE_T update)
{
	assert(bg != NULL && display != 0 && update != 0);

	VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE,255,0 };

	VC_RECT_T src_rect;
	vc_dispmanx_rect_set(&src_rect, 0, 0, 1, 1);

	VC_RECT_T dst_rect;
	vc_dispmanx_rect_set(&dst_rect, 0, 0, 0, 0);

	bg->element = vc_dispmanx_element_add(update, display, bg->layer, &dst_rect, bg->resource, &src_rect,
		                                  DISPMANX_PROTECTION_NONE, &alpha, NULL, DISPMANX_NO_ROTATE);
	assert(bg->element != NULL);
}

void destory_back_layer(BACK_LAYER_T *bg)
{
	assert(bg != NULL);
	int result = 0;

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	assert(update != NULL);

	result = vc_dispmanx_element_remove(update, bg->element); 
	assert(result == 0);

	result = vc_dispmanx_update_submit_sync(update); 
	assert(result == 0);
	
	result = vc_dispmanx_resource_delete(bg->resource);
	assert(result == 0);
}