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

#include "dispRender.h"

void init_dispmanx_render(DISPRENDER_T *render, int width, int height)
{
	assert(render != NULL && width > 0 && height > 0);

	uint32_t displayNumber = 0;
	DISPMANX_MODEINFO_T info;
	VC_RECT_T showPos;
	int result = 0;

	bcm_host_init();

	render->display = vc_dispmanx_display_open(displayNumber);
	assert(render->display != 0);

	result = vc_dispmanx_display_get_info(render->display, &info);
	assert(result == 0);

	if (width > info.width)
		width = info.width;
	if (height > info.height)
		height = info.height;

	showPos.x = 0;
	showPos.y = 0;
	showPos.width = render->width = width;
	showPos.height = render->height = height;

	init_back_layer(&(render->bg), 0);
	init_image_layer(&(render->il), &showPos, 1);

	render->update = vc_dispmanx_update_start(0);
	assert(render->update != 0);

	add_back_layer_element(&render->bg, render->display, render->update);
	add_image_layer_element(&render->il, render->display, render->update);

	result = vc_dispmanx_update_submit_sync(render->update);
	assert(result == 0);

}
	
void render_image_rgb888(DISPRENDER_T *render, unsigned char *buf)
{
	assert(render!=NULL && buf != NULL);

	int result = 0;

	render->update = vc_dispmanx_update_start(0);
	assert(render->update != 0);

	change_image_layer_source(&render->il, render->update, buf,render->width,render->height);

	result = vc_dispmanx_update_submit_sync(render->update);
	assert(result == 0);
}

void destory_dispmanx_render(DISPRENDER_T *render)
{
	assert(render != NULL);
	int result = 0;

	destory_back_layer(&render->bg);
	destory_image_layer(&render->il);

	result = vc_dispmanx_display_close(render->display);
	assert(result == 0);
}