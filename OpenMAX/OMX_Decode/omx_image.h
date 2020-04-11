/*
* omx_image.h
*/

#ifndef OMX_IMAGE_H
#define OMX_IMAGE_H

#include <pthread.h>
#include <IL/OMX_Core.h>

#include "ilclient.h"
#include "omx_typedef.h"

#define DECODER_BUFFER_COUNT 3

typedef struct IMAGE_DECODER{
	ILCLIENT_T *il_client;
	COMPONENT_T *component;

	OMX_HANDLETYPE handle;

	uint32_t inputPort;
	uint32_t outPort;

	pthread_mutex_t lock;
	pthread_cond_t cond;
	volatile char emptyBDone;

	OMX_BUFFERHEADERTYPE *ppInputBufferHeader[DECODER_BUFFER_COUNT];
	OMX_BUFFERHEADERTYPE *pOutputBufferHeader;

}IMAGE_DECODER;

int omx_decode_image(char *fileName, ILCLIENT_T *ilclient, IMAGE *image);

#endif