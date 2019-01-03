/*
* omx_helper.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Audio.h>
#include <IL/OMX_IVCommon.h>
#include "omx_helper.h"



static OMX_CALLBACKTYPE callBacks = {
	.EventHandler    = OMX_EventCallBack,
	.EmptyBufferDone = OMX_EmptyBufferCallBack,
	.FillBufferDone  = OMX_FillBufferCallBack,
};

OMX_ERRORTYPE OMX_EventCallBack(OMX_IN OMX_HANDLETYPE hComponent,OMX_IN OMX_PTR pAppData,
	OMX_IN OMX_EVENTTYPE eEvent,OMX_IN OMX_U32 nData1,OMX_IN OMX_U32 nData2,OMX_IN OMX_PTR pEventData) 
{

	fprintf(stderr,COLOR_RED"eEvent: %s,  nData1: %x,  nData2: %x\n"COLOR_NC, OMX_EventType2Enum(eEvent),
		nData1, nData2);

	switch (eEvent) {
	case OMX_EventCmdComplete:
		break;
	case OMX_EventError:
		fprintf(stderr,COLOR_RED "ErrorType: %s,  nData2: %x\n" COLOR_NC, OMX_ErrorType2Enum(nData1), nData2);
		if (nData1 != OMX_ErrorStreamCorrupt) {
			assert(NULL);
		}
		break;
	default:
		//printf("Unhandeled Event %x: %x %x\n", eEvent, nData1, nData2);
		break;
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_EmptyBufferCallBack(OMX_IN OMX_HANDLETYPE hComponent,OMX_IN OMX_PTR pAppData,
	OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_FillBufferCallBack(OMX_OUT OMX_HANDLETYPE hComponent,OMX_OUT OMX_PTR pAppData,
	OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) 
{
	return OMX_ErrorNone;
}

const char *OMX_ErrorType2Enum(OMX_ERRORTYPE eError)
{
	switch (eError) {
		case CASE_STRING(OMX_ErrorNone);
		case CASE_STRING(OMX_ErrorInsufficientResources);
		case CASE_STRING(OMX_ErrorUndefined);
		case CASE_STRING(OMX_ErrorInvalidComponentName);
		case CASE_STRING(OMX_ErrorComponentNotFound);
		case CASE_STRING(OMX_ErrorInvalidComponent);
		case CASE_STRING(OMX_ErrorBadParameter);
		case CASE_STRING(OMX_ErrorNotImplemented);
		case CASE_STRING(OMX_ErrorUnderflow);
		case CASE_STRING(OMX_ErrorOverflow);
		case CASE_STRING(OMX_ErrorHardware);
		case CASE_STRING(OMX_ErrorInvalidState);
		case CASE_STRING(OMX_ErrorStreamCorrupt);
		case CASE_STRING(OMX_ErrorPortsNotCompatible);
		case CASE_STRING(OMX_ErrorResourcesLost);
		case CASE_STRING(OMX_ErrorNoMore);
		case CASE_STRING(OMX_ErrorVersionMismatch);
		case CASE_STRING(OMX_ErrorNotReady);
	    case CASE_STRING(OMX_ErrorTimeout);
	    case CASE_STRING(OMX_ErrorSameState);
	    case CASE_STRING(OMX_ErrorResourcesPreempted);
	    case CASE_STRING(OMX_ErrorPortUnresponsiveDuringAllocation);
	    case CASE_STRING(OMX_ErrorPortUnresponsiveDuringDeallocation);
	    case CASE_STRING(OMX_ErrorPortUnresponsiveDuringStop);
	    case CASE_STRING(OMX_ErrorIncorrectStateTransition);
	    case CASE_STRING(OMX_ErrorIncorrectStateOperation);
	    case CASE_STRING(OMX_ErrorUnsupportedSetting);
	    case CASE_STRING(OMX_ErrorUnsupportedIndex);
	    case CASE_STRING(OMX_ErrorBadPortIndex);
	    case CASE_STRING(OMX_ErrorPortUnpopulated);
	    case CASE_STRING(OMX_ErrorComponentSuspended);
	    case CASE_STRING(OMX_ErrorDynamicResourcesUnavailable);
	    case CASE_STRING(OMX_ErrorMbErrorsInFrame);
	    case CASE_STRING(OMX_ErrorFormatNotDetected);
	    case CASE_STRING(OMX_ErrorContentPipeOpenFailed);
	    case CASE_STRING(OMX_ErrorContentPipeCreationFailed);
	    case CASE_STRING(OMX_ErrorSeperateTablesUsed);
	    case CASE_STRING(OMX_ErrorTunnelingUnsupported);
	
	    case CASE_STRING(OMX_ErrorDiskFull);
	    case CASE_STRING(OMX_ErrorMaxFileSize);
	    case CASE_STRING(OMX_ErrorDrmUnauthorised);
	    case CASE_STRING(OMX_ErrorDrmExpired);
	    case CASE_STRING(OMX_ErrorDrmGeneral);
		default: return "OMX_ErrorUnknown";  
	}
}
const char *OMX_CommandType2Enum(OMX_COMMANDTYPE eCommand)
{
	switch (eCommand) {
		case CASE_STRING(OMX_CommandStateSet);
		case CASE_STRING(OMX_CommandFlush);
		case CASE_STRING(OMX_CommandPortDisable);
		case CASE_STRING(OMX_CommandPortEnable);
		case CASE_STRING(OMX_CommandMarkBuffer);
		default: return "OMX_CommandUnknown";
	}
}
const char *OMX_ImageCodingType2Enum(OMX_IMAGE_CODINGTYPE eCompressionFormat)
{
	switch (eCompressionFormat) {
		case CASE_STRING(OMX_IMAGE_CodingUnused);
		case CASE_STRING(OMX_IMAGE_CodingAutoDetect);
		case CASE_STRING(OMX_IMAGE_CodingJPEG);
		case CASE_STRING(OMX_IMAGE_CodingJPEG2K);
		case CASE_STRING(OMX_IMAGE_CodingEXIF);
		case CASE_STRING(OMX_IMAGE_CodingTIFF);
		case CASE_STRING(OMX_IMAGE_CodingGIF);
		case CASE_STRING(OMX_IMAGE_CodingPNG);
		case CASE_STRING(OMX_IMAGE_CodingLZW);
		case CASE_STRING(OMX_IMAGE_CodingBMP);

		case CASE_STRING(OMX_IMAGE_CodingTGA);
		case CASE_STRING(OMX_IMAGE_CodingPPM);
		default: return "OMX_IMAGE_CodingUnknown";		
	}
}
const char *OMX_VideoCodingType2Enum(OMX_VIDEO_CODINGTYPE eCompressionFormat)
{
	switch (eCompressionFormat) {
		case  CASE_STRING(OMX_VIDEO_CodingUnused);
		case  CASE_STRING(OMX_VIDEO_CodingAutoDetect);
		case  CASE_STRING(OMX_VIDEO_CodingMPEG2);
		case  CASE_STRING(OMX_VIDEO_CodingH263);
		case  CASE_STRING(OMX_VIDEO_CodingMPEG4);
		case  CASE_STRING(OMX_VIDEO_CodingWMV);
		case  CASE_STRING(OMX_VIDEO_CodingRV);
		case  CASE_STRING(OMX_VIDEO_CodingAVC);
		case  CASE_STRING(OMX_VIDEO_CodingMJPEG);

		case  CASE_STRING(OMX_VIDEO_CodingVP6);
		case  CASE_STRING(OMX_VIDEO_CodingVP7);
		case  CASE_STRING(OMX_VIDEO_CodingVP8);
		case  CASE_STRING(OMX_VIDEO_CodingYUV);
		case  CASE_STRING(OMX_VIDEO_CodingSorenson);
		case  CASE_STRING(OMX_VIDEO_CodingTheora);
		case  CASE_STRING(OMX_VIDEO_CodingMVC);
		default:return "OMX_VIDEO_CodingUnknown";
	}
}
const char *OMX_AudioCodingType2Enum(OMX_AUDIO_CODINGTYPE eCompressionFormat)
{
	switch (eCompressionFormat){
		case CASE_STRING(OMX_AUDIO_CodingUnused);
		case CASE_STRING(OMX_AUDIO_CodingAutoDetect);
		case CASE_STRING(OMX_AUDIO_CodingPCM);
		case CASE_STRING(OMX_AUDIO_CodingADPCM);
		case CASE_STRING(OMX_AUDIO_CodingAMR);
		case CASE_STRING(OMX_AUDIO_CodingGSMFR);
		case CASE_STRING(OMX_AUDIO_CodingGSMEFR);
		case CASE_STRING(OMX_AUDIO_CodingGSMHR);
		case CASE_STRING(OMX_AUDIO_CodingPDCFR);
		case CASE_STRING(OMX_AUDIO_CodingPDCEFR);
		case CASE_STRING(OMX_AUDIO_CodingPDCHR);
		case CASE_STRING(OMX_AUDIO_CodingTDMAFR);
		case CASE_STRING(OMX_AUDIO_CodingTDMAEFR);
		case CASE_STRING(OMX_AUDIO_CodingQCELP8	);
		case CASE_STRING(OMX_AUDIO_CodingQCELP13);
		case CASE_STRING(OMX_AUDIO_CodingEVRC);
		case CASE_STRING(OMX_AUDIO_CodingSMV);
		case CASE_STRING(OMX_AUDIO_CodingG711);
		case CASE_STRING(OMX_AUDIO_CodingG723);
		case CASE_STRING(OMX_AUDIO_CodingG726);
		case CASE_STRING(OMX_AUDIO_CodingG729);
		case CASE_STRING(OMX_AUDIO_CodingAAC);
		case CASE_STRING(OMX_AUDIO_CodingMP3);
		case CASE_STRING(OMX_AUDIO_CodingSBC);
		case CASE_STRING(OMX_AUDIO_CodingVORBIS);
		case CASE_STRING(OMX_AUDIO_CodingWMA);
		case CASE_STRING(OMX_AUDIO_CodingRA);
		case CASE_STRING(OMX_AUDIO_CodingMIDI);

		case CASE_STRING(OMX_AUDIO_CodingFLAC );
		case CASE_STRING(OMX_AUDIO_CodingDDP);
		case CASE_STRING(OMX_AUDIO_CodingDTS);
		case CASE_STRING(OMX_AUDIO_CodingWMAPRO);
		case CASE_STRING(OMX_AUDIO_CodingATRAC3);
		case CASE_STRING(OMX_AUDIO_CodingATRACX);
		case CASE_STRING(OMX_AUDIO_CodingATRACAAL);
		default:return "OMX_AUDIO_CodingUnknown";
	}
}
const char *OMX_ColorFormatType2Enum(OMX_COLOR_FORMATTYPE eColorFormat)
{
	switch (eColorFormat) {
		case CASE_STRING(OMX_COLOR_FormatUnused);
		case CASE_STRING(OMX_COLOR_FormatMonochrome);
		case CASE_STRING(OMX_COLOR_Format8bitRGB332);
		case CASE_STRING(OMX_COLOR_Format12bitRGB444);
		case CASE_STRING(OMX_COLOR_Format16bitARGB4444);
		case CASE_STRING(OMX_COLOR_Format16bitARGB1555);
		case CASE_STRING(OMX_COLOR_Format16bitRGB565);
		case CASE_STRING(OMX_COLOR_Format16bitBGR565);
		case CASE_STRING(OMX_COLOR_Format18bitRGB666);
		case CASE_STRING(OMX_COLOR_Format18bitARGB1665);
		case CASE_STRING(OMX_COLOR_Format19bitARGB1666);
		case CASE_STRING(OMX_COLOR_Format24bitRGB888);
		case CASE_STRING(OMX_COLOR_Format24bitBGR888);
		case CASE_STRING(OMX_COLOR_Format24bitARGB1887);
		case CASE_STRING(OMX_COLOR_Format25bitARGB1888);
		case CASE_STRING(OMX_COLOR_Format32bitBGRA8888);
		case CASE_STRING(OMX_COLOR_Format32bitARGB8888);
		case CASE_STRING(OMX_COLOR_FormatYUV411Planar);
		case CASE_STRING(OMX_COLOR_FormatYUV411PackedPlanar);
		case CASE_STRING(OMX_COLOR_FormatYUV420Planar);
		case CASE_STRING(OMX_COLOR_FormatYUV420PackedPlanar);
		case CASE_STRING(OMX_COLOR_FormatYUV420SemiPlanar);
		case CASE_STRING(OMX_COLOR_FormatYUV422Planar);
		case CASE_STRING(OMX_COLOR_FormatYUV422PackedPlanar);
		case CASE_STRING(OMX_COLOR_FormatYUV422SemiPlanar);
		case CASE_STRING(OMX_COLOR_FormatYCbYCr);
		case CASE_STRING(OMX_COLOR_FormatYCrYCb);
		case CASE_STRING(OMX_COLOR_FormatCbYCrY);
		case CASE_STRING(OMX_COLOR_FormatCrYCbY);
		case CASE_STRING(OMX_COLOR_FormatYUV444Interleaved);
		case CASE_STRING(OMX_COLOR_FormatRawBayer8bit);
		case CASE_STRING(OMX_COLOR_FormatRawBayer10bit);
		case CASE_STRING(OMX_COLOR_FormatRawBayer8bitcompressed);
		case CASE_STRING(OMX_COLOR_FormatL2);
		case CASE_STRING(OMX_COLOR_FormatL4);
		case CASE_STRING(OMX_COLOR_FormatL8);
		case CASE_STRING(OMX_COLOR_FormatL16);
		case CASE_STRING(OMX_COLOR_FormatL24);
		case CASE_STRING(OMX_COLOR_FormatL32);
		case CASE_STRING(OMX_COLOR_FormatYUV420PackedSemiPlanar);
		case CASE_STRING(OMX_COLOR_FormatYUV422PackedSemiPlanar);
		case CASE_STRING(OMX_COLOR_Format18BitBGR666);
		case CASE_STRING(OMX_COLOR_Format24BitARGB6666);
		case CASE_STRING(OMX_COLOR_Format24BitABGR6666);

		case CASE_STRING(OMX_COLOR_Format32bitABGR8888);
		case CASE_STRING(OMX_COLOR_Format8bitPalette);
		case CASE_STRING(OMX_COLOR_FormatYUVUV128);
		case CASE_STRING(OMX_COLOR_FormatRawBayer12bit);
		case CASE_STRING(OMX_COLOR_FormatBRCMEGL);
		case CASE_STRING(OMX_COLOR_FormatBRCMOpaque);
		case CASE_STRING(OMX_COLOR_FormatYVU420PackedPlanar);
		case CASE_STRING(OMX_COLOR_FormatYVU420PackedSemiPlanar);
		case CASE_STRING(OMX_COLOR_FormatRawBayer16bit);
		//case CASE_STRING(OMX_COLOR_FormatYUV420_16PackedPlanar); 
		//case CASE_STRING(OMX_COLOR_FormatYUVUV64_16);
		default: return "OMX_COLOR_FormatUnknown";
	}
}
const char *OMX_EventType2Enum(OMX_EVENTTYPE eEvent)
{
	switch (eEvent) {
		case CASE_STRING(OMX_EventCmdComplete);
		case CASE_STRING(OMX_EventError);
		case CASE_STRING(OMX_EventMark);
		case CASE_STRING(OMX_EventPortSettingsChanged);
		case CASE_STRING(OMX_EventBufferFlag);
		case CASE_STRING(OMX_EventResourcesAcquired);
		case CASE_STRING(OMX_EventComponentResumed);
		case CASE_STRING(OMX_EventDynamicResourcesAvailable);
		case CASE_STRING(OMX_EventPortFormatDetected);

		case CASE_STRING(OMX_EventParamOrConfigChanged);
		default:return "OMX_EvetnUnknown";
	}
}
const char *OMX_StateType2Enum(OMX_STATETYPE eState)
{
	switch (eState) {
		case CASE_STRING(OMX_StateInvalid);
		case CASE_STRING(OMX_StateLoaded);
		case CASE_STRING(OMX_StateIdle);
		case CASE_STRING(OMX_StateExecuting);
		case CASE_STRING(OMX_StatePause);
		case CASE_STRING(OMX_StateWaitForResources);

		default:return "OMX_StateUnknown";
	}
}


//shoule called after OMX_Init() finished
void OMX_ListAllComponent(void)
{
	char stringBackingStore[256];
	OMX_STRING pComponentName = stringBackingStore;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	int i;
	puts(COLOR_YELLOW "*************************************" COLOR_NC);
	puts(COLOR_YELLOW "**  Listing OpenMAX IL Components  **" COLOR_NC);
	for (i = 0; err != OMX_ErrorNoMore; i++){
		err = OMX_ComponentNameEnum(pComponentName, 256, i);
		if (err == OMX_ErrorNone){
			indent(1) fprintf(stderr, "Component %3d is:%s\n", i, pComponentName);
		}
		else{

		}
	}
	puts(COLOR_YELLOW "*************************************" COLOR_NC);
}
//shoule called after OMX_Init() finished
void OMX_ListComponentRoles(OMX_STRING pComponentName)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_U32 numRoles;
	OMX_U8 **roles;
	int i;

	err = OMX_GetRolesOfComponent(pComponentName, &numRoles, NULL);
	OMX_AssertError(err);
	indent(1) fprintf(stderr, COLOR_GREEN"%s Roles Number is:%d\n"COLOR_NC, pComponentName, numRoles);

	for (i = 0; i < numRoles; i++){
		roles[i] = malloc(256);
	}
	err = OMX_GetRolesOfComponent(pComponentName, &numRoles, roles);
	OMX_AssertError(err);

	for (i = 0; i < numRoles; i++){
		indent(2) fprintf(stderr, "Role %d is :%s\n", i, roles[i]);
		free(roles[i]);
	}
}
static OMX_ERRORTYPE OMX_PrintPortDirType(OMX_HANDLETYPE hComponent,OMX_U32 portIndex,int indentNum)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PARAM_PORTDEFINITIONTYPE portDef;
	OMX_INIT_STRUCTURE(portDef);
	portDef.nPortIndex = portIndex;

	err = OMX_GetParameter(hComponent, OMX_IndexParamPortDefinition, &portDef);
	if (err != OMX_ErrorNone){
		return err;
	}
	indent(indentNum) fprintf(stderr, "Port %d Is :%s\n", portDef.nPortIndex,
		(portDef.eDir == OMX_DirInput ? "Input Port" : "Output Port"));
	return err;
}
void OMX_ListComponentAllPorts(OMX_HANDLETYPE hComponent)
{
	char stringBackingStore[256];
	OMX_STRING pComponentName = stringBackingStore;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PORT_PARAM_TYPE param;
	int i;
	
	
	err = OMX_GetComponentVersion(hComponent, pComponentName, NULL, NULL, NULL);
	OMX_AssertError(err);

	indent(1) fprintf(stderr, COLOR_GREEN"%s Ports Info:\n"COLOR_NC, pComponentName);

	//get and print image port information
	OMX_INIT_STRUCTURE(param);
	err = OMX_GetParameter(hComponent, OMX_IndexParamImageInit, &param);
	OMX_AssertError(err);
	indent(2) fprintf(stderr, COLOR_YELLOW"Image Port Number:%d"COLOR_NC, param.nPorts);
	if (param.nPorts > 0){
		fprintf(stderr, " Start Index:%d\n", param.nStartPortNumber);
		for (i = 0; i < param.nPorts; i++){
			OMX_U32 portIndex = (OMX_U32)param.nStartPortNumber + i;
			err = OMX_PrintPortDirType(hComponent, portIndex, 3);
			OMX_AssertError(err);
		}
	}

	//get and print video port information
	OMX_INIT_STRUCTURE(param);
	err = OMX_GetParameter(hComponent, OMX_IndexParamVideoInit, &param);
	OMX_AssertError(err);
	indent(2) fprintf(stderr, COLOR_YELLOW"Video Port Number:%d"COLOR_NC, param.nPorts);
	if (param.nPorts > 0){
		fprintf(stderr, " Start Index:%d\n", param.nStartPortNumber);
		for (i = 0; i < param.nPorts; i++){
			OMX_U32 portIndex = (OMX_U32)param.nStartPortNumber + i;
			err = OMX_PrintPortDirType(hComponent, portIndex, 3);
			OMX_AssertError(err);
		}
	}

	//get and print audio port information
	OMX_INIT_STRUCTURE(param);
	err = OMX_GetParameter(hComponent, OMX_IndexParamAudioInit, &param);
	OMX_AssertError(err);
	indent(2) fprintf(stderr, COLOR_YELLOW"Audio Port Number:%d"COLOR_NC, param.nPorts);
	if (param.nPorts > 0){
		fprintf(stderr, " Start Index:%d\n",param.nStartPortNumber);
		for (i = 0; i < param.nPorts; i++){
			OMX_U32 portIndex = (OMX_U32)param.nStartPortNumber + i;
			err = OMX_PrintPortDirType(hComponent, portIndex, 3);
			OMX_AssertError(err);
		}
	}

	//get and print other port information
	OMX_INIT_STRUCTURE(param);
	err = OMX_GetParameter(hComponent, OMX_IndexParamOtherInit, &param);
	OMX_AssertError(err);
	indent(2) fprintf(stderr, COLOR_YELLOW"Other Port Number:%d"COLOR_NC, param.nPorts);
	if (param.nPorts > 0){
		fprintf(stderr, " Start Index:%d\n", param.nStartPortNumber);
		for (i = 0; i < param.nPorts; i++){
			OMX_U32 portIndex = (OMX_U32)param.nStartPortNumber + i;
			err = OMX_PrintPortDirType(hComponent, portIndex, 3);
			OMX_AssertError(err);
		}
	}

}
void OMX_ListImagePortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
	OMX_INIT_STRUCTURE(imagePortFormat);
	imagePortFormat.nPortIndex = portIndex;

	indent(1) fprintf(stderr, COLOR_GREEN"Port %d Supported Image Formats are:\n"COLOR_NC, portIndex);
	while (1){
		err = OMX_GetParameter(hComponent, OMX_IndexParamImagePortFormat, &imagePortFormat);
		if (err == OMX_ErrorNoMore){
			indent(1) fprintf(stderr, COLOR_RED"No More Formats Supported\n"COLOR_NC);
			return;
		}
		OMX_AssertError(err);
		indent(2) fprintf(stderr, "Image Coding Type:%30s           Image Color Format:%s\n",
					OMX_ImageCodingType2Enum(imagePortFormat.eCompressionFormat),
					OMX_ColorFormatType2Enum(imagePortFormat.eColorFormat));
		imagePortFormat.nIndex++;
	}
}
void OMX_ListVideoPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VIDEO_PARAM_PORTFORMATTYPE videoPortFormat;
	OMX_INIT_STRUCTURE(videoPortFormat);
	videoPortFormat.nPortIndex = portIndex;

	indent(1) fprintf(stderr, COLOR_GREEN"Port %d Supported Video Formats are:\n"COLOR_NC, portIndex);
	while (1){
		err = OMX_GetParameter(hComponent, OMX_IndexParamVideoPortFormat, &videoPortFormat);
		if (err == OMX_ErrorNoMore){
			indent(1) fprintf(stderr, COLOR_RED"No More Formats Supported\n"COLOR_NC);
			return;
		}
		OMX_AssertError(err);
		indent(2) fprintf(stderr, "Video Coding Type:%30s           Video Color Format:%s\n",
			OMX_VideoCodingType2Enum(videoPortFormat.eCompressionFormat),
			OMX_ColorFormatType2Enum(videoPortFormat.eColorFormat));
		videoPortFormat.nIndex++;
	}
}
void OMX_ListAudioPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_PARAM_PORTFORMATTYPE audioPortFormat;
	OMX_INIT_STRUCTURE(audioPortFormat);
	audioPortFormat.nPortIndex = portIndex;
	indent(1) fprintf(stderr, COLOR_GREEN"Port %d Supported Audio Formats are:\n"COLOR_NC, portIndex);
	while (1){
		err = OMX_GetParameter(hComponent, OMX_IndexParamAudioPortFormat, &audioPortFormat);
		if (err == OMX_ErrorNoMore){
			indent(1) fprintf(stderr, COLOR_RED"No More Formats Supported\n"COLOR_NC);
			return;
		}
		OMX_AssertError(err);
		indent(2) fprintf(stderr, "Audio Coding Type:%30s \n", OMX_AudioCodingType2Enum(audioPortFormat.eEncoding));
		audioPortFormat.nIndex++;
	}
}
void OMX_ListOtherPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex)
{

}



void OMX_PrintComponentVersion(OMX_HANDLETYPE hComponent)
{
	char stringBackingStore[256];
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;
	OMX_STRING pComponentName = stringBackingStore;

	err = OMX_GetComponentVersion(hComponent, pComponentName, &compVersion, &specVersion, &uid);
	OMX_AssertError(err);
	indent(1) fprintf(stderr, "Component Name:%s\n", pComponentName);
	indent(1) fprintf(stderr, "Component Version:%d.%d\n", compVersion.s.nVersionMajor, compVersion.s.nVersionMinor);
	indent(1) fprintf(stderr, "Spec Version:%d.%d\n", specVersion.s.nVersionMajor, specVersion.s.nVersionMinor);
}
void OMX_PrintComponentInfo(OMX_U32 componentIndex)
{
	char stringBackingStore[256];
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;
	OMX_STRING pComponentName = stringBackingStore;
	OMX_HANDLETYPE hComponent;

	//get the index component name  
	err = OMX_ComponentNameEnum(pComponentName, 256, componentIndex);
	OMX_AssertError(err);

	fprintf(stderr,COLOR_GREEN "*************************************************************\n" COLOR_NC);
	fprintf(stderr,COLOR_GREEN "**  Showing informations for %30s  **\n" COLOR_NC, pComponentName);
	

	//get component handle
	err = OMX_GetHandle(&hComponent, pComponentName, NULL, &callBacks);
	OMX_AssertError(err);

	OMX_ListComponentRoles(pComponentName);
	OMX_PrintComponentVersion(hComponent);
	OMX_ListComponentAllPorts(hComponent);

	err = OMX_FreeHandle(hComponent);
	OMX_AssertError(err);
	fprintf(stderr,COLOR_GREEN "*************************************************************\n" COLOR_NC);

}
void OMX_PrintPortInfo(OMX_HANDLETYPE hComponent, OMX_U32 portIndex)
{
	char stringBackingStore[256];
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;
	OMX_STRING pComponentName = stringBackingStore;

	OMX_ListAudioPortSupportFormat(hComponent, portIndex);
	OMX_ListVideoPortSupportFormat(hComponent, portIndex);
	OMX_ListImagePortSupportFormat(hComponent, portIndex);
}
void OMX_PrintComponentState(OMX_HANDLETYPE hComponent)
{
	char stringBackingStore[256];
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_STATETYPE state=OMX_StateInvalid;
	OMX_STRING pComponentName = stringBackingStore;

	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;

	err = OMX_GetComponentVersion(hComponent, pComponentName, &compVersion, &specVersion, &uid);
	//err = OMX_GetComponentVersion(hComponent, pComponentName, NULL, NULL, NULL);
	OMX_AssertError(err);

	err = OMX_GetState(hComponent, &state);
	OMX_AssertError(err);

	indent(1) fprintf(stderr, COLOR_GREEN"%s state is:%s\n"COLOR_NC, pComponentName, OMX_StateType2Enum(state));
}
void OMX_CheckComponentState(OMX_HANDLETYPE hComponent, OMX_STATETYPE state)
{

}



int OMX_GetInputPortIndex(OMX_HANDLETYPE hComponent, OMX_INDEXTYPE eIndex)
{
	int portIndex = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PORT_PARAM_TYPE param;
	OMX_INIT_STRUCTURE(param);

	err = OMX_GetParameter(hComponent, eIndex, &param);
	OMX_AssertError(err);
	
	portIndex = param.nStartPortNumber;
	return portIndex;
}
int OMX_GetOutputPortIndex(OMX_HANDLETYPE hComponent, OMX_INDEXTYPE eIndex)
{
	int portIndex = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PORT_PARAM_TYPE param;
	OMX_INIT_STRUCTURE(param);

	err = OMX_GetParameter(hComponent, eIndex, &param);
	OMX_AssertError(err);

	if (param.nPorts != 2){
		return portIndex;
	}
	portIndex = param.nStartPortNumber + 1;
	return portIndex;
}