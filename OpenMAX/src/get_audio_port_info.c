#include <stdio.h>
#include <stdlib.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Audio.h>


#include <bcm_host.h>

void get_port_information(int portIndex, OMX_PARAM_PORTDEFINITIONTYPE portdef)
{
	fprintf(stderr, "port %d requires %d buffer \n", portIndex, portdef.nBufferCountMin);
	fprintf(stderr, "port %d has min buffer %d bytes\n", portIndex, portdef.nBufferSize);
	if (portdef.eDir == OMX_DirInput)
		fprintf(stderr, "port %d is an input port\n", portIndex);
	else
		fprintf(stderr, "port %d is an output port\n", portIndex);
	switch (portdef.eDomain)
	{
	case OMX_PortDomainAudio:
	{
		fprintf(stderr, "the port %d is an audio port \n", portdef.nPortIndex);
		fprintf(stderr, "the port mimetype is :%s\n", portdef.format.audio.cMIMEType);
		switch (portdef.format.audio.eEncoding)
		{
		case OMX_AUDIO_CodingPCM:
			printf("Port encoding is PCM\n");
			break;
		case OMX_AUDIO_CodingVORBIS:
			printf("Port encoding is Ogg Vorbis\n");
			break;
		case OMX_AUDIO_CodingMP3:
			printf("Port encoding is MP3\n");
			break;
		default:
			printf("Port encoding is not PCM or MP3 or Vorbis, is %d\n",
				portdef.format.audio.eEncoding);
			break;
		}
		break;
	}
	case OMX_PortDomainVideo:
	{
		fprintf(stderr, "the port %d is a video port \n", portdef.nPortIndex);
		fprintf(stderr, "the port mimetype is :%s\n", portdef.format.video.cMIMEType);
		break;
	}
	case OMX_PortDomainImage:
	{
		fprintf(stderr, "the port %d is an image port \n", portdef.nPortIndex);
		fprintf(stderr, "the port mimetype is :%s\n", portdef.format.image.cMIMEType);
		break;
	}
	case OMX_PortDomainOther:
	{
		fprintf(stderr, "the port %d is the other port \n", portdef.nPortIndex);
		break;
	}
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	int i;
	char componentName[OMX_MAX_STRINGNAME_SIZE];
	int potIndex;

	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	OMX_HANDLETYPE handle;
	OMX_ERRORTYPE err;
	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;
	OMX_SendCommand();
}