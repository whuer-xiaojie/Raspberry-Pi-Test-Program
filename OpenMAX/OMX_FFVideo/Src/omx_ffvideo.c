/*
* omx_ffvideo.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "omx_ffvideo.h"

extern int sign_width;
extern int sign_height;
extern int srcWidth;
extern int srcHeight;

#define ALIGN16(x) (((x+0xf)>>4)<<4)

#define OMX_INIT_STRUCTURE(a) \
  memset (&(a), 0, sizeof (a)); \
  (a).nSize = sizeof (a); \
  (a).nVersion.nVersion = OMX_VERSION; \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

static char *omx_err_to_str(OMX_ERRORTYPE err)
{
	switch (err){
	case OMX_ErrorNone:return "OMX_ErroeNone";
	case OMX_ErrorInsufficientResources: return "OMX_ErrorInsufficientResources";
	case OMX_ErrorUndefined: return "OMX_ErrorUndefined";
	case OMX_ErrorInvalidComponentName: return "OMX_ErrorInvalidComponentName";
	case OMX_ErrorComponentNotFound: return "OMX_ErrorComponentNotFound";
	case OMX_ErrorInvalidComponent: return "OMX_ErrorInvalidComponent";
	case OMX_ErrorBadParameter: return "OMX_ErrorBadParameter";
	case OMX_ErrorNotImplemented: return "OMX_ErrorNotImplemented";
	case OMX_ErrorUnderflow: return "OMX_ErrorUnderflow";
	case OMX_ErrorOverflow: return "OMX_ErrorOverflow";
	case OMX_ErrorHardware: return "OMX_ErrorHardware";
	case OMX_ErrorInvalidState: return "OMX_ErrorInvalidState";
	case OMX_ErrorStreamCorrupt: return "OMX_ErrorStreamCorrupt";
	case OMX_ErrorPortsNotCompatible: return "OMX_ErrorPortsNotCompatible";
	case OMX_ErrorResourcesLost: return "OMX_ErrorResourcesLost";
	case OMX_ErrorNoMore: return "OMX_ErrorNoMore";
	case OMX_ErrorVersionMismatch: return "OMX_ErrorVersionMismatch";
	case OMX_ErrorNotReady: return "OMX_ErrorNotReady";
	case OMX_ErrorTimeout: return "OMX_ErrorTimeout";
	case OMX_ErrorSameState: return "OMX_ErrorSameState";
	case OMX_ErrorResourcesPreempted: return "OMX_ErrorResourcesPreempted";
	case OMX_ErrorPortUnresponsiveDuringAllocation: return "OMX_ErrorPortUnresponsiveDuringAllocation";
	case OMX_ErrorPortUnresponsiveDuringDeallocation: return "OMX_ErrorPortUnresponsiveDuringDeallocation";
	case OMX_ErrorPortUnresponsiveDuringStop: return "OMX_ErrorPortUnresponsiveDuringStop";
	case OMX_ErrorIncorrectStateTransition: return "OMX_ErrorIncorrectStateTransition";
	case OMX_ErrorIncorrectStateOperation: return "OMX_ErrorIncorrectStateOperation";
	case OMX_ErrorUnsupportedSetting: return "OMX_ErrorUnsupportedSetting";
	case OMX_ErrorUnsupportedIndex: return "OMX_ErrorUnsupportedIndex";
	case OMX_ErrorBadPortIndex: return "OMX_ErrorBadPortIndex";
	case OMX_ErrorPortUnpopulated: return "OMX_ErrorPortUnpopulated";
	case OMX_ErrorComponentSuspended: return "OMX_ErrorComponentSuspended";
	case OMX_ErrorDynamicResourcesUnavailable: return "OMX_ErrorDynamicResourcesUnavailable";
	case OMX_ErrorMbErrorsInFrame: return "OMX_ErrorMbErrorsInFrame";
	case OMX_ErrorFormatNotDetected: return "OMX_ErrorFormatNotDetected";
	case OMX_ErrorContentPipeOpenFailed: return "OMX_ErrorContentPipeOpenFailed";
	case OMX_ErrorContentPipeCreationFailed: return "OMX_ErrorContentPipeCreationFailed";
	case OMX_ErrorSeperateTablesUsed: return "OMX_ErrorSeperateTablesUsed";
	case OMX_ErrorTunnelingUnsupported: return "OMX_ErrorTunnelingUnsupported";
	case OMX_ErrorMax: return "OMX_ErrorMax_ILClientError";
	default:return "OMX_UnknownErrorType";
	}
}

static void resize_empty_buffer_done_callback(void *pData, COMPONENT_T *component)
{
	OMX_FFVIDEO_DECODER *ffVideo = (OMX_FFVIDEO_DECODER *)pData;
	fprintf(stderr, "%s ():\n", __FUNCTION__);
	ffVideo->emptyBufferDone = 1;
}

static void resize_fill_buffer_done_callback(void *pData, COMPONENT_T *component)
{
	OMX_FFVIDEO_DECODER *ffVideo = (OMX_FFVIDEO_DECODER *)pData;
	fprintf(stderr, "%s ():\n", __FUNCTION__);
	ffVideo->fillBufferDone = 1;
}

static int init_resize_component(OMX_FFVIDEO_DECODER *ffVideo)
{
	int ret = -1; OMX_ERRORTYPE err;
	if (ffVideo->ilClient == NULL){
		if ((ffVideo->ilClient = ilclient_init()) == NULL){
			fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
			return ret;
		}
	}
	ret=ilclient_create_component(ffVideo->ilClient,&ffVideo->resizeComponent,"resize",
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return ret;
	}

	ffVideo->handle = ilclient_get_handle(ffVideo->resizeComponent);

	OMX_PORT_PARAM_TYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamImageInit, &portParam);
	if (err != OMX_ErrorNone || portParam.nPorts != 2){
		fprintf(stderr, "%s():get resize component's image port parameter failed err:%s",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}

	ffVideo->inputPort = portParam.nStartPortNumber;
	ffVideo->outputPort = portParam.nStartPortNumber + 1;

	ffVideo->pInputBufferHeader = NULL;
	ffVideo->pSettingsChanged = 0;

	ffVideo->emptyBufferDone = 1;
	ffVideo->fillBufferDone = 0;

	ilclient_set_empty_buffer_done_callback(ffVideo->ilClient, resize_empty_buffer_done_callback, ffVideo);
	ilclient_set_fill_buffer_done_callback(ffVideo->ilClient, resize_fill_buffer_done_callback, ffVideo);
	return 0;
}

static int set_resize_input_info(OMX_FFVIDEO_DECODER *ffVideo, int inWidth, int inHeight)
{
	int ret = -1; OMX_ERRORTYPE err;

	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateIdle);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to idle failed\n", __FUNCTION__);
		return ret;
	}

// 	OMX_CONFIG_PORTBOOLEANTYPE brcmSupportsSlices;
// 	OMX_INIT_STRUCTURE(brcmSupportsSlices);
// 	brcmSupportsSlices.nPortIndex = ffVideo->inputPort;
// 	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamBrcmSupportsSlices, &brcmSupportsSlices);
// 	if (err != OMX_ErrorNone){
// 		fprintf(stderr, "%s():get resize component's input port brcmSupportsSlices parameter failed err:%s\n", __FUNCTION__,
// 			omx_err_to_str(err));
// 		return -1;
// 	}

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_INIT_STRUCTURE(portDef);
	portDef.nPortIndex = ffVideo->inputPort;
	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's input port definition parameter failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	portDef.format.image.nFrameWidth = inWidth;
	portDef.format.image.nFrameHeight = inHeight;

	//portDef.format.image.nSliceHeight = (brcmSupportsSlices.bEnabled == OMX_TRUE) ? 16 : 0;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;
	portDef.nBufferSize = ffVideo->nData;
	
	err = OMX_SetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():set resize somponent's input port definition parameter failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}

// 	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
// 	if (err != OMX_ErrorNone){
// 		fprintf(stderr, "%s():get resize component's input port definition parameter failed err:%s\n",
// 			__FUNCTION__, omx_err_to_str(err));
// 		return -1;
// 	}

	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortEnable, ffVideo->inputPort, NULL);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable resize component's input port command failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}

	err = OMX_UseBuffer(ffVideo->handle, &ffVideo->pInputBufferHeader, ffVideo->inputPort, NULL,
		portDef.nBufferSize, (OMX_U8*)ffVideo->pData);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():use buffer for resize component's input port failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}

// 	err = OMX_AllocateBuffer(ffVideo->handle, &ffVideo->pInputBufferHeader, ffVideo->inputPort, NULL, portDef.nBufferSize);
// 	if (err != OMX_ErrorNone){
// 		fprintf(stderr, "%s():alloc buffer for resize component's input port failed err:%s\n",
// 			__FUNCTION__, omx_err_to_str(err));
// 		return -1;
// 	}

	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateExecuting);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to executing failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}
	return 0;
}

static int set_resize_output_info(OMX_FFVIDEO_DECODER *ffVideo, int outWidth, int outHeight)
{
	int ret = -1; OMX_ERRORTYPE err;

	OMX_CONFIG_PORTBOOLEANTYPE brcmSupportsSlices;
	OMX_INIT_STRUCTURE(brcmSupportsSlices);
	brcmSupportsSlices.nPortIndex = ffVideo->outputPort;
	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamBrcmSupportsSlices, &brcmSupportsSlices);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's output port brcmSupportsSlices parameter failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_INIT_STRUCTURE(portDef);
	portDef.nPortIndex = ffVideo->outputPort;
	fprintf(stderr, "%s():22222222222222\n", __FUNCTION__);
	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's output port definition parameter failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	//portDef.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	portDef.format.image.nFrameWidth = outWidth;
	portDef.format.image.nFrameHeight = outHeight;
	portDef.format.image.nSliceHeight = (brcmSupportsSlices.bEnabled == OMX_TRUE) ? 16 : outHeight;
	portDef.format.image.nStride = 0;

	//outFrame->nData = outFrame->nFrameWidth*outFrame->nFrameHeight + (outFrame->nFrameWidth*outFrame->nFrameHeight) / 2;
	//portDef.nBufferSize = outFrame->nData;

	err = OMX_SetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():set reszie component's output port definition parameter failed err:%s", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortEnable, ffVideo->outputPort, NULL);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	// 	ret = ilclient_wait_for_event(ffVideo->resizeComponent, OMX_EventCmdComplete, OMX_CommandPortEnable, 0,
	// 		ffVideo->outputPort, 0, ILCLIENT_EVENT_ERROR | ILCLIENT_PORT_ENABLED, TIMEOUT_MS);
	// 	if (ret != 0){
	// 		fprintf(stderr, "%s():ilclient wait resize component's output port enable aommand complete event failed\n",
	// 			__FUNCTION__);
	// 		return -1;
	// 	}

	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get set reszie component's output port definition parameter failed(after) err:%s", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	fprintf(stderr, "%s():portDef.nBuffersize=%d  calSize=%d \n", __FUNCTION__, portDef.nBufferSize,
		outWidth*outHeight + (outWidth*outHeight) / 2);

	// 	err = OMX_UseBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, 
	// 		outFrame->nData, (OMX_U8*)outFrame->pData);
	// 	if (err != NULL){
	// 		fprintf(stderr, "%s():use buffer for resize component's output port failed err:%s\n", __FUNCTION__,
	// 			omx_err_to_str(err));
	// 		return -1;
	// 	}

	//err = OMX_AllocateBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, portDef.nBufferSize);
	err = OMX_AllocateBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, portDef.nBufferSize);

	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():alloc buffer for resize component's output port failed err:%s\n",
			__FUNCTION__, omx_err_to_str(err));
		return -1;
	}
	fprintf(stderr, "%s():33333333333333\n", __FUNCTION__);
	err = OMX_FillThisBuffer(ffVideo->handle, ffVideo->pOutputBufferHeader);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():fill this buffer failed err:%s\n", __FUNCTION__, omx_err_to_str(err));
		return -1;
	}
	return 0;
}

static int resize_output_settings_changed(OMX_FFVIDEO_DECODER *ffVideo,OMX_FFVIDEO_FRAME *outFrame)
{
	int ret=-1; OMX_ERRORTYPE err;

// 	OMX_CONFIG_PORTBOOLEANTYPE brcmSupportsSlices;
// 	OMX_INIT_STRUCTURE(brcmSupportsSlices);
// 	brcmSupportsSlices.nPortIndex = ffVideo->outputPort;
// 	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamBrcmSupportsSlices, &brcmSupportsSlices);
// 	if (err != OMX_ErrorNone){
// 		fprintf(stderr, "%s():get resize component's output port brcmSupportsSlices parameter failed err:%s\n", __FUNCTION__,
// 			omx_err_to_str(err));
// 		return -1;
// 	}

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_INIT_STRUCTURE(portDef);
	portDef.nPortIndex = ffVideo->outputPort;
	fprintf(stderr, "%s():22222222222222\n", __FUNCTION__);
	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's output port definition parameter failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	//portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	portDef.format.image.nFrameWidth  = outFrame->nFrameWidth;
	portDef.format.image.nFrameHeight = outFrame->nFrameHeight;
	//portDef.format.image.nSliceHeight = (brcmSupportsSlices.bEnabled == OMX_TRUE) ? 16 : outFrame->nFrameHeight;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;
	portDef.nBufferSize = outFrame->nFrameWidth*outFrame->nFrameHeight + (outFrame->nFrameWidth*outFrame->nFrameHeight) / 2;
	//outFrame->nData = outFrame->nFrameWidth*outFrame->nFrameHeight + (outFrame->nFrameWidth*outFrame->nFrameHeight) / 2;
	//portDef.nBufferSize = outFrame->nData;

	err = OMX_SetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():set reszie component's output port definition parameter failed err:%s", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortEnable, ffVideo->outputPort, NULL);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

// 	ret = ilclient_wait_for_event(ffVideo->resizeComponent, OMX_EventCmdComplete, OMX_CommandPortEnable, 0,
// 		ffVideo->outputPort, 0, ILCLIENT_EVENT_ERROR | ILCLIENT_PORT_ENABLED, TIMEOUT_MS);
// 	if (ret != 0){
// 		fprintf(stderr, "%s():ilclient wait resize component's output port enable aommand complete event failed\n",
// 			__FUNCTION__);
// 		return -1;
// 	}

	err = OMX_GetParameter(ffVideo->handle, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get set reszie component's output port definition parameter failed(after) err:%s", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	outFrame->nData = portDef.nBufferSize;
	//outFrame->nData = outFrame->nFrameWidth*outFrame->nFrameHeight + (outFrame->nFrameWidth*outFrame->nFrameHeight) / 2;
	outFrame->pData = (uint8_t *)malloc(portDef.nBufferSize);
	if (outFrame->pData == NULL){
		fprintf(stderr, "%s():alloc for out frame date buffer failed size=%d\n", __FUNCTION__, outFrame->nData);
		return -1;
	}

	fprintf(stderr, "%s():portDef.nBuffersize=%d  calSize=%d \n", __FUNCTION__, portDef.nBufferSize,
		outFrame->nFrameWidth*outFrame->nFrameHeight+(outFrame->nFrameWidth*outFrame->nFrameHeight)/2);

	err = OMX_UseBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, 
		outFrame->nData, (OMX_U8*)outFrame->pData);
	if (err != NULL){
		fprintf(stderr, "%s():use buffer for resize component's output port failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
		return -1;
	}

	//err = OMX_AllocateBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, portDef.nBufferSize);
// 	err = OMX_AllocateBuffer(ffVideo->handle, &ffVideo->pOutputBufferHeader, ffVideo->outputPort, NULL, portDef.nBufferSize);
// 
// 	if (err != OMX_ErrorNone){
// 		fprintf(stderr, "%s():alloc buffer for resize component's output port failed err:%s\n",
// 			__FUNCTION__, omx_err_to_str(err));
// 		return -1;
// 	}
	fprintf(stderr, "%s():33333333333333\n", __FUNCTION__);
	return 0;
}

static int save_output_buffer(OMX_BUFFERHEADERTYPE *pBufferHeader, OMX_FFVIDEO_FRAME *outFrame)
{
	fprintf(stderr, "%s():44444444444444\n", __FUNCTION__);
	fprintf(stderr, "%s():noffset=%d allLen=%d fillLen=%d\n", __FUNCTION__, pBufferHeader->nOffset, pBufferHeader->nAllocLen,
		pBufferHeader->nFilledLen);
	//memset(outFrame->pData, 0, outFrame->nData);
	
	memcpy(outFrame->pData+outFrame->nData, pBufferHeader->pBuffer + pBufferHeader->nOffset,
		pBufferHeader->nFilledLen);
	outFrame->nData += pBufferHeader->nFilledLen;
}

static int omx_resize_picture(OMX_FFVIDEO_DECODER *ffVideo,OMX_FFVIDEO_FRAME *outFrame)
{
	int ret; OMX_ERRORTYPE err;
	OMX_BUFFERHEADERTYPE *pBufferHead=ffVideo->pInputBufferHeader;
	
	//pBufferHead->pBuffer = (OMX_U8*)ffVideo->pData;
	fprintf(stderr, "%s():nalloclen=%d ndata=%d nfilllen=%d\n", __FUNCTION__, pBufferHead->nAllocLen, ffVideo->nData,
		pBufferHead->nFilledLen);
	
	pBufferHead->nFilledLen = ffVideo->nData;
	pBufferHead->nFlags = OMX_BUFFERFLAG_EOS;

	//err = OMX_EmptyThisBuffer(ffVideo->handle, ffVideo->pInputBufferHeader);
	err = OMX_EmptyThisBuffer(ffVideo->handle, pBufferHead);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():empty this buffer faied err:%s\n", __FUNCTION__, omx_err_to_str(err));
		return -1;
	}
	fprintf(stderr, "%s():_________________\n", __FUNCTION__);
	if (ffVideo->pSettingsChanged == 0 && (/*ilclient_remove_event(ffVideo->resizeComponent,OMX_EventPortSettingsChanged,
		ffVideo->outputPort,0,0,1)==0 || */ilclient_wait_for_event(ffVideo->resizeComponent,OMX_EventPortSettingsChanged,
		ffVideo->outputPort, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS)==0)){

		ret = resize_output_settings_changed(ffVideo,outFrame);
		if (ret != 0){
			return -1;
		}
		ffVideo->pSettingsChanged = 1;
		if ((err = OMX_FillThisBuffer(ffVideo->handle, ffVideo->pOutputBufferHeader) != OMX_ErrorNone)){
			fprintf(stderr, "%s():fill this buffer failed err:%s\n", __FUNCTION__, omx_err_to_str(err));
			return -1;
		}
		
	}

	
	if (ilclient_wait_for_event(ffVideo->resizeComponent, OMX_EventBufferFlag, ffVideo->outputPort, 0,
		OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS) != 0){
		fprintf(stderr, "%s():ilclient not receive buffer EOS event\n", __FUNCTION__);
		//return -1;
	}
	//save_output_buffer(ffVideo->outputPort, outFrame);
	return 0;
	//pBufferHead = ilclient_get_output_buffer();

}

static int omx_resize_picture1(OMX_FFVIDEO_DECODER *ffVideo, OMX_FFVIDEO_FRAME *outFrame)
{
	int ret; OMX_ERRORTYPE err;
	OMX_BUFFERHEADERTYPE *pBufferHead = ffVideo->pInputBufferHeader;
	size_t sliceSize = pBufferHead->nAllocLen;
	size_t curPos = 0;
	while (1){
		if (ffVideo->fillBufferDone == 1){
			ffVideo->fillBufferDone = 0;
			save_output_buffer(ffVideo->pOutputBufferHeader, outFrame);
			
			err = OMX_FillThisBuffer(ffVideo->handle, ffVideo->pOutputBufferHeader);
			if (ffVideo->pOutputBufferHeader->nFlags & OMX_BUFFERFLAG_EOS){
				break;
			}
		}
		if (ffVideo->emptyBufferDone == 1){
			ffVideo->emptyBufferDone = 0;

			if (curPos == ffVideo->nData){
				continue;
			}

			if (outFrame->nData - curPos < sliceSize){
				sliceSize = outFrame->nData - curPos;
				pBufferHead->nFlags = OMX_BUFFERFLAG_EOS;
			}
			memcpy(pBufferHead->pBuffer, outFrame->pData + curPos, sliceSize);
			pBufferHead->nOffset = 0;
			pBufferHead->nFilledLen = sliceSize;
			curPos += sliceSize;
			err = OMX_EmptyThisBuffer(ffVideo->handle, pBufferHead);
		}
	}
	return 0;
}

static int do_resize(OMX_FFVIDEO_DECODER *ffVideo, OMX_FFVIDEO_FRAME *outFrame)
{
	int ret; OMX_ERRORTYPE err;
	OMX_BUFFERHEADERTYPE *pBufferHead = ffVideo->pInputBufferHeader;

	pBufferHead->nFilledLen = ffVideo->nData;
	pBufferHead->nFlags = OMX_BUFFERFLAG_EOS;

	err = OMX_EmptyThisBuffer(ffVideo->handle, pBufferHead);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():empty this buffer faied err:%s\n", __FUNCTION__, omx_err_to_str(err));
		return -1;
	}

	if (ffVideo->pSettingsChanged == 0 && (/*ilclient_remove_event(ffVideo->resizeComponent,OMX_EventPortSettingsChanged,
		ffVideo->outputPort,0,0,1)==0 || */ilclient_wait_for_event(ffVideo->resizeComponent, OMX_EventPortSettingsChanged,
		ffVideo->outputPort, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS) == 0)){

		ret = resize_output_settings_changed(ffVideo, outFrame);
		if (ret != 0){
			return -1;
		}
		ffVideo->pSettingsChanged = 1;
		if ((err = OMX_FillThisBuffer(ffVideo->handle, ffVideo->pOutputBufferHeader) != OMX_ErrorNone)){
			fprintf(stderr, "%s():fill this buffer failed err:%s\n", __FUNCTION__, omx_err_to_str(err));
			return -1;
		}

	}


	if (ilclient_wait_for_event(ffVideo->resizeComponent, OMX_EventBufferFlag, ffVideo->outputPort, 0,
		OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS) != 0){
		fprintf(stderr, "%s():ilclient not receive buffer EOS event\n", __FUNCTION__);
		return -1;
	}
	return 0;
	
}
static int close_resize_component(OMX_FFVIDEO_DECODER *ffVideo)
{
	int ret; OMX_ERRORTYPE err;

	if (ffVideo->resizeComponent == NULL){
		return 0;
	}

	if (ffVideo->pOutputBufferHeader){
		err = OMX_FreeBuffer(ffVideo->handle, ffVideo->outputPort, ffVideo->pOutputBufferHeader);
		if (err != NULL){
			fprintf(stderr, "%s():free resize component's output port buffer failed:err:%s\n", __FUNCTION__,
				omx_err_to_str(err));
		}
		else{
			ffVideo->pOutputBufferHeader = NULL;
		}
	}
	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortDisable, ffVideo->outputPort, NULL);
	if (err != NULL){
		fprintf(stderr, "%s():send disable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
	}

	if (ffVideo->pInputBufferHeader){
		err = OMX_FreeBuffer(ffVideo->handle, ffVideo->inputPort, ffVideo->pInputBufferHeader);
		if (err != NULL){
			fprintf(stderr, "%s():free resize component's input port buffer failed:err:%s\n", __FUNCTION__,
				omx_err_to_str(err));
		}
		else{
			ffVideo->pInputBufferHeader = NULL;
		}
	}
	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortDisable, ffVideo->inputPort, NULL);
	if (err != NULL){
		fprintf(stderr, "%s():send disable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
	}

	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateIdle);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to idle failed\n", __FUNCTION__);
	}
	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateLoaded);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to loaded failed\n", __FUNCTION__);
	}

	COMPONENT_T *list[2];
	list[0] = ffVideo->resizeComponent;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	ffVideo->resizeComponent = NULL;

	ffVideo->pSettingsChanged = 0;
	return 0;
}
static int omx_resize_picture2(OMX_FFVIDEO_DECODER *ffVideo, OMX_FFVIDEO_FRAME *outFrame)
{
	int ret = -1;

	if (init_resize_component(ffVideo) != 0){
		fprintf(stderr, "%s():init resize component failed \n", __FUNCTION__);
		return ret;
	}
	if (set_resize_input_info(ffVideo, ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height) != 0){
		fprintf(stderr, "%s():set resize component's input port info failed\n", __FUNCTION__);
		return ret;
	}
	if (do_resize(ffVideo, outFrame) != 0){
		fprintf(stderr, "%s():do resize failed \n", __FUNCTION__);
		return ret;
	}
	if (close_resize_component(ffVideo) != 0){

	}
	return 0;
}

static void convert_rgb2rgba(OMX_FFVIDEO_DECODER *ffVideo, OMX_FFVIDEO_FRAME *outFrame)
{
	
	char *buf=NULL; size_t bufSize = ffVideo->nData; 
	char *src;
	if ((buf = (char *)malloc(bufSize)) == NULL){
		fprintf(stderr, "%s():malloc for buf failed \n", __FUNCTION__);
		return;
	}
	fprintf(stderr, "%s()______________\n", __FUNCTION__);
	memcpy(buf, ffVideo->pFrameYUV420->data[0], bufSize);
	size_t rBytes = srcWidth * 4; size_t stride = ALIGN16(srcWidth) * 4;
	int x, y, line, i, j, offset = 0;
	
	for (line = 0; line < srcHeight;line++){
		src = buf + line*rBytes;
		offset += stride;
		for (x = 0, y = 0 ; x < rBytes; x += 4, y += 3){
			outFrame->pData[offset + x + 3] = 255;
			memcpy(outFrame->pData + offset + x, src + y, 3);
		}
	}
	fprintf(stderr, "%s():+++++++++\n", __FUNCTION__);
	if (buf){
		free(buf); buf = NULL;
	}
}

int omx_init_video_info(char *filePath,OMX_FFVIDEO_DECODER *ffVideo)
{
	int ret = -1;
	av_register_all();
	if (ffVideo->pFormatCtx == NULL){
		ffVideo->pFormatCtx = avformat_alloc_context();
	}
	if (avformat_open_input(&ffVideo->pFormatCtx, filePath, NULL,NULL) != 0){
		fprintf(stderr, "%s():open the input file failed:%s\n", __FUNCTION__, filePath);
		return ret;
	}

	if (avformat_find_stream_info(ffVideo->pFormatCtx, NULL) < 0){
		fprintf(stderr, "%s():find stream info failed\n", __FUNCTION__);
		return ret;
	}

	ffVideo->video_stream_index = -1; int i;
	for (i = 0; i < ffVideo->pFormatCtx->nb_streams; i++){
		if (ffVideo->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			ffVideo->video_stream_index = i;
			break;
		}
	}

	if (ffVideo->pCodecCtx == NULL){
		ffVideo->pCodecCtx = ffVideo->pFormatCtx->streams[ffVideo->video_stream_index]->codec;
	}

	ffVideo->pCodec = avcodec_find_decoder(ffVideo->pCodecCtx->codec_id);
	if (ffVideo->pCodec == NULL){
		fprintf(stderr, "%s():find %d decoder failed\n", __FUNCTION__, ffVideo->pCodecCtx->codec_id);
		return ret;
	}

	if (avcodec_open2(ffVideo->pCodecCtx, ffVideo->pCodec, NULL) < 0){
		fprintf(stderr, "%s():open decoder failed\n", __FUNCTION__);
		return ret;
	}

	ffVideo->pFrame = av_frame_alloc();
	ffVideo->pFrameYUV420 = av_frame_alloc();

// 	ffVideo->nData =(size_t)avpicture_get_size(AV_PIX_FMT_YUV420P,
// 		(ffVideo->pCodecCtx->width), ffVideo->pCodecCtx->height);
// 
// 	ffVideo->pData = (uint8_t *)av_malloc(ffVideo->nData);
// 	avpicture_fill((AVPicture*)ffVideo->pFrameYUV420, ffVideo->pData, AV_PIX_FMT_YUV420P,
// 		(ffVideo->pCodecCtx->width), ffVideo->pCodecCtx->height);
// 
// 	ffVideo->yLen = (ffVideo->pCodecCtx->width)*ffVideo->pCodecCtx->height;
// 	ffVideo->uLen = ((ffVideo->pCodecCtx->width)*ffVideo->pCodecCtx->height) / 4;
// 	ffVideo->vLen = ((ffVideo->pCodecCtx->width)*ffVideo->pCodecCtx->height) / 4;
//	srcWidth = ffVideo->pCodecCtx->width;
//	srcHeight = ffVideo->pCodecCtx->height;
// 
// 	fprintf(stderr, "%s():width=%d height=%d nDate=%d yLen=%d uLen=%d vLen=%d sum=%d\n", __FUNCTION__,
// 		ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height, ffVideo->nData,
// 		ffVideo->yLen, ffVideo->uLen, ffVideo->vLen, ffVideo->yLen + ffVideo->uLen + ffVideo->vLen);

// 	ffVideo->nData = (size_t)avpicture_get_size(AV_PIX_FMT_RGB32,
// 		ALIGN16(ffVideo->pCodecCtx->width), ALIGN16(ffVideo->pCodecCtx->height));
// 
// 	ffVideo->pData = (uint8_t *)av_malloc(ffVideo->nData);
// 	avpicture_fill((AVPicture*)ffVideo->pFrameYUV420, ffVideo->pData, AV_PIX_FMT_RGB32,
// 		ALIGN16(ffVideo->pCodecCtx->width), ALIGN16(ffVideo->pCodecCtx->height));
// 
// 	srcWidth = ALIGN16(ffVideo->pCodecCtx->width);
// 	srcHeight = ALIGN16(ffVideo->pCodecCtx->height);

	//fprintf(stderr, "%s():srcWidth=%d srcHeight=%d nData=%d calData=%d\n", __FUNCTION__, srcWidth, srcHeight,
	//	ffVideo->nData, srcWidth*srcHeight * 3);

	ffVideo->nData = (size_t)avpicture_get_size(AV_PIX_FMT_RGB32,
		ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height);

	ffVideo->pData = (uint8_t *)av_malloc(ffVideo->nData);
	avpicture_fill((AVPicture*)ffVideo->pFrameYUV420, ffVideo->pData, AV_PIX_FMT_RGB32,
		ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height);

	srcWidth = ffVideo->pCodecCtx->width;
	srcHeight = ffVideo->pCodecCtx->height;


	ffVideo->sws_ctx = sws_getContext(ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height, ffVideo->pCodecCtx->pix_fmt, srcWidth, srcHeight,
		AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

	fprintf(stderr, "%s():pix_fmt=%d sws_pix_fmt=%d\n", __FUNCTION__, ffVideo->pCodecCtx->pix_fmt,ffVideo->pCodecCtx->sw_pix_fmt);
	AVStream *stream = ffVideo->pFormatCtx->streams[ffVideo->video_stream_index];
	ffVideo->videoTime = (ffVideo->pFormatCtx->duration) / 1000000;
	ffVideo->frameRate = stream->avg_frame_rate.num / stream->avg_frame_rate.den;

// 	if (init_resize_component(ffVideo) != 0){
// 		fprintf(stderr, "%s():init resize component failed \n", __FUNCTION__);
// 		return ret;
// 	}
// 	if (set_resize_input_info(ffVideo, ffVideo->pCodecCtx->width, ffVideo->pCodecCtx->height) != 0){
// 		fprintf(stderr, "%s():set resize component's input port info failed\n", __FUNCTION__);
// 		return ret;
// 	}
// 	if (set_resize_output_info(ffVideo, sign_width, sign_height) != 0){
// 		fprintf(stderr, "%s():set resize component's output port info failed\n", __FUNCTION__);
// 		return ret;
// 	}

	fprintf(stderr, "%s():init video down\n", __FUNCTION__);
	return 0;
}

int omx_decode_video_next_frame(OMX_FFVIDEO_DECODER *ffVideo,OMX_FFVIDEO_FRAME *outFrame)
{
	
	int got_picture, ret;
	AVPacket packet;
	av_init_packet(&packet);
	while (av_read_frame(ffVideo->pFormatCtx,&packet)>=0){
		if (packet.stream_index == ffVideo->video_stream_index){
			//ret = avcodec_decode_video2(ffVideo->pCodecCtx, ffVideo->pFrameYUV420, &got_picture, &packet);
			ret = avcodec_decode_video2(ffVideo->pCodecCtx, ffVideo->pFrame, &got_picture, &packet);
			if (ret < 0){
				fprintf(stderr, "%s():decode current video frame failed\n", __FUNCTION__);
				av_packet_unref(&packet);
				av_free_packet(&packet);
				omx_deinit_video_info(ffVideo);
				return -1;
			}
			if (got_picture){
				//fprintf(stderr, "%s():format=%d\n", __FUNCTION__, ffVideo->pFrame->format);
				sws_scale(ffVideo->sws_ctx, (const uint8_t* const*)ffVideo->pFrame->data, ffVideo->pFrame->linesize,
					0, ffVideo->pCodecCtx->height, ffVideo->pFrameYUV420->data, ffVideo->pFrameYUV420->linesize);
// 				memcpy(ffVideo->pData, ffVideo->pFrameYUV420->data[0], ffVideo->yLen);
// 				memcpy(ffVideo->pData + ffVideo->yLen, ffVideo->pFrameYUV420->data[1], ffVideo->uLen);
// 				memcpy(ffVideo->pData + ffVideo->yLen + ffVideo->uLen, ffVideo->pFrameYUV420->data[2],
// 					ffVideo->vLen);

// 				memcpy(outFrame->pData, ffVideo->pFrameYUV420->data[0], ffVideo->yLen);
// 				memcpy(outFrame->pData + ffVideo->yLen, ffVideo->pFrameYUV420->data[2], ffVideo->uLen);
// 				memcpy(outFrame->pData + ffVideo->yLen + ffVideo->uLen, ffVideo->pFrameYUV420->data[1], ffVideo->vLen);

// 				memcpy(outFrame->pData, ffVideo->pFrame->data[0], ffVideo->yLen);
// 				memcpy(outFrame->pData + ffVideo->yLen, ffVideo->pFrame->data[1], ffVideo->uLen);
//  			memcpy(outFrame->pData + ffVideo->yLen + ffVideo->uLen, ffVideo->pFrame->data[2], ffVideo->vLen);


				//memcpy(outFrame->pData, ffVideo->pFrameYUV420->data[0], ffVideo->nData);
				int x, y, linBytes; linBytes = srcWidth * 4; 
				size_t offset = 0; size_t off = 0;
				size_t stride = ALIGN16(srcWidth) * 4;
				for (y = 0; y < srcHeight; y++){
					offset = y*linBytes;
					memcpy(outFrame->pData + off, ffVideo->pFrameYUV420->data[0] + offset, linBytes);
					off += stride;
				}
				//fprintf(stderr, "%s():1111111111111111\n", __FUNCTION__);
				//convert_rgb2rgba(ffVideo, outFrame);
				
				//if (omx_resize_picture(ffVideo, outFrame) != 0)
				//	break;
// 				if (omx_resize_picture1(ffVideo, outFrame) != 0)
// 					break;
				//if (omx_resize_picture2(ffVideo, outFrame) != 0)
				//	break;

				av_packet_unref(&packet);
				av_free_packet(&packet);
				return 0;
			}
		}
		av_packet_unref(&packet);
		av_free_packet(&packet);
	}
	av_packet_unref(&packet);
	av_free_packet(&packet);
	omx_deinit_video_info(ffVideo);
	return -1;
}

static int deinit_resize_component(OMX_FFVIDEO_DECODER *ffVideo)
{
	int ret; OMX_ERRORTYPE err;

	if (ffVideo->resizeComponent == NULL){
		return 0;
	}

	if (ffVideo->pOutputBufferHeader){
		err = OMX_FreeBuffer(ffVideo->handle, ffVideo->outputPort, ffVideo->pOutputBufferHeader);
		if (err != NULL){
			fprintf(stderr, "%s():free resize component's output port buffer failed:err:%s\n", __FUNCTION__,
				omx_err_to_str(err));
		}
		else{
			ffVideo->pOutputBufferHeader = NULL;
		}
	}
	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortDisable, ffVideo->outputPort, NULL);
	if (err != NULL){
		fprintf(stderr, "%s():send disable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
	}

	if (ffVideo->pInputBufferHeader){
		err = OMX_FreeBuffer(ffVideo->handle, ffVideo->inputPort, ffVideo->pInputBufferHeader);
		if (err != NULL){
			fprintf(stderr, "%s():free resize component's input port buffer failed:err:%s\n", __FUNCTION__,
				omx_err_to_str(err));
		}
		else{
			ffVideo->pInputBufferHeader = NULL;
		}
	}
	err = OMX_SendCommand(ffVideo->handle, OMX_CommandPortDisable, ffVideo->inputPort, NULL);
	if (err != NULL){
		fprintf(stderr, "%s():send disable resize component's output port command failed err:%s\n", __FUNCTION__,
			omx_err_to_str(err));
	}

	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateIdle);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to idle failed\n", __FUNCTION__);
	}
	ret = ilclient_change_component_state(ffVideo->resizeComponent, OMX_StateLoaded);
	if (ret == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to loaded failed\n", __FUNCTION__);
	}

	COMPONENT_T *list[2];
	list[0] = ffVideo->resizeComponent;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	ffVideo->resizeComponent = NULL;

	if (ffVideo->ilClient){
		ilclient_destroy(ffVideo->ilClient);
		ffVideo->ilClient = NULL;
	}
	return 0;
}

int omx_deinit_video_info(OMX_FFVIDEO_DECODER *ffVideo)
{
	int ret;

	//ret = deinit_resize_component(ffVideo);
	if (ffVideo->sws_ctx){
		sws_freeContext(ffVideo->sws_ctx); ffVideo->sws_ctx = NULL;
	}
	if (ffVideo->pData){
		av_free(ffVideo->pData); ffVideo->pData = NULL;
	}
	
	if (ffVideo->pFrameYUV420){
		av_frame_free(&ffVideo->pFrameYUV420); ffVideo->pFrameYUV420 = NULL;
	}
	if (ffVideo->pFrame){
		av_frame_free(&ffVideo->pFrame); ffVideo->pFrame = NULL;
	}
	if (ffVideo->pCodecCtx){
		avcodec_close(ffVideo->pCodecCtx); ffVideo->pCodec = NULL; ffVideo->pCodecCtx = NULL;
	}
	if (ffVideo->pFormatCtx){
		avformat_close_input(&ffVideo->pFormatCtx);
		avformat_free_context(ffVideo->pFormatCtx);
		ffVideo->pFormatCtx = NULL;
	}
	return 0;
}