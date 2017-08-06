/****
 *
 * GL Scene Class for Windows platform
 *
 ****/
#include "vixen.h"
#include "ogl/vxgdev.h"
#include "ogl/vxrendergl.h"


#ifdef VIXEN_GLFW
namespace Vixen
{

VX_IMPLEMENT_CLASS(DeviceInfo, Property);

bool GLRenderer::MakeGLContext()
{
	DeviceInfo*	dinfo = (DeviceInfo*) m_Scene->GetDevInfo();
	int			width, height;

#ifndef VIXEN_GLES2
	if (glewInit() != GLEW_OK)
		VX_ERROR(("GLRenderer::MakeGLContext ERROR failed to initialize GLEW\n"), false);
#endif
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &NumTexUnits);
	glfwSwapInterval(1);
	if (m_Scene->GetViewport().Width() == 0)
	{
		glfwGetWindowSize(&width, &height);
		m_Scene->SetViewport(0, 0, (float) width, (float) height);
	}
	if (NumTexUnits == 0)
		NumTexUnits = 8;
	Scene::SupportDDS = false;
	return true;
}

void GLRenderer::ReleaseGL()
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (devinfo == NULL)
		return;
	m_PBuffer.Free();
	DeleteTextures();
	glfwCloseWindow();
	glfwTerminate();
}

void GLRenderer::SelectGLContext(int changed)
{
	DeviceInfo* devinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (changed & SCENE_FBChanged)
	{
		ChangeRenderTarget((Bitmap*) m_Scene->GetColorBuffer());
		ChangeRenderTarget((Bitmap*) m_Scene->GetDepthBuffer());
	}
}

void GLRenderer::Flip(int frame)
{
	GeoSorter::Flip(frame);
	glfwSwapBuffers();	
}


} // end Vixen


#endif