/****
 *
 * GL Scene Class for Windows platform
 *
 ****/
#include "vixen.h"
#include "ogl/vxgdev.h"
#include "ogl/vxrendergl.h"

#if defined(VIXEN_GLES2)
namespace Vixen
{

#ifndef VIXEN_GLFW
	
VX_IMPLEMENT_CLASS(DeviceInfo, Property);

bool GLRenderer::MakeGLContext()
{
	DeviceInfo*	dinfo = (DeviceInfo*) m_Scene->GetDevInfo();
	RECT		r;

	if (dinfo->Device == NULL)
	{
		EGLint fbattrs16[] =
		{
				EGL_RED_SIZE, 5,
				EGL_GREEN_SIZE, 6,
				EGL_BLUE_SIZE, 5,
				EGL_DEPTH_SIZE, 16,
				EGL_SURFACE_TYPE,	EGL_WINDOW_BIT,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_NONE
		};
		EGLint fbattrs32[] =
		{
			EGL_LEVEL,				0,
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL_NATIVE_RENDERABLE,	EGL_FALSE,
			EGL_RED_SIZE,			8,
			EGL_GREEN_SIZE,			8,
			EGL_BLUE_SIZE,			8,
            EGL_ALPHA_SIZE,			EGL_DONT_CARE,
            EGL_DEPTH_SIZE,			24,
            EGL_STENCIL_SIZE,		0,
			EGL_NONE
		};
		EGLint fbattrsdef[] =
		{
			EGL_LEVEL,				0,
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL_NATIVE_RENDERABLE,	EGL_FALSE,
			EGL_RED_SIZE,			EGL_DONT_CARE,
			EGL_GREEN_SIZE,			EGL_DONT_CARE,
			EGL_BLUE_SIZE,			EGL_DONT_CARE,
            EGL_ALPHA_SIZE,			EGL_DONT_CARE,
            EGL_DEPTH_SIZE,			EGL_DONT_CARE,
            EGL_STENCIL_SIZE,		EGL_DONT_CARE,
			EGL_NONE
		};

		EGLint contextattrs[] =
		{
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		EGLConfig	configs[2];
		EGLint		numconfigs;
	
		m_Display = eglGetDisplay((EGLNativeDisplayType) EGL_NO_DISPLAY);
		eglInitialize(m_Display, NULL, NULL);
		eglBindAPI(EGL_OPENGL_ES_API);
		if (Scene::DeviceDepth == 32)
			eglChooseConfig(m_Display, fbattrs32, configs, 1, &numconfigs);
		else if (Scene::DeviceDepth == 16)
			eglChooseConfig(m_Display, fbattrs16, configs, 1, &numconfigs);
		else
			eglChooseConfig(m_Display, fbattrsdef, configs, 1, &numconfigs);

		if (numconfigs == 0)
			VX_ERROR(("GLRenderer::MakeGLContext ERROR no GL configurations found"), false);
		m_Surface = eglCreateWindowSurface(m_Display, configs[0], (EGLNativeWindowType) dinfo->WinHandle, NULL);
		if (m_Surface == EGL_NO_SURFACE)
			VX_ERROR(("GLRenderer::MakeGLContext ERROR cannot create GL surface"), false);
		dinfo->Device = eglCreateContext(m_Display, configs[0], EGL_NO_CONTEXT, contextattrs);
		if (dinfo->Device == EGL_NO_CONTEXT)
			VX_ERROR(("GLRenderer::MakeGLContext ERROR cannot create GL context"), false);
		VX_WARNING(("Scene::MakeGLContext GL context = %X", dinfo->Device));
	}
/*
 * Bind GL render context to the Window and initialize OpenGL viewport
 */
	if (!eglMakeCurrent(m_Display, m_Surface, m_Surface, dinfo->Device))
		VX_ERROR(("Scene::MakeGLContext: Failed to create OpenGL context"), false);
	::GetClientRect(dinfo->WinHandle, &r);
	glViewport(0, 0, r.right - r.left, r.bottom - r.top);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &NumTexUnits);
	if (NumTexUnits == 0)
		NumTexUnits = 8;
	Scene::SupportDDS = false;
	Bitmap::RGBADepth = 16;
	Bitmap::RGBDepth = 16;
#ifdef _DEBUG
	GLuint err = glGetError();
	if (err)
		VX_WARNING(("GLRenderer::MakeGLContext ERROR %x", err));
#endif
	return true;
}

void GLRenderer::ReleaseGL()
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (devinfo == NULL)
		return;
	if (m_Display == NULL)
		return;
	if (devinfo->Device)
	{
		eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		m_PBuffer.Free();
		eglDestroySurface(m_Display, m_Surface);
		DeleteTextures();
		eglDestroyContext(m_Display, devinfo->Device);
		devinfo->Device = NULL;
		m_Surface = NULL;
	}
	eglTerminate(m_Display);
	m_Display = NULL;

}

void GLRenderer::SelectGLContext(int changed)
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (devinfo->Device == NULL)
		return;
	if (changed & SCENE_FBChanged)
	{
		ChangeRenderTarget((Bitmap*) m_Scene->GetColorBuffer());
		ChangeRenderTarget((Bitmap*) m_Scene->GetDepthBuffer());
	}
//	if (devinfo->Device)
//		eglMakeCurrent(m_Display, m_Surface, m_Surface, devinfo->Device);
}

void GLRenderer::Flip(int frame)
{
	GeoSorter::Flip(frame);
	if (m_Display)
		eglSwapBuffers(m_Display, m_Surface);	
}
#endif		// end VIXEN_GLFW

void	GLRenderer::PBuffer::SetDev(DeviceInfo* d)
{
	m_DevInfo = d;
}

void GLRenderer::PBuffer::Free()
{
	GLuint	pbuf = (GLuint) m_Buffer;

	if (pbuf != 0)
		glDeleteRenderbuffers(1, &pbuf);
	m_Buffer = 0;
}

bool GLRenderer::PBuffer::SetSize(int width, int height)
{
	GLuint	pbuf = (GLuint) m_Buffer;
	GLint	w, h, format = GL_RGBA4;

	if (pbuf == NULL)
	{
		glGenRenderbuffers(1, &pbuf);
		if (pbuf == NULL)
			VX_ERROR(("GLRenderer ERROR cannot generate render buffer"), false);
	}
	glGetRenderbufferParameteriv(pbuf, GL_RENDERBUFFER_WIDTH, &w);
	glGetRenderbufferParameteriv(pbuf, GL_RENDERBUFFER_HEIGHT, &h);
	glGetRenderbufferParameteriv(pbuf, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
	if ((w != width) && (h != height))
		glRenderbufferStorage(pbuf, format, w, h);
	return true;
}


} // end Vixen

#endif
