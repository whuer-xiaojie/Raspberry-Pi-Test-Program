/*
* omx_typedef.h
*/

/*
* Copyright (C) 2018 by the second group of Sansi Software Institute
*
* Define all the structures needed,While using GPU to decode image\audio
* \video on RespberryPi with OpenMAX Integration Layer(OpenMAX|IL)
*
*/
#ifndef OMX_TYPEDEF_H
#define OMX_TYPEDEF_H

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <stdint.h>

#define FreeImage(Im) { free((Im)->pData);(Im)->pData=NULL;}
#define FreeAudio(Au) { free((Au)->pData);(Au)->pData=NULL;}
#define FreeVideo(Vi) { free((Vi)->pData);(Vi)->pData=NULL;}


#define TIMEOUT_MS 1000
#define MAX_EMPTY_BUFFER_WAIT_TIME 50

typedef struct IMAGE{
	uint8_t *pData; //Image pixel data buf 
	size_t nData;   //Alloc data length

	uint32_t width; //Image frame width
	uint32_t height;//Image frame height
	OMX_IMAGE_CODINGTYPE codingType;  //Image encoding type
	OMX_COLOR_FORMATTYPE colorFormat; //Image color format
}IMAGE;

typedef struct GIF_IMAGE{
	IMAGE *curFrame;//Current frame of this GIF
	IMAGE *frames;  //All frames of this GIF

	uint32_t decodeCount;
	uint32_t frameNum;

	uint32_t *pData; //Current frame pixel data of this GIF
	size_t nData;    //Alloc data length

	void *pExtraData;
	int (*decodeNextFrame)(struct GIF_IMAGE *);
	void(*lastDecoding)(struct GIF_IMAGE *);

	uint32_t frameCount;
	unsigned long frameDelay;
	uint32_t loopCount;
}GIF_IMAGE;

typedef struct ANIM_IMAGE{
	IMAGE *curFrame;
	IMAGE *frames;
	unsigned int decodeCount;
	unsigned int frameNum;

	uint8_t* imData;
	size_t size;

	void* pExtraData;
	int(*decodeNextFrame)(struct ANIM_IMAGE *);
	void(*finaliseDecoding)(struct ANIM_IMAGE *);

	unsigned int frameCount;
	unsigned int frameDelayCs;
	int loopCount;

} ANIM_IMAGE;

char *OMX_ErrorToString(OMX_ERRORTYPE err);











#endif