/****
 *
 * GL Scene Class
 *
 ****/
#include "vixen.h"
#include "ogl/vxgdev.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxlightgl.h"
#include "ogl/vbufgl.h"
#include "ogl/vxshadergl.h"

namespace Vixen {

VX_IMPLEMENT_CLASS(GLRenderer, GeoSorter);

Renderer*	Renderer::CreateRenderer(int options)
{
	Messenger::SysVecSize = 4;
	return new GLRenderer();
}

GLRenderer::GLRenderer() : GeoSorter()
{
	b_Transpose = false;
}

GLRenderer& GLRenderer::operator=(const GLRenderer& src)
{
	GeoSorter::operator=(src);
	b_Transpose = false;
	return *this;
}


void GLRenderer::Exit(bool shutdown)
{
	s_DefaultAppear = NULL;
	GeoSorter::Exit(shutdown);
	GLShader::ReleaseAll();
	GLProgram::ReleaseAll();
	if (shutdown)
		ReleaseGL();
}

bool GLRenderer::Init(Scene* scene, Vixen::Window win, const TCHAR* options)
{
	const DeviceInfo* dinfo;

	if (!Renderer::Init(scene, win, options))
		return false;
	if (win == 0)
		return false;
	if (Debug == 0)
		GLRenderer::Debug = Scene::Debug;
	dinfo = m_Scene->GetDevInfo();
	if (dinfo->Device != NULL)
		return true;
	if (!MakeGLContext())
		return false;
	s_DefaultAppear = new Appearance();
	DefaultMaterial = new PhongMaterial();
	m_NoLightSource = "vec3 LightPixel(Surface s) { return s.diffuse.xyz; }";

	MaxLights = LIGHT_MaxLights;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MakeDefaultShaders();
#ifdef _DEBUG
	GLuint err = glGetError();
	if (err)
		VX_WARNING(("GLRenderer::Init %x\n", err));
#endif
 	return true;
}


/*****
 *
 * GLScene::End
 *
 * Updates the screen with what has been rendered. In a double
 * buffering system, the back buffer which has been rendered
 * into is blitted onto the screen or swapped with the front buffer.
 *
 *****/
void GLRenderer::End(int frame)
{
	const Bitmap* bmap = m_Scene->GetColorBuffer();
	int		w;
	int		h;
	int		format;
	intptr	texhandle;

	m_Changed = false;
	s_CurAppear = NULL;
	if (bmap && bmap->DevHandle)
	{
		bmap->Lock();
		w = bmap->Width;
		h = bmap->Height;
		format = bmap->Format;
		texhandle = bmap->DevHandle;	// GL texture handle is 32 bit integer
		bmap->Unlock();

		glBindTexture(GL_TEXTURE_2D, (GLuint) texhandle);
		if (format & Bitmap::HASCOLOR)
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
		else
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, w, h, 0);
	}

	bmap = m_Scene->GetDepthBuffer();
	if (bmap && bmap->DevHandle)
	{
		bmap->Lock();
		w = bmap->Width;
		h = bmap->Height;
		texhandle = bmap->DevHandle;
		bmap->Unlock();

		glBindTexture(GL_TEXTURE_2D, (GLuint) texhandle);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
	}
	GeoSorter::End(frame);
#ifdef _DEBUG
	GLenum err = glGetError();
	if (err)
		VX_ERROR_RETURN(("GLRenderer::End GL ERROR %x\n", err));
#endif
}

SharedObj*	GLRenderer::Clone() const
{
	GLRenderer* render = new GLRenderer();
	*render = *this;
	return render;
}

void	GLRenderer::SetViewport(float l, float t, float r, float b)
{
	GLint	x = GLint(l);
	GLint	y = GLint(t);
	GLsizei	w = GLint(r - l);
	GLsizei	h = GLint(b - t);

	w = abs(w); h = abs(h);
	glViewport(x, y, w, h);
}


void GLRenderer::ChangeRenderTarget(Bitmap* bmap)
{
	DeviceInfo*	dinfo = (DeviceInfo*) m_Scene->GetDevInfo();

	if (bmap == NULL)
	{
		m_PBuffer.Free();
#ifdef _WIN32
		if ((dinfo->DC == NULL) && dinfo->WinHandle)
			dinfo->DC = ::GetDC(dinfo->WinHandle);
#endif
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
		UpdateTexture(&image, 0, true);
		m_PBuffer.SetDev(dinfo);
		m_PBuffer.SetSize(bmap->Width, bmap->Height);
	}
	SetViewport(0, 0, float(bmap->Width), float(bmap->Height));
	SetChanged(false);
}

static int32 glopts[4] = { 0, GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT,
							GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT };

/****
 *
 * GLRenderer::Begin
 *
 *	Called internally during scene traversal at the beginning
 *	of each frame. Establishes initial conditions for
 *	transformation matrix, fog, default light, camera projection.
 *
 ****/
void GLRenderer::Begin(int changed, int frame)
{
	const Col4&		bc = m_Scene->GetBackColor();
	Camera*			cam = m_Scene->GetCamera();
	Box3			viewvol(cam->GetViewVol());
	Vec3			c = cam->GetCenter();
	int				opts = m_Scene->GetOptions();
	Matrix			proj;
	float			f = (float) (unsigned char) (m_Scene->GetTLS()->Frame & 0xFF) / 255.0f;

	m_Changed = changed;
	SelectGLContext(changed);
	GeoSorter::Begin(changed, frame);
//	glClearColor(bc.r, bc.g, bc.b, bc.a);
	glClearColor(f, bc.g, bc.b, bc.a);
	glClear(glopts[opts & Scene::CLEARALL]);	
	m_ViewMatrix.Copy(m_Scene->GetCamera()->GetViewTrans());
	if (changed & SCENE_CameraChanged)
	{
		switch (cam->GetType())
		{
			case Camera::PERSPECTIVE:
			cam->ProjPerspective(viewvol, m_ProjMatrix);
			break;
	
			case Camera::ORTHOGRAPHIC:
			cam->ProjOrtho(viewvol, m_ProjMatrix);
			break;
		}
	}
#ifdef _DEBUG
	GLint err = glGetError();
	if (err)
		VX_WARNING(("GLRenderer::Begin GL ERROR %x\n", err));
#endif
	if (m_LightList.LightsChanged() != 0)
		m_Changed |= SCENE_LightsChanged;
	if (m_LightShaderChanged || (m_LightList.GetNumLights() != NumLights))
	{
		if (m_LightList.GetNumLights() > 0)
			NumLights = LoadLights();
		else
			NumLights = 0;
		GLShader::ReleasePixelShaders();
	}
}

void GLRenderer::InitPerFrame(intptr program, int changed)
{
	GLuint	glp = (GLuint) program;
	GLint	loc;
	Vec3	c;

	if (m_CurProgram == 0)
		return;
	if (changed & SCENE_CameraChanged)
	{
		loc = glGetUniformLocation(glp, TEXT("ProjMatrix"));
		if (loc >= 0)
			glUniformMatrix4fv(loc, 1, false, m_ProjMatrix.GetMatrix());
	}
	if (changed & SCENE_WindowChanged)
	{
		loc = glGetUniformLocation(glp, TEXT("ImageSize"));
		if (loc > 0)
			glUniform2i(loc, BackWidth, BackHeight);
	}
	if (changed & SCENE_LightsChanged)
	{
		LightList::Iter iter(m_LightList);
		GLLight*	l;
		bool		enabled;

		loc = glGetUniformLocation(glp, TEXT("LightsEnabled"));
		if (loc >= 0)
			glUniform1iv(loc, LIGHT_MaxLights, LightsEnabled);
		loc = glGetUniformLocation(glp, TEXT("NumLights"));
		if (loc >= 0)
			glUniform1i(loc, NumLights);
		while (l = (GLLight*) iter.NextProp(enabled))
		{
			Core::String lname(TEXT("light"));

			lname += Core::String(l->ID) + TEXT('.');
			GLBuffer::Update(l, glp, lname);
		}
	}
	loc = glGetUniformLocation(glp, TEXT("ViewMatrix"));
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, false, m_ViewMatrix.GetMatrix());
	loc = glGetUniformLocation(glp, TEXT("CameraPos"));
	m_ViewMatrix.GetTranslation(c);
	if (loc >= 0)
		glUniform3f(loc, c.x, c.y, c.z);
#ifdef _DEBUG
	GLenum err = glGetError();
	if (err && (GLRenderer::Debug > 1))
		VX_ERROR_RETURN(("GLRender::InitPerFrame failed to update some elements %x\n", err));
#endif
}

void GLRenderer::RenderState(int32 stateindex)
{
	if (stateindex == 0)		// state 0 is transparent stuff
	{
		glDepthMask(false);		// disable Z buffering
		glEnable(GL_BLEND);		// enable alpha blending
	}
	else						// other states are opaque
	{
		glDepthMask(true);		// enable Z buffering
		glDisable(GL_BLEND);	// disable blending
	}
	GeoSorter::RenderState(stateindex);
}

#ifndef VIXEN_GLES2
void	GLRenderer::PBuffer::SetDev(DeviceInfo* d)
{
	m_DevInfo = d;
}

void GLRenderer::PBuffer::Free()
{
	HPBUFFER pbuf = (HPBUFFER) m_Buffer;
	if (pbuf == NULL)
		return;
	DeviceInfo* di = m_DevInfo;
	glReleasePbufferDC(pbuf, di->DC);
	glDestroyPbuffer(pbuf);
	m_Buffer = NULL;
	di->DC = NULL;
}

bool GLRenderer::PBuffer::SetSize(int width, int height)
{
	DeviceInfo*	devinfo = m_DevInfo;
	HPBUFFER	pbuf = (HPBUFFER) m_Buffer;

	if (m_DevInfo == NULL)
		return false;
	if (pbuf)					// already have a PBuffer
	{
		int	w, h;
		glQueryPbuffer(pbuf, WGL_PBUFFER_WIDTH_ARB, &w);
		glQueryPbuffer(pbuf, WGL_PBUFFER_HEIGHT_ARB, &h);
		if ((w == width) && (h == height))
			return true;			// dimensions match, use the buffer
		glReleasePbufferDC(pbuf, devinfo->DC);
		glDestroyPbuffer(pbuf);
		m_Buffer = NULL;
		devinfo->DC = NULL;
	}

	int		iattrs[] = { WGL_SUPPORT_OPENGL_EXT, TRUE,
						 WGL_DRAW_TO_PBUFFER_ARB, TRUE,
						 WGL_ACCELERATION_EXT, WGL_FULL_ACCELERATION_EXT,
						 WGL_PIXEL_TYPE_EXT, WGL_TYPE_RGBA_EXT, 0 };
	float	fattrs[] = { 0 };
	int		pattrs[] = { 0 };
	int		pixformat[8];
	UINT	nformats;
	
	if (!glChoosePixelFormat(devinfo->DC, iattrs, fattrs, 8, pixformat, &nformats))
		VX_ERROR(("Scene::SetColorBuffer display device does not support this format\n"), false);	
	m_Buffer = (intptr) glCreatePbuffer(devinfo->DC, pixformat[0], width, height, pattrs);
	devinfo->DC = glGetPbufferDC((HPBUFFER) m_Buffer);
	return true;
}


#endif


}	// end Vixen