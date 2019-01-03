/*
* omx_ffvideo.h
*/

#ifndef OMX_FFVIDEO_H
#define OMX_FFVIDEO_H

#include <stdint.h>
//coding
#include <libavcodec/avcodec.h>
//Encapsulation format processing
#include <libavformat/avformat.h>
//Pixel processing
#include <libswscale/swscale.h>
#include <IL/OMX_Core.h>

#include "ilclient.h"
#include "bcm_host.h"

#define TIMEOUT_MS 1000
#define FreeOutFrame(outFrame) { free((outFrame)->pData);(outFrame)->pData=NULL;}

typedef struct OMX_FFVIDEO_FRAME{
	uint8_t *pData;
	size_t nData;

	size_t nFrameWidth;
	size_t nFrameHeight;

}OMX_FFVIDEO_FRAME;

typedef struct OMX_FFVIDEO_DECODER{
	/*video decoding attributes*/
	AVFormatContext *pFormatCtx; /*封装格式上下文,保存了视频文件封装格式的相关信息*/
	AVCodecContext  *pCodecCtx; /*编解码上下文*/
	AVCodec			*pCodec;
	AVFrame         *pFrame;
	AVFrame			*pFrameYUV420;
	uint8_t			*pData;
	size_t          nData;
	struct SwsContext *sws_ctx;
	int yLen, uLen, vLen;
	int video_stream_index;
	unsigned long frameRate;
	unsigned long  videoTime;

	COMPONENT_T *resizeComponent;
	ILCLIENT_T *ilClient;
	OMX_HANDLETYPE handle;

	int inputPort; int outputPort;
	OMX_BUFFERHEADERTYPE *pInputBufferHeader;
	OMX_BUFFERHEADERTYPE *pOutputBufferHeader;
	volatile char pSettingsChanged;
	volatile char emptyBufferDone;
	volatile char fillBufferDone;

}OMX_FFVIDEO_DECODER;

int omx_init_video_info(char *filePath, OMX_FFVIDEO_DECODER *ffVideo);
int omx_decode_video_next_frame(OMX_FFVIDEO_DECODER *ffVideo, OMX_FFVIDEO_FRAME *outFrame);
int omx_deinit_video_info(OMX_FFVIDEO_DECODER *ffVideo);



#endif