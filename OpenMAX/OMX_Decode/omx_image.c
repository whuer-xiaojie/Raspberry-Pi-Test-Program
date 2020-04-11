/*
*omx_image.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Types.h>

#include "ilclient.h"
#include "omx_typedef.h"
#include "omx_image.h"

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       empty_buffer_done_callback
* Description:while the component send a signal emptyBufferDone this 
*             function will be called,update the decoder emptied buffer
*             count,and send a thread signal to the lock
* Input:      void * pData  
* Input:      COMPONENT_T * component
* Output:     
* Returns:    void
*********************************************************************/
static void empty_buffer_done_callback(void *pData, COMPONENT_T *component)
{
	IMAGE_DECODER *decoder = (IMAGE_DECODER *)pData;
	decoder->emptyBDone--;
	pthread_mutex_lock(&decoder->lock);
	pthread_cond_signal(&decoder->cond);
	pthread_mutex_unlock(&decoder->lock);
}

static void fill_buffer_done_callBack(void *pData, COMPONENT_T *component)
{

}

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       port_settings_changed_event
* Description:according the output port info to get the image pixel info
*             allocate buf and get image pixel data
* Input:      IMAGE_DECODER * decoder
* Input:      IMAGE * image
* Output:     
* Returns:    OMX_ERRORTYPE  --If without any error, the return code will
*             be OMX_ErrorNone.  Otherwise the appropriate
*			  OMX error will be returned.
*********************************************************************/
static OMX_ERRORTYPE port_settings_changed_event(IMAGE_DECODER *decoder, IMAGE *image)
{
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_ERRORTYPE ret;

	//init the structure
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = decoder->outPort;

	//get output port info to the structure
	ret = OMX_GetParameter(decoder->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get parameter portdefinition failed err:%s\n",
			__FUNCTION__,OMX_ErrorToString(ret));
		return ret;
	}

	//get the image size info from output port setting
	image->width = portDef.format.image.nFrameWidth;
	image->height = portDef.format.image.nFrameHeight;
	image->nData = portDef.nBufferSize;
	image->codingType = portDef.format.image.eCompressionFormat;
	image->colorFormat = portDef.format.image.eColorFormat;

	//enable output port
	ret = OMX_SendCommand(decoder->handle, OMX_CommandPortEnable,
		decoder->outPort,NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():enable output port %d failed err:%s\n",
			__FUNCTION__, decoder->outPort, OMX_ErrorToString(ret));
		return ret;
	}

	//alloc image pixel data buf
	image->pData = (uint8_t *)malloc(image->nData);
	if (image->pData == NULL){
		fprintf(stderr, "%s():alloc %d byte buf failed \n", __FUNCTION__, image->nData);
		return OMX_ErrorMax;
	}
	memset(image->pData, 0, image->nData);
	//get the output port buf to the image date buf
	ret = OMX_UseBuffer(decoder->handle, &decoder->pOutputBufferHeader, decoder->outPort,
		NULL, portDef.nBufferSize, (OMX_U8*)image->pData);

	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s(): use output port %d buffer failed err:%s\n", __FUNCTION__,
			decoder->outPort, OMX_ErrorToString(ret));
		return ret;
	}
	return OMX_ErrorNone;
}

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       init_image_decoder
* Description:create component and get handler,get image input/output info  
* Input:      IMAGE_DECODER * decoder
* Output:     
* Returns:    OMX_ERRORTYPE
*********************************************************************/
static OMX_ERRORTYPE init_image_decoder(IMAGE_DECODER *decoder)
{
	OMX_ERRORTYPE ret; int ilRet;

	//init the image_decode component and get it's handle
	//the component in loaded state and all ports disable
	ilRet = ilclient_create_component(decoder->il_client, &decoder->component,"image_decode", 
		ILCLIENT_DISABLE_ALL_PORTS |ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create image_decode component failed\n", 
			__FUNCTION__);
		return OMX_ErrorMax;
	}
	//get image_decode component handler
	decoder->handle = ilclient_get_handle(decoder->component);

	//get the image_deocder input port and output port info
	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;

	ret = OMX_GetParameter(decoder->handle, OMX_IndexParamImageInit, &portParam);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get parameter image port param failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	if (portParam.nPorts != 2){
		fprintf(stderr, "%s():component image port not 2\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	//set decoder input and output port index
	decoder->inputPort = portParam.nStartPortNumber;
	decoder->outPort = portParam.nStartPortNumber + 1;
	decoder->pOutputBufferHeader = NULL;

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE close_image_decoder(IMAGE_DECODER *decoder,int outPortSettingChanged,
	int bFilled)
{
	OMX_ERRORTYPE ret;
	int i; int ilRet;
	for (i = 0; i < DECODER_BUFFER_COUNT;i++){
		ret = OMX_FreeBuffer(decoder->handle, decoder->inputPort, decoder->ppInputBufferHeader[i]);
		if (ret != OMX_ErrorNone){
			fprintf(stderr, "%s():free input port %d pbuffer %d failed err:%s\n", __FUNCTION__,
				decoder->inputPort, i, OMX_ErrorToString(ret));
			return ret;
		}
	}

	ret = OMX_SendCommand(decoder->handle, OMX_CommandPortDisable, decoder->inputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set command disable input port %d failed err:%s\n", __FUNCTION__,
			decoder->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	ilRet=ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete, OMX_CommandPortDisable,
		0, decoder->inputPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);
	if (ilRet != 0){
		fprintf(stderr, "%s():disable input port command not finish err=%d\n", __FUNCTION__, ilRet);
	}

	if (bFilled == 1){
		ret=OMX_SendCommand(decoder->handle, OMX_CommandFlush, decoder->outPort, NULL);
		if (ret != OMX_ErrorNone){
			fprintf(stderr, "%s():send command flush output port %d failed err:%s\n", __FUNCTION__,
				decoder->outPort, OMX_ErrorToString(ret));
			return ret;
		}

		ilRet=ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete, OMX_CommandFlush,
			0, decoder->outPort, 0, ILCLIENT_PORT_FLUSH, TIMEOUT_MS);
		if (ilRet != 0){
			fprintf(stderr, "%s():flush output port command not finish err=%d\n", __FUNCTION__, ilRet);
		}

		ret = OMX_FreeBuffer(decoder->handle, decoder->outPort, decoder->pOutputBufferHeader);
		if (ret != OMX_ErrorNone){
			fprintf(stderr, "%s():free output port buffer failed err:%s\n", __FUNCTION__, OMX_ErrorToString(ret));
			return ret;
		}

	}

	if (outPortSettingChanged == 1){
		ret = OMX_SendCommand(decoder->handle, OMX_CommandPortDisable, decoder->outPort, NULL);
		if (ret != OMX_ErrorNone){
			fprintf(stderr, "%s():send disable port command failed err:%s\n", __FUNCTION__, OMX_ErrorToString(ret));
			return ret;
		}

		ilRet = ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete, OMX_CommandPortDisable,
			0, decoder->outPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);
		if (ilRet != 0){
			fprintf(stderr, "%s():disable output port command not finish err=%d\n", __FUNCTION__, ilRet);
		}
	}

	ilRet=ilclient_change_component_state(decoder->component, OMX_StateIdle);
	if (ilRet != 0){
		fprintf(stderr, "%s():change component state to idle failed\n", __FUNCTION__);
	}
	ilRet=ilclient_change_component_state(decoder->component, OMX_StateLoaded);
	if (ilRet != 0){
		fprintf(stderr, "%s():change component state to loaded failed\n", __FUNCTION__);
	}

	return OMX_ErrorNone;
}

//should call after init_image_decodee() called 
static OMX_ERRORTYPE startup_image_decoder(IMAGE_DECODER *decoder,
	OMX_IMAGE_CODINGTYPE codingType)
{
	int ilRet; OMX_ERRORTYPE ret;
	//change component from loaded state to idle stale
	ilRet = ilclient_change_component_state(decoder->component, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change component to idle state failed\n",
			__FUNCTION__);
		return OMX_ErrorMax;
	}

	//tell the input port input image encoding type
	OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
	//init the structure
	memset(&imagePortFormat, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
	imagePortFormat.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
	imagePortFormat.nVersion.nVersion = OMX_VERSION;
	imagePortFormat.nPortIndex = decoder->inputPort;
	imagePortFormat.eCompressionFormat = codingType;
	ret=OMX_SetParameter(decoder->handle, OMX_IndexParamImagePortFormat,
		&imagePortFormat);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set parameter image port format %d failed err:%s\n",
			__FUNCTION__, decoder->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = decoder->inputPort;
	ret = OMX_GetParameter(decoder->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get parameter port definition %d failed err:%s\n",
			__FUNCTION__, decoder->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	//set input port needed buffer count
	portDef.nBufferCountActual = DECODER_BUFFER_COUNT;
	ret = OMX_SetParameter(decoder->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set parameter port definition %d failed err:%s\n",
			__FUNCTION__, decoder->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	//enable input port and alloc buffer
	//ilclient_enable_port_buffers(decoder->component, decoder->inputPort, NULL, NULL, NULL);
	//ilclient_enable_port(decoder->component, decoder->inputPort);
	ret = OMX_SendCommand(decoder->handle, OMX_CommandPortEnable, decoder->inputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send command enable port %d failed err:%s\n", __FUNCTION__,
			decoder->inputPort, OMX_ErrorToString(ret));
		return ret;
	}
	int i;
	for (i = 0; i < DECODER_BUFFER_COUNT; i++){
		if ((ret = OMX_AllocateBuffer(decoder->handle, &(decoder->ppInputBufferHeader[i]),
			decoder->inputPort, NULL, portDef.nBufferSize)) != OMX_ErrorNone){
			fprintf(stderr, "%s():alloc output buffer[%d] failed err:%s\n", __FUNCTION__,
				i, OMX_ErrorToString(ret));
			return ret;
		}
	}

	ilRet = ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete, OMX_CommandPortEnable
		, 0, decoder->inputPort, 0, 0, TIMEOUT_MS);
	if (ilRet != 0){
		fprintf(stderr, "%s():enable input port %d command not finished\n", __FUNCTION__,
			decoder->inputPort);
		return OMX_ErrorMax;
	}

	//change image_decode component form idle state to executing state
	//ret = OMX_SendCommand(decoder->handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
	//if (ret != OMX_ErrorNone) {
	//	return ret;
	//}
	ilRet=ilclient_change_component_state(decoder->component, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change component statr to executing failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE decode_image(IMAGE_DECODER *decoder, FILE *fp, IMAGE *image)
{
	int8_t pSettingsChanged = 0;
	int8_t fileEnd = 0;
	int8_t bufFlagEos = 0;//the buffer header end flag 
	int8_t bFilled = 0;   //fill this buffer header flag
	int bufferIndex = 0;
	int ilRet = 0;
	OMX_ERRORTYPE ret = OMX_ErrorNone;
	struct timespec wait;

	OMX_BUFFERHEADERTYPE *pBufferHeader = decoder->ppInputBufferHeader[bufferIndex];
	ilclient_set_empty_buffer_done_callback(decoder->il_client, empty_buffer_done_callback,
		decoder);

	bufferIndex =1;
	pBufferHeader->nFilledLen = fread(pBufferHeader->pBuffer, 1, pBufferHeader->nAllocLen, fp);
	pBufferHeader->nOffset = 0; pBufferHeader->nFlags = 0;

	//if arrive the end of this image
	if (feof(fp)){
		pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
	}
	else if(pBufferHeader->nFilledLen !=pBufferHeader->nAllocLen){
		ret = OMX_ErrorMax;
		fileEnd = 1;
	}

	decoder->emptyBDone = 0;//the count of component emptied buffer 
	pthread_mutex_init(&decoder->lock, NULL);
	pthread_cond_init(&decoder->cond, NULL);

	//decode the file all data if with non-error
	while (fileEnd==0 && ret==OMX_ErrorNone){
		// We've got an EOS event early this usually means that we are done decoding
		if (ilclient_remove_event(decoder->component, OMX_EventBufferFlag, decoder->outPort,
			0, OMX_BUFFERFLAG_EOS, 0) == 0){
			bufFlagEos = 1; break;
		}
		//decode one buffer
		decoder->emptyBDone++;
		ret = OMX_EmptyThisBuffer(decoder->handle, pBufferHeader);
		if (ret != OMX_ErrorNone){
			fprintf(stderr, "%s():empty this buffer failed err:%s\n", __FUNCTION__,
				OMX_ErrorToString(ret));
			break;
		}

		//if file not end continue to read and decode
		if (!feof(fp)){
			pBufferHeader = decoder->ppInputBufferHeader[bufferIndex];
			pBufferHeader->nFilledLen = fread(pBufferHeader->pBuffer, 1, pBufferHeader->nAllocLen, fp);
			pBufferHeader->nOffset = 0; pBufferHeader->nFlags = 0;

			//update the buffer index
			bufferIndex = (bufferIndex + 1) % DECODER_BUFFER_COUNT;

			//if arrive the end of this image
			if (feof(fp)){
				pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
			}
			else if (pBufferHeader->nFilledLen != pBufferHeader->nAllocLen){
				ret = OMX_ErrorMax;
				break;
			}
		}
		else{
			fileEnd = 1;
		}

		//the output port had fill the buffer
		if (pSettingsChanged == 0 && ilclient_remove_event(decoder->component,
			OMX_EventPortSettingsChanged, decoder->outPort, 0, 0, 1) == 0){
			pSettingsChanged = 1;
			ret = port_settings_changed_event(decoder, image);
		}

		//wait for empty buffer done
		int n;
		for (n = 0; n < 20 && decoder->emptyBDone != 0; n++){
			clock_gettime(CLOCK_REALTIME, &wait);
			wait.tv_nsec += MAX_EMPTY_BUFFER_WAIT_TIME * 1000000L;
			wait.tv_sec += wait.tv_nsec / 1000000000L;
			wait.tv_nsec %= 1000000000L;
			if (decoder->emptyBDone != 0){
				pthread_mutex_lock(&decoder->lock);
				pthread_cond_timedwait(&decoder->cond, &decoder->lock, &wait);
				pthread_mutex_unlock(&decoder->lock);
				if (decoder->emptyBDone < DECODER_BUFFER_COUNT - 1)
					break;
			}
			else
				break;
		}

		if (pSettingsChanged == 0 && ilclient_remove_event(decoder->component,
			OMX_EventPortSettingsChanged, decoder->outPort, 0, 0, 1) == 0){
			pSettingsChanged = 1;
			ret = port_settings_changed_event(decoder, image);
		}

		//fill the output port buffer again if not 
		if (bFilled == 0 && pSettingsChanged == 1 && ret == OMX_ErrorNone){
			ret = OMX_FillThisBuffer(decoder->handle, decoder->pOutputBufferHeader);
			if (ret != OMX_ErrorNone){
				fprintf(stderr, "%s():fill this buffer failed\n", __FUNCTION__);
				break;
			}
			bFilled = 1;
		}
	}
	ilclient_set_empty_buffer_done_callback(decoder->il_client, NULL, NULL);

	if (pSettingsChanged == 0 && ilclient_wait_for_event(decoder->component,
		OMX_EventPortSettingsChanged, decoder->outPort, 0, 0, 1,
		ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS) == 0){
		pSettingsChanged = 1;
		ret = port_settings_changed_event(decoder, image);
		if (ret == OMX_ErrorNone){
			ret = OMX_FillThisBuffer(decoder->handle, decoder->pOutputBufferHeader);
			bFilled = 1;
		}
	}
	if (bFilled == 1 && !bufFlagEos && ilclient_wait_for_event(decoder->component, OMX_EventBufferFlag,
		decoder->outPort, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS) != 0){
		fprintf(stderr, "%s():ilclient wait for buffer event EOS failed\n", __FUNCTION__);
		ret = OMX_ErrorMax;
	}

	pthread_mutex_destroy(&decoder->lock);
	pthread_cond_destroy(&decoder->cond);

	ret = close_image_decoder(decoder, pSettingsChanged, bFilled);
// 	int i = 0;
// 	for (i = 0; i < DECODER_BUFFER_COUNT; i++) {
// 		 ret = OMX_FreeBuffer(decoder->handle, decoder->inputPort, decoder->ppInputBufferHeader[i]);
// 	}
// 
// 	ret = OMX_SendCommand(decoder->handle, OMX_CommandPortDisable, decoder->inputPort, NULL);
// 
// 	ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete,OMX_CommandPortDisable, 
// 		0, decoder->inputPort, 0,ILCLIENT_PORT_DISABLED, TIMEOUT_MS);
// 
// 	if (bFilled == 1){
// 		OMX_SendCommand(decoder->handle, OMX_CommandFlush, decoder->outPort, NULL);
// 
// 		ilclient_wait_for_event(decoder->component, OMX_EventCmdComplete, OMX_CommandFlush,
// 			0, decoder->outPort, 0, ILCLIENT_PORT_FLUSH, TIMEOUT_MS);
// 
// 		ret = OMX_FreeBuffer(decoder->handle, decoder->outPort, decoder->pOutputBufferHeader);
// 
// 		if (ret != OMX_ErrorNone){
// 			ret = OMX_ErrorMax;
// 		}
// 
// 	}
// 
// 	if (pSettingsChanged == 1){
// 		ret = OMX_SendCommand(decoder->handle, OMX_CommandPortDisable, decoder->outPort, NULL);
// 
// 		if (ret != OMX_ErrorNone){
// 			ret = OMX_ErrorMax;
// 		}
// 	}
// 
// 	ilclient_change_component_state(decoder->component, OMX_StateIdle);
// 	ilclient_change_component_state(decoder->component, OMX_StateLoaded);

	return ret;	
}

int omx_decode_image(char *fileName,ILCLIENT_T *ilclient,IMAGE *image)
{
	IMAGE_DECODER decoder;
	FILE *fp;
	OMX_ERRORTYPE ret;

	if ((fp = fopen(fileName, "r")) == NULL){
		fprintf(stderr, "%s():open %s failed \n", __FUNCTION__, fileName);
		return -1;
	}

	decoder.il_client = ilclient;
	ret = init_image_decoder(&decoder); 
	if (ret != OMX_ErrorNone){
		return -1;
	}

	//ret = startup_image_decoder(&decoder, OMX_IMAGE_CodingJPEG);
	//ret = startup_image_decoder(&decoder, OMX_IMAGE_CodingBMP);
	//ret = startup_image_decoder(&decoder, OMX_IMAGE_CodingPNG);
	ret = startup_image_decoder(&decoder, OMX_IMAGE_CodingGIF);
	if (ret != OMX_ErrorNone){
		return -1;
	}

	ret = decode_image(&decoder, fp, image);
	if (ret != OMX_ErrorNone){
		return -1;
	}

	COMPONENT_T *list[2];
	list[0] = decoder.component;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	return 0;
}