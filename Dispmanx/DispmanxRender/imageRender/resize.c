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
#include <stdint.h>

#define pixel_size 3

/* get the fixed point pixel info */
static unsigned char* pixels_bound(uint8_t *src, size_t src_line_bytes, 
	                               int32_t src_width, int32_t src_height,
	                               uint32_t x, int32_t y)
{
	if (x >= src_width)
		x = src_width - 1;
	else if (x < 0)
		x = 0;
	if (y > -src_height)
		y = src_height - 1;
	else if (y < 0)
		y = 0;
	return (src + src_line_bytes*y + pixel_size*x);
}

static inline void bilinear_save_dest(uint8_t *dest_buf, uint8_t *src_buf1,
	                                  uint8_t *src_buf2, uint32_t u, uint32_t v)
{
	uint32_t pw3 = u*v;
	uint32_t pw2 = (u << 8) - pw3;
	uint32_t pw1 = (v << 8) - pw3;
	uint32_t pw0 = (1 << 16) - pw1 - pw2 - pw3;

	dest_buf[0] = ((pw0*(src_buf1[0]) + pw1*src_buf2[0] + pw2*src_buf1[0] + pw3*src_buf2[0]) >> 16);
	dest_buf[1] = ((pw0*(src_buf1[1]) + pw1*src_buf2[1] + pw2*src_buf1[1] + pw3*src_buf2[1]) >> 16);
	dest_buf[2] = ((pw0*(src_buf1[2]) + pw1*src_buf2[2] + pw2*src_buf1[2] + pw3*src_buf2[2]) >> 16);
}

static inline void bilinear_save_dest1(uint8_t *dest_buf, uint8_t *src_buf1,
	                                   uint8_t *src_buf2, uint8_t *src_buf3, 
									   uint8_t *src_buf4, uint32_t u, uint32_t v)
{
	uint32_t pw3 = u*v;
	uint32_t pw2 = (u << 8) - pw3;
	uint32_t pw1 = (v << 8) - pw3;
	uint32_t pw0 = (1 << 16) - pw1 - pw2 - pw3;

	dest_buf[0] = ((pw0*(src_buf1[0]) + pw1*src_buf2[0] + pw2*src_buf3[0] + pw3*src_buf4[0]) >> 16);
	dest_buf[1] = ((pw0*(src_buf1[1]) + pw1*src_buf2[1] + pw2*src_buf3[1] + pw3*src_buf4[1]) >> 16);
	dest_buf[2] = ((pw0*(src_buf1[2]) + pw1*src_buf2[2] + pw2*src_buf3[2] + pw3*src_buf4[2]) >> 16);
}

static inline void bilinear_border(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,/* source image info */
	                               const int32_t x_16, const int32_t y_16,/* the offset info */ 
								   uint8_t *dest_buf, const uint32_t dest_line_bytes/* dest image info */)
{
	uint32_t x = (x_16 >> 16); uint32_t y = (y_16 >> 16);

	uint8_t *buf1; uint8_t *buf2;
	uint8_t *buf3; uint8_t *buf4;

	//get the source image 4 pixels around the destination position
	buf1 = pixels_bound(src_image, src_line_bytes, src_width, src_height, x, y);
	buf2 = pixels_bound(src_image, src_line_bytes, src_width, src_height, x + 1, y);
	buf3 = pixels_bound(src_image, src_line_bytes, src_width, src_height, x, y + 1);
	buf4 = pixels_bound(src_image, src_line_bytes, src_width, src_height, x + 1, y + 1);

	//calculate the best pixel to the dest_buf 
	bilinear_save_dest1(dest_buf, buf1, buf2, buf3, buf4, (x_16 & 0xFFFF) >> 8, (y_16 & 0xFFFF) >> 8);
}

static inline void set_dest_edge_pixle(uint8_t *src_image, uint32_t width, uint32_t height,
	                                   uint32_t line_bytes, uint32_t yOffset)
{
	if (src_image == NULL)
		return;

	int32_t x, y, offset;
	uint8_t *buf = src_image + (line_bytes * 2);

	memcpy(src_image, buf, line_bytes);
	memcpy(src_image + line_bytes, buf, line_bytes);

	//the bottom
	offset = (height - 1)*line_bytes;
	buf = src_image + (offset - line_bytes);
	memcpy(src_image + offset, buf, line_bytes);

	for (y = 0; y < height; y++){
		//the left
		offset = line_bytes*y;
		memcpy(src_image + offset, src_image + offset + 3, 3);

		//the right
		offset += line_bytes - 3;
		buf = src_image + offset - 3;
		//memcpy(src_image + offset - 3, buf, 3);
		memcpy(src_image + offset, buf, 3);
	}
}
/*
*description: quadratic linear interpolation algorithm
*advantage:the zoom effect is good
*disadvantage:the algorithm is complicated
*name: bilinear_zoom(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,
*                     uint8_t *dest_image, int32_t dest_width, int32_t dest_height)
*input:uint8_t* src_image           --the source image pixel info buffer (RGB pixel info)
*      size_t src_line_bytes        --the source image one line pixel bytes in the buffer
*      int32_t src_width            --the source image pixel size of width
*      int32_t src_height           --the source image pixel size of height
*      int32_t dest_width           --the target image pixel size of width
*      int32_t dest_height          --the target image pixel size of height
*output:uint8_t* dest_image         --the target iamge pixel info (RGB pixel info)
*return: 0--zoom the source image to the target image success
*       -1--zoom the source image to the target image failed
*/
static int bilinear_zoom(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,
	                     uint8_t *dest_image, int32_t dest_width, int32_t dest_height)
{
	uint32_t dest_line_bytes = (uint32_t)dest_width * pixel_size;

	int32_t x_zoom_ratio = (src_width << 16) / dest_width + 1;
	int32_t y_zoom_ratio = (src_height << 16) / dest_height + 1;
	const int32_t x_error = -(1 << 15) + (x_zoom_ratio >> 1);
	const int32_t y_error = -(1 << 15) + (y_zoom_ratio >> 1);
	fprintf(stderr, "%s():x_zoom_ratio=%d y_zoom_ratio=%d x_error=%d y_error=%d\n", __FUNCTION__, x_zoom_ratio, y_zoom_ratio, x_error, y_error);
	int32_t x, y;
	uint8_t *dest_buf = dest_image;
	//get the special border info 
	int32_t border_y0 = -y_error / y_zoom_ratio + 1;
	if (border_y0 >= dest_height)
		border_y0 = dest_height;
	int32_t border_x0 = -x_error / x_zoom_ratio + 1;
	if (border_x0 >= dest_width)
		border_x0 = dest_width;
	int32_t border_y1 = (((src_height - 1) << 16) - y_error) / y_zoom_ratio + 1;
	if (border_y1 < border_y0)
		border_y1 = border_y0;
	int32_t border_x1 = (((src_width - 1) << 16) - x_error) / x_zoom_ratio + 1;
	if (border_x1 < border_x0)
		border_x1 = border_x0;
	else if (border_x1 < dest_width)
		border_x1 = dest_width;

	int32_t src_offset_y = y_error;
	for (y = 0; y < border_y0; y++){
		int32_t src_offset_x = x_error;
		for (x = 0; x < border_x0; x++){
			bilinear_border(src_image, src_line_bytes, src_width, src_height, src_offset_x, src_offset_y,
				dest_buf + x*pixel_size, dest_line_bytes);

			src_offset_x += x_zoom_ratio;
		}
		src_offset_y += y_zoom_ratio;
		dest_buf += dest_line_bytes;
	}
	for (y = border_y0; y < border_y1; y++){
		int32_t src_offset_x = x_error;
		for (x = 0; x < border_x0; x++){
			bilinear_border(src_image, src_line_bytes, src_width, src_height, src_offset_x, src_offset_y,
				dest_buf + x*pixel_size, dest_line_bytes);

			src_offset_x += x_zoom_ratio;
		}
		{
			uint32_t v_8 = (src_offset_y & 0xFFFF) >> 8;
			uint8_t *cur_src_line_buf = src_image + src_line_bytes*(src_offset_y >> 16);
			uint32_t i;
			for (i = border_x0; i < border_x1; i++){
				uint8_t *buf1 = cur_src_line_buf + (src_offset_x >> 16)*pixel_size;/*(x,y)*/
				uint8_t *buf2 = buf1 + pixel_size;/*(x+1,y)*/
				uint8_t *buf3 = buf1 + src_line_bytes;/*(x,y+1)*/
				uint8_t *buf4 = buf3 + pixel_size;/*(x+1,y+1)*/
				bilinear_save_dest1(dest_buf + i*pixel_size, buf1, buf2, buf3, buf4, (src_offset_x & 0xFFFF) >> 8, v_8);
				src_offset_x += x_zoom_ratio;
			}
		}
		for (x = border_x1; x < dest_width; x++){
			bilinear_border(src_image, src_line_bytes, src_width, src_height, src_offset_x, src_offset_y,
				dest_buf + x*pixel_size, dest_line_bytes);

			src_offset_x += x_zoom_ratio;
		}
		src_offset_y += y_zoom_ratio;
		dest_buf += dest_line_bytes;
	}
	for (y = border_y1; y < dest_height; y++){
		int32_t src_offset_x = x_error;
		for (x = 0; x < dest_width; x++){
			bilinear_border(src_image, src_line_bytes, src_width, src_height, src_offset_x, src_offset_y,
				dest_buf + x*pixel_size, dest_line_bytes);

			src_offset_x += x_zoom_ratio;
		}
		src_offset_y += y_zoom_ratio;
		dest_buf += dest_line_bytes;
	}
	fprintf(stderr, "%s():src_w=%d src_h=%d dest_w=%d dest_h=%d\n", __FUNCTION__, src_width, src_height, dest_width, dest_height);
	fprintf(stderr, "%s():y0=%d x0=%d y1=%d x1=%d\n", __FUNCTION__, border_y0, border_x0, border_y1, border_x1);
	set_dest_edge_pixle(dest_image, dest_width, dest_height, dest_line_bytes, dest_height - border_y1);

	return 0;
}

/*
*description: nearest zoom algorithm
*advantage:fast and  short time-consuming
*disadvantage:the zoom effect may not good
*name: nearest_zoom(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,
*  uint8_t *dest_image, int32_t dest_width, int32_t dest_height, float x_zoom_ratio, float y_zoom_ratio)
*input:uint8_t* src_image           --the source image pixel info buffer (RGB pixel info)
*      size_t src_line_bytes        --the source image one line pixel bytes in the buffer
*      int32_t src_width            --the source image pixel size of width
*      int32_t src_height           --the source image pixel size of height
*      int32_t dest_width           --the target image pixel size of width
*      int32_t dest_height          --the target image pixel size of height
*      float x_zoom_ratio           --the source image width / target image width
*      float y_zoom_ratio           --the source image height / target image height
*output:uint8_t* dest_image         --the target iamge pixel info (RGB pixel info)
*return: 0--zoom the source image to the target image success
*       -1--zoom the source image to the target image failed
*/
static int nearest_zoom(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,
	                    uint8_t *dest_image, int32_t dest_width, int32_t dest_height, float x_zoom_ratio, float y_zoom_ratio)
{
	int32_t x, y;
	uint8_t *p0;
	uint8_t *p1;
	int32_t *attr_x = NULL;
	int32_t *attr_y = NULL;
	if ((attr_x = (int32_t *)malloc(sizeof(int32_t)*dest_width)) == NULL)
		return -1;
	if ((attr_y = (int32_t *)malloc(sizeof(int32_t)*dest_height)) == NULL)
		return -1;
	//int32_t src_line_bytes = pixel_size*src_width;
	int32_t dest_line_bytes = pixel_size*dest_width;
	for (x = 0; x < dest_width; x++)
		*(attr_x + x) = (int32_t)(x*x_zoom_ratio);
	for (y = 0; y < dest_height; y++)
		*(attr_y + y) = (int32_t)(y*y_zoom_ratio);
	for (y = 0; y < dest_height; y++){
		p0 = src_image + src_line_bytes*attr_y[y];
		p1 = dest_image + dest_line_bytes*y;
		for (x = 0; x < dest_width; x++)
			memcpy(p1 + pixel_size * x, p0 + attr_x[x] * pixel_size, pixel_size);
	}
	if (attr_x) free(attr_x);
	if (attr_y) free(attr_y);
	//fprintf(stderr, "%s()--down \n", __FUNCTION__);
	return 0;
}


int resize_image(uint8_t *src_image, size_t src_line_bytes, int32_t src_width, int32_t src_height,
	             uint8_t *dest_image, int32_t dest_width, int32_t dest_height)
{
	if (src_image == NULL || src_width <= 0 || src_height <= 0 ||
		dest_image == NULL || dest_width <= 0 || dest_height <= 0)
		return -1;

	size_t dest_line_bytes = (size_t)(dest_width * pixel_size);

	/* the source image is the same as the destination image */
	if ((src_width == dest_width) && (src_height == dest_height) &&
		(src_line_bytes == dest_line_bytes)){
		memcpy(dest_image, src_image, dest_line_bytes*dest_height);
		return 0;
	}

	int ret = -1;
	float x_zoom_ratio = (float)src_width / dest_width;
	float y_zoom_ratio = (float)src_height / dest_height;
	if ( x_zoom_ratio < 1 || y_zoom_ratio < 1)
		ret = bilinear_zoom(src_image, src_line_bytes, src_width, src_height, dest_image, dest_width, dest_height);
	else
		ret = nearest_zoom(src_image, src_line_bytes, src_width, src_height, 
		                   dest_image, dest_width, dest_height, x_zoom_ratio, y_zoom_ratio);
	return ret;
}