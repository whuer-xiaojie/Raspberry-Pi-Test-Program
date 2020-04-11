/*
* OMX/omx_graphics.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Broadcom.h>


#include "bcm_host.h"
#include "omx_graphics.h"

#define ALIGN16(x) (((x+0xf)>>4)<<4)
/* screen size box.c */
extern int sign_width;
extern int sign_height;
/* ../display.c */
extern size_t pixel_size;

ILCLIENT_T *il_client = NULL;
RENDER_DISPLAY_CONFIG dispConfig;
RENDER render;
static size_t nBufferSize;

static char* OMX_ErrorToString(int err)
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

	case ILC_ErrorComponentCreate:
	case ILC_ErrorHandleGet:
	case ILC_ErrorEventWait:
	case ILC_ErrorStateChange:
	default:return "OMX_UnknownErrorType";
	}
}

static OMX_ERRORTYPE init_render(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;

	render->ilClient = il_client;
	render->renderConfig = &dispConfig;
	render->pSettingsChanged = 0;

	//create render component and get it's handle
	ilRet = ilclient_create_component(render->ilClient, &render->renderComponent, "video_render",
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create video_render component failed\n", __FUNCTION__);
		return ILC_ErrorComponentCreate;
	}
	render->renderHandle = ilclient_get_handle(render->renderComponent);
	if (render->renderHandle == NULL){
		fprintf(stderr, "%s():ilclient get video_render component's handle failed\n", __FUNCTION__);
		return ILC_ErrorHandleGet;
	}

	//get render input port info and change state to idle
	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;
	ret = OMX_GetParameter(render->renderHandle, OMX_IndexParamVideoInit, &portParam);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get video_render input port parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	render->renderInputPort = portParam.nStartPortNumber;

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change video_render component's state to idle failed\n", __FUNCTION__);
		return ILC_ErrorStateChange;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE startup_render(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component's state to executing failed \n", __FUNCTION__);
		return ILC_ErrorStateChange;
	}
	fprintf(stderr, "%s():++++++++++++++++++++++\n", __FUNCTION__);
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = render->renderInputPort;

	ret = OMX_GetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's output port definition parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.image.nFrameWidth = render->renderConfig->screen_width;
	portDef.format.image.nFrameHeight = render->renderConfig->screen_height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

// 	portDef.format.video.bFlagErrorConcealment = OMX_FALSE;
// 	portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
// 	portDef.format.video.eColorFormat = OMX_COLOR_Format24bitRGB888;
// 	portDef.format.video.nFrameWidth = render->renderConfig->screen_width;;
// 	portDef.format.video.nFrameHeight = render->renderConfig->screen_height;
// 	portDef.format.video.nSliceHeight = 0;
// 	portDef.format.video.nStride = 0;

	ret = OMX_SetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set render component's input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(render->renderHandle, OMX_CommandPortEnable, render->renderInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable render component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_AllocateBuffer(render->renderHandle, &render->pInputBuffer, render->renderInputPort, NULL, nBufferSize);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():allocate for resize component's input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	//memset(render->pInputBuffer->pBuffer, 255, nBufferSize);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE init_resize(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;

	//create resize component and get it's handle
	ilRet = ilclient_create_component(render->ilClient, &render->resizeComponent, "resize",
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return ILC_ErrorComponentCreate;
	}
	render->resizeHandle = ilclient_get_handle(render->resizeComponent);
	if (render->resizeHandle == NULL){
		fprintf(stderr, "%s():ilclient get resize component's handle failed\n", __FUNCTION__);
		return ILC_ErrorHandleGet;
	}

	//get resize component's input and output port info
	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamImageInit, &portParam);
	if (ret != OMX_ErrorNone || portParam.nPorts != 2){
		fprintf(stderr, "%s():get resize component's image port info failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	render->resizeInputPort = portParam.nStartPortNumber;
	render->resizeOutputPort = portParam.nStartPortNumber + 1;

	//change resize component's state and set input image info
	ilRet = ilclient_change_component_state(render->resizeComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to idle failed\n", __FUNCTION__);
		return ILC_ErrorStateChange;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = render->resizeInputPort;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}
 	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
 	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
 	//portDef.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
 	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.nFrameWidth = (OMX_U32)dispConfig.screen_width;
	portDef.format.image.nFrameHeight = (OMX_U32)dispConfig.screen_height;
	fprintf(stderr, "%s():w=%d h=%d nbuffersize=%d\n", __FUNCTION__, dispConfig.screen_width, dispConfig.screen_height,
		nBufferSize);
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;
	portDef.nBufferSize = (OMX_U32)nBufferSize;

	ret = OMX_SetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set resize component's input port parameter definition failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	
	//enable the input port and alloc for buf
	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortEnable, render->resizeInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable resize component's input port commaned failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
// 	ilRet = ilclient_wait_for_event(render->resizeComponent, OMX_EventCmdComplete, OMX_CommandPortEnable, 0,
// 		render->resizeInputPort, 0, ILCLIENT_PORT_ENABLED, TIMEOUT_MS);
// 	if (ilRet != 0){
// 		fprintf(stderr, "%s():ilclient wait for resize component's input port enable event failed\n", __FUNCTION__);
// 		return ILC_ErrorEventWait;
// 	}

	ret = OMX_AllocateBuffer(render->resizeHandle, &render->pInputBuffer, render->resizeInputPort, NULL, nBufferSize);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():allocate for resize component's input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	//change resize component's state to executing
	ilRet = ilclient_change_component_state(render->resizeComponent, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change resize component's state to executing failed\n", __FUNCTION__);
		return ILC_ErrorStateChange;
	}

	return OMX_ErrorNone;

}

static OMX_ERRORTYPE set_render_display_config(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_DISPLAYSETTYPE set;
	OMX_CONFIG_DISPLAYREGIONTYPE dispRegionConfig;

	set = OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_NOASPECT |
		OMX_DISPLAY_SET_MODE | OMX_DISPLAY_SET_TRANSFORM | OMX_DISPLAY_SET_NUM;

	memset(&dispRegionConfig, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
	dispRegionConfig.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
	dispRegionConfig.nVersion.nVersion = OMX_VERSION;

	//set the render input port 
	dispRegionConfig.nPortIndex = render->renderInputPort;
	dispRegionConfig.noaspect = OMX_TRUE;

	if (render->renderConfig->screen_width > 0 && render->renderConfig->screen_height > 0){
		set |= OMX_DISPLAY_SET_DEST_RECT;
		dispRegionConfig.dest_rect.width = render->renderConfig->screen_width;
		dispRegionConfig.dest_rect.height = render->renderConfig->screen_height;
		dispRegionConfig.dest_rect.x_offset = render->renderConfig->xOffset;
		dispRegionConfig.dest_rect.y_offset = render->renderConfig->yOffset;
		dispRegionConfig.fullscreen = OMX_FALSE;
	}
	else
		dispRegionConfig.fullscreen = OMX_TRUE;

	dispRegionConfig.transform = OMX_DISPLAY_ROT0;
	dispRegionConfig.num = 0;
	dispRegionConfig.mode = 0;
	dispRegionConfig.set = set;

	ret = OMX_SetConfig(render->renderHandle, OMX_IndexConfigDisplayRegion, &dispRegionConfig);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set render component's dispaly region configuration failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE resize_port_settings_changed(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;

	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;

	portDef.nPortIndex = render->resizeOutputPort;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component's output port definition parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
	portDef.format.image.nFrameWidth = render->renderConfig->screen_width;
	portDef.format.image.nFrameWidth = render->renderConfig->screen_height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;
	ret = OMX_SetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set resizer component's output port definition parameter failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component's state to executing failed \n", __FUNCTION__);
		return ILC_ErrorStateChange;
	}

	//set render input port info
	portDef.nPortIndex = render->renderInputPort;
	ret = OMX_GetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get render component's input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
	portDef.format.image.nFrameWidth = render->renderConfig->screen_width;
	portDef.format.image.nFrameHeight = render->renderConfig->screen_height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

	ret = OMX_SetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set render component's input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	//setup tunnel resize component's output port with render component's input port
	ret = OMX_SetupTunnel(render->resizeHandle, render->resizeOutputPort,
		render->renderHandle, render->renderInputPort);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():setup resize and render component tunnel failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortEnable, render->resizeOutputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable resize component's output port command failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}
	//ilclient_wait_for_command_complete(render->resizeComponent, OMX_CommandPortEnable, render->resizeOutPort);

	ret = OMX_SendCommand(render->renderHandle, OMX_CommandPortEnable, render->renderInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send enable render component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}
	//ilclient_wait_for_command_complete(render->renderComponent, OMX_CommandPortEnable, render->renderInputPort);

	fprintf(stderr, "%s()___________________\n", __FUNCTION__);
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE close_render(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	ret = OMX_SendCommand(render->renderHandle, OMX_CommandFlush, render->renderInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send flush render component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}
	ilclient_wait_for_event(render->renderComponent, OMX_EventCmdComplete, OMX_CommandFlush, 0,
		render->renderInputPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);

	ret = OMX_SendCommand(render->renderHandle, OMX_CommandPortDisable, render->renderInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send disable render component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilclient_wait_for_event(render->renderComponent, OMX_EventCmdComplete, OMX_CommandPortDisable, 0,
		render->renderInputPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to idle failed\n", __FUNCTION__);
	}
	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateLoaded);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to loaded failed\n", __FUNCTION__);
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE close_resize(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;

	ret = OMX_FreeBuffer(render->resizeComponent, render->resizeInputPort, render->pInputBuffer);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s() :free resize component's input port buffer failed err : %s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortDisable, render->resizeInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send disable resizer component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilclient_wait_for_event(render->resizeComponent, OMX_EventCmdComplete, OMX_CommandPortDisable, 0,
		render->resizeInputPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandFlush, render->resizeOutputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send flush resize component command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilclient_wait_for_event(render->resizeComponent, OMX_EventCmdComplete, OMX_CommandFlush, 0,
		render->resizeOutputPort, 0, ILCLIENT_PORT_FLUSH, TIMEOUT_MS);

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortDisable, render->resizeOutputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send disable resize component port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ilRet = ilclient_change_component_state(render->resizeComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change resize component to state idle failed\n", __FUNCTION__);
	}

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateLoaded);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilcilent change resize component to state loaded failed \n", __FUNCTION__);
	}
}

static void get_render_display_config(RENDER_DISPLAY_CONFIG *dispConf)
{
	int width, height;
	if (sign_width <= 0 || sign_width > 10000 || sign_height <= 0 || sign_height > 10000){
		graphics_get_display_size(0, &width, &height);
	}
	else{
		width = sign_width;
		height = sign_height;
	}
	dispConf->screen_width = width;
	dispConf->screen_height = height;
	dispConf->xOffset = 0;
	dispConf->yOffset = 0;

	nBufferSize =width * height * 4;
}

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       omx_init_graphics
* Description:initialize render component and set display configuration  
* Input:      void
* Output:     
* Returns:    int 0 on success, -1 on failure
*********************************************************************/
int omx_init_graphics(void)
{
	OMX_ERRORTYPE ret;

	bcm_host_init();
	get_render_display_config(&dispConfig);
	il_client = ilclient_init();
	if (il_client == NULL){
		fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
		return -1;
	}
	if ((ret = OMX_Init()) != OMX_ErrorNone){
		fprintf(stderr, "%s():init openMAX FAILED err:%s\n", __FUNCTION__, OMX_ErrorToString(ret));
		return -1;
	}
	if ((ret = init_render(&render)) != OMX_ErrorNone){
		fprintf(stderr, "%s():init render failed\n", __FUNCTION__);
		ilclient_destroy(il_client); il_client = NULL;
		return -1;
	}

// 	if ((ret = init_resize(&render)) != OMX_ErrorNone){
// 		fprintf(stderr, "%s():init resize failed \n", __FUNCTION__);
// 		ilclient_destroy(il_client); il_client = NULL;
// 		return -1;
// 	}

	if ((ret = set_render_display_config(&render)) != OMX_ErrorNone){
		fprintf(stderr, "%s():set render dispaly config failed \n", __FUNCTION__);
		ilclient_destroy(il_client); il_client = NULL;
		return -1;
	}
	if ((ret = startup_render(&render)) != OMX_ErrorNone){
		fprintf(stderr, "%s():startup render failed \n", __FUNCTION__);
		ilclient_destroy(il_client); il_client = NULL;
		return -1;
	}
	fprintf(stderr, "%s():omx_init down\n");
	return 0;
}

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       omx_close_graphics
* Description:  
* Input:      void
* Output:     
* Returns:    int
*********************************************************************/
int omx_close_graphics(void)
{
	close_resize(&render);
	close_render(&render);
	COMPONENT_T *list[3];
	list[0] = render.resizeComponent;
	list[1] = render.renderComponent;
	list[2] = NULL;
	ilclient_cleanup_components(list);
	render.renderComponent = NULL;
	render.resizeComponent = NULL;

	OMX_Deinit();
	if (il_client){
		ilclient_destroy(il_client);
		il_client = NULL;
	}
}

/*******************************************************************
* Copyright (C) 2018 by the second group of Sansi Software Institute
* Name:       omx_render_buf_r8g8b8
* Description:  
* Input:      unsigned char * buf
* Output:     
* Returns:    void
*********************************************************************/
void omx_render_buf_r8g8b8(unsigned char *buf)
{
	render.pInputBuffer->pBuffer = (OMX_U8*)buf;
	render.pInputBuffer->nFilledLen = nBufferSize;
	render.pInputBuffer->nOffset = 0;
	render.pInputBuffer->nFlags = OMX_BUFFERFLAG_EOS;

	OMX_BUFFERHEADERTYPE *pBufferHeader;
	pBufferHeader = render.pInputBuffer;
	//OMX_ERRORTYPE err = OMX_EmptyThisBuffer(render.resizeHandle, pBufferHeader);
	OMX_ERRORTYPE err = OMX_EmptyThisBuffer(render.renderHandle, pBufferHeader);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():empty this buffer failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
		return;
	}
	fprintf(stderr, "%s():+++++++++++start\n", __FUNCTION__);
// 	if (render.pSettingsChanged == 0 && ilclient_wait_for_event(render.resizeComponent, OMX_EventPortSettingsChanged,
// 		render.resizeOutputPort, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS) == 0){
// 		if ((err = resize_port_settings_changed(&render)) != OMX_ErrorNone){
// 			fprintf(stderr, "%s():failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
// 			return ;
// 		}
// 		render.pSettingsChanged = 1;
// 	}

	int ilRet = ilclient_wait_for_event(render.renderComponent, OMX_EventBufferFlag, render.renderInputPort,
		0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS);
	if (ilRet != 0){
		fprintf(stderr, "%s():render component not recieve buffer EOS event ret=%d\n", __FUNCTION__, ilRet);
		return ;
	}

	return ;
}