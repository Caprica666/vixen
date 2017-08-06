#pragma once


#if defined(_WIN32)
	#ifdef VIXEN_GLES2
		#include "GLES2/gl2.h"
		#include "GLES2/gl2ext.h"
		#include "EGL/egl.h"
		#ifdef VIXEN_GLFW
			#include <GL/glfw.h>
		#endif

		#define	glChoosePixelFormat		eglChoosePixelFormatARB
		#define	glGetExtensionsString	wglGetExtensionsStringARB
		#define	glMakeCurrent			eglMakeCurrent
		#define	glDeleteContext			eglDeleteContext
		#define	glCreateContext			eglCreateContext
		#define	glCreatePbuffer			wglCreatePbufferARB
		#define	glReleasePbufferDC		wglReleasePbufferDCARB
		#define	glDestroyPbuffer		wglDestroyPbufferARB
		#define	glReleasePbuffer		wglReleasePbufferARB
		#define	glGetPbufferDC			wglGetPbufferDCARB
		#define	glQueryPbuffer			wglQueryPbufferARB
		#define	HPBUFFER				HPBUFFERARB
		#define	GL_DEPTH_COMPONENT32	GL_DEPTH_COMPONENT32_OES
		#define	glDrawRangeElements(mode, start, end, count, type, indices)	glDrawElements(mode, count, type, indices)


	#else	// not GLES2
		#include <windows.h>
		#include "GL/glew.h"
		#include "GL/wglew.h"
		#ifdef VIXEN_GLFW
			#define GLFW_INCLUDE_GLU
			#include <GL/glfw.h>
		#else
			#include <GL/gl.h>
			#include <GL/glu.h>
		#endif

		#define	glChoosePixelFormat		wglChoosePixelFormatARB
		#define	glGetExtensionsString	wglGetExtensionsStringARB
		#define	glMakeCurrent			wglMakeCurrent
		#define	glDeleteContext			wglDeleteContext
		#define	glCreateContext			wglCreateContext
		#define	glCreatePbuffer			wglCreatePbufferARB
		#define	glReleasePbufferDC		wglReleasePbufferDCARB
		#define	glDestroyPbuffer		wglDestroyPbufferARB
		#define	glReleasePbuffer		wglReleasePbufferARB
		#define	glGetPbufferDC			wglGetPbufferDCARB
		#define	glQueryPbuffer			wglQueryPbufferARB
		#define	HPBUFFER				HPBUFFERARB
	#endif	// end VIXEN_OGLES2

#else	// end WIN32
	#ifdef VIXEN_GLES2
		#define	ptrdiff_t khronos_ssize_t		// workaround for Emscripten/GLES2 compile error
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
		#include <EGL/egl.h>
		#ifdef VIXEN_GLFW
			#include <GL/glfw.h>
		#endif
	#else		// not GLES2
		#ifdef VIXEN_GLFW
			#define GLFW_INCLUDE_GLU
			#include <GLFW/glfw.h>
		#else
			#include <GL/gl.h>
			#include <GL/glu.h>
			#include <GL/glx.h>
			#include <GL/glext.h>
		#endif
	#endif
#endif

