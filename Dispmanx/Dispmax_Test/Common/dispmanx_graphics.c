#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"backLayer.h"
#include"dispmanx_graphics.h"

#ifndef ELEMENT_CHANGE_SRC_RECT
#define ELEMENT_CHANGE_SRC_RECT (1<<3)
#endif

#ifndef ALIGN_TO_16
#define ALIGN_TO_16(x)  ((x + 15) & ~15)
#endif
static DISPMANX_DISPLAY_HANDLE_T display = NULL;
static DISPMANX_UPDATE_HANDLE_T update = NULL;
static GRAPHICS_LAYER_T renderLayer;
static BACK_LAYER_T backLayer;

/***********************************************************************/
static inline void INIT_IMAGE(IMAGE_T *image, int32_t width, int32_t height)
{
	if (image == NULL || width < 0 || height < 0)
		return;

	image->type = VC_IMAGE_RGB888;
	image->bitsPerPixel = 24;
	image->width = width;
	image->height = height;
	image->pitch = (ALIGN_TO_16(width) * image->bitsPerPixel) / 8;
	image->alignedHeight = ALIGN_TO_16(height);
	image->size = image->pitch * image->alignedHeight;
	image->buffer = (unsigned char *)malloc(image->size);
	if (image->buffer == NULL) {
		fprintf(stderr, "%s():Not enough memory!\n", __FUNCTION__);
		exit(-1);
	}

	memset(image->buffer, 0, image->size);
}

static inline void COPY_IMAGE(IMAGE_T *image, unsigned char *buf)
{
	if (image == NULL || buf == NULL)
		return;
	int y = 0;
	unsigned char *dbuf = image->buffer;
	unsigned char *sbuf = buf;
	int32_t dline_bytes = image->pitch * 3;
	int32_t sline_bytes = image->width * 3;
	for (y = 0; y < image->height; y++) {
		memcpy(dbuf, sbuf, sline_bytes);
		sbuf += sline_bytes;
		dbuf += dline_bytes;
	}
}
static void init_graphics_layer(GRAPHICS_LAYER_T *gl,int32_t viewWidth,int32_t viewHeight)
{
	uint32_t vc_image_ptr;
	if (gl == NULL || viewHeight<0 || viewWidth<0) {
		fprintf(stderr, "%s():Error input parameter!\n", __FUNCTION__);
		exit(-1);
	}

	INIT_IMAGE(&gl->image, viewWidth, viewHeight);
	gl->viewWidth = viewWidth;
	gl->viewHeight = viewHeight;
	gl->xOffset = 0;
	gl->xOffsetMax = viewWidth - 1;
	gl->yOffset = 0;
	gl->yOffsetMax = viewHeight - 1;
	gl->layer = 1;

	gl->frontResource = vc_dispmanx_resource_create(VC_IMAGE_RGB888,
		                                            gl->image.width | (gl->image.pitch << 16),
		                                            gl->image.height | (gl->image.alignedHeight << 16),
		                                            &vc_image_ptr);
	if (gl->frontResource == NULL) {
		fprintf(stderr, "%s():dispmanx resource create!\n", __FUNCTION__);
		exit(-1);
	}

	gl->backResource = vc_dispmanx_resource_create(VC_IMAGE_RGB888,
		                                           gl->image.width | (gl->image.pitch << 16),
		                                           gl->image.height | (gl->image.alignedHeight << 16),
		                                           &vc_image_ptr);
	if (gl->backResource == NULL) {
		fprintf(stderr, "%s():dispmanx resource create!\n", __FUNCTION__);
		exit(-1);
	}

	vc_dispmanx_rect_set(&(gl->bmpRect), 0, 0, gl->image.width, gl->image.height);

	int result = 0;
	result = vc_dispmanx_resource_write_data(gl->frontResource,
		                                     gl->image.type,
		                                     gl->image.pitch,
		                                     gl->image.buffer,
		                                     &(gl->bmpRect));
	if (result != 0) {
		fprintf(stderr, "%s():dispmanx resource write data failed!\n", __FUNCTION__);
		exit(-1);
	}

	result = vc_dispmanx_resource_write_data(gl->backResource,
		                                     gl->image.type,
		                                     gl->image.pitch,
		                                     gl->image.buffer,
		                                     &(gl->bmpRect));
	if (result != 0) {
		fprintf(stderr, "%s():dispmanx resource write data failed!\n", __FUNCTION__);
		exit(-1);
	}
}

static void add_graphics_layer_element(GRAPHICS_LAYER_T *gl)
{
	if (gl == NULL) {
		fprintf(stderr, "%s():Error input parameter!\n", __FUNCTION__);
		exit(-1);
	}

	VC_DISPMANX_ALPHA_T alpha ={
		DISPMANX_FLAGS_ALPHA_FROM_SOURCE,
		255,
		0
	};

	vc_dispmanx_rect_set(&gl->srcRect, gl->xOffset << 16, gl->yOffset << 16, gl->viewWidth << 16, gl->viewHeight << 16);

	vc_dispmanx_rect_set(&(gl->dstRect), 0, 0, gl->viewWidth, gl->viewHeight);

	gl->element = vc_dispmanx_element_add(update, display, gl->layer, &(gl->dstRect), gl->frontResource, &(gl->srcRect),
		                                  DISPMANX_PROTECTION_NONE, &alpha, NULL, DISPMANX_NO_ROTATE);
}

static void free_graphics_layer(GRAPHICS_LAYER_T *gl)
{
	if (gl == NULL)
		return;

	DISPMANX_UPDATE_HANDLE_T m_update = vc_dispmanx_update_start(0);
	if (m_update == NULL) {
		fprintf(stderr, "%s():dispmanx update start failed!\n", __FUNCTION__);
		exit(-1);
	}

	if (vc_dispmanx_element_remove(m_update, gl->element) != 0) {
		fprintf(stderr, "%s():dispmanx element remove failed!\n", __FUNCTION__);
		exit(-1);
	}

	if (vc_dispmanx_update_submit_sync(m_update) != 0) {
		fprintf(stderr, "%s():dispmanx update submit sync failed!\n", __FUNCTION__);
		exit(-1);
	}


	if (vc_dispmanx_resource_delete(gl->frontResource) != 0) {
		fprintf(stderr, "%s():dispmanx resource delete failed!\n", __FUNCTION__);
		exit(-1);
	}

	if (vc_dispmanx_resource_delete(gl->backResource) != 0) {
		fprintf(stderr, "%s():dispmanx resource delete failed!\n", __FUNCTION__);
		exit(-1);
	}
}
/***********************************************************************/
int dispmanx_init_graphics(int32_t viewWidth, int32_t viewHeight)
{
	uint32_t displayNumber = 0;
	DISPMANX_MODEINFO_T info;

	bcm_host_init();

	if ((display = vc_dispmanx_display_open(displayNumber)) == NULL) {
		fprintf(stderr, "%s():dispmanx display open failed:%d\n", __FUNCTION__, displayNumber);
		return -1;
	}

	if (vc_dispmanx_display_get_info(display, &info) != 0) {
		fprintf(stderr, "%s():dispmanx display get info failed\n", __FUNCTION__);
		return -1;
	}

	if (viewWidth < 0 || viewWidth < info.width)
		viewWidth = info.width;
	if (viewHeight < 0 || viewHeight < info.height)
		viewHeight = info.height;

	if ((update = vc_dispmanx_update_start(0)) == NULL) {
		fprintf(stderr, "%s():dispmanx display update start failed\n", __FUNCTION__);
		return -1;
	}
	
	//init_back_layer(&backLayer, viewWidth, viewHeight, 0);
	//add_back_layer_element(&backLayer, display, update);

	init_graphics_layer(&renderLayer, viewWidth, viewHeight);
	add_graphics_layer_element(&renderLayer);

	if (vc_dispmanx_update_submit_sync(update) != 0) {
		fprintf(stderr, "%s():dispmanx display update submit sync\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

int dispmanx_close_graphics(void)
{
	free_graphics_layer(&renderLayer);
	//free_back_layer(&backLayer);

	vc_dispmanx_display_close(display);
}

void dispmanx_render_buffer_RGB888(unsigned char *buf)
{
	int result = 0;
	if (buf == NULL) {
		fprintf(stderr, "%s():NULLNULLN\n",__FUNCTION__);
		return;
	}
	fprintf(stderr, "%s():ssssssssssssssssssssss\n", __FUNCTION__);
	DISPMANX_UPDATE_HANDLE_T m_update = vc_dispmanx_update_start(0);
	assert(m_update != 0);
	fprintf(stderr, "%s():++++++++++++++++\n", __FUNCTION__);
	COPY_IMAGE(&renderLayer.image, buf);
	fprintf(stderr, "%s():aaaaaaaaaaaaaaaaa\n", __FUNCTION__);
	result = vc_dispmanx_resource_write_data(renderLayer.backResource,
		                                     renderLayer.image.type,
		                                     renderLayer.image.pitch,
		                                     renderLayer.image.buffer,
		                                     &(renderLayer.bmpRect));
	assert(result == 0);
	fprintf(stderr, "%s():bbbbbbbbbbbbbbbbbbb\n", __FUNCTION__);
	result = vc_dispmanx_element_change_source(m_update,
		                                       renderLayer.element,
		                                       renderLayer.backResource);
	fprintf(stderr, "%s():ccccccccccccccccccc\n", __FUNCTION__);
	assert(result == 0);

	//vc_dispmanx_rect_set(&(renderLayer.srcRect),
	//	                 renderLayer.xOffset << 16,
	//	                 renderLayer.yOffset << 16,
	//	                 renderLayer.viewWidth << 16,
	//	                 renderLayer.viewHeight << 16);

	result = vc_dispmanx_element_change_attributes(update,renderLayer.element,ELEMENT_CHANGE_SRC_RECT,0,255,
			                                       &(renderLayer.dstRect),
			                                       &(renderLayer.srcRect),
			                                       0,
			                                       DISPMANX_NO_ROTATE);
	assert(result == 0);

	//---------------------------------------------------------------------

	DISPMANX_RESOURCE_HANDLE_T tmp = renderLayer.frontResource;
	renderLayer.frontResource = renderLayer.backResource;
	renderLayer.backResource = tmp;
	result = vc_dispmanx_update_submit_sync(m_update);
	assert(result == 0);
}