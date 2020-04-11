#include <stdio.h>
#include <stdlib.h>

#include "IL/OMX_Core.h"
#include "IL/OMX_Component.h"

#include "bcm_host.h"
#include "ilclient.h"

static void printState(OMX_HANDLETYPE handle)
{
	OMX_STATETYPE state;
	OMX_ERRORTYPE err;

	err = OMX_GetState(handle, &state);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_GetState()failed err=%d\n", __FUNCTION__, err);
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

int main(int argc, char **argv)
{
	int i; int err;
	char *componentName;
	COMPONENT_T *compoent;
	ILCLIENT_T *handle;

	if (argc < 2){
		fprintf(stderr, "Usage:%s component-name \n", argv[0]);
		exit(1);
	}
	componentName = argv[1];
	bcm_host_init();
	handle = ilclient_init();
	if (handle == NULL){
		fprintf(stderr, "%s():ilclient_init() failed\n", __FUNCTION__);
		exit(1);
	}
	if (OMX_Init() != OMX_ErrorNone){
		fprintf(stderr, "%s():OMX_Init() failed\n", __FUNCTION__);
		exit(1);
	}
	err = ilclient_create_component(handle, &compoent, componentName, ILCLIENT_DISABLE_ALL_PORTS);
	if (err == -1){
		fprintf(stderr, "%s():ilclient_create_component() failed \n", __FUNCTION__);
		exit(1);
	}
	printState(ilclient_get_handle(compoent));
	err = ilclient_change_component_state(compoent, OMX_StateIdle);
	if (err == -1){
		fprintf(stderr, "%s():change component's state to StateEdle failed \n ", __FUNCTION__);
		exit(1);
	}
	printState(ilclient_get_handle(compoent));

	err=ilclient_change_component_state(compoent,OMX_StateExecuting);
	if (err == -1){
		fprintf(stderr, "%s():change component's state to StateExecuting failed \n ", __FUNCTION__);
		exit(1);
	}

	printState(ilclient_get_handle(compoent));
	return 0;
}