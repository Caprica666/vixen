#include "dx11/vixendx11.h"

namespace Vixen {

struct	PerFrameConstants
{
	float	ViewMatrix[16];
	float	ProjMatrix[16];
	Vec4	CameraPos;
	int32	NumLights;
	int32	LightsEnabled;
	Vec2	ImageSize;
};

struct PerObjectConstants
{
	float	WorldMatrix[16];
};

DXGI_FORMAT	DXRenderer::FontFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
Ref<Appearance>	DXRenderer::CurrentAppear;
	
VX_IMPLEMENT_CLASS(DeviceInfo, Property);
static int FindColorDepth(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
		default: return 0;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		return 24;

		case DXGI_FORMAT_B5G6R5_UNORM: 
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;
	}
}

Renderer*	Renderer::CreateRenderer(int options)
{
	World3D::Get()->GetMessenger()->SysVecSize = VX_VEC3_SIZE;
	return new DXRenderer();
}

/*
 * Create DXRenderer to encapsulate DX11 rendering functionality.
 * Creates Vixen data structures used during DX11 rendering.
 */
DXRenderer::DXRenderer()
  :	GeoSorter()
{
	VertexArray::VertexAlloc = new Core::AlignedAllocator(128L);
	BackWidth = 0;
	BackHeight = 0;
	m_ConstantBuffers[CBUF_PERFRAME] = new DeviceBuffer(TEXT("float16 ViewMatrix, float16 ProjMatrix. float3 CameraPos"), sizeof(PerFrameConstants));
	m_ConstantBuffers[CBUF_PERFRAME]->SetName(TEXT("PerFrame"));
	m_ConstantBuffers[CBUF_PEROBJECT] = new DeviceBuffer(TEXT("float16 WorldMatrix"), sizeof(PerObjectConstants));
	m_ConstantBuffers[CBUF_PEROBJECT]->SetName(TEXT("PerObject"));
	m_ConstantBuffers[CBUF_MATERIAL] = new DeviceBuffer(TEXT("float4 Diffuse, float4 Ambient, float4 Specular, float4 Emission,	float Shine, int HasNormalMap, int HasDiffuseMap"), sizeof(PhongMaterial::ShaderConstants));
	m_ConstantBuffers[CBUF_MATERIAL]->SetName(TEXT("PhongMaterial"));
	m_ConstantBuffers[CBUF_LIGHT] = NULL;
	m_DefaultAppear = new Appearance();
}

DXRenderer& DXRenderer::operator=(const DXRenderer& src)
{
	GeoSorter::operator=(src);
	m_Device = NULL;
	if (src.m_Device)
	{
		m_Device = src.m_Device;
		m_Device->AddRef();
	}
	m_ColorFormat = src.m_ColorFormat;
	m_DepthFormat = src.m_ColorFormat;
	m_ColorSurface = src.m_ColorSurface;
	m_DepthSurface = src.m_DepthSurface;
	for (int i =0; i <= CBUF_MAX; ++i)
		m_ConstantBuffers[i] = src.m_ConstantBuffers[i];
	return *this;
}

SharedObj*	DXRenderer::Clone() const
{
	DXRenderer* render = new DXRenderer();
	*render = *this;
	return render;
}


/*!
 * @fn void DXRenderer::Exit(bool shutdown)
 * bool shutdown	true if Vixen is in final shutdown
 *
 * This routine is called when the render thread exits.
 * It releases the DirectX resources attached to this renderer.
 * During multi-threaded operation, it may be called twice.
 * Only the last call should set shutdown to true.
 *
 * @see Scene::EndThread
 */
void DXRenderer::Exit(bool shutdown)
{
	CurrentAppear = NULL;
	GeoSorter::Exit();
	m_ColorSurface = NULL;
	m_DepthSurface = NULL;
	if (shutdown)
	{
		DXShader::ShaderDict.Empty();
		DXRefPtr::FreeAll();
		FreeResources();
	}
}

/*
 * Sets the DX11 viewport.
 * @param l		leftmost X coordinate
 * @param t		topmost Y coordinate
 * @param r		rightmost X coordinate
 * @param b		bottom Y coordinate
 */
void	DXRenderer::SetViewport(float l, float t, float r, float b)
{
	D3D11_VIEWPORT	d3dViewport;
	bool			resize = false;
	HRESULT			hr;
	float			w = r - l;
	float			h = b - t;

	if (m_Device == 0)
		return;
	VX_ASSERT((l >= 0) && (t >= 0));
	VX_ASSERT((b > t) && (r > l));
	d3dViewport.MinDepth = 0;
	d3dViewport.MaxDepth = 1.0f;
	d3dViewport.TopLeftX = l;
	d3dViewport.Width = w;
	if (d3dViewport.Width > BackWidth)
		resize = true;
	d3dViewport.TopLeftY = t;
	d3dViewport.Height = h;
	if (d3dViewport.Height > BackHeight)
		resize = true;
	m_Context->RSSetViewports(1, &d3dViewport);
	if (resize)
	{
		FreeBuffers();
		hr = m_SwapChain->ResizeBuffers(1, d3dViewport.Width, d3dViewport.Height, m_ColorFormat, 0);
		if (FAILED(hr))
			{ VX_ERROR_RETURN(("Scene::SetViewport cannot resize DX buffers\n")); }
		CreateBuffers(d3dViewport.Width, d3dViewport.Height);
	}
}

/*
 * Called to flip the DX11 buffers (Present)
 */
void DXRenderer::Flip(int frame)
{
	const Box2&	vport = m_Scene->GetViewport();
	RECT	srcRect;
	RECT	dstRect;
	HRESULT	hr;

	srcRect.left = (LONG) vport.min.x;
	srcRect.top = (LONG) vport.min.y;
	srcRect.right = (LONG) vport.max.x;
	srcRect.bottom = (LONG) vport.max.y;
	if (srcRect.right > BackWidth)
		srcRect.right = BackWidth;	// don't exceed back buffer size
	if (srcRect.bottom > BackHeight)
		srcRect.bottom = BackHeight;
	dstRect.left = (LONG) vport.min.x;
	dstRect.top = (LONG) vport.min.y;
	dstRect.right = (LONG) vport.max.x;
	dstRect.bottom = (LONG) vport.max.y;
	if (m_SwapChain == NULL)
		return;
	hr = m_SwapChain->Present(0, 0);	// Present the backbuffer contents to the display
	if (hr == 0x80070578)		// Invalid window handle?
		return;					// window was closed
	VX_ASSERT(hr == S_OK);
}

/*
 * Called to change the DX11 render target.
 * Will cause DX11 to render to a texture associated with the given Vixen bitmap.
 * @param bmap		Vixen bitmap to render to
 * @param format	DXGI_FORMAT format of render target
 * @param type		type of DX11 target (D3D11_BIND_DEPTH_STENCIL or D3D11_BIND_RENDER_TARGET)
 *
 * @return DX11 view for surface
 */
ID3D11View* DXRenderer::ChangeRenderTarget(Bitmap* bmap, DXGI_FORMAT format, D3D11_BIND_FLAG type)
{
	ID3D11View*	surface = NULL;

	if ((bmap == NULL) || (format == DXGI_FORMAT_UNKNOWN))
		return NULL;
/*
 * The device texture handle is stored in the DevHandle area of the 
 * bitmap this image references. It is managed internally as a DxRefPtr
 * (smart pointer to DirectX resources). These get cleaned up when the
 * scene shuts down.
 */
	D3DTEXTURE2D* texhandle = (D3DTEXTURE2D*) bmap->DevHandle;
	HRESULT hr;

	if (!(bmap->Format & Texture::RENDERTARGET) || (texhandle == NULL))
	{
		D3D11_TEXTURE2D_DESC texdesc;

		texdesc.Width = bmap->Width;
		texdesc.Height = bmap->Height;
		texdesc.MipLevels = 1;
		texdesc.ArraySize = 1;
		texdesc.Format = format;
		texdesc.SampleDesc.Count = 1;
		texdesc.SampleDesc.Quality = 0;
		texdesc.Usage = D3D11_USAGE_DEFAULT;
		texdesc.BindFlags = type;
		texdesc.CPUAccessFlags = 0;
		texdesc.MiscFlags = 0;
		hr = m_Device->CreateTexture2D(&texdesc, NULL, &texhandle);
		if (SUCCEEDED(hr))
		{
			if (type == D3D11_BIND_DEPTH_STENCIL)
				hr = m_Device->CreateDepthStencilView(texhandle, NULL, (ID3D11DepthStencilView**) &surface);
			else if (type == D3D11_BIND_RENDER_TARGET)
				hr = m_Device->CreateRenderTargetView(texhandle, NULL, (ID3D11RenderTargetView**) &surface);
			else
				return NULL;
		}
		if (SUCCEEDED(hr))
		{
			bmap->Format |= Texture::RENDERTARGET;
			bmap->Kill();
			bmap->Type = Bitmap::TEXHANDLE;
			bmap->DevHandle = (intptr) texhandle;
		}
	}
	if (FAILED(hr))
		{ VX_ERROR(("DXRenderer::ChangeRenderTarget cannot create DX render target view\n"), NULL); }
	if (texhandle)
		SetViewport(0, bmap->Width, 0, bmap->Height);
	return surface;
}


void DXRenderer::Begin(int changed, int frame)
{
	bool			changetarget = false;
	ID3D11View*		newbuf;
	Bitmap*			cbuf = m_Scene->GetColorBuffer();
	Bitmap*			dbuf = m_Scene->GetDepthBuffer();
	bool			fbchanged = (changed & SCENE_FBChanged) != 0;
	Col4			backcolor(m_Scene->GetBackColor());
	ID3D11RenderTargetView* cview = m_ColorSurface;
	ID3D11DepthStencilView* dview = m_DepthSurface;

/*
 * check if color buffer has changed and if it is in the correct format
 */
	if (cbuf && (fbchanged || cbuf->HasChanged() || (cbuf->Format & Texture::RENDERTARGET) == 0))
	{
		ObjectLock lock(cbuf);
		if (cbuf->Depth == FindColorDepth(m_ColorFormat))
			changetarget = true;
		else
			{ VX_WARNING(("Scene::SetColorBuffer image depth does not match display color buffer\n")); }
		if (changetarget && (newbuf = ChangeRenderTarget(cbuf, m_ColorFormat, D3D11_BIND_RENDER_TARGET)))
		{
			m_ColorSurface = (ID3D11RenderTargetView*) newbuf;
			cbuf->SetChanged(false);
		}
	}
/*
 * check if depth buffer has changed and if it is in the correct format
 */
	if (dbuf && (fbchanged || dbuf->HasChanged() || (dbuf->Format & Texture::RENDERTARGET) == 0))
	{
		ObjectLock dlock(dbuf);
		if (dbuf->Depth)
			changetarget = true;
		else
			changetarget = false;
		if (changetarget && (newbuf = ChangeRenderTarget(dbuf, m_DepthFormat, D3D11_BIND_DEPTH_STENCIL)))
		{
			dbuf->SetChanged(false);
			m_DepthSurface = (ID3D11DepthStencilView*) newbuf;
		}
	}
	/*
	 * If the color or depth buffer has changed since the last scene,
	 * update the DirectX device to use the new one
	 */
	changetarget = false;	
	if ((ID3D11RenderTargetView*) m_ColorSurface != m_ColorSurface)
		changetarget = true;
	if ((ID3D11DepthStencilView*) m_DepthSurface != m_DepthSurface)
		changetarget = true;
	if (changetarget)
		m_Context->OMSetRenderTargets(1, &cview, dview);
	/*
	 * Load the constant buffers with the view and projection matrix
	 */
	Camera*			cam = m_Scene->GetCamera();
	Matrix			view(*cam->GetViewTrans());
	Matrix			proj;
	Box3			viewvol(cam->GetViewVol());
	DeviceBuffer*	constbuf = m_ConstantBuffers[CBUF_PERFRAME];
	PerFrameConstants* cdata = (PerFrameConstants*) constbuf->GetData();

	switch (cam->GetType())
	{
		case Camera::PERSPECTIVE:
		cam->ProjPerspective(viewvol, proj);
		break;
	
		case Camera::ORTHOGRAPHIC:
		cam->ProjOrtho(viewvol, proj);
		break;
	}
	GeoSorter::Begin(changed, frame);
	m_Context->ClearRenderTargetView(cview, (const FLOAT*) &backcolor);
	m_Context->ClearDepthStencilView(dview, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	cdata->ImageSize.x = BackWidth;
	cdata->ImageSize.y = BackHeight;
	memcpy(cdata->ProjMatrix, proj.GetMatrix(), 16 * sizeof(float));
	memcpy(cdata->ViewMatrix, view.GetMatrix(), 16 * sizeof(float));
	cdata->CameraPos = cam->GetCenter();
	cdata->LightsEnabled = m_LightList.LightsEnabled() & m_LightList.LightsActive();
	cdata->NumLights = LoadLights();
	constbuf->SetChanged(true);
	CurrentAppear = NULL;
	/*
	 * Copy constant buffers that changed this frame to DX
	 */
	D3DBUFFER*	buflist[CBUF_MAX + 1];

	for (int i = 0; i <= CBUF_MAX; ++i)
	{
		constbuf = m_ConstantBuffers[i];
		if (constbuf && constbuf->HasChanged())
			DXConstantBuf::Update(this, constbuf);
		buflist[i] = DXConstantBuf::GetBuffer(constbuf);
	}
	m_Context->PSSetConstantBuffers(0, 4, buflist);
	m_Context->VSSetConstantBuffers(0, 2, buflist);
}

/*
 * Primitives are separated according to render state.
 * Opaque primitives are sorted by appearance.
 * Z buffering is enabled when they are rendered and alpha blending is not.
 * Transparent primitives are sorted by the Z value of their center.
 * Z buffering is disabled and alpha blending is enabled when they are rendered.
 */
void DXRenderer::RenderState(int32 stateindex)
{
	RenderStateIndex = stateindex;
	GeoSorter::RenderState(stateindex);
}

/*
 * Render a mesh with a given appearance and transformation matrix.
 * @param geo	TriMesh to render. Only triangle meshes are supported currently.
 * @param app	Appearance to use for rendering. If NULL, a default appearance is used.
 * @param mtx	Matrix to use to transform the mesh vertices into world space.
 *
 * If the Vixen vertex and/or index buffers associated with the mesh have changed,
 * they are copied to the DX11 equivalents. The input layout of the vertices
 * determines which vertex shader will be used (no user-programmable vertex shaders).
 *
 * The user may supply a pixel shader or Vixen will default to a built-in Phong shader.
 * In the default case, the Material associated with a Vixen Appearance must be a PhongMaterial
 * to supply the ambient, diffuse and specular components. For user-supplied pixel shaders,
 * the Material format is defined by the user and passed as a constant buffer to the shader.
 */
void	DXRenderer::RenderMesh(const Geometry* geo, const Appearance* app, const Matrix* mtx)
{
	VX_ASSERT(geo->IsClass(VX_Mesh));

	UINT				nidx = 0;
	const Mesh*			mesh = (Mesh*) geo;
	const VertexArray*	verts = mesh->GetVertices();
	const IndexArray*	inds = mesh->GetIndices();

	if (verts == NULL)
		return;
	/*
	 * Update vertex and index buffers if changed
	 */
	DXVertexBuf&	vbuf = (DXVertexBuf&) verts->DevHandle;
	bool			mesh_changed = geo->HasChanged();
	bool			app_changed = app->HasChanged();
	UINT			stride = verts->GetVtxSize() * sizeof(float);
	UINT			offset = 0;
	D3DBUFFER*	dxbuf;

	/*
	 * Select vertex shader and input layout, supply vertex shader constant buffers
	 */
	Shader*				vxshader = VertexShaders[verts->GetStyle()];
	ID3D11VertexShader*	dxshader = (ID3D11VertexShader*) Compile(vxshader);
	const DataLayout*	layout = vxshader->GetInputLayout();
	DXLayout&			dxlayout = (DXLayout&) layout->DevHandle;
	D3DINPUTLAYOUT*		inlayout = *dxlayout;

	if (dxshader)
		m_Context->VSSetShader(dxshader, NULL, 0);
	else
		return;
	if (inlayout)
	{
		VX_ASSERT(layout == vxshader->GetInputLayout());
		m_Context->IASetInputLayout(inlayout);
		VX_TRACE2(Debug, ("DXRenderer::RenderMesh set input layout %s\n", layout->Descriptor));
	}
	else
		VX_ERROR_RETURN(("DXRenderer::RenderMesh: ERROR no input layout for vertex shader %s\n", vxshader->GetName()));
	/*
	 * Update vertex and index buffers if changed
	 */
	if (!vbuf.HasBuffer() || verts->HasChanged())
	{
		vbuf.Update(this, verts, verts->IsSet(VertexPool::MORPH));
		verts->SetChanged(false);
	}
	dxbuf = *vbuf;
	m_Context->IASetVertexBuffers(0, 1, &dxbuf, &stride, &offset);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (inds)
	{
		DXIndexBuf& ibuf = (DXIndexBuf&) geo->DevHandle;
		if (!ibuf.HasBuffer())
		{
			ibuf.Update(this, inds, 0);
			inds->SetChanged(false);
		}
		nidx = inds->GetSize();
		dxbuf = *ibuf;
		if (dxbuf)
			m_Context->IASetIndexBuffer(dxbuf, DXGI_FORMAT_R32_UINT, 0);
	}
	geo->SetChanged(false);
	/*
	 * Update appearance (pixel shader state)
	 */
	UpdateAppearance(app, verts);
	/*
	 * Update constant buffer with render matrix
	 */
	DeviceBuffer*	constbuf = m_ConstantBuffers[CBUF_PEROBJECT];

	if (constbuf)
	{
		constbuf->Set(TEXT("WorldMatrix"), mtx->GetMatrix());
		DXConstantBuf::Update(this, constbuf);
	}
	/*
	 * Draw the mesh
	 */
	UINT	nvtx = verts->GetNumVtx()- mesh->GetStartVtx();

	if (mesh->GetEndVtx() > 0)
		nvtx = mesh->GetEndVtx() - mesh->GetStartVtx() + 1;
	if (nidx)
		m_Context->DrawIndexed(nidx, 0, 0);
	else
		m_Context->Draw(nvtx, mesh->GetStartVtx());	
}

}  // end Vixen


