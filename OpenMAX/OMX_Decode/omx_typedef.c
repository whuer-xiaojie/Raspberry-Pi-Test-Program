/*
*omx_typedef.c
*/

#include <stdio.h>
#include <stdlib.h>

#include "omx_typedef.h"

char *OMX_ErrorToString(OMX_ERRORTYPE err)
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