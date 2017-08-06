/****
 *
 * GL Scene Class for Linux platform
 *
 ****/
#include "vixen.h"
namespace Vixen {

bool GLScene::MakeGLContext()
{
	XWindowAttributes xattr;
	int				glattr[] = { GLX_USE_GL, GLX_RGBA, GLX_DOUBLEBUFFER, NULL };
	int				dummy;

	VX_ASSERT(m_pDevInfo);
	VX_ASSERT(m_pDevInfo->WinHandle);
	VX_ASSERT(m_pDevInfo->DispHandle);
	if (!XGetWindowAttributes(m_pDevInfo->DispHandle, m_pDevInfo->WinHandle, &xattr))
		VX_ERROR(("Scene::SetWindow: Cannot get window attributes"), false);
	if (!m_pDevInfo->DispHandle)
		VX_ERROR(("Scene::SetWindow: Null Display"), false);
	if (!glXQueryExtension(m_pDevInfo->DispHandle, &dummy, &dummy))
		VX_ERROR(("Scene::SetWindow: OpenGL extension missing, cannot create OpenGL context\n"), false);
/*
 * Bind GL render context to the Window
 */

	m_pDevInfo->VisHandle = glXChooseVisual(m_pDevInfo->DispHandle, 0, glattr);
	if (!m_pDevInfo->VisHandle)
		VX_ERROR(("Scene""SetWindow: Cannot find X11 visual suitable for OpenGL"), false);
	m_pDevInfo->Device = glXCreateContext(m_pDevInfo->DispHandle, m_pDevInfo->VisHandle, 0, GL_TRUE);
	VX_WARNING(("Scene::SetWindow GL context = %X", m_pDevInfo->Device));
	if (!m_pDevInfo->Device || !glXMakeCurrent(m_pDevInfo->DispHandle, m_pDevInfo->WinHandle, m_pDevInfo->Device))
		VX_ERROR(("Scene::SetWindow: Failed to create OGL context"), false);
	SetViewport();
/*
 * Initialize OpenGL state
 */
	char* glextensions = (char*) glGetString(GL_EXTENSIONS);
	char* glversion = (char*) glGetString(GL_VERSION);
	if (!glextensions)
		glextensions = "";
	if (!glversion)
		glversion = "";
	Extensions = glextensions;
	b_TexEnvAdd = (strstr(glextensions, "texture_env_add") != 0);
	VX_WARNING(("Scene::SetWindow GL V%s  %s", glversion, glextensions));
//
// Check for multi-texturing extension and PBuffer extensions and determine
// the number of texture units available
//
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &NumTexUnits);
	b_Transpose = 	b_TexEnvAdd = (strstr(glextensions, "transpose") != 0);
	return true;
}

void GLScene::Append(Scene* child)
{
	GLScene* glchild = (GLScene*) child;
	glchild->NumTexUnits = NumTexUnits;
	glchild->b_Transpose = b_Transpose;
	glchild->b_TexEnvAdd = b_TexEnvAdd;
	Scene::Append(child);
}

void GLScene::ReleaseGL()
{
	DeviceInfo devinfo = *m_pDevInfo;

	if (devinfo.Device)
	{
		glXMakeCurrent(devinfo.DispHandle, devinfo.WinHandle, devinfo.Device);
		m_PBuffer.Free();
		DeleteTextures();
		glXDestroyContext(devinfo.DispHandle, devinfo.Device);
		devinfo.Device = NULL;
	}

}

void GLScene::SelectGLContext()
{
	if (m_pDevInfo->Device == NULL)
		return;
	if (m_Changed & SCENE_FBChanged)
	{
		ChangeRenderTarget(m_ColorBuffer);
		ChangeRenderTarget(m_DepthBuffer);
	}
	if (m_PBuffer.GetBuffer())
		glXMakeCurrent(m_pDevInfo->DispHandle, (GLXPbuffer) m_PBuffer.GetBuffer(), m_pDevInfo->Device);
	else if (m_pDevInfo->Device)
		glXMakeCurrent(m_pDevInfo->DispHandle, m_pDevInfo->WinHandle, m_pDevInfo->Device);
#ifdef _DEBUG
	GLenum err = glGetError();
	if (err)
		VX_PRINTF(("GL ERROR %s\n", gluErrorString(err)));
#endif
}

void GLScene::Flip()
{
	if (m_pDevInfo->DispHandle)
		glXSwapBuffers(m_pDevInfo->DispHandle, m_pDevInfo->WinHandle);
}

void GLScene::ChangeRenderTarget(Bitmap* bmap)
{
	if (bmap == NULL)
	{
		m_PBuffer.Free();
		return;
	}
	if (bmap->HasChanged() || (bmap->DevHandle == 0))
	{
		Texture image;

		image.SetBitmap(bmap);
		ObjectLock lock(bmap);
		if (bmap->Data == NULL)
		{
			void* pixels = malloc(bmap->Width * bmap->Height * bmap->Depth << 2);
			bmap->Type = Bitmap::DXBITMAP;
			bmap->Data = pixels;
			bmap->Format = Bitmap::HASCOLOR | Bitmap::HASALPHA;
		}
		bmap->SetChanged(false);
		image.CreateTexture(this, true, 0);
		m_PBuffer.SetDev(m_pDevInfo);
		m_PBuffer.SetSize(bmap->Width, bmap->Height);
	}
	SetViewport(0, 0, float(bmap->Width), float(bmap->Height));
	SetChanged(false);
}


void	GLScene::PBuffer::SetDev(DeviceInfo* d)
{
}

void GLScene::PBuffer::Free()
{
	if (m_Buffer == NULL)
		return;
	DeviceInfo* di = m_DevInfo;
	glXDestroyPbuffer(di->DispHandle, (GLXPbuffer) m_Buffer);
	m_Buffer = NULL;
}

bool GLScene::PBuffer::SetSize(int width, int height)
{
	DeviceInfo*	devinfo = m_DevInfo;

	if (devinfo == NULL)
		return false;
	if (m_Buffer)					// already have a PBuffer
	{
		unsigned int	w, h;
		w = width;
		h = height;
		if ((w == m_Width) && (h == m_Height))
			return true;			// dimensions match, use the buffer
		glXDestroyPbuffer(devinfo->DispHandle, (GLXPbuffer) m_Buffer);
		m_Buffer = NULL;
		m_Width = m_Height = 0;
	}
	int				fbattrs[] = { GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT, 0 };
	int				pattrs[] = { GLX_PBUFFER_WIDTH, width,  GLX_PBUFFER_HEIGHT, height, 0 };
	int				nconfigs;
	GLXFBConfig*	config = glXGetFBConfigs(devinfo->DispHandle, 0, &nconfigs);
	while (config)
	{
		m_Buffer = (void*) glXCreatePbuffer(devinfo->DispHandle, *config, pattrs);
		if (!m_Buffer)
			return false;
		m_Width = width;
		m_Height = height;
		return glXMakeCurrent(devinfo->DispHandle, (GLXPbuffer) m_Buffer, devinfo->Device);
	}
	VX_ERROR(("Scene::SetColorBuffer display device does not support this format"), false);
}

}
