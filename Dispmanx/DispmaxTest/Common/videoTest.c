#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include "bcm_host.h"

static char *IMG = "/opt/vc/src/hello_pi/hello_video/test.h264";

AVCodec *pVideoCodecParameters = NULL;
AVStream *pAVStream = NULL;
AVPacket avPacket;
AVFormatContext *pFormatContext = NULL;
AVCodecContext *pAVCodecContext = NULL;
AVCodec *pAVCodec = NULL;

int videoStreamIndex = -1;
int videoWidth, videoHeight;

int setupDemuxer(const char *filename) {
	pFormatContext = avformat_alloc_context();
	if (avformat_open_input(&pFormatContext, filename, NULL, NULL) != 0) {
		printf("Can't get format\n");
		return -1;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatContext, NULL)) {
		printf("Couldn't find stream information\n");
		return -1;
	}

	//av_dump_format(pFormatContext, 0, filename, 0);

	// Find the first video stream
	videoStreamIndex = -1;
	// Now pFormatCtx->streams is just an array of pointers, of size pFormatCtx->nb_streams, so let's walk through it until we find a video stream
	int i;
	for ( i = 0; i < pFormatContext->nb_streams; i++) {
		if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIndex = i;
			break;
		}
	}

	if (videoStreamIndex < 0) {
		return -1;
	}

	pAVStream = pFormatContext->streams[videoStreamIndex];
	pAVCodecContext = pAVStream->codec;

	videoWidth = pAVStream->codec->width;
	videoHeight = pAVStream->codec->height;
	AVCodec *codec = avcodec_find_decoder(pAVCodecContext->codec->codec_id);
	//AVCodec *codec = avcodec_find_decoder_by_name("h264_mmal");

	pAVCodecContext = avcodec_alloc_context3(NULL);
	if (avcodec_parameters_to_context(pAVCodecContext, pAVStream->codec) < 0) {
		printf("Could not copy data to context\n");
		return -1;
	}

	// Open codec
	if (avcodec_open2(pAVCodecContext, codec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	uint32_t displayNumber = 0;
	VC_IMAGE_TYPE_T type = VC_IMAGE_YUV420;

	bcm_host_init();

	if (argc > 1) {
		IMG = argv[1];
	}
	setupDemuxer(IMG);


	DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(displayNumber);

	DISPMANX_MODEINFO_T info;
	int result = vc_dispmanx_display_get_info(display, &info);

	uint32_t pVCImage;

	DISPMANX_RESOURCE_HANDLE_T bgResource = vc_dispmanx_resource_create(type, pAVCodecContext->width, pAVCodecContext->height, &pVCImage);
	assert(bgResource != 0);

	VC_RECT_T sourceRectangle;
	VC_RECT_T destinationRectangle;

	vc_dispmanx_rect_set(&destinationRectangle, 0, 0, pAVCodecContext->width, pAVCodecContext->height);
	uint32_t background = 1;

	vc_dispmanx_resource_write_data(bgResource, type, sizeof(background), &background, &destinationRectangle);

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);

	VC_DISPMANX_ALPHA_T alpha =
	{
		DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS,
		255, /*alpha 0->255*/ 0
	};

	vc_dispmanx_rect_set(&sourceRectangle, 0, 0, pAVCodecContext->width, pAVCodecContext->height);
	vc_dispmanx_rect_set(&destinationRectangle, 0, 0, pAVCodecContext->width, pAVCodecContext->height);

	DISPMANX_ELEMENT_HANDLE_T bgElement = vc_dispmanx_element_add(update, display, 1 /* layer */, &destinationRectangle, bgResource, &sourceRectangle, DISPMANX_PROTECTION_NONE, &alpha, NULL /* clamp */, DISPMANX_NO_ROTATE);
	assert(bgElement != 0);

	result = vc_dispmanx_update_submit_sync(update);
	assert(result == 0);

	int frameNumber = 0;
	AVFrame *pFrame = av_frame_alloc();

	uint8_t *buffer = NULL;
	int numBytes;

	// Determine required buffer size and allocate buffer
	numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pAVCodecContext->width, pAVCodecContext->height, 1);

	buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	// Assign appropriate parts of buffer to image planes in pFrameRGB
	av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pAVCodecContext->width, pAVCodecContext->height, 1);
	int pitch = ALIGN_UP(pAVCodecContext->width, 32);

	vc_dispmanx_rect_set(&destinationRectangle, 0, 0, pAVCodecContext->width, (3 * ALIGN_UP(pAVCodecContext->height, 16)) / 2);
	while (av_read_frame(pFormatContext, &avPacket) >= 0) {
		if (avPacket.stream_index == videoStreamIndex) {
			// For decoding, call avcodec_send_packet() to give the decoder raw compressed data in an AVPacket.
			int frameFinished = avcodec_send_packet(pAVCodecContext, &avPacket);

			while (!frameFinished) {
				// For decoding, call avcodec_receive_frame(). On success, it will return an AVFrame containing uncompressed audio or video data.
				frameFinished = avcodec_receive_frame(pAVCodecContext, pFrame);
				if (!frameFinished) {
					vc_dispmanx_rect_set(&destinationRectangle, 0, 0, pAVCodecContext->width, (3 * ALIGN_UP(pAVCodecContext->height, 16)) / 2);
					result = vc_dispmanx_resource_write_data(bgResource, VC_IMAGE_YUV420, ALIGN_UP(pAVCodecContext->width, 32), *pFrame->data, &destinationRectangle);
					update = vc_dispmanx_update_start(0);
					vc_dispmanx_element_change_source(update, bgElement, bgResource);
					vc_dispmanx_update_submit_sync(update);
					av_frame_unref(pFrame);
				}
			}
		}
		av_packet_unref(&avPacket);
	}

	update = vc_dispmanx_update_start(0);
	vc_dispmanx_element_remove(update, bgElement);
	vc_dispmanx_update_submit_sync(update);
	vc_dispmanx_resource_delete(bgResource);
	vc_dispmanx_display_close(display);

	return 0;
}