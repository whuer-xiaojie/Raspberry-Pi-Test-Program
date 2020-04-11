#ifndef _OMX_COMMON_H_
#define _OMX_COMMON_H_

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

void setHeader(OMX_PTR header, OMX_U32 size, OMX_VERSIONTYPE specVersion, OMX_VERSIONTYPE compVersion);

char *OMX_ErrorToString(OMX_ERRORTYPE err);

#endif