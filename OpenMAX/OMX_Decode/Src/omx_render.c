/*
* omx_render.c
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "omx_render.h"
#include "bcm_host.h"
#include "omx_typedef.h"

static OMX_ERRORTYPE init_render(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;

	ilRet = ilclient_create_component(render->il_client, &render->renderComponent,
		"video_render", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create video_render component failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	render->renderHandle = ilclient_get_handle(render->renderComponent);
	if (render->renderHandle == NULL){
		fprintf(stderr, "%s():ilclient get render component handle failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;
	ret = OMX_GetParameter(render->renderHandle, OMX_IndexParamVideoInit, &portParam);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get render video port parameter failed err:%d\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	render->renderInputPort = portParam.nStartPortNumber;

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to idle failed\n",
			__FUNCTION__);
		return OMX_ErrorMax;
	}

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

	ilclient_wait_for_event(render->renderComponent,OMX_EventCmdComplete,OMX_CommandPortDisable,0,
		render->renderInputPort, 0, ILCLIENT_PORT_DISABLED, TIMEOUT_MS);

	ilRet=ilclient_change_component_state(render->renderComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to idle failed\n", __FUNCTION__);
	}
	ilRet=ilclient_change_component_state(render->renderComponent, OMX_StateLoaded);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to loaded failed\n", __FUNCTION__);
	}
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE init_resizer(RENDER *render,IMAGE *inImage)
{
	OMX_ERRORTYPE ret; int ilRet;

	ilRet = ilclient_create_component(render->il_client, &render->resizeComponent,
		"resize", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS
		| ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient create resize component failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	render->resizeHandle = ilclient_get_handle(render->resizeComponent);
	if (render->renderHandle == NULL){
		fprintf(stderr, "%s():ilclient get resize component handle failed\n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	OMX_PORT_PARAM_TYPE portParam;
	portParam.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portParam.nVersion.nVersion = OMX_VERSION;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamImageInit, &portParam);
	if (ret != OMX_ErrorNone || portParam.nPorts != 2){
		fprintf(stderr, "%s():get resize image port parameter failed err:%d\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	render->resizeInputPort = portParam.nStartPortNumber;
	render->resizeOutPort = portParam.nStartPortNumber + 1;

	ilRet = ilclient_change_component_state(render->resizeComponent, OMX_StateIdle);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change resize component state to idle failed\n",
			__FUNCTION__);
		return OMX_ErrorMax;
	}
	//set input port info
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = render->resizeInputPort;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get resize component input port %d definition parameter failed err:%s\n",
			__FUNCTION__, render->resizeInputPort, OMX_ErrorToString(ret));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = inImage->colorFormat;
	portDef.format.image.nFrameWidth = inImage->width;
	portDef.format.image.nFrameHeight = inImage->height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

	portDef.nBufferSize = inImage->nData;

	ret = OMX_SetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set resize component input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortEnable, render->resizeInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send eanble resize component input port command failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	//ilclient_wait_for_command_complete(render->resizeComponent, OMX_CommandPortEnable,
	//	render->resizeInputPort);
	
	ret = OMX_UseBuffer(render->resizeHandle, &render->pInputBufferHeader, render->resizeInputPort,
		NULL, portDef.nBufferSize, (OMX_U8*)inImage->pData);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():use resize component input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	render->pSettingsChanged = 0;
	ilRet = ilclient_change_component_state(render->resizeComponent, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():change resize component state to excuting failed \n", __FUNCTION__);
		return OMX_ErrorMax;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE close_resizer(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	ret = OMX_FreeBuffer(render->resizeComponent, render->resizeInputPort, render->pInputBufferHeader);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():free resize component input port buffer failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortDisable, render->resizeInputPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send disable resizer component input port command failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
	}
	
	ilclient_wait_for_event(render->resizeComponent, OMX_EventCmdComplete,OMX_CommandPortDisable, 0, 
		render->resizeInputPort, 0,ILCLIENT_PORT_DISABLED, TIMEOUT_MS);
	
	ret=OMX_SendCommand(render->resizeHandle, OMX_CommandFlush, render->resizeOutPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send flush resize component command failed err:%s\n", __FUNCTION__, 
			OMX_ErrorToString(ret));
	}

	ilclient_wait_for_event(render->resizeComponent, OMX_EventCmdComplete, OMX_CommandFlush, 0,
		render->resizeOutPort, 0, ILCLIENT_PORT_FLUSH, TIMEOUT_MS);

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortDisable, render->resizeOutPort, NULL);
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

static OMX_ERRORTYPE resizer_port_settingd_changed(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;

	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = render->resizeOutPort;
	ret = OMX_GetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get resizer component output port parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
	//portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.image.nFrameWidth = render->dispConfig->sign_width;
	portDef.format.image.nFrameHeight = render->dispConfig->sign_height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

	fprintf(stderr, "%s():nbufsize=%d\n", __FUNCTION__, portDef.nBufferSize);
	ret = OMX_SetParameter(render->resizeHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set resize component output port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	ilRet = ilclient_change_component_state(render->renderComponent, OMX_StateExecuting);
	if (ilRet == -1){
		fprintf(stderr, "%s():ilclient change render component state to excuting failed \n", __FUNCTION__);
		return OMX_ErrorMax;
	}

	//set render input port info
	portDef.nPortIndex = render->renderInputPort;
	ret = OMX_GetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():get render component input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	portDef.format.image.bFlagErrorConcealment = OMX_FALSE;
	portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portDef.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
	//portDef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	portDef.format.image.nFrameWidth = render->dispConfig->sign_width;
	portDef.format.image.nFrameHeight = render->dispConfig->sign_height;
	portDef.format.image.nSliceHeight = 0;
	portDef.format.image.nStride = 0;

	ret = OMX_SetParameter(render->renderHandle, OMX_IndexParamPortDefinition, &portDef);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set render component input port definition parameter failed err:%s\n",
			__FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SetupTunnel(render->resizeHandle, render->resizeOutPort,
		render->renderHandle, render->renderInputPort);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():setup resize and render component tunnel failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	ret = OMX_SendCommand(render->resizeHandle, OMX_CommandPortEnable, render->resizeOutPort, NULL);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():send resize component output port enable command failed err:%s\n",
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

static OMX_ERRORTYPE set_render_dispaly_config(RENDER *render)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_DISPLAYSETTYPE set;
	OMX_CONFIG_DISPLAYREGIONTYPE dispRegionConf;

	set = OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_NOASPECT |
		OMX_DISPLAY_SET_MODE | OMX_DISPLAY_SET_TRANSFORM | OMX_DISPLAY_SET_NUM;

	memset(&dispRegionConf, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
	dispRegionConf.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
	dispRegionConf.nVersion.nVersion = OMX_VERSION;

	dispRegionConf.nPortIndex = render->renderInputPort;
	dispRegionConf.noaspect = OMX_TRUE;

	if (render->dispConfig->sign_height > 0 && render->dispConfig->sign_width > 0){
		set |= OMX_DISPLAY_SET_DEST_RECT;
		dispRegionConf.dest_rect.x_offset = render->dispConfig->xOffset;
		dispRegionConf.dest_rect.y_offset = render->dispConfig->yOffset;
		dispRegionConf.dest_rect.width = render->dispConfig->sign_width;
		dispRegionConf.dest_rect.height = render->dispConfig->sign_height;
		dispRegionConf.fullscreen = OMX_FALSE;
	}
	else{
		dispRegionConf.fullscreen = OMX_TRUE;
	}

	dispRegionConf.transform = OMX_DISPLAY_MIRROR_ROT180;
	dispRegionConf.num = 0;
	dispRegionConf.mode = 0;
	dispRegionConf.set = set;

	ret = OMX_SetConfig(render->renderHandle, OMX_IndexConfigDisplayRegion, &dispRegionConf);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():set render component display region config failed err:%s\n", __FUNCTION__,
			OMX_ErrorToString(ret));
		return ret;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE do_render(RENDER *render, IMAGE *inImage)
{
	OMX_ERRORTYPE ret; int ilRet;
	OMX_BUFFERHEADERTYPE *pBufferHeader = render->pInputBufferHeader;
	pBufferHeader->nFilledLen = inImage->nData;
	pBufferHeader->nFlags = OMX_BUFFERFLAG_EOS;

	ret = OMX_EmptyThisBuffer(render->resizeHandle, pBufferHeader);
	if (ret != OMX_ErrorNone){
		fprintf(stderr, "%s():empty this buffer failed err:%s\n", __FUNCTION__, OMX_ErrorToString(ret));
		return ret;
	}

	if (render->pSettingsChanged == 0 && ilclient_wait_for_event(render->resizeComponent, OMX_EventPortSettingsChanged,
		render->resizeOutPort, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, TIMEOUT_MS) == 0){
		if ((ret = resizer_port_settingd_changed(render)) != OMX_ErrorNone){
			fprintf(stderr, "%s():failed err:%s\n", __FUNCTION__, OMX_ErrorToString(ret));
			return ret;
		}
		render->pSettingsChanged = 1;
	}

	ilRet=ilclient_wait_for_event(render->renderComponent, OMX_EventBufferFlag, render->renderInputPort,
		0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, TIMEOUT_MS);
	if (ilRet != 0){
		fprintf(stderr, "%s():render component not recieve buffer EOS event ret=%d\n", __FUNCTION__, ilRet);
		return OMX_ErrorMax;
	}

	return OMX_ErrorNone;
}

static void calculate_resize(RENDER *render, uint32_t *pWidth, uint32_t *pHeight)
{

}

int omx_render_image(RENDER *render, IMAGE *inImage)
{
	OMX_ERRORTYPE ret;

	ret = init_render(render);
	if (ret != OMX_ErrorNone){
		return -1;
	}

	ret = init_resizer(render,inImage);
	if (ret != OMX_ErrorNone){
		return -1;
	}
	fprintf(stderr, "%s():++++++++++++++++++\n", __FUNCTION__);
	ret = set_render_dispaly_config(render);
	if (ret != OMX_ErrorNone){
		return -1;
	}
	fprintf(stderr, "%s():111111111111111111\n", __FUNCTION__);
	ret = do_render(render, inImage);
	if (ret != OMX_ErrorNone){
		return -1;
	}
	return 0;

}