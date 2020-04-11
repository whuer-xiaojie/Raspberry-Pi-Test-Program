/*
* OMX/omx_graphics.h
*/

#ifndef OMX_GRAPHICS_H
#define OMX_GRAPHICS_H

#include <IL/OMX_Broadcom.h>
#include <IL/OMX_Core.h>

#include "ilclient.h"

#define TIMEOUT_MS 1000

typedef enum ILC_ERRORTYPE{
	ILC_ErrorNone,
	ILC_ErrorComponentCreate,
	ILC_ErrorHandleGet,
	ILC_ErrorEventWait,
	ILC_ErrorStateChange,
}ILC_ERRORTYPE;

typedef struct RENDER_DISPLAY_CONFIG{
	int screen_width;
	int screen_height;
	int xOffset;
	int yOffset;
}RENDER_DISPLAY_CONFIG;

typedef struct RENDER{
	ILCLIENT_T *ilClient;

	COMPONENT_T *renderComponent;
	OMX_HANDLETYPE renderHandle;
	int renderInputPort;

	RENDER_DISPLAY_CONFIG *renderConfig;

	COMPONENT_T *resizeComponent;
	OMX_HANDLETYPE resizeHandle;
	int resizeInputPort;
	int resizeOutputPort;

	OMX_BUFFERHEADERTYPE *pInputBuffer;
	volatile char pSettingsChanged;//the flag of output port settings changed

}RENDER;

int omx_init_graphics(void);
int omx_close_graphics(void);
void omx_render_buf_r8g8b8(unsigned char *buf);

#endif