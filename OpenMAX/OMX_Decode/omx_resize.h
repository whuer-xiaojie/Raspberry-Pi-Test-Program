/*
*	omx_resize.h
*/

#ifndef OMX_RESIZE_H
#define OMX_RESIZE_H

#include <pthread.h>
#include <IL/OMX_Core.h>

#include "ilclient.h"
#include "omx_typedef.h"

typedef struct RESIZER{

	ILCLIENT_T *il_client;
	COMPONENT_T *component;

	OMX_HANDLETYPE handle;

	uint32_t inputPort;
	uint32_t outPort;
	OMX_BUFFERHEADERTYPE *pInputBufferHeader;
	OMX_BUFFERHEADERTYPE *pOutputBufferHeader;
}RESIZER;

int omx_resize_image(ILCLIENT_T *ilclient, IMAGE *inImage, IMAGE *outImage);

#endif