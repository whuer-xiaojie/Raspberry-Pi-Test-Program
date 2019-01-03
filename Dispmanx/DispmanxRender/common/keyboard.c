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

#include <termios.h>
#include <unistd.h>

#include "keyboard.h"

static struct termios old_termios;
static struct termios cur_termios;
static int keyboard_initialized = 0;

int init_keyboard(void)
{
	if (tcgetattr(STDIN_FILENO, &old_termios) != 0)
		return -1;

	cur_termios = old_termios;	/* structure copy */

	cur_termios.c_lflag &= ~(ECHO | ICANON | ISIG);
	cur_termios.c_cc[VMIN] = 1;
	cur_termios.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_termios) != 0)
		return -1;

	keyboard_initialized = 1;
	return 0;
}

int close_keyboard(void)
{
	if (keyboard_initialized) {
		keyboard_initialized = 0;
		if (tcsetattr(STDIN_FILENO, TCSANOW, &old_termios) != 0)
			return -1;
	}
	return 0;
}

int __getch(void)
{
	ssize_t n;
	unsigned char c;

	if (!keyboard_initialized)
		return -1;
	cur_termios.c_cc[VMIN] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_termios) != 0)
		return -1;
	n = read(STDIN_FILENO, &c, 1);
	cur_termios.c_cc[VMIN] = 1;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_termios) != 0)
		return -1;
	return (n == 1) ? c : -1;
}
