#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"backLayer.h"

void init_back_layer(BACK_LAYER_T *bg, int32_t layer)
{
	assert(bg != NULL && layer >= 0);

	VC_IMAGE_TYPE_T type = VC_IMAGE_RGB888;
	uint8_t backColor[3] = { 0xFF,0xFF,0xFF };
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
	VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE,255,0 };

	VC_RECT_T src_rect;
	vc_dispmanx_rect_set(&src_rect, 0, 0, 1, 1);

	VC_RECT_T dst_rect;
	vc_dispmanx_rect_set(&dst_rect, 0, 0, 0, 0);

	bg->element = vc_dispmanx_element_add(update, display, bg->layer, &dst_rect, bg->resource, &src_rect,
		                                  DISPMANX_PROTECTION_NONE, &alpha, NULL, DISPMANX_NO_ROTATE);
	assert(bg->element != NULL);
}

void free_back_layer(BACK_LAYER_T *bg)
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