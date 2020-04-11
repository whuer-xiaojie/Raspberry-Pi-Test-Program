/*
* omx_video_decoder.c
*/

#ifndef OMX_VIDEO_DECODER_H
#define OMX_VIDEO_DECODER_H

#include <IL/OMX_Core.h>

#include "ilclient.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mathematics.h"

typedef struct OMX_VIDEO_FRAME{
	uint8_t *pData;
	size_t nData;

	size_t nFrameWidth;
	size_t nFrameHeight;

	size_t nSliceHeight;
	size_t nStride;

}OMX_VIDEO_FRAME;

typedef struct OMX_VIDEO_DECODER{

	ILCLIENT_T *ilClient;

	COMPONENT_T *decodeComponent;
	OMX_HANDLETYPE decodeHandle;
	int decodeInputPort; int decodeOutputPort;
	OMX_BUFFERHEADERTYPE *pInputBufferHeader;

	COMPONENT_T *resizeComponent;
	OMX_HANDLETYPE resizeHandle;
	int resizeInputPort; int resizeOutputPort;
	OMX_BUFFERHEADERTYPE *pOutputBufferHeader;

	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVBitStreamFilterContext *pBitStreamFilterCtx;
	int videoStreamIndex;

	char portSettingsChanged;
	volatile char emptyBufferDone;//Initial value should be true 1
	volatile char fillBufferDone;// Initial value should be false 0

}OMX_VIDEO_DECODER;


int init_video_decoder(char *fileName, OMX_VIDEO_DECODER *videoDecoder, ILCLIENT_T *ilClient);
int deinit_video_decoder(OMX_VIDEO_DECODER *videoDecoder);
int decode_video_next_frame(OMX_VIDEO_DECODER *videoDecoder, OMX_VIDEO_FRAME *outFrame);





#endif