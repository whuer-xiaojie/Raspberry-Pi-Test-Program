/*
* main.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "bcm_host.h"
#include "ilclient.h"
#include "omx_ffvideo.h"
#include "omx_render.h"
#include "clock.h"

#include "omx_helper.h"
#include "omx_video_decoder.h"

//#define ALIGN16(x) (((x+0xf)>>4)<<4)

int sign_width=1920 ;
int sign_height=1200 ;

int srcWidth;
int srcHeight;

ILCLIENT_T *ilClient = NULL;

void static exit_handle()
{
	if (ilClient != NULL){
		ilclient_destroy(ilClient);
		ilClient = NULL;
	}
	OMX_Deinit();
	bcm_host_deinit();
}
static void signal_handler(const int sig)
{
	exit(1);
}
int main(int argc, char **argv)
{
	atexit(exit_handle);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	srcWidth = sign_width; srcHeight = sign_height;
	if (argc < 2){
		fprintf(stderr, "Usage:%s video_name\n", argv[0]);
		exit(1);
	}
	char *videoName = argv[1];
	bcm_host_init();
	ilClient = ilclient_init();
	if (ilClient == NULL){
		fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
		exit(1);
	}
	OMX_ERRORTYPE err = OMX_Init();
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s() :omx init FAILED\n", __FUNCTION__);
		exit(1);
	}

	OMX_ListAllComponent();

	OMX_VIDEO_FRAME outFream; OMX_VIDEO_DECODER videoDecoder;
	outFream.nData = 0;
	outFream.pData = (uint8_t *)malloc(ALIGN16(sign_width)*ALIGN16(sign_height) * 4);
	if (outFream.pData = NULL){
		exit(1);
	}
	outFream.nFrameWidth = sign_width;
	outFream.nFrameHeight = sign_height;

	videoDecoder.decodeComponent = NULL; videoDecoder.pBitStreamFilterCtx = NULL;
	videoDecoder.pCodecCtx = NULL; videoDecoder.pFormatCtx = NULL;
	//videoDecoder.pInputBufferHeader = NULL;
	//videoDecoder.pOutputBufferHeader = NULL;
	videoDecoder.resizeComponent = NULL;

	if (omx_init_graphics() != NULL){
		fprintf(stderr, "%s():init graphics failed \n", __FUNCTION__);
		exit(1);
	}

	if (init_video_decoder(videoName, &videoDecoder, ilClient) != 0){
		fprintf(stderr, "%s():init video decoder failed\n", __FUNCTION__);
		exit(1);
	}

	while (1){
		if (decode_video_next_frame(&videoDecoder, &outFream) != 0){
			break;
		}

		omx_render_buf_r8g8b8(outFream.pData);

		outFream.nData = 0;

	}

	deinit_video_decoder(&videoDecoder);
	omx_close_graphics();
	if (outFream.pData){
		free(outFream.pData);
	}
	exit(1);
}

/*
int main(int argc, char **argv)
{
	if (argc < 2){
		fprintf(stderr, "Usage:%s video-name\n", argv[0]);
		exit(1);
	}
	char *videoName = argv[1];
	
	OMX_FFVIDEO_DECODER ffVideo; 
	ffVideo.pFormatCtx = NULL;
	ffVideo.pCodecCtx = NULL;
	//ffVideo.nData=
	
	OMX_FFVIDEO_FRAME outFrame;
	outFrame.nFrameWidth = sign_width; outFrame.nFrameHeight = sign_height;
	//outFrame.pData = (uint8_t *)malloc(ALIGN16(sign_width)*ALIGN16(sign_height) * 4);
	//outFrame.nData = 0;

// 	outFrame.nData = sign_width * sign_height + (sign_width * sign_height) / 2;
// 	outFrame.pData = (uint8_t *)malloc(outFrame.nData);
// 	memset(outFrame.pData, 0, outFrame.nData);

	size_t bufsize = sign_width * sign_height + (sign_width * sign_height) / 2;
	unsigned char *buf = (unsigned char *)malloc(bufsize);
	if (buf == NULL){
		fprintf(stderr, "%s():malloc for buf failed\n", __FUNCTION__);
		exit(1);
	}
	memset(buf, 100, bufsize);

	if (omx_init_video_info(videoName, &ffVideo) != 0){
		fprintf(stderr, "%s():init video info failed \n", __FUNCTION__);
		exit(0);
	}

// 	outFrame.nData = srcWidth * srcHeight + (srcWidth * srcHeight) / 2;
// 	outFrame.pData = (uint8_t *)malloc(outFrame.nData);
// 	memset(outFrame.pData, 0, outFrame.nData);

 	outFrame.nData = ALIGN16(srcWidth)*ALIGN16(srcHeight) * 4;
 	outFrame.pData = (uint8_t *)malloc(outFrame.nData);
 	memset(outFrame.pData, 0, outFrame.nData);

	unsigned long delay = 1000 / ffVideo.frameRate;

	fprintf(stderr, "%s():delay=%d\n", __FUNCTION__, delay);
	//sign_width = srcWidth; sign_height = srcHeight;

	if (omx_init_graphics() != 0){
		fprintf(stderr, "%s():init graphics failed\n", __FUNCTION__);
		exit(1);
	}

	int i;
	unsigned long t1 = __clock(); int s = 0;
	while (1){
		unsigned long t3 = __clock();
		if (omx_decode_video_next_frame(&ffVideo, &outFrame) != 0){
			fprintf(stderr, "%s():finish display video \n", __FUNCTION__);
			break;
		}
		unsigned long t4 = __clock();
		if (s <10 ){
			fprintf(stderr, "decode time=%d delay=%d\n", t4 - t3, delay);
			s++;
		}
// 		for (i = 0; i < 1000; i++){
// 			fprintf(stderr, "%s():pData[%d]=%d\n", __FUNCTION__, i, outFrame.pData[i]);
// 		}
		//memcpy(buf, outFrame.pData, bufsize);
		//FreeOutFrame(&outFrame); outFrame.nData = 0;

		omx_render_buf_r8g8b8(outFrame.pData);
		//memset(outFrame.pData, 0, outFrame.nData);
		unsigned long t5 = __clock();
		if (s < 10){
			fprintf(stderr, "%s():render time=%d\n", __FUNCTION__, t5 - t4);
		}
		//omx_render_buf_r8g8b8(ffVideo.pData);
		//omx_render_buf_r8g8b8(buf);
		//__sleep(delay-20);
	}
	unsigned long t2 = __clock();

	fprintf(stderr, "%s():video time=%d  play_time=%d \n", __FUNCTION__, ffVideo.videoTime, t2 - t1);
	FreeOutFrame(&outFrame);
	omx_deinit_video_info(&ffVideo);
	omx_close_graphics();
	return 0;
}
*/