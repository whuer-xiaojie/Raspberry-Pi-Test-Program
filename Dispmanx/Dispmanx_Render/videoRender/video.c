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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "video.h"

int init_video_decoder(VIDEO_DECODER_T *decoder, char *fileName, int outWidth, int outHeight)
{
	assert(decoder != NULL && fileName != NULL && outWidth > 0 && outHeight > 0);
	
	int result = 0;

	av_register_all();

	decoder->pFormatCtx = avformat_alloc_context();
	assert(decoder->pFormatCtx != NULL);

	result = avformat_open_input(&decoder->pFormatCtx, fileName, NULL, NULL);
	assert(result == 0);

	result = avformat_find_stream_info(decoder->pFormatCtx, NULL);
	assert(result >= 0);

	decoder->v_stream_idx = -1;
	int i = 0;
	for (; i < decoder->pFormatCtx->nb_streams; i++){
		if (decoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			decoder->v_stream_idx = i;
			break;
		}
	}
	assert(decoder->v_stream_idx != -1);

	decoder->pCodecCtx = decoder->pFormatCtx->streams[decoder->v_stream_idx]->codec;
	assert(decoder->pCodecCtx != NULL);

	decoder->pCodec = avcodec_find_decoder(decoder->pCodecCtx->codec_id);
	assert(decoder->pCodec != NULL);

	result = avcodec_open2(decoder->pCodecCtx, decoder->pCodec, NULL);
	assert(result >= 0);

	decoder->pFrame = av_frame_alloc();
	assert(decoder->pFrame != NULL);

	decoder->pFrameRGB = av_frame_alloc();
	assert(decoder->pFrameRGB != NULL);

	decoder->out_width = outWidth;
	decoder->out_height = outHeight;
	decoder->out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, outWidth, outHeight));
	assert(decoder->out_buffer != NULL);

	avpicture_fill((AVPicture*)decoder->pFrameRGB, decoder->out_buffer, AV_PIX_FMT_RGB24, outWidth, outHeight);

	decoder->sws_ctx = sws_getContext(decoder->pCodecCtx->width, decoder->pCodecCtx->height, decoder->pCodecCtx->pix_fmt,
		                              outWidth,outHeight,AV_PIX_FMT_RGB24,
		                              SWS_FAST_BILINEAR,NULL,NULL,NULL);
	assert(decoder->sws_ctx != NULL);

	AVStream *stream = decoder->pFormatCtx->streams[decoder->v_stream_idx];/* the video stream */
	decoder->video_time_s = (decoder->pFormatCtx->duration) / 1000000;//video time,second(s)
	decoder->time_base = av_q2d(stream->time_base);

	int frame_rate = stream->avg_frame_rate.num / stream->avg_frame_rate.den;
	decoder->delay_ms = 1000 / frame_rate;
}

int decode_video_next_frame(VIDEO_DECODER_T *decoder, char *outBuf)
{
	assert(decoder != NULL && outBuf != NULL);
	int got_picture;
	int result;
	AVPacket packet;

	av_init_packet(&packet);

	while (av_read_frame(decoder->pFormatCtx, &packet) >= 0){
		if (packet.stream_index == decoder->v_stream_idx){
			if ((result = avcodec_decode_video2(decoder->pCodecCtx, decoder->pFrame, &got_picture, &packet)) < 0){
				goto __outfinish;
			}
			if (got_picture){
				sws_scale(decoder->sws_ctx, (const uint8_t * const*)decoder->pFrame->data, decoder->pFrame->linesize, 0,
					      decoder->pCodecCtx->height, decoder->pFrameRGB->data, decoder->pFrameRGB->linesize);
				//the rgb color
				//memcpy(outBuf, decoder->pFrameRGB->data[0], decoder->out_width*decoder->out_height * 3);
				av_packet_unref(&packet);
				av_free_packet(&packet);
				return 0;
			}
		}
		av_packet_unref(&packet);
		av_free_packet(&packet);
	}

__outfinish:
	av_packet_unref(&packet);
	av_free_packet(&packet);
	destory_video_decoder(decoder);
	return -1;
}

int destory_video_decoder(VIDEO_DECODER_T *decoder)
{
	assert(decoder != NULL);
	if (decoder->sws_ctx){
		sws_freeContext(decoder->sws_ctx);
		decoder->sws_ctx = NULL;
	}
	if (decoder->out_buffer){
		//must do this after sws_freeContext() called ,not do this way may cause mempry leak
		av_free(decoder->out_buffer);
		decoder->out_buffer = NULL;
	}
	if (decoder->pFrameRGB){
		av_frame_free(&decoder->pFrameRGB);
		decoder->pFrameRGB = NULL;
	}
	if (decoder->pFrame){
		av_frame_free(&decoder->pFrame);
		decoder->pFrame = NULL;
	}
	if (decoder->pCodecCtx){
		avcodec_close(decoder->pCodecCtx);
		decoder->pCodecCtx = NULL;
		decoder->pCodec = NULL;
	}
	if (decoder->pFormatCtx){
		avformat_close_input(&decoder->pFormatCtx);
		avformat_free_context(decoder->pFormatCtx);
		decoder->pFormatCtx = NULL;
	}
}