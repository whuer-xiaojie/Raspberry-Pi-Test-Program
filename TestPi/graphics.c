/*
*graphics.c
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"


#include "lcd.h"
#include "../display.h"
#include "../graphics.h"
#include "graphics.h"
#include "../pc/vram.h"

//x9 Show Window Size
#define SCREEN_WIDTH		800
#define SCREEN_HEIGHT	600

//the x9 show region size
extern int sign_width;
extern int sign_height;

extern int view_width;
extern int view_height;

struct CUBE_STATE_T
{
	uint32_t EGL_screen_width;
	uint32_t EGL_screen_height;

	// OpenGL|ES objects
	EGLDisplay display;
	EGLSurface surface;
	EGLConfig config;
	EGLContext gl_context;

	//GLuint textureId;
	GLuint program;/*the x9 shader program*/
	GLint  positionLoc;/*the x9 shader variable entrance*/
	GLint  texCoordLoc;
	GLint  samplerLoc;

	GLuint textureId;/*the x9 paint texture name*/
};

struct CUBE_STATE_T *state;
/************************************************************************/
/*
*vertex Shader Դ��
*attribute     �������ڻ�ȡ����������Ϣ(�������꣬�������꣬��ɫ��)
*a_position    ��Ŷ���λ��������Ϣ��x,y��
*a_texCoord    �������������Ϣ(u,v)
*varying       �������ں�flagment Shader�������ݴ���
*v_texCoord    ��������������Ϣ��ƬԪ��ɫ��  (����GPU����ת������)
*/
/************************************************************************/

const char *vertex_shader_src =//"#version 200 core   \n"
"attribute vec4 a_position;    \n"
"attribute vec2 a_texCoord;    \n"
"varying vec2 v_texCoord;      \n"
"void main() {                 \n"
"    gl_Position = a_position; \n"
"    v_texCoord = a_texCoord;  \n"
"}                             \n";

/************************************************************************/
/*
*fragment Shader         Դ��
*precision mediump float ָ��float���ͱ����ľ���Ϊ�еȾ���
*v_texCoord              ������vertex Shader����������������
*s_texture               ���ڷ�����������Ľӿڣ��൱��ȫ�ֱ�����
*/
/************************************************************************/

const char *fragment_shader_src =//"#version 200 core   \n"
"precision mediump float;                             \n"
"varying vec2 v_texCoord;                             \n"
"uniform sampler2D s_texture;                         \n"
"void main() {                                        \n"
"    gl_FragColor = texture2D(s_texture, v_texCoord); \n"
"}                                                    \n";

/************************************************************************/
/*
*EGL surface���������б�
*�졢�̡�������ɫ�ĸ�ʽ��Ϊ8λ��Ҳ����24λ
*/
/************************************************************************/

const EGLint attribute_list[] =
{
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_NONE
};

const EGLint context_attributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};
/************************************************************************/
/*
*��������load_shader(const char *source, GLenum type)
*����  ��������ɫ����Shader����ָ����ɫ����Դ�벢���б���
*����  ��source �����ɫ����Shader����Դ���ָ�룬type ��Ҫ������Shader������
*       ��GL_VERTEX_SHADER��GL_FRAGMENT_SHADER��
*����ֵ��shader�����ɹ�����shader������������ʧ�ܻ���Դ������򷵻�0
*/
/************************************************************************/
static GLuint load_shader(const char *source, GLenum type)
{
	GLuint shader;
	const char *shaders[1] = { NULL };
	GLint status;
	//Create Shader
	if ((shader = glCreateShader(type)) == 0)
	{
		fprintf(stderr, "%s():Create Shader failed\n",__FUNCTION__);
		return 0;
	}

	//Get Shader Source Code
	shaders[0] = source;
	glShaderSource(shader, 1, shaders, NULL);
	shaders[0] = NULL;

	//Compile Shader Source Code
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		//Get Shader Compiled Error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			fprintf(stderr, "Shaderlog:\n%s", log);
			free(log);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
/************************************************************************/
/*
*��������init_graphics(void)
*����  ����ʼ��һ��EGL����������ʾ�������һ��OpenGL�Ŀ�ִ�г������program����
*        ����ȣ�����ȡ�ö���Ķ�����ɫ���������ݵġ���㡱
*����  ����
*����ֵ����ʼ����ɷ��� 0
*/
/************************************************************************/
int init_graphics(void)
{
	bcm_host_init();

	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;


	state = (struct CUBE_STATE_T *)malloc(sizeof(struct CUBE_STATE_T));
	if (state == NULL)
	{
		fprintf(stderr, "%s():malloc for struct CUBE_STATE_T state failed\n", __FUNCTION__);
		return -1;
	}
	// get an EGL display connection
	if ((state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
	{
		fprintf(stderr, "%s():get the display failed\n", __FUNCTION__);
		return -1;
	}

	// initialize the EGL display connection
	result = eglInitialize(state->display, NULL, NULL);
	if (result == EGL_FALSE)
	{
		fprintf(stderr, "%s():initilize display failed\n", __FUNCTION__);
		return -1;
	}

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &state->config, 1, &num_config);
	if (result == EGL_FALSE)
	{
		fprintf(stderr, "%s():get configs failed\n", __FUNCTION__);
		return -1;
	}

	// get an appropriate EGL frame buffer configuration
	result = eglBindAPI(EGL_OPENGL_ES_API);
	if (result == EGL_FALSE)
	{
		fprintf(stderr, "%s():bind to opengl ES API failed\n", __FUNCTION__);
		return -1;
	}

	/*
	// create an EGL rendering context
	if ((state->gl_context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes)) == EGL_NO_CONTEXT)
	{
	fprintf(stderr, "%s():create contex failed \n", __FUNCTION__);
	return -1;
	}
	*/

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */, &state->EGL_screen_width, &state->EGL_screen_height);
	if (success<0)
	{
		fprintf(stderr, "%s():get diaplay size failed\n", __FUNCTION__);
		return -1;
	}


	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = state->EGL_screen_width;
	dst_rect.height = state->EGL_screen_height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = state->EGL_screen_width << 16;
	src_rect.height = state->EGL_screen_height << 16;

	dispman_display = vc_dispmanx_display_open(0 /* LCD */);
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
		0/*layer*/, &dst_rect, 0/*src*/,
		&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

	nativewindow.element = dispman_element;
	nativewindow.width = state->EGL_screen_width;
	nativewindow.height = state->EGL_screen_height;
	vc_dispmanx_update_submit_sync(dispman_update);
	
	if ((state->surface = eglCreateWindowSurface(state->display, state->config, &nativewindow, NULL)) == EGL_NO_SURFACE)
	{
	fprintf(stderr, "%s():create window sufrface failed\n", __FUNCTION__);
	return -1;
	}

/*
	// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->gl_context);
	if (result == EGL_FALSE)
	{
	fprintf(stderr, "%s():make current context and surface failed\n", __FUNCTION__);
	return -1;
	}
*/
	
	// Set background color and clear buffers
	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//fprintf(stderr, "%s():state->screen_width=%d,state->screen_height=%d\n", __FUNCTION__, state->EGL_screen_width, state->EGL_screen_height);
	
	return 0;
}

int create_texture(void)
{
	GLint  status;
	GLuint vertex_shader;
	GLuint fragment_shader;
	int w, h;

	// create an EGL rendering context
	if ((state->gl_context = eglCreateContext(state->display, state->config, EGL_NO_CONTEXT, context_attributes)) == EGL_NO_CONTEXT)
	{
		fprintf(stderr, "%s():create contex failed \n", __FUNCTION__);
		return -1;
	}
	if ((eglMakeCurrent(state->display, state->surface, state->surface, state->gl_context)) == EGL_FALSE)
	{
		fprintf(stderr, "%s():make current context and surface failed\n", __FUNCTION__);
		return -1;
	}
	glEnable(GL_TEXTURE_2D);

	//Create Vertex Shader
	vertex_shader = load_shader(vertex_shader_src, GL_VERTEX_SHADER);
	if (vertex_shader == 0)
	{
		fprintf(stderr, "%s():Load vertex shader failed\n", __FUNCTION__);
		close_texture();
		exit(1);
	}
	//Create Fragment Shader
	fragment_shader = load_shader(fragment_shader_src, GL_FRAGMENT_SHADER);
	if (fragment_shader == 0)
	{
		fprintf(stderr, "%s():Load fragment shader failed\n", __FUNCTION__);
		close_texture();
		exit(1);
	}

	//Create a program
	if ((state->program = glCreateProgram()) == 0)
	{
		fprintf(stderr, "%s():glCreateProgram() failed\n", __FUNCTION__);
		close_texture();
		exit(1);
	}

	//Attach Shader to Program
	glAttachShader(state->program, vertex_shader);
	glAttachShader(state->program, fragment_shader);

	//Link program( A GPU executable program)
	glLinkProgram(state->program);

	//Get link Information
	glGetProgramiv(state->program, GL_LINK_STATUS, &status);
	if (!status) {
		//Get link Error Info
		GLint infoLen;
		glGetProgramiv(state->program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char *infoLog = malloc(infoLen);
			glGetProgramInfoLog(state->program, infoLen, NULL, infoLog);
			fprintf(stderr, "%s():Link program: \"%s\"\n", __FUNCTION__, infoLog);
			free(infoLog);
		}
		glDeleteProgram(state->program);
		close_texture();
		exit(1);
	}
	//fprintf(stderr, "%s():1111111111\n", __FUNCTION__);
	//Delete the shader
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	//Get Shader Start Point(position,texCoord,texture)
	state->positionLoc = glGetAttribLocation(state->program, "a_position");
	state->texCoordLoc = glGetAttribLocation(state->program, "a_texCoord");

	state->samplerLoc = glGetUniformLocation(state->program, "s_texture");

	//Create the View_port Start from Left_Top 
	glViewport(0, state->EGL_screen_height - view_height, view_width, view_height);

	//Init the Start Color
	glClearColor(0.0f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(state->display, state->surface);
	glFlush();
	glFinish();
	//fprintf(stderr, "%s():222222222\n", __FUNCTION__);

	//get the x9 texture that used to update the show info
	glGenTextures(1, &state->textureId);

	//Show the First Texture
	glBindTexture(GL_TEXTURE_2D, state->textureId);

	//Set the Textrue's Attributes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
int close_texture(void)
{
	glDeleteTextures(1, &state->textureId);
	if (glIsProgram(state->program) == GL_TRUE)
	{
		glDeleteProgram(state->program);
	}
	eglDestroyContext(state->display, state->gl_context);
	fprintf(stderr, "%s():close_texture down\n", __FUNCTION__);
}

/************************************************************************/
/*
*��������int close_graphics(void)
*����  ���ͷų�ʼ���д�����SDL���ں�Shader����program�Լ�Opengl��
*        �����Ķ���gl_context
*����  ����
*����ֵ��0��ʾ�ͷ����
*/
/************************************************************************/
int close_graphics(void)
{

	// clear screen
	glClear(GL_COLOR_BUFFER_BIT);
	//eglSwapBuffers(state->display, state->surface);

	//glDeleteTextures(1, state->gl_context);
	eglDestroySurface(state->display, state->surface);

	// Release OpenGL resources
	eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(state->display, state->gl_context);
	eglTerminate(state->display);

	fprintf(stderr, "%s():lose_graphics down\n", __FUNCTION__);
	return 0;
}

/************************************************************************/
/*
*/
/************************************************************************/
void listbox_add(const char *s)
{
	fprintf(stderr, "%s\n", s);
}


/************************************************************************/
/*
*��������render_buffer_R8G8B8(const unsigned char *buf)
*����  ������Opengl����Ⱦ�ܵ��Ѵ����buf�е�ͼ����ʾ����ʼ���Ĵ�����
*
*����  �����룺buf �����Ҫ��ʾ��ͼ���λͼ��Ϣ��24λRGB��ʽ��ţ�
*����ֵ����
*/
/************************************************************************/
void render_buffer_R8G8B8(const unsigned char *buf)
{
	//Init the Vertex Shader Info

	//Start From Left_Top 
	GLfloat vertices[] = { 
		-1.0f,  1.0f,  0.0f,	/* Position 0 */ 0.0f, 0.0f,		/* TexCooed 0 */
		-1.0f, -1.0f,  0.0f,	/* Position 1 */ 0.0f, 1.0f,		/* TexCooed 1 */
		 1.0f,  -1.0f,  0.0f,	/* Position 2 */ 1.0f, 1.0f,		/* TexCooed 2 */
		 1.0f,   1.0f,  0.0f,	/* Position 3 */ 1.0f, 0.0f		   /* TexCooed 3 */};

	//Include Position Color and texCoord 
	//GLfloat vertices[] = {
	//   -1.0f, 1.0f,0.0f, 1.0f,0.0f,0.0f, 0.0f,0.0f,
	//    0.0f,-1.0f,0.0f, 0.0f,1.0f,0.0f, 1.0f,0.0f,
	//    0.0f, 1.0f,0.0f, 0.0f,0.0f,1.0f, 1.0f,1.0f,
	//   -1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f, 0.0f,1.0f };

	//Start from the Left_Bottom
	//GLfloat vertices[] = {
	//	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,		/* Bottom Left  */
	//	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,		/* Bottom Right */
	//	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,		/* Top Right */
	//	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f		/* Top Left */
	//};

	GLushort indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};


	if (buf == NULL)
	{
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		glFinish();
		fprintf(stderr, "%s():no show data \n",__FUNCTION__);
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Get the Show Data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, view_width , view_height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, buf);

	//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imageWidth, imageHeight, 
	//	GL_RGB, GL_UNSIGNED_BYTE, buf);

	glUseProgram(state->program);

	//Transfer the Position and TexCoord Data to Vertex Shader
	glVertexAttribPointer(state->positionLoc, 3, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), vertices);
	glVertexAttribPointer(state->texCoordLoc, 2, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), &vertices[3]);
	glEnableVertexAttribArray(state->positionLoc);
	glEnableVertexAttribArray(state->texCoordLoc);

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureId);

	glUniform1i(state->samplerLoc, 0);
	//Draw the Picture
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	eglSwapBuffers(state->display, state->surface);
	//Show picture to the Window
	glFlush();
	glFinish();
}

void render_buffer_R8G8B8A8(const unsigned char *buf)
{

}


