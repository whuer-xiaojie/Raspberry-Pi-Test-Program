/*
* MIT License
*
* Copyright (c) 2018 Whuer_XiaoJie
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

#include "mypng.h"

static inline int check_if_png(char *filename, FILE **fp)
{
	char buf[PNG_BYTES_TO_CHECK];

	if ((*fp = fopen(filename, "rb")) == NULL){
		fprintf(stderr,"%s():Open The Item File Failed:%s\n", __FUNCTION__,filename);
		return 0;
	}

	if (fread(buf, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
		return 0;

	return (!png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}

int decode_png_rgb888(char *filename, unsigned char **buf,int *width,int *height)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	FILE *fp = NULL;
	int bit_depth;
	int color_type;
	int w, h;//the source image width and height
	size_t line_bytes;
	unsigned char *dbuf = NULL;
	size_t src_line_bytes = 0;
	size_t src_size = 0;
	int x, y, offset;

	if (filename == NULL)
		return -1;

	if (!check_if_png(filename, &fp)){
		if (fp != NULL)
			fclose(fp);
		return -1;
	}

	if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL){
		fprintf(stderr, "%s():png_create_read_struct() Failed!\n", __FUNCTION__);
		if (fp != NULL)
			fclose(fp);
		return -1;
	}

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL){
		fprintf(stderr, "%s():png_create_info_struct() Failed!\n", __FUNCTION__);
		goto __outFailed;
	}

	if (setjmp(png_jmpbuf(png_ptr))){
		fprintf(stderr, "%s():Read PNG File Failed!\n",__FUNCTION__);
		goto __outFailed;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);


	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);

	*width = w; *height = h;

	color_type = png_get_color_type(png_ptr, info_ptr);

	if (color_type >= 4 && color_type <= 6)
		offset = 4;
	else
		offset = 3;

	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

	line_bytes = w*offset;
	src_line_bytes = w*3;
	src_size = src_line_bytes*h;

	if ((*buf = (unsigned char *)malloc(src_size + 1)) == NULL){
		fprintf(stderr, "%s():Not Enough Memory\n", __FUNCTION__);
		goto __outFailed;
	}

	dbuf = *buf;
	for (y = 0; y < h; y++)
		for (x = 0; x < line_bytes; x += offset){
			*dbuf++ = row_pointers[y][x];     //red
			*dbuf++ = row_pointers[y][x + 1]; //green
			*dbuf++ = row_pointers[y][x + 2]; //blue
		}

	if (info_ptr != NULL)
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (fp != NULL)
		fclose(fp);
	return 0;

__outFailed:
	if (info_ptr != NULL)
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (fp != NULL)
		fclose(fp);
	return -1;
}