/*
 * MIT License
 *
 * Copyright (c) 2018 Whuer_XiaoJie <1939346428@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef VIDEO_H_
#define VIDEO_H_

//coding
#include <libavcodec/avcodec.h>
//Encapsulation format processing
#include <libavformat/avformat.h>
//Pixel processing
#include <libswscale/swscale.h>

typedef struct VIDEO_DECODER_T_{
	/*video decoding attributes*/
	AVFormatContext *pFormatCtx;
	AVCodecContext  *pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame;
	AVFrame			*pFrameRGB;
	uint8_t			*out_buffer;
	int out_width;
	int out_height;
	struct SwsContext *sws_ctx;
	double           time_base;

	int video_time_s;
	int v_stream_idx;
	int delay_ms;
}VIDEO_DECODER_T;

int init_video_decoder(VIDEO_DECODER_T *decoder, char *fileName, int outWidth, int outHeight);

int decode_video_next_frame(VIDEO_DECODER_T *decoder, char *outBuf);

int destory_video_decoder(VIDEO_DECODER_T *decoder);

#endif // VIDEO_H_
