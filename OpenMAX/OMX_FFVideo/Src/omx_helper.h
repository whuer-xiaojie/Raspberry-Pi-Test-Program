/*
* omx_helper.h
*/

#ifndef OMX_HELPER_H
#define OMX_HELPER_H

#include <assert.h>
#include <IL/OMX_Core.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_IVCommon.h>


#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_NC      "\033[0m"

#define ALIGN16(x) (((x+0xf)>>4)<<4)

#define OMX_INIT_STRUCTURE(a) \
  memset (&(a), 0, sizeof (a)); \
  (a).nSize = sizeof (a); \
  (a).nVersion.nVersion = OMX_VERSION; \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

#define indent(level) {int n = 0; while (n++ < level*2) fprintf(stderr," ");}

//return the string name of enum
#define CASE_STRING(x) x: return #x

#define OMX_AssertError(x) if (x != OMX_ErrorNone) { \
    fprintf(stderr,COLOR_RED"File:%s Func:%s Line:%d OMX_Error: %s\n"COLOR_NC,\
	__FILE__,__FUNCTION__,__LINE__, OMX_ErrorType2Enum(x)); \
    assert(x == OMX_ErrorNone); \
}

OMX_ERRORTYPE OMX_EventCallBack(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_PTR pAppData,
	OMX_IN OMX_EVENTTYPE eEvent, OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData);

OMX_ERRORTYPE OMX_EmptyBufferCallBack(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_PTR pAppData,
	OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

OMX_ERRORTYPE OMX_FillBufferCallBack(OMX_OUT OMX_HANDLETYPE hComponent, OMX_OUT OMX_PTR pAppData,
	OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

const char *OMX_ErrorType2Enum(OMX_ERRORTYPE eError);
const char *OMX_CommandType2Enum(OMX_COMMANDTYPE eCommand);
const char *OMX_ImageCodingType2Enum(OMX_IMAGE_CODINGTYPE eCompressionFormat);
const char *OMX_AudioCodingType2Enum(OMX_AUDIO_CODINGTYPE eCompressionFormat);
const char *OMX_VideoCodingType2Enum(OMX_VIDEO_CODINGTYPE eCompressionFormat);
const char *OMX_ColorFormatType2Enum(OMX_COLOR_FORMATTYPE eColorFormat);
const char *OMX_EventType2Enum(OMX_EVENTTYPE eEvent);
const char *OMX_StateType2Enum(OMX_STATETYPE eState);

void OMX_ListAllComponent(void);
void OMX_ListComponentRoles(OMX_STRING pComponentName);
void OMX_ListComponentAllPorts(OMX_HANDLETYPE hComponent);
void OMX_ListImagePortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex);
void OMX_ListVideoPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex);
void OMX_ListAudioPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex);
void OMX_ListOtherPortSupportFormat(OMX_HANDLETYPE hComponent, OMX_U32 portIndex);


void OMX_PrintComponentVersion(OMX_HANDLETYPE hComponent);
void OMX_PrintComponentInfo(OMX_U32 componentIndex);
void OMX_PrintPortInfo(OMX_HANDLETYPE hComponent, OMX_U32 portIndex);
void OMX_PrintComponentState(OMX_HANDLETYPE hComponent);

void OMX_CheckComponentState(OMX_HANDLETYPE hComponent, OMX_STATETYPE state);

int OMX_GetInputPortIndex(OMX_HANDLETYPE hComponent, OMX_INDEXTYPE eIndex);
int OMX_GetOutputPortIndex(OMX_HANDLETYPE hComponent, OMX_INDEXTYPE eIndex);
#endif