#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"
#include "ilclient.h"

#include <IL/OMX_Core.h>

static  COMPONENT_T *image_encode = NULL;
static  char* filename;
int width, height;

void  donecallback(void *data, COMPONENT_T *comp)
{
	OMX_ERRORTYPE result;
	FILE *outf;
	OMX_BUFFERHEADERTYPE *out;
	out = ilclient_get_output_buffer(image_encode, 341, 1);
	if (out->nFilledLen == 0)
	{
		printf("\nSOmthing is wrong in encoder"); fflush(stdout);
		return;
	}
	outf = fopen(filename, "w");
	if (outf == NULL)
	{
		printf("Failed to open '%s' for writing image\n", filename);
		return;
	}
	if (out != NULL)
	{
		printf("%d bytes are ready to be put in JPG file ", out->nFilledLen); fflush(stdout);
		result = fwrite(out->pBuffer, 1, out->nFilledLen, outf);
		if (result != out->nFilledLen)
		{
			printf(" \n%x Error emptying buffer to file", result);
		}
		else
		{
			printf("Writing frame  %d bytes\n", out->nFilledLen);
		}
		out->nFilledLen = 0;
	}
	else
	{
		printf(" \n No Buffer on  Image Encoding Output Port");
	}
	fclose(outf);
	return;
}

static int image_encoder(char *outputfilename, OMX_U8* data, int len)
{
	OMX_IMAGE_PARAM_PORTFORMATTYPE format;
	OMX_PARAM_PORTDEFINITIONTYPE def;
	COMPONENT_T *list[5];
	OMX_BUFFERHEADERTYPE *buf;
	OMX_BUFFERHEADERTYPE *out;
	OMX_ERRORTYPE result;
	ILCLIENT_T *client;
	int status = 0;
	filename = outputfilename;
	bcm_host_init();
	memset(list, 0, sizeof(list));
	if ((client = ilclient_init()) == NULL) {
		return -3;
	}
	if (OMX_Init() != OMX_ErrorNone) {
		ilclient_destroy(client);
		return -4;
	}
	ilclient_set_fill_buffer_done_callback(client, donecallback, NULL);
	// create image_encode
	result = ilclient_create_component(client, &image_encode, "image_encode",
		ILCLIENT_DISABLE_ALL_PORTS |
		ILCLIENT_ENABLE_INPUT_BUFFERS |
		ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (result != 0)
	{
		printf("ilclient_create_component() for image_encode failed with %x!\n", result);
		return -1;
	}
	list[0] = image_encode;
	// get current settings of image_encode component from port 340
	memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	def.nVersion.nVersion = OMX_VERSION;
	def.nPortIndex = 340;
	if (OMX_GetParameter(ILC_GET_HANDLE(image_encode), OMX_IndexParamPortDefinition, &def) != OMX_ErrorNone)
	{
		printf("%s:%d : OMX_GetParameter() for image_encode port 340 failed!\n", __FUNCTION__, __LINE__);
		exit(1);
	}
	def.format.image.nFrameWidth = width;
	def.format.image.nFrameHeight = height;
	def.format.image.eCompressionFormat = OMX_COLOR_FormatUnused;
	def.format.image.nSliceHeight = def.format.image.nFrameHeight;
	def.format.image.nStride = def.format.image.nFrameWidth;
	def.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
	result = OMX_SetParameter(ILC_GET_HANDLE(image_encode),
		OMX_IndexParamPortDefinition, &def);
	if (result != OMX_ErrorNone)
	{
		printf("%s:%d : OMX_SetParameter() for image_encode port 340 failed with %x!\n", __FUNCTION__, __LINE__, result);
		exit(1);
	}
	memset(&format, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 341;
	format.eCompressionFormat = OMX_IMAGE_CodingJPEG;
	printf("OMX_SetParameter for image_encode:341¡­\n");
	result = OMX_SetParameter(ILC_GET_HANDLE(image_encode),
		OMX_IndexParamImagePortFormat, &format);
	if (result != OMX_ErrorNone)
	{
		printf("%s:%d : OMX_SetParameter() for image_encode port 341 failed with %x!\n", __FUNCTION__, __LINE__, result);
		exit(1);
	}
	printf("encode to idle¡­\n");
	if (ilclient_change_component_state(image_encode, OMX_StateIdle) == -1)
	{
		printf("%s:%d : ilclient_change_component_state(image_encode, OMX_StateIdle) failed", __FUNCTION__, __LINE__);
	}
	printf("enabling port buffers for 340¡­\n");
	if (ilclient_enable_port_buffers(image_encode, 340, NULL, NULL, NULL) != 0)
	{
		printf("enabling port buffers for 340 failed!\n");
		exit(1);
	}
	printf("enabling port buffers for 341¡­\n");
	if (ilclient_enable_port_buffers(image_encode, 341, NULL, NULL, NULL) != 0)
	{
		printf("enabling port buffers for 341 failed!\n");
		exit(1);
	}
	printf("encode to executing¡­\n");
	ilclient_change_component_state(image_encode, OMX_StateExecuting);
	printf("looping for buffers¡­\n");
	do {
		out = ilclient_get_output_buffer(image_encode, 341, 1);
		result = OMX_FillThisBuffer(ILC_GET_HANDLE(image_encode), out);
		if (result != OMX_ErrorNone)
		{
			printf("Error filling buffer : %x\n", result);
		}
		buf = ilclient_get_input_buffer(image_encode, 340, 1);
		if (buf == NULL)
		{
			printf("Doh, no buffers for me!\n");
		}
		else
		{
			printf("filling %d bytes\n", len);
			memcpy(buf->pBuffer, data, len);
			buf->nFilledLen = len;
			if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(image_encode), buf) !=
				OMX_ErrorNone) {
				printf("Error emptying buffer!\n");
			}
		}
	} while (0);
	//EOS management
	buf->nFilledLen = 0;
	buf->nFlags = OMX_BUFFERFLAG_EOS;
	//OMX_FillThisBuffer(ILC_GET_HANDLE(image_encode), out);
	OMX_EmptyThisBuffer(ILC_GET_HANDLE(image_encode), buf);
	// wait for EOS from encoder
	ilclient_wait_for_event(image_encode, OMX_EventBufferFlag, 341, 0, OMX_BUFFERFLAG_EOS, 0,
		ILCLIENT_BUFFER_FLAG_EOS, 10000);
	printf("disabling port buffers for 340 and 341¡­\n");
	//ilclient_disable_port_buffers(image_encode, 340, NULL, NULL, NULL);
	//ilclient_disable_port_buffers(image_encode, 341, NULL, NULL, NULL);
	//ilclient_state_transition(list, OMX_StateIdle);
	//ilclient_state_transition(list, OMX_StateLoaded);
	printf("\n Encoding Complete, check JPG at input path"); fflush(stdout);
	ilclient_cleanup_components(list);
	OMX_Deinit();
	ilclient_destroy(client);
	return status;
}

static int video_decode_resize_encode(char *filename, char* outputfilename, int resize_width, int resize_height,
	int frequency)
{
	OMX_VIDEO_PARAM_PORTFORMATTYPE format;
	COMPONENT_T *video_decode = NULL, *resize = NULL;
	COMPONENT_T *list[3];
	TUNNEL_T tunnel[2];
	ILCLIENT_T *client;
	int eos_flag = 0;
	FILE *in;
	int status = 0;
	unsigned int data_length = 0;
	int packet_size = 80 << 10;
	int resize_port_active = 0;
	int frame_number = 0;
	width = resize_width;
	height = resize_height;
	memset(list, 0, sizeof(list));
	memset(tunnel, 0, sizeof(tunnel));
	if ((in = fopen(filename, "rb")) == NULL)
		return -1;
	if ((client = ilclient_init()) == NULL)
	{
		fclose(in);
		return -2;
	}
	if (OMX_Init() != OMX_ErrorNone)
	{
		ilclient_destroy(client);
		fclose(in);
		return -3;
	}
	// create video_decode
	if (ilclient_create_component(client, &video_decode, "video_decode", 
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
		status = -4;
	list[0] = video_decode;
	// create resize
	if (status == 0 && ilclient_create_component(client, &resize, "resize", 
		ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS) != 0)
		status = -5;
	list[1] = resize;
	set_tunnel(tunnel, video_decode, 131, resize, 60);;
	if (status == 0)
		ilclient_change_component_state(video_decode, OMX_StateIdle);
	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 130;
	format.eCompressionFormat = OMX_VIDEO_CodingAVC;
	if (status == 0 &&
		OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) 
		== OMX_ErrorNone &&ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
	{
		OMX_BUFFERHEADERTYPE *buf, *outbuf;
		int port_settings_changed = 0;
		int first_packet = 1;
		ilclient_change_component_state(video_decode, OMX_StateExecuting);
		while ((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
		{
			// feed data and wait until we get port settings changed
			unsigned char *dest = buf->pBuffer;
			OMX_PARAM_PORTDEFINITIONTYPE portdef;
			data_length += fread(dest, 1, packet_size - data_length, in);
			printf("\n Reading %d bytes from  file ", data_length); fflush(stdout);
			if (port_settings_changed == 0 &&
				((data_length > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged,
				131, 0, 0, 1) == 0) ||(data_length == 0 && ilclient_wait_for_event(video_decode, 
				OMX_EventPortSettingsChanged, 131, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED,
				10000) == 0)))
			{
				port_settings_changed = 1;
				// need to setup the input for the resizer with the output of the decoder
				portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
				portdef.nVersion.nVersion = OMX_VERSION;
				portdef.nPortIndex = 131;
				OMX_GetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamPortDefinition, &portdef);
				// tell resizer input what the decoder output will be providing
				portdef.nPortIndex = 60;
				OMX_SetParameter(ILC_GET_HANDLE(resize), OMX_IndexParamPortDefinition, &portdef);
				// need to setup the input for the resizer with the output of the decoder
				portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
				portdef.nVersion.nVersion = OMX_VERSION;
				portdef.nPortIndex = 61;
				OMX_GetParameter(ILC_GET_HANDLE(resize), OMX_IndexParamPortDefinition, &portdef);
				// change output color format and dimensions to match input
				portdef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
				portdef.format.image.eColorFormat = OMX_COLOR_Format32bitARGB8888;
				portdef.format.image.nFrameWidth = width;
				portdef.format.image.nFrameHeight = height;
				portdef.format.image.nStride = 0;
				portdef.format.image.nSliceHeight = 0;
				portdef.format.image.bFlagErrorConcealment = OMX_FALSE;
				OMX_SetParameter(ILC_GET_HANDLE(resize), OMX_IndexParamPortDefinition, &portdef);
				// establish tunnel between video output and resizer input
				if (ilclient_setup_tunnel(tunnel, 0, 0) != 0)
				{
					status = -12;
					break;
				}
				// enable output of scheduler and input of resizer (ie enable tunnel)
				OMX_SendCommand(ILC_GET_HANDLE(video_decode), OMX_CommandPortEnable, 131, NULL);
				OMX_SendCommand(ILC_GET_HANDLE(resize), OMX_CommandPortEnable, 60, NULL);
				// put resizer in idle state (this allows the outport of the decoder to become enabled)
				ilclient_change_component_state(resize, OMX_StateExecuting);
				// once the state changes, both ports should become enabled and the resizer output 
				//should generate a settings changed event
				if ((ilclient_remove_event(resize, OMX_EventPortSettingsChanged, 61, 0, 0, 1) == 0))
				{
					printf("\n Enabling Resizer output ");
					portdef.nPortIndex = 61;
					OMX_GetParameter(ILC_GET_HANDLE(resize), OMX_IndexParamPortDefinition, &portdef);
					if (ilclient_enable_port_buffers(resize, 61, NULL, NULL, NULL) != 0)
					{
						status = -13;
						break;
					}
					ilclient_change_component_state(resize, OMX_StateExecuting);
					resize_port_active = 1;
				}
			}
			//nothing left to read
			if (!data_length)
			{
				//add eos if already not added
				if (!eos_flag)
				{
					printf("\n adding EOS on input"); fflush(stdout);
					buf->nFilledLen = 0;
					buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;
					if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
						status = -20;
					eos_flag = 1;
				}
			}
			//output is ready
			if (resize_port_active)
			{
				outbuf = NULL;
				outbuf = ilclient_get_output_buffer(resize, 61, 1);
				if (outbuf != NULL && (outbuf->nFilledLen != 0))
				{
					portdef.nPortIndex = 61;
					OMX_GetParameter(ILC_GET_HANDLE(resize), OMX_IndexParamPortDefinition, &portdef);
					frame_number++;
					//need to get fps from decode data to write logic,just to check show first and then 1 fps
					if ((frame_number == 1) || (frame_number % 25 == 0))
					{
						printf("\n need to convert %d bytes RGB to JPG  \n", outbuf->nFilledLen); fflush(stdout);
						image_encoder(outputfilename, outbuf->pBuffer, outbuf->nFilledLen);
					}
					else
					{
						printf("\n need to skip this"); fflush(stdout);
					}
					//anyhow free the memory
					OMX_FreeBuffer(ILC_GET_HANDLE(resize), 61, outbuf);
					outbuf->nFilledLen = 0;
				}
				printf("\n filling buffer with %d bytes", outbuf->nFilledLen); fflush(stdout);
				OMX_FillThisBuffer(ILC_GET_HANDLE(resize), outbuf);
				printf("\n  buffer filled"); fflush(stdout);
			}
			//resizer sees EOS in the stream on its output port ie work is done for this stream
			if ((ilclient_remove_event(resize, OMX_EventBufferFlag, 61, 0, OMX_BUFFERFLAG_EOS, 0) == 0))
			{
				//  flush to allow video_decode to disable its input port
				printf("\n  received EOS on resizer output\n"); fflush(stdout);
				break;
			}
			buf->nFilledLen = data_length;
			data_length = 0;
			buf->nOffset = 0;
			if (first_packet)
			{
				buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
				first_packet = 0;
			}
			else
				buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
			printf("\n emptying buffer %d bytes", buf->nFilledLen); fflush(stdout);
			if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
			{
				status = -14;
				break;
			}
			printf("\n  buffer emptied"); fflush(stdout);
		}
	}
	fclose(in);
	ilclient_flush_tunnels(tunnel, 0);
	//ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
	ilclient_disable_tunnel(tunnel);
	ilclient_teardown_tunnels(tunnel);
	ilclient_state_transition(list, OMX_StateIdle);
	ilclient_state_transition(list, OMX_StateLoaded);
	ilclient_cleanup_components(list);
	OMX_Deinit();
	ilclient_destroy(client);
	return status;
}

int main(int argc, char **argv)
{
	bcm_host_init();
	return video_decode_resize_encode(argv[1], argv[2], argv[3], argv[4], argv[5]);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bcm_host.h"
#include "ilclient.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mathematics.h"
#include <error.h>

#define WIDTH     1920//736//640//320//1280//
#define HEIGHT    1080//1088//400//368//((WIDTH*9)/16)//720//
#define PITCH     ((WIDTH+31)&~31)
#define HEIGHT16  ((HEIGHT+15)&~15)
#define SIZE      ((WIDTH * HEIGHT16 * 3)/2)//(WIDTH*HEIGHT*12/8)//

//http://fossies.org/dox/xbmc-12.0/DVDClock_8h_source.html
#define DVD_NOPTS_VALUE (-1LL<<52) // should be possible to represent in both double and int64_t

static inline OMX_TICKS ToOMXTime(int64_t pts)
{
	OMX_TICKS ticks;
	ticks.nLowPart = pts;
	ticks.nHighPart = pts >> 32;
	return ticks;
}

static AVBitStreamFilterContext *dofiltertest(AVPacket *rp)
{
	AVBitStreamFilterContext *bsfc;
	bsfc = NULL;

	if (!(rp->data[0] == 0x00 && rp->data[1] == 0x00 &&
		rp->data[2] == 0x00 && rp->data[3] == 0x01)) {
		bsfc = av_bitstream_filter_init("h264_mp4toannexb");
		if (!bsfc) {
			printf("Failed to open filter.  This is bad.\n");
		}
		else {
			printf("Have a filter at %p\n", bsfc);
		}
	}
	else
		printf("No need for a filter.\n");

	return bsfc;
}

static AVPacket filter(AVBitStreamFilterContext *bsfc, AVFormatContext *ctx, AVPacket *rp, int *video_stream_index)
{
	if (ctx == NULL){ printf("!!! ctx is NULL, filed set filter!\n"); exit(-1); }
	//AVBitStreamFilterContext *bsfc=dofiltertest(rp);
	AVPacket *p;
	AVPacket *fp;
	int rc;
	if (bsfc){
		fp = calloc(sizeof(AVPacket), 1);

		if (bsfc) {
			rc = av_bitstream_filter_filter(bsfc,
				ctx->streams[*video_stream_index]->codec,
				NULL, &(fp->data), &(fp->size),
				rp->data, rp->size,
				rp->flags & AV_PKT_FLAG_KEY);
			if (rc > 0) {
				av_free_packet(rp);
				fp->destruct = av_destruct_packet;
				p = fp;
			}
			else {
				printf("Failed to filter frame: "
					"%d (%x)\n", rc, rc);
				p = rp;
			}
		}
		else
			p = rp;

		return *p;
	}//if
	else{ return *rp; }
}

static int getInputHendle(char *filename, AVFormatContext *pFmtCtx, AVCodecContext pCodCtx, int *video_stream_index){
	printf("Start getInputHandle %s\n", filename);
	AVFormatContext *pFormatCtx;
	// Open video file
	if (av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL) != 0){ printf("Couldn't open file\n"); return -1; }
	// Retrieve stream information
	if (av_find_stream_info(pFormatCtx) < 0){ printf("Couldn't find stream information\n"); return -1; }
	// print file file info
	dump_format(pFormatCtx, 0, filename, false);
	//memcpy(pFmtCtx,pFormatCtx,sizeof(pFormatCtx));
	//find video stream
	int i;
	*video_stream_index = -1;
	AVCodecContext *pCodecCtx;
	// Find the first video stream
	for (i = 0; i < pFormatCtx->nb_streams; i++){ if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){ *video_stream_index = i; break; } }
	if (*video_stream_index == -1){ printf("Didn't find a video stream\n"); return -1; }
	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[*video_stream_index]->codec;

	printf("\n=>My info about video: \n Bitrate: %d\n Frame width: %d\n Frame height: %d\n Frame rate: %d:%d\n Aspect ratio: %d:%d\n ", pCodecCtx->bit_rate, pCodecCtx->width, pCodecCtx->height, pCodecCtx->time_base.num, pCodecCtx->time_base.den, pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

	FILE *outFile = fopen("encoded.h264", "w");

	OMX_ERRORTYPE r;
	OMX_VIDEO_PARAM_PORTFORMATTYPE in_dec_format, out_enc_format, out_dec_Format;
	OMX_PARAM_PORTDEFINITIONTYPE in_enc_format;
	COMPONENT_T *video_decode = NULL, *video_encode = NULL, *video_resize = NULL, *video_render = NULL;
	COMPONENT_T *list[5];
	TUNNEL_T tunnel[4];
	ILCLIENT_T *client;
	int status = 0;

	memset(list, 0, sizeof(list));
	memset(tunnel, 0, sizeof(tunnel));

	if ((client = ilclient_init()) == NULL){ printf("=>Can't init ilclient_init()!\n"); exit(-1); }
	if (OMX_Init() != OMX_ErrorNone){ ilclient_destroy(client); printf("=>Can't init omx!\n"); exit(-1); }

	// create video_decode
	if (ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
		status = -14;
	list[0] = video_decode;

	// create resize
	if (ilclient_create_component(client, &video_resize, "resize", ILCLIENT_DISABLE_ALL_PORTS) != 0){ status = -14; printf("=> ilclient_create_component() for video_resize failed!!\n"); exit(1); }
	list[1] = video_resize;

	// create video_encode
	if (ilclient_create_component(client, &video_encode, "video_encode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS) != 0){ status = -14; printf("=> ilclient_create_component() for video_encode failed!!\n"); exit(1); }
	list[2] = video_encode;

	// create video_render
	if (status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
		status = -14;
	list[3] = video_render;


	//   set_tunnel(tunnel, video_decode, 131, video_render, 90);
	set_tunnel(tunnel, video_decode, 131, video_encode, 200);


	if (status == 0){
		ilclient_change_component_state(video_decode, OMX_StateIdle);
		ilclient_change_component_state(video_encode, OMX_StateIdle);
	}//if status

	//input dec format
	memset(&in_dec_format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	in_dec_format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	in_dec_format.nVersion.nVersion = OMX_VERSION;
	in_dec_format.nPortIndex = 130;
	in_dec_format.eCompressionFormat = OMX_VIDEO_CodingAVC;
	r = OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &in_dec_format);
	if (r != OMX_ErrorNone){ printf("%s:%d: OMX_SetParameter() for video_decode port 130 failed with 0x%08x!\n", __FUNCTION__, __LINE__, r); exit(-1); }


	//output dec format
	memset(&out_dec_Format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	out_dec_Format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	out_dec_Format.nVersion.nVersion = OMX_VERSION;
	out_dec_Format.nPortIndex = 131;
	// out_dec_Format.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	//if (OMX_GetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamPortDefinition, &out_dec_Format) != OMX_ErrorNone) {printf("%s:%d: OMX_GetParameter() for video_encode port 131 failed!\n", __FUNCTION__, __LINE__);status=-1;exit(-1);}

	if (OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &out_dec_Format) != OMX_ErrorNone){ printf("=> Failed set  output port formats of buffers for port (video_decode) : 131 \n"); status = -1; exit(-1); }

	//   //input video_encode
	memset(&in_enc_format, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	in_enc_format.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	in_enc_format.nVersion.nVersion = OMX_VERSION;
	in_enc_format.nPortIndex = 200;
	if (OMX_GetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamPortDefinition, &in_enc_format) != OMX_ErrorNone) { printf("%s:%d: OMX_GetParameter() for video_encode port 200 failed!\n", __FUNCTION__, __LINE__); exit(1); }
	// Port 200: in 1/1 115200 16 enabled,not pop.,not cont. 320x240 320x240 @1966080 20
	in_enc_format.format.video.nFrameWidth = WIDTH;
	in_enc_format.format.video.nFrameHeight = HEIGHT;
	in_enc_format.format.video.xFramerate = 30 << 16;
	in_enc_format.format.video.nSliceHeight = in_enc_format.format.video.nFrameHeight;
	in_enc_format.format.video.nStride = in_enc_format.format.video.nFrameWidth;
	in_enc_format.format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
	if ((r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamPortDefinition, &in_enc_format)) != OMX_ErrorNone){ printf("%s:%d: OMX_SetParameter() for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__, r); exit(1); }

	//output video_encode
	memset(&out_enc_format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	out_enc_format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	out_enc_format.nVersion.nVersion = OMX_VERSION;
	out_enc_format.nPortIndex = 201;
	out_enc_format.eCompressionFormat = OMX_VIDEO_CodingAVC;
	if ((r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoPortFormat, &out_enc_format)) != OMX_ErrorNone){ printf("%s:%d: OMX_SetParameter() for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__, r); exit(1); }


	if (status != 0 &&
		ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) != 0 &&
		ilclient_enable_port_buffers(video_encode, 201, NULL, NULL, NULL) != 0){
		printf("Failed ilclient_enable_port_buffers\n"); exit(-1);
	}

	OMX_BUFFERHEADERTYPE *buf, *out;


	AVPacket *p_packet;
	AVBitStreamFilterContext *p_btsf;
	p_packet = malloc(sizeof(AVPacket));
	//int video_index=0;
	int index = 0;
	int pstatus = 0;
	int first_frame = true;

	//int t_error=ilclient_setup_tunnel(tunnel, 0, 0);
	//if( t_error!= 0){status = -7;printf("=>Can't setup tunnel, error: %d!!!\n",t_error);exit(status);}




	ilclient_change_component_state(video_decode, OMX_StateExecuting);
	//ilclient_change_component_state(video_render, OMX_StateExecuting);
	ilclient_change_component_state(video_encode, OMX_StateExecuting);



	//int result = m_pSubtitleCodec->Decode(pkt->data, pkt->size, pkt->pts, pkt->duration);




	do{
		printf("###before DO!\n");

		pstatus = av_read_frame(pFormatCtx, p_packet);

		if (first_frame == true){
			p_btsf = dofiltertest(p_packet);
			first_frame = false;
		}//if

		AVPacket packet = filter(p_btsf, pFormatCtx, p_packet, video_stream_index);

		//only for video
		if (packet.stream_index == *video_stream_index){
			printf("=>Read frame, status: %d, index: %d, stream index: %d, packet duration: %d, size: %d\n", pstatus, index++, packet.stream_index, packet.duration, packet.size);

			int psize = packet.size;
			int preaded = 0;
			//double pts=packet.duration;

			while (psize != 0){
				printf("while\n");
				buf = ilclient_get_input_buffer(video_decode, 130, 1);
				buf->nFilledLen = (psize > buf->nAllocLen) ? buf->nAllocLen : psize;
				memcpy(buf->pBuffer, packet.data + preaded, buf->nFilledLen);
				psize -= buf->nFilledLen;
				preaded += buf->nFilledLen;

				if (psize == 0){ buf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME; printf("#######################################OMX_BUFFERFLAG_ENDOFFRAME\n"); }

				printf("=>BUFF size: %d\n", (int)buf->nFilledLen);
				r = OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf);
				if (pstatus == 0){ if (r != OMX_ErrorNone){ status = -6; printf("Failed, OMX_EmptyThisBuffer, error: 0x%08x , buf allocate: %d, buf lenght: %d \n", r, (int)buf->nAllocLen, (int)buf->nFilledLen); break; } }



			}//while psize
			av_free_packet(&packet);



			//################################################################### encode
			//if(psize == 0){
			printf("###Start encoder..\n");

			out = ilclient_get_output_buffer(video_encode, 201, 1);

			r = OMX_FillThisBuffer(ILC_GET_HANDLE(video_encode), out);
			if (r != OMX_ErrorNone) { printf("### Error filling buffer: %x\n", r); }

			printf("############## Encoded buf, size: %d\n", (int)out->nFilledLen);

			if (out != NULL) {
				if (out->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
					int i;
					for (i = 0; i < out->nFilledLen; i++)
						printf("%x ", out->pBuffer[i]);
					printf("\n");
				}

				size_t len = fwrite(out->pBuffer, 1, out->nFilledLen, outFile);
				if (len != out->nFilledLen) { printf("### fwrite: Error emptying encoder buffer: %d!\n", r); }
				else { printf("### Writing encoded frame, size: %d\n", (int)len); }
				out->nFilledLen = 0;
			}//out!=NULL
			else {
				printf("### Not getting encode buf !\n");
			}
			//}//if psize



		}//if index
	}//do
	while (pstatus == 0);

	printf("=>After while! \n");












	// wait for EOS from render
	/*      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,*/
	/*                              ILCLIENT_BUFFER_FLAG_EOS, 10);*/

	// need to flush the renderer to allow video_decode to disable its input port
	ilclient_flush_tunnels(tunnel, 0);

	ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);




	ilclient_disable_tunnel(tunnel);
	ilclient_teardown_tunnels(tunnel);

	ilclient_state_transition(list, OMX_StateIdle);
	ilclient_state_transition(list, OMX_StateLoaded);

	ilclient_cleanup_components(list);

	OMX_Deinit();

	ilclient_destroy(client);
	return status;
}



int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	av_register_all();//  initialize libavformat/libavcodec: 
	bcm_host_init();
	AVFormatContext *in_formatCtx;
	AVCodecContext in_codecCtx;
	//AVPacket *packet;
	int video_stream_index;
	//get input codec params object
	if (getInputHendle(argv[1], in_formatCtx, in_codecCtx, &video_stream_index) == -1){ exit(-1); }
	if (&in_formatCtx == NULL){ printf("before formatCtx is NULL! \n"); }

	//printf("DATAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: H= %d",in_formatCtx.streams[video_stream_index]->codec->height);

	//video_decode_test(&in_formatCtx,video_stream_index);


	return 0;
}