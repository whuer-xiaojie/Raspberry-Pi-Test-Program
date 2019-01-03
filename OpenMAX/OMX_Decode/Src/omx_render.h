/*
* omx_render.h
*/

#ifndef OMXRENDER_H
#define OMXRENDER_H
 
#include <pthread.h>

#include <IL/OMX_Broadcom.h>
#include <IL/OMX_Core.h>

#include "omx_typedef.h"
#include "ilclient.h"

typedef struct RENDER_DISPLAY_CONFIGS{
	int sign_width;
	int sign_height;
	int xOffset;
	int yOffset;
}RENDER_DISPLAY_CONFIGS;

typedef struct RENDER{
	ILCLIENT_T *il_client;

	COMPONENT_T *renderComponent;
	OMX_HANDLETYPE renderHandle;
	int renderInputPort;

	COMPONENT_T *resizeComponent;
	OMX_HANDLETYPE *resizeHandle;
	int resizeInputPort;
	int resizeOutPort;

	RENDER_DISPLAY_CONFIGS *dispConfig;

	OMX_BUFFERHEADERTYPE *pInputBufferHeader;
	volatile char pSettingsChanged;

}RENDER;

int omx_render_image(RENDER *render, IMAGE *inImage);

#endif
