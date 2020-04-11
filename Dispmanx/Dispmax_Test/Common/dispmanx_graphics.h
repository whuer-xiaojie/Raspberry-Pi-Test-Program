#ifndef DISPMANX_GRAPHICS_H_
#define DISPMANX_GRAPHICS_H_

#include<bcm_host.h>
#include<stdint.h>


typedef struct IMAGE_T_
{
	VC_IMAGE_TYPE_T type;
	int32_t width;
	int32_t height;
	int32_t pitch;
	int32_t alignedHeight;
	uint16_t bitsPerPixel;
	uint32_t size;
	void *buffer;
}IMAGE_T;

typedef struct GRAPHICS_LAYER_T_ {
	IMAGE_T image;
	int32_t viewWidth;
	int32_t viewHeight;
	int32_t xOffsetMax;
	int32_t xOffset;
	int32_t yOffsetMax;
	int32_t yOffset;
	VC_RECT_T bmpRect;
	VC_RECT_T srcRect;
	VC_RECT_T dstRect;
	int32_t layer;
	DISPMANX_RESOURCE_HANDLE_T frontResource;
	DISPMANX_RESOURCE_HANDLE_T backResource;
	DISPMANX_ELEMENT_HANDLE_T element;
}GRAPHICS_LAYER_T;

int dispmanx_init_graphics(int32_t viewWidth, int32_t viewHeight);
int dispmanx_close_graphics(void);
void dispmanx_render_buffer_RGB888(unsigned char *buf);
#endif
