/*
* omx_resize.c
*/

#include <stdio.h>
#include <stdlib.h>

#include <IL/OMX_Core.h>
#include "omx_image.h"
#include "omx_typedef.h"
#include "omx_resize.h"

static OMX_ERRORTYPE init_resizer(RESIZER *resizer)
{
	OMX_ERRORTYPE ret; int ilRet;

	//init resize component and get it's handle
	//the component in loaded state and all ports disable

	ilRet = ilclient_create_component(resizer->il_client, &resizer->component, "resize",
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	resizer->handle = ilclient_get_handle(resizer->component);

	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;
	ret=OMX_GetParameter(resizer->handle, OMX_IndexParamImageInit, &portParam);
	if (ret != OMX_ErrorNone || portParam.nPorts!=2){
		fprintf(stderr, "%s():resizer image port parameter error\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	resizer->inputPort = portParam.nStartPortNumber;
	resizer->outPort = portParam.nStartPortNumber + 1;

	resizer->pOutputBufferHeader = NULL;

	return OMX_ErrorNone;

}
static OMX_ERRORTYPE close_resizer(RESIZER *resizer)
{
	OMX_ERRORTYPE ret; int ilRet;

	ret = OMX_FreeBuffer(resizer->handle, resizer->inputPort, resizer->pInputBufferHeader);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():free input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ret = OMX_SendCommand(resizer->handle, OMX_CommandPortDisable, resizer->inputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send disable input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilRet = ilclient_wait_for_event(resizer->component, OMX_EventCmdComplete,
		OMX_CommandPortDisable, 0, resizer->inputPort, 0,ILCLIENT_PORT_DISABLED, TIMEOUT_MS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient wait input port disable failed ilRet=%d\n", __FUNCTION__, ilRet);
	}

	ret=OMX_SendCommand(resizer->handle, OMX_CommandFlush, resizer->outPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send port flush command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilRet=ilclient_wait_for_event(resizer->component, OMX_EventCmdComplete, OMX_CommandFlush,
		0, resizer->outPort, 0, ILCLIENT_PORT_FLUSH, TIMEOUT_MS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient wait output port flush failed ilRet=%d\n", __FUNCTION__, ilRet);
	}

	ret = OMX_FreeBuffer(resizer->handle, resizer->outPort, resizer->pOutputBufferHeader);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():free output port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ret = OMX_SendCommand(resizer->handle, OMX_CommandPortDisable, resizer->outPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send port disable command for ourput port %d failed err:%s\n", __FUNCTION__,
			resizer->outPort, OMX_ErrorToString(ret));
	}

	ilclient_change_component_state(resizer->component, OMX_StateIdle);
	ilclient_change_component_state(resizer->component, OMX_StateLoaded);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE startup_resizer(RESIZER *resizer,IMAGE *inImage)
{
	OMX_ERRORTYPE ret; int ilRet;

	ilclient_change_component_state(resizer->component, OMX_StateIdle);

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = resizer->inputPort;
	ret = OMX_GetParameter(resizer->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get port definition parameter of port %d failed err:%s\n",
			__FUNCTION__, resizer->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	//set the input image info
	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = inImage->colorFormat;
	portDef.format.image.nFrameWidth = inImage->width;
	portDef.format.image.nSliceHeight = inImage->height;
	portDef.format.image.nStride = 0;
	portDef.format.image.nSliceHeight = 0;

	portDef.nBufferSize = inImage->nData;

	ret = OMX_SetParameter(resizer->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set resizer input port %d parameter failed err:%s\n", __FUNCTION__,
			resizer->inputPort, OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(resizer->handle, OMX_CommandPortEnable, resizer->inputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send port enable command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_UseBuffer(resizer->handle, &resizer->pInputBufferHeader, resizer->inputPort,
		NULL, portDef.nBufferSize, (OMX_U8*)inImage->pData);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():use input image pixel data buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ilRet=ilclient_change_component_state(resizer->component, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change component to excuting state failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE resizer_port_settings_changed(RESIZER *resizer,IMAGE *outImage)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = resizer->outPort;
	ret = OMX_GetParameter(resizer->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get out port definition parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	//set the output port image info
	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	//portDef.format.image.eColorFormat = OMX_COLOR_Format24bitRGB888;
	//portDef.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.image.nFrameWidth = outImage->width;
	portDef.format.image.nFrameHeight = outImage->height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

	ret = OMX_SetParameter(resizer->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set output port %d definition parameter failed err:%s\n", __FUNCTION__,
			resizer->outPort,OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_GetParameter(resizer->handle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get input port definition parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(resizer->handle, OMX_CommandPortEnable, resizer->outPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send enabld port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	outImage->nData = portDef.nBufferSize;
	outImage->pData = (uint8_t *)malloc(outImage->nData);
	if (outImage->pData == NULL){
		fprintf(stderr, "%():alloc for %d byte buf failed\n", __FUNCTION__, outImage->nData);
		return OMX_ErrorMax;
	}

	ret = OMX_UseBuffer(resizer->handle, &resizer->pOutputBufferHeader, resizer->outPort,
		NULL, portDef.nBufferSize, (OMX_U8*)outImage->pData);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():use output port %d buffer failed err:%s\n", __FUNCTION__,
			resizer->outPort, OMX_ErrorToString(ret));
		return ret;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE do_resize(RESIZER *resizer,IMAGE *inImage,IMAGE *outImage)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_BUFFERHEADERTYPE *pBufferHeader = resizer->pInputBufferHeader;
	pBufferHeader->nFilledLen = inImage->nData;
	pBufferHeader->nFlags = OMX_BUFFERFLAG_EOS;

	ret = OMX_EmptyThisBuffer(resizer->handle, pBufferHeader);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():empty input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	if (ilclient_wait_for_event(resizer->component, OMX_EventPortSettingsChanged, resizer->outPort,
		0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS) == 0){

		FreeImage(inImage);
		ret= resizer_port_settings_changed(resizer, outImage);
		if (ret == OMX_ErrorNone){
			if ((ret=OMX_FillThisBuffer(resizer->handle,resizer->pOutputBufferHeader))
				!= OMX_ErrorNone) {
				fprintf(stderr, "%s():fill output port buffer failed err:%s\n", __FUNCTION__,
					OMX_ErrorToString(ret));
				return ret;
			}
		}
	}

	//not receive the end signal
	if (ilclient_wait_for_event(resizer->component, OMX_EventBufferFlag, resizer->outPort, 0,
		OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS) != 0){
		fprintf(stderr, "%s():ilclient not receive buffer EOS event\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	ret = close_resizer(resizer);
	return ret;
}

int omx_resize_image(ILCLIENT_T *ilclient, IMAGE *inImage, IMAGE *outImage)
{
	RESIZER resizer;
	resizer.il_client = ilclient;
	resizer.pInputBufferHeader = NULL;
	OMX_ERRORTYPE ret = init_resizer(&resizer);
	if (ret != OMX_ErrorNone){
		return -1;
	}
	ret = startup_resizer(&resizer, inImage);
	if (ret != OMX_ErrorNone){
		return -1;
	}
	ret = do_resize(&resizer, inImage, outImage);
	if (ret != OMX_ErrorNone){
		return -1;
	}

	COMPONENT_T *list[2];
	list[0] = resizer.component;
	list[1] = NULL;
	ilclient_cleanup_components(list);
	return ret;
}