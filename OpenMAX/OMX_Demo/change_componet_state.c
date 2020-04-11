#include <stdio.h>
#include <stdlib.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

#include <bcm_host.h>
OMX_ERRORTYPE cEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
	OMX_EVENTTYPE eEvent,OMX_U32 Data1,OMX_U32 Data2,OMX_PTR pEventData )
{
	printf("Hi there, I am in the %s callback function\n", __FUNCTION__);
	printf("Event is %i\n", (int)eEvent);
	printf("Param1 is %i\n", (int)Data1);
	printf("Param2 is %i\n", (int)Data2);
	return OMX_ErrorNone;
}
OMX_CALLBACKTYPE callBacks = {
	.EventHandler    = cEventHandler,
	.EmptyBufferDone = NULL,
	.FillBufferDone  = NULL
};
static char *OMX_Error_To_String(OMX_ERRORTYPE err)
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
	default:
		break;
	}
}

static void printState(OMX_HANDLETYPE handle)
{
	OMX_STATETYPE state; OMX_ERRORTYPE err;
	err = OMX_GetState(handle, &state);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get handle state error\n", __FUNCTION__);
		exit(0);
	}
	switch (state){
	case OMX_StateLoaded:printf("StateLoaded\n"); break;
	case OMX_StateIdle: printf("StateIdle\n"); break;
	case OMX_StateExecuting: printf("StateExecuting\n"); break;
	case OMX_StatePause: printf("StatePause\n"); break;
	case OMX_StateWaitForResources: printf("StateWaiteForResource\n"); break;
	case OMX_StateInvalid:printf("StateInvalid\n"); break;
	default:printf("State Unknown\n");break;
	}
}

static void disableSomePorts(OMX_HANDLETYPE handle,OMX_INDEXTYPE index)
{
	OMX_PORT_PARAM_TYPE param; OMX_ERRORTYPE err;
	int startPortNumber, endPortNumber;
	int nPorts, n;
	
	memset(&param, 0, sizeof(OMX_PORT_PARAM_TYPE));
	param.nSize = sizeof(OMX_PORT_PARAM_TYPE);
	param.nVersion.nVersion = OMX_VERSION;

	err = OMX_GetParameter(handle, index, &param);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "%s():get paramer failed \n", __FUNCTION__);
		exit(0);
	}
	startPortNumber = param.nStartPortNumber;
	nPorts = param.nPorts;
	endPortNumber = startPortNumber + nPorts;
	for (n = startPortNumber; n < endPortNumber; n++){
		OMX_SendCommand(handle, OMX_CommandPortDisable, n, NULL);
	}
}

static void disableAllPorts(OMX_HANDLETYPE handler)
{
	disableSomePorts(handler, OMX_IndexParamAudioInit);
	disableSomePorts(handler, OMX_IndexParamVideoInit);
	disableSomePorts(handler, OMX_IndexParamImageInit);
	disableSomePorts(handler, OMX_IndexParamOtherInit);
}

int main(int argc, char **argv)
{
	if (argc < 2){
		fprintf(stderr, "Usage:%s component-name \n", argv[0]);
		exit(0);
	}
	char *componentName = argv[1];
	OMX_HANDLETYPE handler;
	OMX_ERRORTYPE err;

	bcm_host_init();
	err = OMX_Init();
	if (err != OMX_ErrorNone) {
		fprintf(stderr, "OMX_Init() failed %s\n", OMX_Error_To_String(err));
		exit(1);
	}

	err = OMX_GetHandle(&handler, componentName, NULL, &callBacks);
	if (err != OMX_ErrorNone){
		fprintf(stderr, "OMX_GetHandle() failed %s\n", OMX_Error_To_String(err));
		exit(1);
	}
	//sleep(3);
	printState(handler);

	sleep(3);
	OMX_SendCommand(handler, OMX_CommandStateSet, OMX_StateIdle, NULL);
	printState(handler);
	
	sleep(3);
	disableAllPorts(handler);
	OMX_SendCommand(handler, OMX_CommandStateSet, OMX_StatePause, NULL);
	//printState(handler);
	int n = 0;
	while (n++ < 10) {
		sleep(1);
		// are we there yet?
		printState(handler);
	}

	return 0;
}