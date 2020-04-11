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

#include "dispResizer.h"

#ifndef ALIGN_TO_16
#define ALIGN_TO_16(x) ((x+15)&~15)
#endif

int init_disp_resizer(DISPRESIZER_T *resizer, int32_t srcW, int32_t srcH, VC_IMAGE_TYPE_T srcType,
	int32_t destW, int32_t destH, VC_IMAGE_TYPE_T destType, bool keepSrcRatio)
{
	assert(resizer != NULL);

	resizer->srcWidth = srcW;
	resizer->srcHeight = srcH;
	resizer->srcType = srcType;
	//resizer->destWidth = ALIGN_TO_16(destW);
	resizer->destWidth = destW;
	resizer->destHeight = destH;
	resizer->destType = destType;

	resizer->xRatio = (((int32_t)srcW << 16) / destW) + 1;
	resizer->yRatio = (((int32_t)srcH << 16) / destH) + 1;

	if (keepSrcRatio){
		if (resizer->xRatio < resizer->yRatio){
			resizer->xRatio = resizer->yRatio;
			resizer->destWidth = ALIGN_TO_16((srcW*resizer->destHeight) / srcH);
		}
		else{
			resizer->yRatio = resizer->xRatio;
			resizer->destHeight = (resizer->destWidth*srcH) / srcW;
		}
	}

	bcm_host_init();

	uint32_t srcImagePtr;
	uint32_t destImagePtr;

	resizer->destResource = vc_dispmanx_resource_create(destType, resizer->destWidth, resizer->destHeight,
		                                                &destImagePtr);
	assert(resizer->destResource != 0);

	resizer->srcResource = vc_dispmanx_resource_create(srcType, srcW, srcH, &srcImagePtr);
	assert(resizer->srcResource != 0);

	resizer->display = vc_dispmanx_display_open_offscreen(resizer->destResource, DISPMANX_NO_ROTATE);
	assert(resizer->display != 0);

	vc_dispmanx_rect_set(&resizer->destRect, 0, 0, resizer->destWidth, resizer->destHeight);
	vc_dispmanx_rect_set(&resizer->srcRect, 0, 0, srcW, srcH);
	vc_dispmanx_rect_set(&resizer->tempRect, 0, 0, srcW << 16, srcH << 16);

}

int disp_resize_image(DISPRESIZER_T *resizer, uint8_t *srcPixel, uint8_t *destPixel)
{
	assert(resizer != NULL && srcPixel != NULL && destPixel != NULL);

	int32_t srcPatch = ALIGN_TO_16(resizer->srcWidth) * 3;
	int32_t destPatch = ALIGN_TO_16(resizer->destWidth) * 3;
	vc_dispmanx_resource_write_data(resizer->srcResource, resizer->srcType, srcPatch, srcPixel, &resizer->srcRect);

	VC_DISPMANX_ALPHA_T alpha = { DISPMANX_NO_HANDLE,
		                          DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_MIX,
		                          255 };

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	assert(update != 0);

	DISPMANX_ELEMENT_HANDLE_T element;
	element = vc_dispmanx_element_add(update, resizer->display, 0, &resizer->destRect,
		                              resizer->srcResource, &resizer->tempRect,
		                              DISPMANX_PROTECTION_NONE,&alpha, NULL, DISPMANX_NO_ROTATE);
	assert(element != 0);
	vc_dispmanx_update_submit_sync(update);

	vc_dispmanx_resource_read_data(resizer->destResource, &resizer->destRect, destPixel, destPatch);

	vc_dispmanx_element_remove(update, element);
}

int destory_disp_resizer(DISPRESIZER_T *resizer)
{
	assert(resizer != NULL);

	vc_dispmanx_display_close(resizer->display);
	vc_dispmanx_resource_delete(resizer->srcResource);
	vc_dispmanx_resource_delete(resizer->destResource);
}

int dispmanx_resize_RGB24(uint8_t *src_image, int32_t src_width, int32_t src_height,
	                      uint8_t *dest_image, int32_t dest_width, int32_t dest_height)
{
	assert(src_image != NULL && dest_image != NULL);
	
	if ((src_width == dest_width) && (src_height == dest_height)){
		memcpy(dest_image, src_image, dest_width*dest_height * 3);
		return 0;
	}

	int32_t srcPatch = 0;
	int32_t destPatch = 0;
	int32_t srcLineBytes = src_width * 3;
	int32_t destLineBytes = dest_width * 3;
	uint8_t *sbuf = NULL;
	uint8_t *dbuf = NULL;
	int32_t sOffset = 0;
	int32_t dOffset = 0;
	int i;

	if (ALIGN_TO_16(src_width) != src_width){
		srcPatch = ALIGN_TO_16(src_width) * 3;
		fprintf(stderr, "%s():srcPatch=%d\n", __FUNCTION__, srcPatch);
		sbuf = (uint8_t *)malloc(srcPatch*src_height);
		assert(sbuf != NULL);
		for (i = 0; i < src_height; i++){
			memcpy(sbuf+dOffset, src_image+sOffset, srcLineBytes);
			dOffset += srcPatch;
			sOffset += srcLineBytes;
		}
	}
	else
		sbuf = src_image;

	if (ALIGN_TO_16(dest_width) != dest_width){
		destPatch = ALIGN_TO_16(dest_width) * 3;
		fprintf(stderr, "%s():destPAtch=%d\n", __FUNCTION__, destPatch);
		dbuf = (uint8_t *)malloc(destPatch*dest_height);
		assert(dbuf != NULL);
	}
	else
		dbuf = dest_image;

	DISPRESIZER_T resizer;

	init_disp_resizer(&resizer, src_width, src_height, VC_IMAGE_RGB888, dest_width, dest_height, VC_IMAGE_RGB888, false);

	disp_resize_image(&resizer, sbuf, dbuf);

	destory_disp_resizer(&resizer);

	if (srcPatch != NULL && sbuf != NULL){
		free(sbuf);
		sbuf = NULL;
	}

	if (destPatch != NULL && dbuf != NULL){
		dOffset = sOffset = 0;
		for (i = 0; i < dest_height; i++){
			memcpy(dest_image + dOffset, dbuf + sOffset, destPatch);
			dOffset += destLineBytes;
			sOffset += destPatch;
		}

		free(dbuf); dbuf = NULL;
	}

	fprintf(stderr, "%s():srcW=%d srcH=%d dstW=%d dstH=%d\n", __FUNCTION__, src_width, src_height, dest_width, dest_height);
	return 0;
}