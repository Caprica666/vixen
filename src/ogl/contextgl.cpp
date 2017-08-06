/****
 *
 * GL Scene Class for Windows platform
 *
 ****/
#include "vixen.h"
#include "ogl/vxgdev.h"
#include "ogl/vxrendergl.h"

#if !defined(VIXEN_GLES2) && !defined(VIXEN_GLFW)
namespace Vixen
{

VX_IMPLEMENT_CLASS(DeviceInfo, Property);

bool GLRenderer::MakeGLContext()
{
	DeviceInfo*	dinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (dinfo->Device == NULL)
/*
 * Get pixel format
 */
	{
		int nFlags = PFD_DRAW_TO_WINDOW |
					 PFD_SUPPORT_OPENGL | 
					 PFD_GENERIC_ACCELERATED |
					 PFD_SWAP_COPY |
					 PFD_DOUBLEBUFFER;


 		int		nPixelFormat, depth = Scene::DeviceDepth;
		if (depth == 32)
			depth = 24;
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof (PIXELFORMATDESCRIPTOR),// Size of this structure
			1,                             // Version number
			nFlags,
			PFD_TYPE_RGBA,                  // RGBA pixel values
			depth,
			0, 0, 0, 0, 0, 0,               // Don't care about these
			0, 0,                           // No alpha buffer
			0, 0, 0, 0, 0,                  // No accumulation buffer
			uchar(Scene::DeviceDepth),      // 32-bit depth buffer
			0,                              // No stencil buffer
			0,                              // No auxiliary buffers
			PFD_MAIN_PLANE,                 // Layer type
			0,                              // Reserved (must be 0)
			0, 0, 0                         // No layer masks
		};
		dinfo->DC = ::GetDC(dinfo->WinHandle);
		if (!dinfo->DC)
			VX_ERROR(("GLRenderer::MakeGLContext: Null Device Context"), false);
		nPixelFormat = ChoosePixelFormat(dinfo->DC, &pfd);
		if (nPixelFormat == 0)
			VX_ERROR(("GLRenderer::MakeGLContext cannot choose pixel format"), false);
		BOOL rc = SetPixelFormat(dinfo->DC, nPixelFormat, &pfd);
		DescribePixelFormat(dinfo->DC, nPixelFormat, sizeof (PIXELFORMATDESCRIPTOR), &pfd);
		VX_WARNING(("GLRenderer::MakeGLContext pixel format chosen"));
		dinfo->Device = glCreateContext(dinfo->DC);
		if (dinfo->Device == NULL)
			VX_ERROR(("GLRenderer::MakeGLContext ERROR cannot create GL context"), false);
		if (!glewInit())
			VX_ERROR(("GLRenderer::MakeGLContext ERROR failed to initialize GLEW"), false);
	}
/*
 * Bind GL render context to the Window and initialize OpenGL viewport
 */
	if (dinfo->DC)
	{
		RECT	r;
		if (!dinfo->Device || !glMakeCurrent(dinfo->DC, HGLRC(dinfo->Device)))
			VX_ERROR(("GLRenderer::MakeGLContext: Failed to create OpenGL context"), false);
		if (glewInit() != GLEW_OK)
			VX_ERROR(("GLRenderer::MakeGLContext: Failed to initialize GLEW"), false);
		::GetClientRect(dinfo->WinHandle, &r);
		glViewport(0, 0, r.right - r.left, r.bottom - r.top);
	}

//
// Check for multi-texturing extension and PBuffer extensions and determine
// the number of texture units available
//
	Core::String extensions = Core::String((const char*) glGetString(GL_VERSION));
	extensions += TEXT(" ");
	extensions += Core::String((const char*) glGetString(GL_EXTENSIONS));
//	extensions += Core::String((const char*) glGetExtensionsString(dinfo->DC));
//	VX_PRINTF(("Scene::SetWindow GL %s", (const TCHAR*) Extensions));	// VX_PRINTF here crashes!!
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &NumTexUnits);
	b_Transpose = glLoadTransposeMatrixf == 0;
	if (glCompressedTexImage2D)
		Scene::SupportDDS = true;
 	return true;
}

void GLRenderer::ReleaseGL()
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (devinfo == NULL)
		return;
	if (devinfo->Device)
	{
		glMakeCurrent(devinfo->DC, NULL);
		m_PBuffer.Free();
		DeleteTextures();
		glDeleteContext((HGLRC) devinfo->Device);
		devinfo->Device = NULL;
	}
	if (devinfo->DC)
	{
		::ReleaseDC(devinfo->WinHandle, devinfo->DC);
		devinfo->DC = NULL;
	}
}

void GLRenderer::SelectGLContext(int changed)
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();
	int err = glGetError();

	if (devinfo->Device == NULL)
		return;
	if (changed & SCENE_FBChanged)
	{
		ChangeRenderTarget((Bitmap*) m_Scene->GetColorBuffer());
		ChangeRenderTarget((Bitmap*) m_Scene->GetDepthBuffer());
	}
	if (devinfo->DC)
		glMakeCurrent(devinfo->DC, (HGLRC) devinfo->Device);
}

void GLRenderer::Flip(int frame)
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();
	int err = glGetError();
	GeoSorter::Flip(frame);
	if (devinfo->DC)
		SwapBuffers(devinfo->DC);	// no just swap buffers then
}


} // end Vixen

#endif