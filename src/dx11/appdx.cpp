
#include "dx11/vixendx11.h"

namespace Vixen
{

/*
 * Encapsulates DX11 render state handling.
 * This class is not persistent. It is instantiated
 * to update the DX11 render state based on a Vixen Appearance
 * by selecting the DX11 raster state, blend state, sampler state
 * and textures. This class also constructs the DX11 pixel shader.
 *
 * The Vixen Appearance attributes are mapped to a fixed set of
 * DX11 render, blend and sampler states. The Appearance is used
 * to select from among them at run time, creating those that are
 * necessary dynamically.
 */
class DXAppearance
{
public:
	DXAppearance(const Appearance* source = NULL);

	// Update DX11 render state based on Vixen Appearance and vertex style.
	void		Update(DXRenderer* r, int vertex_style);

protected:
	// Create and/or select the DX11 raster state.
	ID3D11RasterizerState*	LoadRasterState(DXRenderer* r);

	// Create and/or select the DX11 blend state.
	ID3D11BlendState*		LoadBlendState(DXRenderer* r);

	// Create and/or select the DX11 sampler states and textures.
	D3DSAMPLER*				LoadSamplerState(D3DDEVICE* device, const Sampler* sampler);

	const Appearance*	Source;			// Vixen Appearance to load render state from

	static DXRef<D3DSAMPLER>			SamplerState[8];
	static DXRef<D3DSHADERVIEW>			SamplerView[8];
	static DXRef<ID3D11RasterizerState>	RasterState[16];
	static DXRef<ID3D11BlendState>		BlendState[2];
};

/*
 * DX11 blend states.
 * There are two states - one for opaque primitives with no alpha blending
 * and one for transparent primitives with alpha blending enabled on the first render target.
 */
DXRef<ID3D11BlendState>			DXAppearance::BlendState[2];

/*
 * The DX11 raster states controlled by the Vixen Appearance are
 * culling, transparency, z buffering and solid/wireframe/points.
 * These are combined at run time to select one of 16 different states.
 */
DXRef<ID3D11RasterizerState>	DXAppearance::RasterState[16];

/*
 * The DX11 sampler states are selected based on the filtering
 * attributes in the Vixen Sampler object.
 */
DXRef<D3DSAMPLER>				DXAppearance::SamplerState[8];

#pragma region Statics
static D3D11_RASTERIZER_DESC AppRasterDefault = 
{
	D3D11_FILL_SOLID,	// FillMode
	D3D11_CULL_NONE,	// CullMode
	FALSE,				// FrontCounterClockwise
	0,					// DepthBias
	0.0f,				// DepthBiasClamp
	0.0f,				// SlopeScaledDepthBias
	TRUE,				// DepthClipEnable
	FALSE,				// ScissorEnable
	FALSE,				// MultisampleEnable
	FALSE,				// AntialiasedLineEnable
};

/*
 * Default DX11 blend state - blending
 * disabled for all render targets.
 */
static D3D11_BLEND_DESC AppBlendDefault = 
{
	FALSE,				// AlphaToCoverageEnable
	FALSE,				// IndependentBlendEnable
// Render Target 0
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 1
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 2
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 3
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 4
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 5
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 6
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
// Render Target 7
	FALSE,
	D3D11_BLEND_ONE,	// SrcBlend
	D3D11_BLEND_ZERO,	// DestBlend
	D3D11_BLEND_OP_ADD,	// BlendOp
	D3D11_BLEND_ONE,	// SrcBlendAlpha
	D3D11_BLEND_ZERO,	// DestBlendAlpha
	D3D11_BLEND_OP_ADD,	// BlendOpAlpha
	D3D11_COLOR_WRITE_ENABLE_ALL,
};

/*
 * Mapping of Vixen Sampler filtering attributes to DX11 filter values.
 */
static D3D11_FILTER dxfilter[] = {
	D3D11_FILTER_MIN_MAG_MIP_POINT,					// 000 min nearest, mag nearest
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,			// 001 min linear, mag nearest
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,			// 010 min nearest, mag nearest, interpolate
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,	// 011 min linear, mag nearest, interpolate
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,	// 100 min nearest, mag linear
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,			// 101 min linear, mag linear
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,			// 110 min nearest, mag linear, interpolate
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,				// 111 min linear, mag linear, interpolate
};

/*
 * Default DX11 sampler descriptor.
 */
D3D11_SAMPLER_DESC AppSamplerDefault = {
    D3D11_FILTER_MIN_MAG_MIP_POINT,	// Filter
    D3D11_TEXTURE_ADDRESS_WRAP,		// AddressU
    D3D11_TEXTURE_ADDRESS_WRAP,		// AddressV
    D3D11_TEXTURE_ADDRESS_CLAMP,	// AddressW
    0.0f,							// MipLODBias
    16,								// MaxAnisotropy
    D3D11_COMPARISON_NEVER,			// ComparisonFunc
	{ 0.0f, 0.0f, 0.0f, 0.0f },		// BorderColor
    0,								// MinLOD
    FLT_MAX,						// MaxLOD
};

#pragma endregion

DXAppearance::DXAppearance(const Appearance* app)
  :	Source(app)
{
	for (int i = 0; i < IMAGE_MaxTexUnits; ++i)
		SamplerState[i] = 0;
}

/*
 * Selects the DX11 rasterizer state to use for the Vixen Appearance.
 * The DX11 raster states controlled by the Vixen Appearance are
 * culling, transparency, z buffering and solid/wireframe/points.
 * These are combined at run time to select one of 16 different DX11 rasterizer states.
 * Only those rasterizer states which are actually used at run time are created.
 */
ID3D11RasterizerState* DXAppearance::LoadRasterState(DXRenderer* render)
{
	D3DDEVICE*				device = render->GetDevice();
	D3DCONTEXT*				ctx = render->GetContext();
	int						opts = 1;
	int						fillmode = Source->Get(Appearance::SHADING);
	ID3D11RasterizerState*	rstate;

	/*
	 * Accumulate the appearance options in a 0-based index
	 * into a table of precomputed raster states
	 * Culling		0001
	 * Wireframe	0010
	 * Points		0110
	 * Transparent	1000
	 */
	if (Source)
	{
		opts = Source->Get(Appearance::CULLING);
		if (fillmode == Appearance::WIRE)
			opts |= 4;
		else if (fillmode == Appearance::POINTS)
			opts |= 5;
	}
	if (render->RenderStateIndex == 0)
		opts |= 8;
	VX_ASSERT(opts < 16);
	rstate = RasterState[opts];
	if (rstate == NULL)
	{
		D3D11_RASTERIZER_DESC rdesc = AppRasterDefault;

//		rdesc.DepthClipEnable = Source->Get(Appearance::ZBUFFER);
		rdesc.DepthClipEnable = (render->RenderStateIndex > 0) ? 1 : 0;
		rdesc.CullMode = (opts & 1) ? D3D11_CULL_BACK : D3D11_CULL_NONE;
		switch (fillmode)   
		{
			default:
			rdesc.FillMode = D3D11_FILL_SOLID;
			break;

			case Appearance::WIRE:
			rdesc.FillMode = D3D11_FILL_WIREFRAME;
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;

			case Appearance::POINTS:
			rdesc.FillMode = D3D11_FILL_WIREFRAME;
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			break;
		}
		HRESULT hr = device->CreateRasterizerState(&rdesc, &rstate);
		VX_TRACE(DXRenderer::Debug, ("DXAppearance::LoadRasterState creating rasterizer state\n"));
		if (FAILED(hr))
			{ VX_ERROR(("DXAppearance::LoadRasterState ERROR cannot create DX raster state\n"), NULL); }
		RasterState[opts] = rstate;
	}
	if (render->RasterState != rstate)
	{
		VX_TRACE2(DXRenderer::Debug, ("DXAppearance: setting rasterizer state\n"));
		ctx->RSSetState(rstate);
		render->RasterState = rstate;
		switch (fillmode)   
		{
			default:					ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); break;
			case Appearance::WIRE:		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); break;
			case Appearance::POINTS:	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); break;
		}
	}
	return rstate;
}

/*
 * Selects DX11 blend state for use with this Vixen Appearance.
 * There are two states - one for opaque primitives with no alpha blending
 * and one for transparent primitives with alpha blending enabled on the first render target.
 *
 * The DXRenderer sorts all primitives based on Appearance. All of the opaque primitives
 * are rendered first, followed by the transparent ones sorted in Z order.
 */
ID3D11BlendState*	DXAppearance::LoadBlendState(DXRenderer* render)
{
	D3DDEVICE*			device = render->GetDevice();
	D3DCONTEXT*			ctx = render->GetContext();
	ID3D11BlendState*	bstate;
	float				blendfactor[4] = { 0, 0, 0, 0 };
	int					ofs = (render->RenderStateIndex == 0) ? 1 : 0;
	bool				useblendcolor = false;

	bstate = BlendState[ofs];
	if (bstate == NULL)
	{
		D3D11_BLEND_DESC bdesc = AppBlendDefault;

		if (ofs)			// transparent, enable blending on render target 0
		{
			bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			bdesc.RenderTarget[0].BlendEnable = TRUE;
		}
		else				// do not enable blending
		{
			bdesc.RenderTarget[0].BlendEnable = FALSE;
			bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			bdesc.RenderTarget[0].BlendEnable = FALSE;	
		}
		HRESULT hr = device->CreateBlendState(&bdesc, &bstate);
		VX_TRACE(DXRenderer::Debug, ("DXAppearance::LoadBlendState creating blend state\n"));
		if (FAILED(hr))
			{ VX_ERROR(("DXAppearance::LoadBlendState ERROR cannot create DX sampler state\n"), NULL); }
		BlendState[ofs] = bstate;
	}
	if (Source && ofs)
	{
		const PhongMaterial* mtl = (const PhongMaterial*) Source->GetMaterial();
		if (mtl && mtl->IsClass(VX_PhongMaterial))
		{
			const Col4& c = mtl->GetDiffuse();
			blendfactor[0] = c.r;
			blendfactor[1] = c.g;
			blendfactor[2] = c.b;
			blendfactor[3] = c.a;
			useblendcolor = true;
		}
	}
	if (useblendcolor || (render->BlendState != bstate))
	{
		render->BlendState = bstate;
		ctx->OMSetBlendState(bstate, blendfactor, 0xffffffff);
		VX_TRACE2(DXRenderer::Debug, ("DXAppearance: setting blend state\n"));
	}
	return bstate;
}

/*
 * Selects the DX11 sampler state to use with the given Vixen Sampler.
 * The DX11 sampler states are selected based on the filtering
 * attributes in the Vixen Sampler object.
 * These are combined at run time to select one of 8 different DX11 sampler states.
 * Only those sampler states which are actually used at run time are created.
 */
D3DSAMPLER* DXAppearance::LoadSamplerState(D3DDEVICE* device, const Sampler* sampler)
{
	D3D11_SAMPLER_DESC	sdesc = AppSamplerDefault;
	D3DSAMPLER*			sstate;
	int					ofs = 0;
	HRESULT				hr;

	if (sampler->Get(Sampler::MINFILTER))
		ofs = 1;
	if (sampler->Get(Sampler::MAGFILTER))
		ofs = ofs | 2;
	if (sampler->Get(Sampler::MIPMAP))
		ofs = ofs | 4;
	sstate = SamplerState[ofs];
	if (sstate != NULL)
		return sstate;
	sdesc.Filter = dxfilter[ofs];
	hr = device->CreateSamplerState(&sdesc, &sstate);
	if (FAILED(hr))
		{ VX_ERROR(("DXAppearance::LoadSamplerState cannot set DX11 sampler state\n"), 0); }
	VX_TRACE(DXRenderer::Debug, ("DXAppearance: creating sampler state %d\n", ofs));
	SamplerState[ofs] = sstate;
	return sstate;
}

/*
 * Updates the DX11 rasterizer, blend and sampler states for the Vixen Appearance.
 * @param app	Vixen Appearance to load state for.
 * @param verts	Vixen vertex array being renderered.
 *
 * The Vixen Appearance attributes are mapped to a fixed set of
 * DX11 render, blend and sampler states. The Appearance is used
 * to select from among them at run time, creating those that are
 * necessary dynamically.
 *
 * The DXRenderer sorts the primitives being rendered by Appearance so
 * all of the meshes with a given render state are groups and drawn together.
 * This minimizes the number of DX11 calls to manage render state.
 *
 * This function also handles selecting the vertex and pixel shader.
 * Vertex shaders are fixed and based on the format of the input vertex array.
 *
 * If no pixel shader is specified, a default one is chosen based on a Phong shading model.
 * The user can either replace the pixel shader entirely or replace only the surface shader
 * (the part of the shader that produces the color of the surface independent of lighting).
 */
void DXRenderer::UpdateAppearance(const Appearance* app, const VertexArray* verts)
{
	int		vertex_style = 0;

	if (app == NULL)
		app = m_DefaultAppear;
	if (CurrentAppear == app)
		return;
	CurrentAppear = app;
	if (verts)
		vertex_style = verts->GetStyle();
	DXAppearance	dxstate(app);
	dxstate.Update(this, vertex_style);
	app->SetChanged(false);
}

/*
 * Updates the DX11 rasterizer, blend and sampler states for the Vixen Appearance.
 * @param app			Vixen Appearance to load state for.
 * @param vertex_style	style of vertices being rendered.
 *
 * The Vixen Appearance attributes are mapped to a fixed set of
 * DX11 render, blend and sampler states. The Appearance is used
 * to select from among them at run time, creating those that are
 * necessary dynamically.
 *
 * This function also handles selecting the vertex and pixel shader.
 * Vertex shaders are fixed and based on the input vertex style.
 *
 * If no pixel shader is specified, a default one is chosen based on a Phong shading model.
 * In this case, the Material associated with the Appearance must be a PhongMaterial -
 * it cannot have a user-defined format. The user can either replace the pixel shader
 * entirely or replace only the surface shader (the part of the shader that produces
 * the color of the surface independent of lighting). In the latter case,
 * Vixen will still calculate lighting using a Phong model, using the surface shader
 * as part of a pixel shader constructed dynamically.
 *
 * @see VertexPool::SetStyle VertexPool::SetLayout DXRenderer::RenderMesh Appearance::SetPixelShader
 */
void DXAppearance::Update(DXRenderer* render, int vertex_style)
{
	D3DDEVICE*				device = render->GetDevice();
	D3DCONTEXT*				ctx = render->GetContext();
	D3DSAMPLER*				samplers[IMAGE_MaxTexUnits];
	D3DSHADERVIEW*			views[IMAGE_MaxTexUnits];
	const Shader*			pixelshader = NULL;
	DeviceBuffer*			lightbuf = render->GetBuffer(DXRenderer::CBUF_LIGHT);
	bool					hasdiffuse = false;
	bool					hasspecular = false;
	bool					hasnormal = false;
	const TCHAR*			pixshadername = NULL;
	int						numtex = 0;

	/*
	 * Load rasterizer and blend states if necessary
	 */
	LoadRasterState(render);
	LoadBlendState(render);
	samplers[0] = NULL;
	views[0] = NULL;
	/*
	 * Load sampler states and resource views for each texture
	 * This step will create DirectX textures if necessary.
	 * The DX11 shader view for the texture is kept in the DevHandle
	 * field of the Texture object. The sampler state is one of
	 * a set of static samplers provided.
	 * If the texture has not been loaded yet, UpdateTexture will return NULL
	 * and there will be no texture bound to the sampler.
	 */
	for (int i = 0; i < Source->GetSize(); ++i)
	{
		const Sampler*	sampler = Source->GetSampler(i);
		const Texture*	texture = sampler->GetTexture();
		D3DSHADERVIEW*	dxview = NULL;

		VX_ASSERT(sampler);
		samplers[i] = NULL;
		views[i] = NULL;
		++numtex;
		dxview = (D3DSHADERVIEW*) render->UpdateTexture((Texture*) texture, i, sampler->IsMipMap());
		if (dxview)
		{
			D3DSAMPLER*			sstate = LoadSamplerState(device, sampler);
			
			samplers[i] = sstate;
			views[i] = dxview;
			VX_ASSERT(sstate != NULL);
			VX_ASSERT(views[i] != NULL);
		}
		switch (sampler->Get(Sampler::TEXTUREOP))
		{
			case	Sampler::SPECULAR:
			pixshadername = TEXT("PixelPhongSpecular");
			hasspecular = (dxview != NULL);
			VX_TRACE(DXRenderer::Debug && !hasspecular, ("DXAppearance::Update no texture bound to Specular sampler\n"));
			VX_TRACE2(DXRenderer::Debug, ("DXAppearance::Update Setting Specular sampler\n"));
			break;

			case	Sampler::EMISSION:
			pixshadername = TEXT("PixelPhongEmission");
			hasdiffuse = (dxview != NULL);
			VX_TRACE2(DXRenderer::Debug, ("DXAppearance::Update Setting Emission sampler\n"));
			break;

			case	Sampler::DIFFUSE:
			if ((vertex_style & VertexPool::TEXCOORDS) == 0)
				pixshadername = TEXT("PixelPhongNotex");
			else
			{
				pixshadername = TEXT("PixelPhongDiffuse");
				hasdiffuse = (dxview != NULL);
				VX_TRACE(DXRenderer::Debug && !hasdiffuse, ("DXAppearance::Update no texture bound to Diffuse sampler\n"));
				VX_TRACE2(DXRenderer::Debug, ("DXAppearance::Update Setting Diffuse sampler\n"));
			}
			break;

			case	Sampler::BUMP:
			pixshadername = TEXT("PixelPhongBump");
			hasnormal = (dxview != NULL);
			VX_TRACE(DXRenderer::Debug && !hasnormal, ("DXAppearance::Update no texture bound to Bump sampler\n"));
			VX_TRACE2(DXRenderer::Debug, ("DXAppearance::Update Setting Bump sampler\n"));
			break;
		}
	}
	VX_ASSERT(numtex <= IMAGE_MaxTexUnits);
	if (numtex > 0)
	{
		ctx->PSSetSamplers(0, numtex, samplers);
		ctx->PSSetShaderResources(0, numtex, views);
	}
	else
	{
		ctx->PSSetSamplers(0, 1, samplers);
		ctx->PSSetShaderResources(0, 1, views);
	}
	/*
	 * Update material constant buffer. If it is a PhongMaterial,
	 * fill in the HasDiffuseMap, HasSpecularMap and HasNormalMap
	 * flags to indicate which textures are present.
	 */
	const DeviceBuffer*	srcmtl = Source->GetMaterial();

	if (srcmtl)
	{
		D3DBUFFER*	cbuf;

		if (srcmtl->IsClass(VX_PhongMaterial))
		{
			PhongMaterial::ShaderConstants* mtldata;

			DeviceBuffer* mtlbuf = render->GetBuffer(DXRenderer::CBUF_MATERIAL);
			mtldata = (PhongMaterial::ShaderConstants*) mtlbuf->GetData();
			memcpy(mtldata, srcmtl->GetData(), sizeof(PhongMaterial::ShaderConstants));
			mtldata->HasDiffuseMap = hasdiffuse;
			mtldata->HasSpecularMap = hasspecular;
			mtldata->HasNormalMap = hasnormal;
			mtlbuf->SetChanged(true);
			srcmtl = mtlbuf;
		}
		DXConstantBuf::Update(render, srcmtl);
		cbuf = DXConstantBuf::GetBuffer(srcmtl);
		render->GetContext()->PSSetConstantBuffers(DXRenderer::CBUF_MATERIAL, 1, &cbuf);
	}
	/*
	 * Select pixel shader
	 */
	const Shader* src_pixel_shader = Source->GetPixelShader();
	if (src_pixel_shader)
	{
		src_pixel_shader = render->InstallShader(src_pixel_shader);
		pixshadername = src_pixel_shader->GetName();
	}
	else
	{
		 if (pixshadername == NULL)
			if (vertex_style & VertexPool::TEXCOORDS)
				pixshadername = TEXT("PixelPhongDiffuse");
			else
				pixshadername = TEXT("PixelPhongNotex");
		src_pixel_shader = render->FindShader(pixshadername);
	}
	render->CompilePixelShader(src_pixel_shader);
}

}	// end Vixen