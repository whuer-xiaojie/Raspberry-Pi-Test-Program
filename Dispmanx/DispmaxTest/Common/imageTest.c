#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"../Common/backLayer.h"
#include"../Common/image.h"
#include"../Common/imageLayer.h"

static int width = 1920;
static int height = 1080;

static void update_image(IMAGE_T *image, int count)
{
	assert(image != NULL);
	int i = 0;
	unsigned char *buf = (unsigned char*)image->buffer;
	for (i = 0; i < image->size; i++)
		buf[i] = 255;
}
int main(int argc, char**argv)
{
	uint32_t displayNumber = 0;

	BACK_LAYER_T bg;
	IMAGE_LAYER_T imageLayer;

	DISPMANX_MODEINFO_T info;
	int result = 0;


	bcm_host_init();

	DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(displayNumber);
	assert(display != 0);

	result = vc_dispmanx_display_get_info(display, &info);
	assert(result == 0);
	fprintf(stderr, "%s():************w=%d h=%d*********\n", __FUNCTION__, info.width,info.height);
	init_back_layer(&bg, 0);
	fprintf(stderr, "%s():111111111111111111", __FUNCTION__);
	initImageLayer(&imageLayer, width, height, VC_IMAGE_RGB888);
	createResourceImageLayer(&imageLayer, 1);
	fprintf(stderr, "%s():2222222222222222222", __FUNCTION__);
	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	assert(update != 0);
	fprintf(stderr, "%s():333333333333333333", __FUNCTION__);
	add_back_layer_element(&bg, display, update);
	addElementImageLayer(&imageLayer, display, update);
	fprintf(stderr, "%s():44444444444444444", __FUNCTION__);
	result = vc_dispmanx_update_submit_sync(update);
	assert(result == 0);
	int count = 0;
	fprintf(stderr, "%s():5555555555555555", __FUNCTION__);
	while (count <= 200) {
		count++;
		update_image(&imageLayer.image, count);
		fprintf(stderr, "%s():************count=%d*********\n", __FUNCTION__, count);
		changeSourceAndUpdateImageLayer(&imageLayer);
		/*DISPMANX_DISPLAY_HANDLE_T update = vc_dispmanx_update_start(0);
		assert(update != 0);

		changeSourceImageLayer(&imageLayer, update);
	
		result = vc_dispmanx_update_submit_sync(update);
		assert(result == 0);*/

		
	}

	free_back_layer(&bg);
	destroyImageLayer(&imageLayer);

	result = vc_dispmanx_display_close(display);
	assert(result == 0);

	return 0;
}