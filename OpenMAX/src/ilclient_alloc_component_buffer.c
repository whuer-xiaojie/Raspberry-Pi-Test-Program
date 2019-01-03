#include <stdio.h>
#include <stdlib.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

#include <bcm_host.h>
#include <ilclient.h>

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
static void errCallBack(void *userData, COMPONENT_T *component, OMX_U32 Data)
{
	fprintf(stderr, "%s():err=%s\n", __FUNCTION__, OMX_ErrorToString(Data));
}
int main(int argc, char **argv)
{
	int i; int err;
	OMX_ERRORTYPE error;
	char *componentName;
	ILCLIENT_T *handle;
	COMPONENT_T *component;

	if (argc < 2){
		fprintf(stderr, "Usage:%s component-name\n", argv[0]);
		fprintf(stderr, "  the default component is :Image_encode\n");
		componentName = "Image_encode";
	}
	else{
		componentName = argv[1];
	}
	bcm_host_init();
	handle = ilclient_init();
	if (handle == NULL){
		fprintf(stderr, "%s():ilclient_init() failed\n", __FUNCTION__);
		exit(1);
	}
	if ((error=OMX_Init()) != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_Init() failed err:%s\n", __FUNCTION__,OMX_ErrorToString(error));
		exit(1);
	}

	ilclient_set_error_callback(handle, errCallBack, NULL);

	err = ilclient_create_component(handle, &component, componentName, ILCLIENT_DISABLE_ALL_PORTS |
		ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (err == -1){
		fprintf(stderr, "%s():ilclient_create_component() failed \n", __FUNCTION__);
		exit(1);
	}

	printState(ilclient_get_handle(component));

	err = ilclient_change_component_state(component, OMX_StateIdle);
	if (err == -1){
		fprintf(stderr, "%s():change component state to StateIdle faile\n", __FUNCTION__);
		exit(1);
	}

	printState(ilclient_get_handle(component));

	//input buffer
	ilclient_enable_port_buffers(component, 340, NULL, NULL, NULL);
	ilclient_enable_port(component, 340);
	// the input port is enabled and has input buffers allocated

	//output buffer
	ilclient_enable_port_buffers(component, 341, NULL, NULL, NULL);
	ilclient_enable_port(component, 341);

	err = ilclient_change_component_state(component, OMX_StateExecuting);
	if (err == -1){
		fprintf(stderr, "%s():change component state to StateExecuting failed\n", __FUNCTION__);
		exit(1);
	}

	printState(ilclient_get_handle(component));
	return 0;
}
