#ifndef BACK_LAYER_H_
#define BACK_LAYER_H_

#include <bcm_host.h>
#include <stdint.h>

typedef struct BACK_LAYER_T_ {
	int32_t layer;
	DISPMANX_RESOURCE_HANDLE_T resource;
	DISPMANX_ELEMENT_HANDLE_T element;
}BACK_LAYER_T;

void init_back_layer(BACK_LAYER_T *bg, int32_t layer);
void add_back_layer_element(BACK_LAYER_T *bg, DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_UPDATE_HANDLE_T update);
void free_back_layer(BACK_LAYER_T *bg);
#endif

