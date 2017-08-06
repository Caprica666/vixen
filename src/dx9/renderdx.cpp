#include "vixen.h"
#include "dx9/renderdx.h"
#include "dx9/vbufdx.h"

#define		DXRELEASE(x)	{ if( x != NULL) { x->Release(); x = NULL; } }

namespace Vixen {

IDirect3D9*	DXRenderer::s_D3D = NULL;
D3DCAPS		DXRenderer::d3dCaps;
DWORD		DXRenderer::s_Adapter = 0;
DWORD		DXRenderer::s_MinDepthBits = 16;
DWORD		DXRenderer::s_MinStencilBits = 0;
D3DPRESENT_PARAMETERS DXRenderer::s_Present;
D3DSURFACE* DXRenderer::s_ColorSurface;
D3DSURFACE* DXRenderer::s_DepthSurface;
vint32			DXRenderer::s_MaxAppIndex;		// maximum appearance index
const Appearance*	DXRenderer::s_CurAppear = NULL;

VX_IMPLEMENT_CLASS(DeviceInfo, Property);

Renderer*	Renderer::CreateRenderer(int options)
{
	World3D::Get()->GetMessenger()->SysVecSize = VX_VEC3_SIZE;
	return new DXRenderer();
}

/*!
 * @fn DXRenderer::DXRenderer(DXRenderer& src)
 * This newly constructed scene shares the DirectX resources of the
 * source scene (color buffer, depth buffer, device context).
 * The scene can be directed to render to a texture, allowing you
 * to easily set up environment and shadow mapping. The new scene
 * will be displayed until you call Scene::Append.
 *
 * @see Scene::Append Scene::SetColorBuffer Scene::SetDepthBuffer
 */
DXRenderer::DXRenderer(Scene* scene) : GeoSorter()
{
	MaxVerts = 0;
	MaxLights = 0;
	IsFullScreen = false;
	IsStereo = false;
	FontFormat = D3DFMT_A8R8G8B8;
}

DXRenderer& DXRenderer::operator=(const DXRenderer& src)
{
	Renderer::operator=(src);
	d3dDevice = NULL;
	IsStereo = src.IsStereo;
	if (src.d3dDevice)
	{
		d3dDevice = src.d3dDevice;
		d3dDevice->AddRef();
	}
	m_ColorFormat = src.m_ColorFormat;
	m_DepthFormat = src.m_ColorFormat;
	m_ColorSurface = src.m_ColorSurface;
	m_DepthSurface = src.m_DepthSurface;
	return *this;
}

SharedObj*	DXRenderer::Clone() const
{
	DXRenderer* render = new DXRenderer(m_Scene);
	*render = *this;
	return render;
}

/*!
 * @fn void DXRenderer::Exit()
 * This routine is called when the render thread exits.It releases
 * the DirectX resources attached to the scene.
 *
 * @see Scene::EndThread
 */
void DXRenderer::Exit()
{
	GeoSorter::Exit();
	m_ColorSurface = NULL;
	m_DepthSurface = NULL;
	DXRefPtr::FreeAll();
	if (d3dDevice)
	{
		d3dDevice->Release();
		d3dDevice = NULL;
	}
	if (s_D3D)
	{
		s_D3D->Release();
		s_D3D = NULL;
	}
}


/*!
 * @fn void DXRenderer::End(int frame)
 * Updates the screen with what has been rendered into the back buffer.
 * If multiple scenes share the same DirectX device, only one buffer flip is
 * done per frame.
 */
void DXRenderer::End(int frame)
{
	GeoSorter::End(frame);
	HRESULT	hr = d3dDevice->EndScene();
	VX_ASSERT(hr == S_OK);
	UpdateAppearance(NULL);
}

void DXRenderer::Flip(int frame)
{
	RECT	srcRect;
	RECT	dstRect;
	HRESULT	hr;
	const Box2& vp = m_Scene->GetViewport();

	srcRect.left = (LONG) vp.min.x;
	srcRect.top = (LONG) vp.min.y;
	srcRect.right = (LONG) vp.max.x;
	srcRect.bottom = (LONG) vp.max.y;
	if (srcRect.right > BackWidth)
		srcRect.right = BackWidth;	// don't exceed back buffer size
	if (srcRect.bottom > BackHeight)
		srcRect.bottom = BackHeight;
	dstRect.left = (LONG) vp.min.x;
	dstRect.top = (LONG) vp.min.y;
	dstRect.right = (LONG) vp.max.x;
	dstRect.bottom = (LONG) vp.max.y;
	hr = d3dDevice->Present(&srcRect, &dstRect, NULL, NULL );	// Present the backbuffer contents to the display
	if (hr == 0x80070578)		// Invalid window handle?
		return;					// window was closed
	VX_ASSERT(hr == S_OK);
}

/*!
 * @fn void DXRenderer::Begin(int changed, int frame)
 * Establishes the render targets (color buffer and depth buffer),
 * clears the screen and sets the ambient light. The parent routine
 * handles updating of the projection matrix and viewport if necessary.
 *
 * @see DXRenderer::End DXRenderer::Flip DXRenderer::ChangeRenderTarget Scene::Begin
 */
void DXRenderer::Begin(int changed, int frame) 
{
	HRESULT		hr;
	bool		changetarget = false;
	D3DSURFACE* newbuf;
	Bitmap*		cbuf = (Bitmap*) m_Scene->GetColorBuffer();
	Bitmap*		dbuf = (Bitmap*) m_Scene->GetDepthBuffer();
	const Col4&	bc = m_Scene->GetBackColor();
	bool		fbchanged = (m_Scene->GetChanged() & SCENE_FBChanged) != 0;

/*
 * check if color buffer has changed and if it is in the correct format
 */
	if (cbuf && (fbchanged || cbuf->HasChanged() || (cbuf->Format & Texture::RENDERTARGET) == 0))
	{
		ObjectLock lock(cbuf);
		switch (cbuf->Depth)
		{
			case 0:								// depth unknown, still waiting for load
			break;

			case 32:
			if ((m_ColorFormat == D3DFMT_A8R8G8B8) ||
				(m_ColorFormat == D3DFMT_X8R8G8B8))
			{
				changetarget = true;
				break;
			}
			cbuf->Depth = 24;

			case 24:
			if (m_ColorFormat == D3DFMT_R8G8B8)
			{
				changetarget = true;
				break;
			}
			cbuf->Depth = 16;

			case 16:
			if ((m_ColorFormat == D3DFMT_A1R5G5B5) ||
				(m_ColorFormat == D3DFMT_X1R5G5B5) ||
				(m_ColorFormat == D3DFMT_A4R4G4B4) ||
				(m_ColorFormat == D3DFMT_R5G6B5))
			{
				changetarget = true;
				break;
			}

			default:
			VX_WARNING(("Scene::SetColorBuffer image depth does not match display color buffer"));
		}
		if (changetarget && (newbuf = ChangeRenderTarget(cbuf, m_ColorFormat, D3DUSAGE_RENDERTARGET)))
		{
			m_ColorSurface = newbuf;
			cbuf->SetChanged(false);
		}
	}
/*
 * check if depth buffer has changed and if it is in the correct format
 */
	if (dbuf && (fbchanged || dbuf->HasChanged() || (dbuf->Format & Texture::RENDERTARGET) == 0))
	{
		ObjectLock dlock(dbuf);
		switch (dbuf->Depth)
		{
			case 0:								// depth unknown, still waiting for load
			break;

			case 32:
			if ((m_DepthFormat == D3DFMT_D32) ||
				(m_DepthFormat == D3DFMT_D24S8) ||
				(m_DepthFormat == D3DFMT_D24X8) ||
				(m_DepthFormat == D3DFMT_D24X4S4))
			{
				changetarget = true;
				break;
			}
			dbuf->Depth = 16;

			case 16:
			if ((m_DepthFormat == D3DFMT_D16) ||
				(m_DepthFormat == D3DFMT_D15S1))
			{
				changetarget = true;
				break;
			}

			default:
			VX_WARNING(("Scene::SetDepthBuffer image depth does not match display depth buffer"));
		}
		if (changetarget && (newbuf = ChangeRenderTarget(dbuf, m_DepthFormat, D3DUSAGE_DEPTHSTENCIL)))
		{
			dbuf->SetChanged(false);
			m_DepthSurface = newbuf;
		}
	}
	if (s_ColorSurface != m_ColorSurface)
		hr = d3dDevice->SetRenderTarget(0, m_ColorSurface);
	if (s_DepthSurface != m_DepthSurface)
		hr = d3dDevice->SetDepthStencilSurface(m_DepthSurface);

	uint16	red = uint16(bc.r * 255);
	uint16	green = uint16(bc.g * 255);
	uint16	blue = uint16(bc.b * 255);
	Matrix	m;
	Light*	ambientLight = m_Scene->GetAmbient();
	Camera*	cam = m_Scene->GetCamera();
	D3DCOLOR ambientCol(0);
	D3DCOLOR backCol = D3DCOLOR_XRGB(red, green, blue);

	if (ambientLight)
	{
		const Col4& ac = ambientLight->GetColor();
		uint16	red = uint16(ac.r * 255);
		uint16	green = uint16(ac.g * 255);
		uint16	blue = uint16(ac.b * 255);
		ambientCol = D3DCOLOR_XRGB(red, green, blue);
	}
	if (changed & SCENE_CameraChanged)
		SetProjection();
	SetViewMatrix();
	frame &= 1;
	hr = d3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backCol, 1.0f, 0L );
	hr = d3dDevice->BeginScene();	hr = d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	hr = d3dDevice->SetRenderState(D3DRS_AMBIENT, ambientCol);
	VX_ASSERT(hr == S_OK);
	Renderer::Begin(changed, frame);
}


void	DXRenderer::SetViewport(float l, float t, float r, float b)
{
	if (d3dDevice == 0)
		return;

	D3DVIEWPORT	d3dViewport;
	HRESULT		hr = d3dDevice->GetViewport(&d3dViewport);
	float		w = BackWidth;
	float		h = BackHeight;

	d3dViewport.MinZ = 0.0f;
	d3dViewport.MaxZ = 1.0f;
	d3dViewport.X = (DWORD) l;
	d3dViewport.Width = (DWORD) abs(r - l);
	if (d3dViewport.Width > (DWORD) w)
		d3dViewport.Width = (DWORD) w;
	d3dViewport.Y = (DWORD) t;
	d3dViewport.Height = (DWORD) abs(b - t);
	if (d3dViewport.Height > (DWORD) h)
		d3dViewport.Height = (DWORD) h;
	hr = d3dDevice->SetViewport(&d3dViewport);
	VX_ASSERT(hr == S_OK);
}

/*!
 * @fn LPD3DSURFACE DXRenderer::ChangeRenderTarget(Bitmap* bmap, D3DFORMAT format, DWORD usage)
 * @param bitmap to render into
 * @param format DirectX pixel format of input texture
 * @param usage DirectX usage of render target
 *
 * Called at the start of each frame before this scene is rendered to
 * set the color or depth buffer to render into. If necessary, a DirectX
 * texture is created as the render target and attached to the input image.
 * The DirectX call to change render targets automatically sets the device
 * viewport to match the texture dimensions. These are recorded in the scene
 * viewport when a new texture is created. The RENDERTARGET flag is
 * set in the image format to indicate those images which have become render targets.
 *
 * @return new render target surface if one was created, else NULL
 * 
 * @see Scene::SetColorBuffer Scene::SetDepthBuffer Texture::SetFormat Texture 
 */
D3DSURFACE* DXRenderer::ChangeRenderTarget(Bitmap* bmap, D3DFORMAT format, DWORD usage)
{
	D3DSURFACE*	surface = 0;

	if ((bmap == NULL) || (format == D3DFMT_UNKNOWN))
		return NULL;
/*
 * The device texture handle is stored in the DevHandle area of the 
 * bitmap this image references. It is managed internally as a DxRefPtr
 * (smart pointer to DirectX resources). These get cleaned up when the
 * scene shuts down.
 */
	DXRef<D3DTEXTURE>& texref = (DXRef<D3DTEXTURE>&) bmap->DevHandle;
	D3DTEXTURE* texhandle = *texref;	// get texture handle, if any

	if (!(bmap->Format & Texture::RENDERTARGET) || (texhandle == NULL))
	{
		HRESULT hr = d3dDevice->CreateTexture(bmap->Width, bmap->Height, 1,
							usage, format, D3DPOOL_DEFAULT, &texhandle, NULL);
		if (SUCCEEDED(hr) && SUCCEEDED(texhandle->GetSurfaceLevel(0, &surface)))
		{
			bmap->Format |= Texture::RENDERTARGET;
			bmap->Kill();
			bmap->Type = Bitmap::TEXHANDLE;
			texref = texhandle;
		}
		else
			VX_ERROR(("DIRECTX ERROR: hr = %x", hr), NULL);
	}
	if (texhandle)
	{
		m_Scene->SetViewport(0.0f, float(bmap->Width), 0.0f, float(bmap->Height));
		texhandle->GetSurfaceLevel(0, &surface);
		return surface;
	}
	return NULL;
}

void DXRenderer::SetViewMatrix()
{
	D3DDEVICE* d3dDevice = GetDevice();

	if (!d3dDevice)
		return;
	const Matrix& mtx = *(m_Scene->GetCamera()->GetViewTrans());
	D3DMATRIX view;
	const float (&m)[4][4] = *((const float (*)[4][4]) mtx.GetMatrix());
	view._11 = m[0][0]; view._12 = m[1][0]; view._13 = m[2][0]; view._14 = m[3][0];
	view._21 = m[0][1]; view._22 = m[1][1]; view._23 = m[2][1]; view._24 = m[3][1];
	view._31 = m[0][2]; view._32 = m[1][2]; view._33 = m[2][2]; view._34 = m[3][2];
	view._41 = m[0][3]; view._42 = m[1][3]; view._43 = m[2][3]; view._44 = m[3][3];
	d3dDevice->SetTransform(D3DTS_VIEW, &view);
}

void DXRenderer::SetProjection()
{
	D3DDEVICE*	d3d = GetDevice();
	Camera*		cam = m_Scene->GetCamera();
	const Box3&	b = cam->GetCullVol();
	Matrix		mtx;


	if (!d3d)
		return;
	switch (cam->GetType())
	{
		case Camera::PERSPECTIVE:
		cam->ProjPerspective(b, mtx);
		break;
	
		case Camera::ORTHOGRAPHIC:
		cam->ProjOrtho(b, mtx);
		break;
	}
	const float (&m)[4][4] = *((const float (*)[4][4]) mtx.GetMatrix());
	D3DMATRIX	proj;

	proj._11 = m[0][0]; proj._12 = m[1][0]; proj._13 = m[2][0]; proj._14 = m[3][0];
	proj._21 = m[0][1]; proj._22 = m[1][1]; proj._23 = m[2][1]; proj._24 = m[3][1];
	proj._31 = m[0][2]; proj._32 = m[1][2]; proj._33 = m[2][2]; proj._34 = m[3][2];
	proj._41 = m[0][3]; proj._42 = m[1][3]; proj._43 = m[2][3]; proj._44 = m[3][3];
	d3d->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*) m);
}


}	// end Vixen