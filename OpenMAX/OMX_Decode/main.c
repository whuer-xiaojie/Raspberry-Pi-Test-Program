#include <stdio.h>
#include <stdlib.h>

#include "IL/OMX_Core.h"
#include "ilclient.h"
#include "omx_typedef.h"
#include "omx_image.h"
#include "omx_resize.h"
#include "omx_render.h"
#include "bcm_host.h"

#define ALIGN16(x) (((x+0xf)>>4)<<4)

int sign_height = 1024;
int sign_width = 600;

// int main(int argc, char **argv)
// {
// 
// }













int main(int argc, char **argv)
{
	if (argc < 2){
		fprintf(stderr, "Usage:%s image-name\n", argv[0]);
		exit(1);
	}
	bcm_host_init();

	char *file = argv[1];
	IMAGE *image = NULL;
	if ((image = (IMAGE*)malloc(sizeof(IMAGE))) == NULL){
		fprintf(stderr, "%s():malloc for IMAGE pointer failed\n", __FUNCTION__);
		exit(1);
	}
	memset(image, 0, sizeof(IMAGE));

	ILCLIENT_T *ilclient;
	COMPONENT_T *comp;
	ilclient = ilclient_init();
	if (ilclient == NULL){
		fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
		exit(1);
	}

	if (OMX_Init() != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_init failed\n", __FUNCTION__);
		exit(1);
	}

	fprintf(stderr, "%s():image->width=%d image->height=%d image->nData=%d\n", __FUNCTION__, image->width,
		image->height, image->nData);

	if (omx_decode_image(file, ilclient, image) != 0){
		fprintf(stderr, "%s():decode image %s failed \n", __FUNCTION__, file);
		exit(0);
	}
	fprintf(stderr, "%s():image->width=%d image->height=%d image->nData=%d codingType=%d colorType=%d\n",
		__FUNCTION__, image->width, image->height,image->nData,image->codingType,image->colorFormat);

	fprintf(stderr, "%s():before size=%d  after cal size=%d\n", __FUNCTION__,image->width*image->height*4, ALIGN16(image->width)*ALIGN16(image->height) * 4);
// 	int i;
// 	for ( i = 0; i < image->nData; i++){
// 		fprintf(stderr, "%s():", __FUNCTION__);
// 		fprintf(stderr,"pData[%d]=%d\n", i, image->pData[i]);
// 	}

// 	OMX_RENDER_DISP_CONF dispConfig = INIT_OMX_DISP_CONF;
// 	dispConfig.width = 1024;
// 	dispConfig.height = 600;
// 	dispConfig.mode = 0;
// 	dispConfig.rotation = 0;
// 	dispConfig.configFlags = 1;
// 	OMX_RENDER render = INIT_OMX_RENDER;
	int i, y;
// 	for (y = 0; y < ALIGN16(image->height); y++){
// 		size_t offset = y*ALIGN16(image->width) * 4;
// 		for (i = 0; i < ALIGN16(image->width) * 4; i += 4)
// 		{
// 			fprintf(stderr, "pixel y=%d x=%d: %d %d %d %d \n", y,i / 4, image->pData[offset+i], image->pData[offset+i + 1],
// 				image->pData[offset+i + 2], image->pData[offset+i + 3]);
// 		}
// 	}
	RENDER_DISPLAY_CONFIGS dispConfig;
	dispConfig.sign_width = 800;
	dispConfig.sign_height = 600;
	dispConfig.xOffset = 0;
	dispConfig.yOffset = 0;

	RENDER render;
	
	render.il_client = ilclient;
	render.dispConfig = &dispConfig;
	//render.lock = { PTHREAD_MUTEX_INITIALIZER };
	//render.cond = { PTHREAD_COND_INITIALIZER };

	//omxRenderImage(&render, image);
	omx_render_image(&render, image);
	
	sleep(20);

	//INIT_OMX_RENDER
	//stopOmxImageRender(&render);
// 	IMAGE *outImage;
// 	if ((outImage = (IMAGE*)malloc(sizeof(IMAGE))) == NULL){
// 		fprintf(stderr, "%s():malloc for IMAGE pointer failed\n", __FUNCTION__);
// 		exit(1);
// 	}
// 	memset(outImage, 0, sizeof(IMAGE));
// 	outImage->width = 200; outImage->height = 200;
// 	omx_resize_image(ilclient, image, outImage);

// 	fprintf(stderr, "%s():out->width=%d,out->height=%d,out->nData=%d out->format=%d out->colortype=%d\n", __FUNCTION__, outImage->width,
// 		outImage->height, outImage->nData,outImage->codingType,outImage->colorFormat);
// 	outImage->colorFormat = OMX_COLOR_Format32bitARGB8888;
// 	//omxRenderImage(&render, outImage);
// 	//sleep(10);
// 	int i;
// 	for (i = 0; i < 100; i+=4){
// 		fprintf(stderr, "%s():pixex %d:R=%d G=%d B=%d A=%d\n", __FUNCTION__, i, outImage->pData[i],
// 			outImage->pData[i + 1], outImage->pData[i + 2], outImage->pData[i + 3]);
// 	}
// 	fprintf(stderr, "###########\n");
	OMX_Deinit();
}
