#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

#include "ilclient.h"
#include "bcm_host.h"
#include "OMX_Common.h"

static void printState(OMX_HANDLETYPE handle)
{
	OMX_STATETYPE state;
	OMX_ERRORTYPE err;

	err = OMX_GetState(handle, &state);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetState() failed:%s\n", __FUNCTION__, OMX_ErrorToString(err));
		exit(1);
	}
	switch (state){
	case OMX_StateInvalid:printf("Component's state is StatedInValid\n"); break;
	case OMX_StateLoaded:printf("Component's state is StateLoaded\n"); break;
	case OMX_StateIdle:printf("Component's state is StateIdle\n"); break;
	case OMX_StateExecuting:printf("Component's state is StateExcuting\n"); break;
	case OMX_StatePause:printf("Component's state is StatePause\n"); break;
	case OMX_StateWaitForResources:printf("Component's state is StateWaitForResources\n"); break;
	default:printf("Component's state is NotKnown\n"); break;
	}
}

static void eosCallBack(void *userData, COMPONENT_T *component, OMX_U32 Data)
{
	fprintf(stderr, "%s():Got Eos Event:%s\n", __FUNCTION__, OMX_ErrorToString(Data));
}
static void errCallBack(void *userData, COMPONENT_T *component, OMX_U32 Data)
{
	fprintf(stderr, "%s():Got Error Event:%s\n", __FUNCTION__, OMX_ErrorToString(Data));
}
static int getFileSize(char *fileName)
{
	struct stat st;
	if (stat(fileName, &st) == -1){
		fprintf(stderr, "%s():please check %s is exist? \n", __FUNCTION__, fileName);
		return -1;
	}
	return (st.st_size);
}
static void setImageDecoderInputFormat(COMPONENT_T *compoent)
{
	OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
	OMX_ERRORTYPE err;
	memset(&imagePortFormat, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
	imagePortFormat.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
	imagePortFormat.nVersion.nVersion = OMX_VERSION;

	imagePortFormat.nPortIndex = 320;//image_deocde input port
	imagePortFormat.eCompressionFormat = OMX_IMAGE_CodingJPEG;
	//imagePortFormat.eCompressionFormat = OMX_IMAGE_CodingBMP;
	//imagePortFormat.eColorFormat = OMX_COLOR_Format24bitBGR888;

	err=OMX_SetParameter(ilclient_get_handle(compoent), OMX_IndexParamImagePortFormat, 
		&imagePortFormat);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_SetPArameter faile:%s\n", __FUNCTION__, OMX_ErrorToString(err));
		exit(1);
	}
}

static void getImageDecodeInputFormat(COMPONENT_T *component)
{
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_IMAGE_PORTDEFINITIONTYPE imageDef;
	OMX_ERRORTYPE err;
	memset(&portDef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = 320;
	
	err = OMX_GetParameter(ilclient_get_handle(component), OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetPArameter portDef failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
	}
	
	imageDef = portDef.format.image;
	fprintf(stderr, "%s():nPort=%d cMIMEType=%s FrameWidth=%d FrameHeight=%d nStride=%d nSliceHeight=%d Format=%d\n", __FUNCTION__,
		portDef.nPortIndex, imageDef.cMIMEType, imageDef.nFrameWidth, imageDef.nFrameHeight, imageDef.nStride, 
		imageDef.nSliceHeight, imageDef.eCompressionFormat);
	
	OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
	memset(&imagePortFormat, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
	imagePortFormat.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
	imagePortFormat.nVersion.nVersion = OMX_VERSION;
	imagePortFormat.nPortIndex = 320;
	err = OMX_GetParameter(ilclient_get_handle(component), OMX_IndexParamImagePortFormat, &imagePortFormat);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetParameter failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
		//exit(1);
	}
	fprintf(stderr, "%s():image decode port is:%d supported encode type is :%d\n", __FUNCTION__,
		imagePortFormat.nPortIndex, imagePortFormat.eCompressionFormat);

	OMX_PORT_PARAM_TYPE portType;
	memset(&portType, 0, sizeof(OMX_PORT_PARAM_TYPE));
	portType.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	portType.nVersion.nVersion = OMX_VERSION;

	err = OMX_GetParameter(ilclient_get_handle(component), OMX_IndexParamImageInit, &portType);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetParameter port typr failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
	}
	fprintf(stderr, "%s():image nPorts=%d start Port=%d \n", __FUNCTION__, portType.nPorts, portType.nStartPortNumber);
}

static void getOutputPortSettings(COMPONENT_T *component)
{
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_IMAGE_PORTDEFINITIONTYPE imageDef;
	OMX_ERRORTYPE err;
	memset(&portDef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	portDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portDef.nVersion.nVersion = OMX_VERSION;
	portDef.nPortIndex = 321;

	err = OMX_GetParameter(ilclient_get_handle(component), OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetPArameter portDef failed err:%s\n", __FUNCTION__, OMX_ErrorToString(err));
	}

	imageDef = portDef.format.image;
	fprintf(stderr, "%s():nPort=%d cMIMEType=%s FrameWidth=%d FrameHeight=%d nStride=%d nSliceHeight=%d Format=%d colorFormat=%#X\n",
		__FUNCTION__,portDef.nPortIndex, imageDef.cMIMEType, imageDef.nFrameWidth, imageDef.nFrameHeight, 
		imageDef.nStride, imageDef.nSliceHeight, imageDef.eCompressionFormat,imageDef.eColorFormat);

}
static OMX_ERRORTYPE readIntoBufferAndEmpty(FILE *fp, COMPONENT_T *component,
	OMX_BUFFERHEADERTYPE *buffer_header, int *extra_size)
{
	OMX_ERRORTYPE err;
	int buf_size = buffer_header->nAllocLen;
	int nread = fread(buffer_header->pBuffer, 1, buf_size, fp);

	buffer_header->nFilledLen = nread;
	*extra_size -= nread;
	fprintf(stderr, "%s():read %d bytes extra %d bytes\n", __FUNCTION__, nread,*extra_size);

	if (*extra_size <= 0){
		fprintf(stderr, "%s():had read all,set EOS on input port\n", __FUNCTION__);
		buffer_header->nFlags |= OMX_BUFFERFLAG_EOS;
	}

	err = OMX_EmptyThisBuffer(ilclient_get_handle(component), buffer_header);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():Empty this buffer failed:%s\n", __FUNCTION__, OMX_ErrorToString(err));
	}
	return err;
}

static OMX_ERRORTYPE saveInfoFromFilledBuffer(COMPONENT_T *component,OMX_BUFFERHEADERTYPE *buffer_header)
{
	OMX_ERRORTYPE err;
	fprintf(stderr, "%s():Got a filled buffer with %d bytes,allocated %d bytes buf->flag=%d\n", __FUNCTION__,
		buffer_header->nFilledLen, buffer_header->nAllocLen,buffer_header->nFlags);
	if (buffer_header->nFlags & OMX_BUFFERFLAG_EOS){
		fprintf(stderr, "%s():got EOS on output\n", __FUNCTION__);
		exit(0);
	}
	//do nothing 

	//refill it
	err = OMX_FillThisBuffer(ilclient_get_handle(component), buffer_header);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():Fill this buffer failed:%s\n", __FUNCTION__, OMX_ErrorToString(err));
	}
	return err;
}

static void setupDecodeComponent(ILCLIENT_T *handle,char *componentName,COMPONENT_T**decodeComponent)
{
	int err;
	err = ilclient_create_component(handle, decodeComponent, componentName, ILCLIENT_DISABLE_ALL_PORTS |
		ILCLIENT_ENABLE_INPUT_BUFFERS);
	if (err == -1) {
		fprintf(stderr, "DecodeComponent create failed\n");
		exit(1);
	}
	//printState(ilclient_get_handle(*decodeComponent));
	err = ilclient_change_component_state(*decodeComponent,
		OMX_StateIdle);
	if (err < 0) {
		fprintf(stderr, "Couldn't change state to Idle\n");
		exit(1);
	}
	//printState(ilclient_get_handle(*decodeComponent));
	// must be before we enable buffers
	setImageDecoderInputFormat(*decodeComponent);

}

static void setupRenderComponent(ILCLIENT_T *handle, char *componentName, COMPONENT_T**renderComponent)
{
	int err;
	err = ilclient_create_component(handle,renderComponent,componentName,ILCLIENT_DISABLE_ALL_PORTS
		/* |ILCLIENT_ENABLE_INPUT_BUFFERS*/);
	if (err == -1) {
		fprintf(stderr, "RenderComponent create failed\n");
		exit(1);
	}
	//printState(ilclient_get_handle(*renderComponent));
	err = ilclient_change_component_state(*renderComponent,OMX_StateIdle);
	if (err < 0) {
		fprintf(stderr, "Couldn't change state to Idle\n");
		exit(1);
	}
	//printState(ilclient_get_handle(*renderComponent));
}

int main(int argc, char *argv[])
{
	int i; int err;
	char *componentName; char *renderComponentName;
	char *imgName;
	ILCLIENT_T *handle;
	COMPONENT_T *component;
	COMPONENT_T *renderComponent;
	FILE *fp; int file_size;
	OMX_BUFFERHEADERTYPE *buffer_header;

	if (argc < 2){
		fprintf(stderr, "Usage:%s jpeg-image \n", argv[0]);
		exit(1);
	}
	imgName = argv[1];
	if ((fp = fopen(imgName, "r")) == NULL){
		fprintf(stderr, "%s():open image :%s failed\n", __FUNCTION__, imgName);
		exit(1);
	}
	if ((file_size = getFileSize(imgName)) < 0){
		exit(1);
	}
	componentName = "image_decode";
	renderComponentName = "video_render";
	bcm_host_init();
	handle = ilclient_init();
	if (handle == NULL){
		fprintf(stderr, "%s():ilclient init failed\n", __FUNCTION__);
		exit(1);
	}

	if (OMX_Init() != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX init failed\n", __FUNCTION__);
		exit(2);
	}

	ilclient_set_eos_callback(handle, eosCallBack, NULL);
	ilclient_set_error_callback(handle, errCallBack, NULL);

	//create component in loaded state with all ports disable
	err = ilclient_create_component(handle, &component, componentName, ILCLIENT_DISABLE_ALL_PORTS|
		ILCLIENT_ENABLE_INPUT_BUFFERS |ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (err == -1){
		fprintf(stderr, "%s():create component failed \n", __FUNCTION__);
		exit(1);
	}
	printState(ilclient_get_handle(component));

	

	//change component state to idle
	err = ilclient_change_component_state(component, OMX_StateIdle);
	if (err == -1){
		fprintf(stderr, "%s():change component state to StateIdle failed\n", __FUNCTION__);
		exit(1);
	}
	printState(ilclient_get_handle(component));

	//getImageDecodeInputFormat(component);

	//must do it before enable buffer
	setImageDecoderInputFormat(component);

	//getImageDecodeInputFormat(component);
	
	//enable input ports and create input port buffers
	ilclient_enable_port_buffers(component, 320, NULL, NULL, NULL);
	ilclient_enable_port(component,320);

	//change component state to executing
	err = ilclient_change_component_state(component, OMX_StateExecuting);
	if (err == -1){
		fprintf(stderr, "%s():cahnge component state to StateExecuting failed \n", __FUNCTION__);
		exit(1);
	}
	printState(ilclient_get_handle(component));

	//put data into an input port
	buffer_header = ilclient_get_input_buffer(component, 320, 1/*blovk*/);
	if (buffer_header != NULL){
		readIntoBufferAndEmpty(fp, component, buffer_header, &file_size);
		if (file_size <= 0){
			fprintf(stderr, "%s():Rewinding\n", __FUNCTION__);
			//wind back to start and repeat
			fp = freopen(imgName,"r",fp);
			file_size = getFileSize(imgName);
		}
	}
	
	// wait for first input block to set params for output port
	//wait for port settings changed on the output
	i=ilclient_wait_for_event(component, OMX_EventPortSettingsChanged, 321, 0, 0, 1,
		ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000);
	printf("%s():port settings changed  ret=%d\n",__FUNCTION__,i);
	getOutputPortSettings(component);
	//now enable output port since port params have been set
	ilclient_enable_port_buffers(component, 321, NULL, NULL, NULL);
	ilclient_enable_port(component, 321);

	//working through the file
	while (file_size>0){
		OMX_ERRORTYPE r;
		buffer_header = ilclient_get_input_buffer(component, 320, 1);
		if (buffer_header != NULL){
			fprintf(stderr, "%s():111111111 file_size=%d input_buf->nOffset=%d input_buf->nFilledLen=%d input_buf->inputPort=%d input_buf->outPort=%d\n", __FUNCTION__, 
				file_size,buffer_header->nOffset,buffer_header->nFilledLen,buffer_header->nInputPortIndex,buffer_header->nOutputPortIndex);
			readIntoBufferAndEmpty(fp, component, buffer_header, &file_size);
		}

		//i=ilclient_wait_for_event(component, OMX_EventPortSettingsChanged, 321, 0, 0, 1,
		//	ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000);
		//printf("%s():port settings changed ret=%d\n",__FUNCTION__,i);

		buffer_header = ilclient_get_output_buffer(component, 321, 0);
		if (buffer_header != NULL){
			fprintf(stderr, "%s():22222222 file_size=%d input_buf->nOffset=%d input_buf->nFilledLen=%d input_buf->inputPort=%d input_buf->outPort=%d\n", __FUNCTION__,
				file_size, buffer_header->nOffset, buffer_header->nFilledLen, buffer_header->nInputPortIndex, buffer_header->nOutputPortIndex);
			saveInfoFromFilledBuffer(component, buffer_header);
		}
	}
	
	while (1){
		fprintf(stderr, "%s():get last output buffers\n", __FUNCTION__);
		buffer_header = ilclient_get_output_buffer(component, 321, 1);
		if (buffer_header != NULL){
			saveInfoFromFilledBuffer(component, buffer_header);
		}
	}

	fprintf(stderr, "%s():HHHHHHHHHHHHHHHHHH\n", __FUNCTION__);
	return 0;

//render and decode
	/*
	setupDecodeComponent(handle, componentName, &component);
	setupRenderComponent(handle, renderComponentName, &renderComponent);

	//input port
	ilclient_enable_port_buffers(component, 320, NULL, NULL, NULL);
	ilclient_enable_port(component, 320);

	buffer_header = ilclient_get_input_buffer(component, 320, 1);
	if (buffer_header != NULL){
		readIntoBufferAndEmpty(fp, component, buffer_header, &file_size);
		if (file_size <= 0){
			fprintf(stderr, "%s():Rewinding\n", __FUNCTION__);
			//wind back to start and repeat
			fp = freopen(imgName, "r", fp);
			file_size = getFileSize(imgName);
		}
	}
	// wait for first input block to set params for output port
	ilclient_wait_for_event(component, OMX_EventPortSettingsChanged, 321, 0, 0, 1,
		ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 5);
	printf("port settings changed\n");

	TUNNEL_T tunnel;
	set_tunnel(&tunnel, component, 321, renderComponent, 90);
	if ((err = ilclient_setup_tunnel(&tunnel, 0, 0)) < 0){
		fprintf(stderr, "%s():setup tunnle failed \n", __FUNCTION__);
		exit(1);
	}

	//enable the decode output ports
	OMX_SendCommand(ilclient_get_handle(component), OMX_CommandPortEnable, 321, NULL);
	ilclient_enable_port(component, 321);

	//enable render out ports
	ilclient_enable_port(renderComponent, 90);
	// set both components to executing state
	err = ilclient_change_component_state(component, OMX_StateExecuting);
	if (err < 0) {
		fprintf(stderr, "Couldn't change state to Idle\n");
		exit(1);
	}
	err = ilclient_change_component_state(renderComponent,
		OMX_StateExecuting);
	if (err < 0) {
		fprintf(stderr, "Couldn't change state to Idle\n");
		exit(1);
	}
	//working through the file
	while (file_size > 0){
		OMX_ERRORTYPE r;
		fprintf(stderr, "%s():111111111file_size=%d\n", __FUNCTION__, file_size);
		buffer_header = ilclient_get_input_buffer(component, 320, 1);
		if (buffer_header != NULL){
			readIntoBufferAndEmpty(fp, component, buffer_header, &file_size);
		}
	}
	printf("EOS on render\n");
	ilclient_wait_for_event(renderComponent, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
		ILCLIENT_BUFFER_FLAG_EOS, 100);
	printf("EOS on render\n");
	sleep(100);
	*/
}