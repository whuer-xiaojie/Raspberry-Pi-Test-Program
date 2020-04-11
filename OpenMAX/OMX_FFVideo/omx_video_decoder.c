/*
* omx_video_decoder.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

#include "ilclient.h"
#include "bcm_host.h"
#include "omx_video_decoder.h"
#include "omx_helper.h"

void my_fill_buffer_done(void *data, COMPONENT_T *pComponent)
{
	fprintf(stderr, "%s():had down\n", __FUNCTION__);
	OMX_VIDEO_DECODER *videoDecoder = (OMX_VIDEO_DECODER *)data;
	videoDecoder->fillBufferDone = 1;
}
void my_empty_buffer_done(void *data, COMPONENT_T *pComponent)
{
	fprintf(stderr, "%s():had down\n", __FUNCTION__);
	OMX_VIDEO_DECODER *videoDecoder = (OMX_VIDEO_DECODER *)data;
	videoDecoder->emptyBufferDone = 1;
}
int set_resize_output_video_info(OMX_VIDEO_DECODER *videoDecoder, int outWidth, int outHeight)
{
	int ret = -1;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_INIT_STRUCTURE(portDef);
	portDef.nPortIndex = videoDecoder->resizeOutputPort;

	err = OMX_GetParameter(videoDecoder->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	OMX_AssertError(err);

	portDef.format.video.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	portDef.format.video.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.video.nFrameWidth = outWidth;
	portDef.format.video.nFrameHeight = outHeight;
	portDef.format.video.nSliceHeight = 0;
	portDef.format.video.nStride = 0;

	portDef.nBufferSize = ALIGN16(outWidth)*ALIGN16(outHeight) * 4;

	err = OMX_SetParameter(videoDecoder->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	OMX_AssertError(err);

	//err = OMX_SendCommand(videoDecoder->resizeHandle, OMX_CommandPortEnable, videoDecoder->resizeOutputPort, NULL);
	//OMX_AssertError(err);
	ilclient_enable_port(videoDecoder->resizeComponent, videoDecoder->resizeOutputPort);

	err = OMX_AllocateBuffer(videoDecoder->resizeHandle, &videoDecoder->pOutputBufferHeader,
		videoDecoder->resizeOutputPort, NULL, portDef.nBufferSize);
	OMX_AssertError(err);

	ret = ilclient_change_component_state(videoDecoder->resizeComponent, OMX_StateExecuting);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change resize component's state to executing failed\n", __FUNCTION__);
		return ret;
	}

	videoDecoder->fillBufferDone = 0;
	return 0;
}
int set_decode_input_video_info(OMX_VIDEO_DECODER *videoDecoder,OMX_VIDEO_CODINGTYPE eCodingType)
{
	int ret = -1;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VIDEO_PARAM_PORTFORMATTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);

	portFormat.nPortIndex = videoDecoder->decodeInputPort;
	portFormat.eCompressionFormat = eCodingType;
	err = OMX_SetParameter(videoDecoder->decodeHandle, OMX_IndexParamVideoPortFormat, &portFormat);
	OMX_AssertError(err);

	
	ret=ilclient_enable_port_buffers(videoDecoder->decodeComponent, videoDecoder->decodeInputPort, 
		NULL, NULL, NULL);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient enable port buffers failed\n", __FUNCTION__);
		return -1;
	}

// 	OMX_PARAM_PORTDEFINITIONTYPE portDef;
// 	OMX_INIT_STRUCTURE(portDef);
// 	portDef.nPortIndex = videoDecoder->decodeInputPort;
// 
// 	err = OMX_GetParameter(videoDecoder->decodeHandle, OMX_IndexParamPortDefinition, &portDef);
// 	OMX_AssertError(err);
// 
// 	fprintf(stderr, "%s():portIndex=%d  nBufferSize=%d nBufactualCount=%d\n", __FUNCTION__, 
// 		videoDecoder->decodeInputPort,portDef.nBufferSize,portDef.nBufferCountActual);
// 	OMX_PrintComponentState(videoDecoder->decodeHandle);
// 	
// 	err = OMX_AllocateBuffer(videoDecoder->decodeHandle, &videoDecoder->pInputBufferHeader, videoDecoder->decodeInputPort,
// 		NULL, portDef.nBufferSize);
// 	OMX_AssertError(err);
// 	fprintf(stderr, "%s():nBufferSize=%d\n", __FUNCTION__, portDef.nBufferSize);
	
	ilclient_enable_port(videoDecoder->decodeComponent, videoDecoder->decodeInputPort);
	ret = ilclient_change_component_state(videoDecoder->decodeComponent, OMX_StateExecuting);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change decode component's state to executing failed\n", __FUNCTION__);
		return ret;
	}

	videoDecoder->emptyBufferDone = 1;

	return 0;
}

int init_resize_component(OMX_VIDEO_DECODER *videoDecoder, ILCLIENT_T *ilClient)
{
	int ret; OMX_ERRORTYPE err = OMX_ErrorNone;

	if (ilClient == NULL){
		if ((ilClient = ilclient_init()) == NULL){
			fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
			return -1;
		}
	}

	videoDecoder->ilClient = ilClient;

	ret = ilclient_create_component(ilClient, &videoDecoder->resizeComponent, "resize",
		ILCLIENT_DISABLE_ALL_PORTS /*| ILCLIENT_ENABLE_INPUT_BUFFERS */| ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return ret;
	}

	videoDecoder->resizeHandle = ilclient_get_handle(videoDecoder->resizeComponent);
	videoDecoder->resizeInputPort = OMX_GetInputPortIndex(videoDecoder->resizeHandle, OMX_IndexParamImageInit);
	videoDecoder->resizeOutputPort = OMX_GetInputPortIndex(videoDecoder->resizeHandle, OMX_IndexParamImageInit);

	ret = ilclient_change_component_state(videoDecoder->resizeComponent, OMX_StateLoaded);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change resize component's state to loaded failed\n", __FUNCTION__);
		return ret;
	}

	ret = ilclient_change_component_state(videoDecoder->resizeComponent, OMX_StateIdle);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change resize component's state to idle failed\n", __FUNCTION__);
		return ret;
	}
	return 0;
}

int init_video_decoder_component(OMX_VIDEO_DECODER *videoDecoder,ILCLIENT_T *ilClient)
{
	int ret; OMX_ERRORTYPE err = OMX_ErrorNone;

	if (ilClient == NULL){
		if ((ilClient = ilclient_init()) == NULL){
			fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
			return -1;
		}
	}

	videoDecoder->ilClient = ilClient;
	ret = ilclient_create_component(ilClient, &videoDecoder->decodeComponent, "video_decode",
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS /*| ILCLIENT_ENABLE_OUTPUT_BUFFERS*/);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return ret;
	}

	videoDecoder->decodeHandle = ilclient_get_handle(videoDecoder->decodeComponent);
	videoDecoder->decodeInputPort = OMX_GetInputPortIndex(videoDecoder->decodeHandle, OMX_IndexParamVideoInit);
	videoDecoder->decodeOutputPort = OMX_GetInputPortIndex(videoDecoder->decodeHandle, OMX_IndexParamVideoInit);

	ret = ilclient_change_component_state(videoDecoder->decodeComponent, OMX_StateLoaded);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change video_decode component's state to loaded failed\n", __FUNCTION__);
		return ret;
	}

	ret = ilclient_change_component_state(videoDecoder->decodeComponent, OMX_StateIdle);
	if (ret != 0){
		fprintf(stderr, "%s():ilclient change decode component's state to idle failed\n", __FUNCTION__);
		return ret;
	}
	return 0;
}

int init_video_decoder(char *fileName,OMX_VIDEO_DECODER *videoDecoder,ILCLIENT_T *ilClient)
{
	int ret = -1;
	av_register_all();
	if (videoDecoder->pFormatCtx == NULL){
		videoDecoder->pFormatCtx = avformat_alloc_context();
	}
	if ((ret = avformat_open_input(&videoDecoder->pFormatCtx, fileName, NULL,NULL)) != 0){
		fprintf(stderr, "%s():can't open the video file:%s\n", __FUNCTION__, fileName);
		return ret;
	}
	if ((ret=avformat_find_stream_info(videoDecoder->pFormatCtx, NULL)) < 0){
		fprintf(stderr, "%s():find stream info failed\n", __FUNCTION__);
		return ret;
	}
	av_dump_format(videoDecoder->pFormatCtx, 0, fileName, 0);

	videoDecoder->videoStreamIndex = -1; int i;
	for (i = 0; i < videoDecoder->pFormatCtx->nb_streams; i++){
		if (videoDecoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			videoDecoder->videoStreamIndex = i;
			break;
		}
	}

	if (videoDecoder->pCodecCtx == NULL){
		videoDecoder->pCodecCtx = videoDecoder->pFormatCtx->streams[videoDecoder->videoStreamIndex]->codec;
	}
	if (videoDecoder->pBitStreamFilterCtx == NULL){
		videoDecoder->pBitStreamFilterCtx = av_bitstream_filter_init("h264_mp4toannexb");
		if (videoDecoder->pBitStreamFilterCtx == NULL){
			fprintf(stderr, "%s():init bit stream filter failed\n", __FUNCTION__);
			return -1;
		}
	}

	if (init_video_decoder_component(videoDecoder, ilClient) != 0){
		fprintf(stderr, "%s():init video decode component failed\n", __FUNCTION__);
		return -1;
	}
	fprintf(stderr, "%s():11111111111111\n", __FUNCTION__);
	if (init_resize_component(videoDecoder, ilClient) != 0){
		fprintf(stderr, "%s():init resize component failed\n", __FUNCTION__);
		return -1;
	}
	fprintf(stderr, "%s():222222222222222\n", __FUNCTION__);
	if (set_decode_input_video_info(videoDecoder, OMX_VIDEO_CodingAVC) != 0){
		fprintf(stderr, "%s():set video decode input port information failed\n", __FUNCTION__);
		return -1;
	}
	fprintf(stderr, "%s():333333333333333\n", __FUNCTION__);
	ilclient_set_fill_buffer_done_callback(videoDecoder->ilClient, my_fill_buffer_done, videoDecoder);
	ilclient_set_empty_buffer_done_callback(videoDecoder->ilClient, my_empty_buffer_done, videoDecoder);

	fprintf(stderr, "%s():down\n", __FUNCTION__);
	return 0;
}

int deinit_resize_component(OMX_VIDEO_DECODER *videoDecoder)
{
	int ret = -1;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	if (videoDecoder->pOutputBufferHeader != NULL){
		err = OMX_FreeBuffer(videoDecoder->resizeHandle, videoDecoder->resizeOutputPort, videoDecoder->pOutputBufferHeader);
		OMX_AssertError(err);
		videoDecoder->pOutputBufferHeader = NULL;
	}

	ilclient_disable_port(videoDecoder->resizeComponent, videoDecoder->resizeOutputPort);

	//err = OMX_SendCommand(videoDecoder->resizeHandle, OMX_CommandPortDisable, videoDecoder->resizeOutputPort, NULL);
	//OMX_AssertError(err);

	ret = ilclient_change_component_state(videoDecoder->resizeComponent, OMX_StateIdle);
	if (ret != 0){

	}

	ret = ilclient_change_component_state(videoDecoder->resizeComponent, OMX_StateLoaded);
	if (ret != 0){

	}
	COMPONENT_T *list[2];
	list[0] = videoDecoder->resizeComponent;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	videoDecoder->resizeComponent = NULL;
	return 0;
}

int deinit_video_decode_component(OMX_VIDEO_DECODER *videoDecoder)
{
	int ret = -1;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	if (videoDecoder->pInputBufferHeader != NULL){
		err = OMX_FreeBuffer(videoDecoder->decodeComponent, videoDecoder->decodeInputPort, videoDecoder->pInputBufferHeader);
		OMX_AssertError(err);
		videoDecoder->pInputBufferHeader = NULL;
	}

	ilclient_disable_port(videoDecoder->decodeComponent, videoDecoder->decodeInputPort);

	//err = OMX_SendCommand(videoDecoder->decodeHandle, OMX_CommandPortDisable, videoDecoder->decodeInputPort, NULL);
	//OMX_AssertError(err);

	ret = ilclient_change_component_state(videoDecoder->decodeComponent, OMX_StateIdle);
	if (ret != 0){

	}

	ret = ilclient_change_component_state(videoDecoder->decodeComponent, OMX_StateLoaded);
	if (ret != 0){

	}

	COMPONENT_T *list[2];
	list[0] = videoDecoder->decodeComponent;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	videoDecoder->decodeComponent = NULL;
	return 0;
}
int deinit_video_decoder(OMX_VIDEO_DECODER *videoDecoder)
{
	ilclient_set_fill_buffer_done_callback(videoDecoder->ilClient, NULL,NULL);
	ilclient_set_empty_buffer_done_callback(videoDecoder->ilClient, NULL,NULL);

	if (videoDecoder->resizeComponent != NULL){
		deinit_resize_component(videoDecoder);
	}
	if (videoDecoder->decodeComponent != NULL){
		deinit_video_decode_component(videoDecoder);
	}
	if (videoDecoder->pBitStreamFilterCtx != NULL){
		av_bitstream_filter_close(videoDecoder->pBitStreamFilterCtx);
		videoDecoder->pBitStreamFilterCtx = NULL;
	}
	if (videoDecoder->pFormatCtx != NULL){
		avformat_close_input(&videoDecoder->pFormatCtx);
		avformat_free_context(videoDecoder->pFormatCtx);
		videoDecoder->pFormatCtx = NULL;
	}
}
int port_settings_changed(OMX_VIDEO_DECODER *videoDecoder,int outWidth,int outHeight)
{
	int ret = -1;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	err = OMX_SetupTunnel(videoDecoder->decodeHandle, videoDecoder->decodeOutputPort,
		videoDecoder->resizeHandle, videoDecoder->resizeInputPort);
	OMX_AssertError(err);
	fprintf(stderr, "%s():11111111111111\n", __FUNCTION__);
	ret = set_resize_output_video_info(videoDecoder, outWidth, outHeight);

	return ret;
}

int save_resize_output_bufffer(OMX_BUFFERHEADERTYPE *pBufferHeader, OMX_VIDEO_FRAME *outFrame)
{
	int ret;
	memcpy(outFrame->pData + outFrame->nData, pBufferHeader->pBuffer + pBufferHeader->nOffset, pBufferHeader->nFilledLen);
	outFrame->nData += pBufferHeader->nFilledLen;
}

int decode_video_next_frame(OMX_VIDEO_DECODER *videoDecoder, OMX_VIDEO_FRAME *outFrame)
{
	int ret = -1;
	AVPacket packet;
	OMX_BUFFERHEADERTYPE *pBufferHeader;
	//OMX_BUFFERHEADERTYPE *pOutBufferHeader;
	av_init_packet(&packet);
	while (av_read_frame(videoDecoder->pFormatCtx, &packet) >= 0){

		if (packet.stream_index==videoDecoder->videoStreamIndex){
			av_bitstream_filter_filter(videoDecoder->pBitStreamFilterCtx, videoDecoder->pCodecCtx,
				NULL, &packet.data, &packet.size, packet.data, packet.size, 0);

			
			int packetSize   = packet.size;
			int packetOffset = 0;
			fprintf(stderr, "%s():had down bitsream filter  size=%d\n", __FUNCTION__,packetSize);
			while (packetSize != 0){
				if ((pBufferHeader = ilclient_get_input_buffer(videoDecoder->decodeComponent, 
					videoDecoder->decodeInputPort, 1))!=NULL/*videoDecoder->emptyBufferDone*/){

					videoDecoder->emptyBufferDone = 0;
					//pBufferHeader = videoDecoder->pInputBufferHeader;
					fprintf(stderr, "%s():nFilledLen=%d nAllocLen=%d\n", __FUNCTION__, pBufferHeader->nFilledLen,
						pBufferHeader->nAllocLen);
					pBufferHeader->nFilledLen = (packetSize > pBufferHeader->nAllocLen) ? pBufferHeader->nAllocLen : packetSize;
					memcpy(pBufferHeader->pBuffer, packet.data + packetOffset, pBufferHeader->nFilledLen);
					packetSize -= pBufferHeader->nFilledLen;
					packetOffset += pBufferHeader->nFilledLen;

					if (packetSize == 0){
						//pBufferHeader->nFlags = OMX_BUFFERFLAG_ENDOFFRAME;
						pBufferHeader->nFlags = OMX_BUFFERFLAG_EOS;
					}

					OMX_ERRORTYPE err = OMX_EmptyThisBuffer(videoDecoder->decodeHandle, pBufferHeader);
					OMX_AssertError(err);
				}

				if (videoDecoder->portSettingsChanged == 0 &&
					((packetSize > 0 && ilclient_remove_event(videoDecoder->decodeComponent, OMX_EventPortSettingsChanged,
					videoDecoder->decodeOutputPort, 0, 0, 1) == 0) ||
					(packetSize == 0 && ilclient_wait_for_event(videoDecoder->decodeComponent, OMX_EventPortSettingsChanged,
					videoDecoder->decodeOutputPort, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))){

					videoDecoder->portSettingsChanged = 1;
					port_settings_changed(videoDecoder, outFrame->nFrameWidth, outFrame->nFrameHeight);

					//pOutBufferHeader = videoDecoder->pOutputBufferHeader;
					//OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, pOutBufferHeader);
					OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, videoDecoder->pOutputBufferHeader);
					OMX_AssertError(err);
				}

				if (videoDecoder->fillBufferDone){
					videoDecoder->fillBufferDone = 0;
					save_resize_output_bufffer(videoDecoder->pOutputBufferHeader, outFrame);
					//pOutBufferHeader = videoDecoder->pOutputBufferHeader;
					//OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, pOutBufferHeader);
					OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, videoDecoder->pOutputBufferHeader);
					OMX_AssertError(err);
				}	
			}

			if (videoDecoder->fillBufferDone){
				videoDecoder->fillBufferDone = 0;
				save_resize_output_bufffer(videoDecoder->pOutputBufferHeader, outFrame);
				//pOutBufferHeader = videoDecoder->pOutputBufferHeader;
				//OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, pOutBufferHeader);
				OMX_ERRORTYPE err = OMX_FillThisBuffer(videoDecoder->resizeHandle, videoDecoder->pOutputBufferHeader);
				OMX_AssertError(err);
			}

			if (ilclient_wait_for_event(videoDecoder->resizeComponent, OMX_EventBufferFlag, videoDecoder->resizeOutputPort,
				0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 10000) != 0){
				fprintf(stderr, "%s():ilclient not receive buffer EOS event\n", __FUNCTION__);
				break;
			}

			av_packet_unref(&packet);
			av_free_packet(&packet);
			return 0;
		}
		av_packet_unref(&packet);
		av_free_packet(&packet);
	}
	av_packet_unref(&packet);
	av_free_packet(&packet);
	deinit_video_decoder(videoDecoder);
	return -1;

}