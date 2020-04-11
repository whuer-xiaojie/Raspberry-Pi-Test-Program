#include <stdio.h>
#include <stdlib.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>

#include <bcm_host.h>

OMX_CALLBACKTYPE callbacks = {
	.EventHandler = NULL,
	.EmptyBufferDone = NULL,
	.FillBufferDone = NULL
};
OMX_ERRORTYPE get_port_info(OMX_HANDLETYPE handle,OMX_PARAM_PORTDEFINITIONTYPE *portdef)
{
	OMX_ERRORTYPE err= OMX_GetParameter(handle, OMX_IndexParamPortDefinition, portdef);
	fprintf(stderr, "%s():err=%d \n", __FUNCTION__, err);
	return err;
}
void print_port_info(OMX_PARAM_PORTDEFINITIONTYPE *portdef)
{
	char *Domain;
	fprintf(stderr, "port index is:%d\n", portdef->nPortIndex);
	if (portdef->eDir == OMX_DirInput)
		fprintf(stderr, "the port is an input port\n", 0);
	else
		fprintf(stderr, "the port is an output port\n", 0);
	switch (portdef->eDomain)
	{
	case OMX_PortDomainAudio:
		Domain = "Audio";
		break;
	case OMX_PortDomainVideo:
		Domain = "Video";
		break;
	case OMX_PortDomainImage:
		Domain = "Image";
		break;
	case OMX_PortDomainOther:
		Domain = "Other";
		break;
	default:
		Domain = NULL;
		break;
	}
	fprintf(stderr, "Domain is :%s\n", Domain);

	fprintf(stderr, "the actual buffer count is: %d\n", portdef->nBufferCountActual);
	fprintf(stderr, "the minimum buffer count is:%d\n", portdef->nBufferCountMin);
	fprintf(stderr, "the buffer count in bytes is:%d\n", portdef->nBufferSize);
}

int main(int argc, char** argv) 
{
	int i;
	char componentName[128]; // min space required see /opt/vc/include/IL/OMX_Core.h
	OMX_ERRORTYPE err;
	OMX_HANDLETYPE handle;
	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	OMX_VERSIONTYPE specVersion, compVersion;
	OMX_UUIDTYPE uid;
	int portindex;
	if (argc < 3) 
	{
		fprintf(stderr, "Usage: %s component-name port-index\n", argv[0]);
		exit(1);
	}
	strncpy(componentName, argv[1], 128);
	portindex = atoi(argv[2]);
	fprintf(stderr, "%s():name:%s  portindex:%d \n", __FUNCTION__, componentName, portindex);
	bcm_host_init();
	err = OMX_Init();
	if (err != OMX_ErrorNone) 
	{
		fprintf(stderr, "OMX_Init() failed\n", 0);
		exit(1);
	}
	/** Ask the core for a handle to the component
	*/
	err = OMX_GetHandle(&handle, componentName,NULL, &callbacks);
	if (err != OMX_ErrorNone)
	{
		fprintf(stderr, "OMX_GetHandle failed\n", 0);
		exit(1);
	}
	// Get some version info
	err = OMX_GetComponentVersion(handle, componentName,&compVersion, &specVersion,&uid);
	if (err != OMX_ErrorNone) 
	{
		fprintf(stderr, "OMX_GetComponentVersion failed\n", 0);
		exit(1);
	}
	printf("Component name: %s \n version %d.%d \n Spec version %d.%d\n",
		componentName, compVersion.s.nVersionMajor,
		compVersion.s.nVersionMinor,
		specVersion.s.nVersionMajor,
		specVersion.s.nVersionMinor);
	memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = portindex;
	if (get_port_info(handle, &portdef) == OMX_ErrorNone) {
		print_port_info(&portdef);
	}
	exit(0);
}