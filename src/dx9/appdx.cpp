/****
 *
 * class Appearance implementation for Direct3D
 * The Appearance class defines the material and texture properties
 * applied to geometry when displayed. It also contains rendering
 * attributes controlling graphics pipeline behavior.
 *
 ****/
#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen {

inline const TCHAR* DXGetErrorString(UINT hr) { return "DIRECTX ERROR"; }

class DXAppearance
{
public:
	DXAppearance(DXRenderer& render, const Appearance& app)
	  :	m_Appear(&app), m_Render(render) {  }

	DXAppearance(DXRenderer& render)
	  :	m_Appear(NULL), m_Render(render) {  }

	void	Update();
	void	Default();
	static void	SetAlphaBlend(D3DDEVICE* d3d, int blend);

protected:
	bool	UpdateSampler(const Sampler* sampler, int texunit);
	bool	UpdateMaterial(const PhongMaterial* mtl);
	void	DefaultSampler();
	void	DefaultMaterial();

	static	const Material*	s_CurMtl;			// current material
	static	int32			s_CurTexUnit;		// current texture unit
	static	intptr			s_MaxTexID;			// maximum texture ID used by this thread
	static	intptr			s_CurTexID[IMAGE_MaxTexUnits];	// maximum texture ID used by this thread

	const Appearance*	m_Appear;
	DXRenderer&	m_Render;
};

struct D3DState 
{ 
	int32 Type;
	DWORD Val; 
};

// current value of all sampler attributes
static	int32	s_SamplerAttrs[IMAGE_MaxTexUnits][Sampler::NUM_ATTRS];
static	int32	s_AppearAttrs[Appearance::NUM_ATTRS];
const Material*	DXAppearance::s_CurMtl = NULL;
int32			DXAppearance::s_CurTexUnit = 0;
intptr			DXAppearance::s_CurTexID[IMAGE_MaxTexUnits];


static D3DState AppDefault[] = 
{
	D3DRS_FOGENABLE,			FALSE,
	D3DRS_SRCBLEND,				D3DBLEND_SRCALPHA,
	D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA,
	D3DRS_CULLMODE,				D3DCULL_CW,
	D3DRS_ALPHABLENDENABLE,		FALSE,
	D3DRS_ALPHATESTENABLE,		FALSE,  		
	D3DRS_LIGHTING,				TRUE,
	D3DRS_NORMALIZENORMALS,		TRUE,
	D3DRS_ALPHAREF,				0x002,
	D3DRS_ALPHAFUNC,			D3DCMP_GREATEREQUAL,
	D3DRS_FILLMODE,				D3DFILL_SOLID,
};

#define	APPEAR_NumDefault	11
    
static	D3DTRANSFORMSTATETYPE	texstages[8] = {
	D3DTS_TEXTURE0, D3DTS_TEXTURE1, D3DTS_TEXTURE2, D3DTS_TEXTURE3,
	D3DTS_TEXTURE4,	D3DTS_TEXTURE5,	D3DTS_TEXTURE6,	D3DTS_TEXTURE7
};

void	DXRenderer::UpdateAppearance(const Appearance* app)
{
	if (app == NULL)
	{
		DXAppearance default_app(*this);
		s_CurAppear = NULL;
		default_app.Default();
	}
	else
	{
		DXAppearance user_app(*this, *app);
		user_app.Update();
	}
}


void DXAppearance::Default()
{
	D3DDEVICE*	d3d = m_Render.GetDevice();
	int		i;

	DefaultMaterial();
	s_AppearAttrs[Appearance::CULLING] = 1;
	s_AppearAttrs[Appearance::LIGHTING] = 1;
	s_AppearAttrs[Appearance::SHADING] = Appearance::SMOOTH;
	s_AppearAttrs[Appearance::TRANSPARENCY] = false;
#if _TRACE > 1
	if (Appearance::Debug > 2)
	{
		VX_PRINTF(("<appear_default>"));
		VX_PRINTF(("\t<lighting val='1' />"));
		VX_PRINTF(("\t<culling val='1' />"));
		VX_PRINTF(("\t<shading val='smooth' />"));
		VX_PRINTF(("\t<transparency val='0' />"));
		VX_PRINTF(("\t<textureop stage='0' val='modulate' />"));
		VX_PRINTF(("\t<dx_colorarg2 stage='0' val='diffuse' />"));
		VX_PRINTF(("</appear_default>"));
	}
#endif
	if (d3d == NULL)
		return;
	d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	for (i = 0; i < APPEAR_NumDefault; ++i)
		d3d->SetRenderState((D3DRENDERSTATETYPE)AppDefault[i].Type, AppDefault[i].Val);
	DefaultSampler();
}


/*!
 * @fn void DXAppearance::DefaultSampler()
 * Restores the default sampler state in the device.
 * This defines the behavior of an "empty" Sampler object
 * (one that is newly constructed with no attributes set). 
 *
 * @see DXAppearance::DefaultAppearance
 */
void DXAppearance::DefaultSampler()
{
	D3DDEVICE*	d3d = m_Render.GetDevice();
	int		i;

	VX_TRACE(Appearance::Debug > 2, ("<sampler_default>"));
	for (i = 0; i < m_Render.NumTexUnits; ++i)
	{
		s_CurTexID[i] = NULL;
		s_SamplerAttrs[i][Sampler::MINFILTER] = Sampler::LINEAR;
		s_SamplerAttrs[i][Sampler::MAGFILTER] = Sampler::LINEAR;
		s_SamplerAttrs[i][Sampler::TEXTUREOP] = Sampler::DIFFUSE;
		s_SamplerAttrs[i][Sampler::TEXCOORD] = 0;
		s_SamplerAttrs[i][Sampler::MIPMAP] = 0;

#if _TRACE > 1
		if (Appearance::Debug > 2)
		{
			VX_PRINTF(("\t<minfilter stage='%d' value='linear' />", i));
			VX_PRINTF(("\t<magfilter stage='%d' value='linear' />", i, Sampler::LINEAR));
			VX_PRINTF(("\t<texcoord stage='%d' value='0' />", i));
			VX_PRINTF(("\t<mipmap stage='%d' value='0' />", i));
			VX_PRINTF(("\t<dx_colorarg1 stage='%d' val='texture' />", i));
			VX_PRINTF(("\t<dx_alphaarg1 stage='%d' val='texture' />", i));
			VX_PRINTF(("\t<dx_alphaarg2 stage='%d' val='current' />", i));
			VX_PRINTF(("\t<dx_alphaop stage='%d' val='disable' />", i));
		}
#endif
		if (d3d)
		{
			d3d->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3d->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3d->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			d3d->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			d3d->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			d3d->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			d3d->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			d3d->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
			d3d->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			d3d->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);
			d3d->SetTexture(i, NULL);
			d3d->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			if (i > 0)
			{
				d3d->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
				d3d->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
				VX_TRACE(Appearance::Debug > 2, ("\t<textureop stage='%d' val='disable' />", i));
				VX_TRACE(Appearance::Debug > 2, ("\t<dx_colorarg2 stage='%d' val='current' />", i));
			}
		}
	}
	d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
#if _TRACE > 1
	if (Appearance::Debug > 2)
	{
		VX_PRINTF(("\t<textureop stage='0' val='modulate' />", i));
		VX_PRINTF(("\t<dx_colorarg2 stage='0' val='diffuse' />", i));
		VX_PRINTF(("</sampler_default>"));
	}
#endif
}

void DXAppearance::DefaultMaterial()
{
	D3DDEVICE*	d3d = m_Render.GetDevice();
	D3DMATERIAL9 d3dmat;

	s_CurMtl = NULL;
	memset(&d3dmat, 0, sizeof(d3dmat));
	d3dmat.Ambient.r = 0.5f;
	d3dmat.Ambient.g = 0.5f;
	d3dmat.Ambient.b = 0.5f;
	d3dmat.Ambient.a = 1.0f;
	d3dmat.Diffuse.r = 0.5f;
	d3dmat.Diffuse.g = 0.5f;
	d3dmat.Diffuse.b = 0.5f;
	d3dmat.Diffuse.a = 1.0f;
	if (d3d->SetMaterial(&d3dmat) != D3D_OK)
		VX_ERROR_RETURN(("Material::LoadDev failed\n"));
	d3d->SetRenderState(D3DRS_SPECULARENABLE, 0);
}

bool DXAppearance::UpdateMaterial(const PhongMaterial* mtl)
{
	D3DMATERIAL9*	d3dmtl = (D3DMATERIAL9*) mtl->GetData();
	bool			is_transparent = false;
	D3DDEVICE*		d3d = m_Render.GetDevice();
	bool			transparent = d3dmtl->Diffuse.a < 1.0f;

	VX_TRACE2(Appearance::Debug, ("<material_load appid='%p'>", this));
	VX_ASSERT(mtl->IsKindOf(CLASS_(PhongMaterial)));
	mtl->SetChanged(false);
	if (d3d->SetMaterial(d3dmtl) != D3D_OK)
		VX_ERROR(("UpdateMaterial failed\n"), false);
	VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='0' val='diffuse' >"));
	d3d->SetRenderState(D3DRS_SPECULARENABLE, d3dmtl->Power != 0);
	if (transparent)
	{
		if (s_CurTexID[0] != NULL)
		{
			d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			VX_TRACE2(Appearance::Debug, ("\t<dx_alphaop stage='0' val='modulate' />"));
		}
	}
	VX_TRACE2(Appearance::Debug, ("</appear_load>"));
	return transparent;
}

bool DXAppearance::UpdateSampler(const Sampler* sampler, int texunit)
{
	D3DDEVICE*	d3d = m_Render.GetDevice();

	VX_TRACE2(Appearance::Debug, ("<sampler_load stage='%d' >", texunit));
	/*
	 * Load the texture being used if necessary. If the sampler requests
	 * mip-mapping but the texture does not have mip-maps, we turn off mipmapping
	 */
	Texture*	image = (Texture*) sampler->GetTexture();
	bool		hastex = false;
	intptr		prevtex = s_CurTexID[texunit];
	bool		ismipmap = sampler->IsMipMap();
	int			texop = sampler->Get(Sampler::TEXTUREOP);
	static		char*	opnames[4] = { "selectarg1", "modulate", "add", "bump" };
	void*		texhandle;

	if (image && (NULL != (texhandle = m_Render.UpdateTexture(image, texunit, ismipmap))))
	{
		Sampler* cheat = (Sampler*) sampler;

		if (s_CurTexID[texunit] != intptr(texhandle))
		{
			s_CurTexID[texunit] = intptr(texhandle);
			if (d3d->SetTexture(texunit, (D3DTEXTURE*) texhandle) != D3D_OK)
				{ VX_ERROR(("Texture::LoadDev SetTexture failed"), false); }
			VX_TRACE2(Appearance::Debug, ("<image_load stage='%d' file='%s'/>", texunit, image->GetFileName(), this));
			VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg1 stage='%d' val='texture' />", texunit));
			d3d->SetTextureStageState(texunit, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		}
		if (ismipmap && !image->IsMipMap())
			cheat->Set(Sampler::MIPMAP, 0);
		hastex = true;
	}
	if (prevtex != s_CurTexID[texunit])
		s_SamplerAttrs[texunit][Sampler::TEXTUREOP] = -1;	// force TEXTUREOP reload
	/*
	 * Check all sampler attributes against the current state
	 * Only update those that have changed
	 */
	for (int i = 0; i < Sampler::NUM_ATTRS; ++i)
	{
		HRESULT			hr;
		int32			d3dtop;
		int32			filter;
		Core::String	s;
		int				val = sampler->Get(i);

		if (val == s_SamplerAttrs[texunit][i])
			continue;
		s_SamplerAttrs[texunit][i] = val;
		switch (i)
		{
			case Sampler::TEXTUREOP:
			switch (val)
			{
				case Sampler::SPECULAR:
				d3dtop = D3DTOP_ADD;
				VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='%d' val='current' />", texunit));
				d3d->SetTextureStageState(texunit, D3DTSS_COLORARG2, D3DTA_CURRENT);
				break;

				case Sampler::EMISSION:
				d3dtop = D3DTOP_SELECTARG1;
				break;

				default:
				d3dtop = D3DTOP_MODULATE;
				break;
			}
			if (s_CurTexID[texunit] == 0)
				if (texunit == 0)
				{
					VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='%d' val='diffuse' />", texunit));
					d3d->SetTextureStageState(texunit, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
					d3dtop = D3DTOP_SELECTARG2;
				}
				else
				{
					VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='%d' val='current' />", texunit));
					d3d->SetTextureStageState(texunit, D3DTSS_COLORARG2, D3DTA_CURRENT);
					d3dtop = D3DTOP_DISABLE;
				}
			d3d->SetTextureStageState(texunit, D3DTSS_COLOROP, d3dtop);
			VX_TRACE2(Appearance::Debug, ("\t<dx_colorop stage='%d' val='%s' />", texunit, opnames[val]));
			continue;

			case Sampler::MIPMAP:
			VX_TRACE2(Appearance::Debug, ("\t<%s stage='%d' val='%s'/>", sampler->GetAttrName(i), texunit, val ? "linear" : "none"));
			if (val)
				d3d->SetSamplerState(texunit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			else
				d3d->SetSamplerState(texunit, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			break;

			case Sampler::MINFILTER:
			VX_TRACE2(Appearance::Debug, ("\t<%s stage='%d' val='%s'/>", sampler->GetAttrName(i), texunit, val ? "linear" : "nearest"));
			if (val == Sampler::LINEAR)
				filter = D3DTEXF_LINEAR;
			else
				filter = D3DTEXF_POINT;
			if (hr = d3d->SetSamplerState(texunit, D3DSAMP_MINFILTER, filter)!= D3D_OK)
				VX_ERROR(("Appearance::SetDev ERROR in SetTextureStageState %x", hr), hastex);
			break;

			case Sampler::MAGFILTER:
			VX_TRACE2(Appearance::Debug, ("\t<%s stage='%d' val='%s'/>", sampler->GetAttrName(i), texunit, val ? "linear" : "nearest"));
			if (val == Sampler::LINEAR)
				filter = D3DTEXF_LINEAR;
			else
				filter = D3DTEXF_POINT;
			if (hr = d3d->SetSamplerState(texunit, D3DSAMP_MAGFILTER, filter)!= D3D_OK)
				VX_ERROR(("Appearance::SetDev ERROR in SetTextureStageState %x", hr), hastex);
			continue;

			case Sampler::TEXCOORD:
			VX_TRACE2(Appearance::Debug, ("\t<%s stage='%d' val='%d'/>", sampler->GetAttrName(i), texunit, val));
			switch (val)
			{
				case Sampler::TEXGEN_SPHERE:
				case Sampler::TEXGEN_CUBE:
				{
					const Texture* image = sampler->GetTexture();
					if (image && image->IsCubeMap())
					{
						d3d->SetTextureStageState(texunit, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);			
						d3d->SetTextureStageState(texunit, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
					}
					else
					{
						D3DMATRIX mat;			// normal space -1,1 to texture space 0,1
						mat._11 = 0.5f; mat._12 = 0.0f; mat._13 = 0.0f; mat._14 = 0.0f; 
						mat._21 = 0.0f; mat._22 = -0.5f; mat._23 = 0.0f; mat._24 = 0.0f; 
						mat._31 = 0.0f; mat._32 = 0.0f; mat._33 = 1.0f; mat._34 = 0.0f; 
						mat._41 = 0.5f; mat._42 = 0.5f; mat._43 = 0.0f; mat._44 = 1.0f; 
						d3d->SetTransform(texstages[texunit], &mat); 
						d3d->SetTextureStageState(texunit, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
						d3d->SetTextureStageState(texunit, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);			
					}
				}
				break;

				default:
				if (val >= 0)
					d3d->SetTextureStageState(texunit, D3DTSS_TEXCOORDINDEX, sampler->Get(Sampler::TEXCOORD));
				d3d->SetTextureStageState(texunit, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
				//d3d->SetTransform(texstages[texunit], NULL); 
			}
		}
	}
	VX_TRACE2(Appearance::Debug, ("</sampler_load>"));
	return hastex;
}
 
/*!
 * @fn void DXAppearance::Update()
 *
 * Sets the D3D render state for this Appearance object and texture unit.
 */
void DXAppearance::Update()
{
	D3DDEVICE*	d3d = m_Render.GetDevice();
	bool		hastex = false;
	int			i;
	int			numtexunits;
	intptr		prevtex = s_CurTexID[0];
	bool		updated = false;
	int			transparency = m_Appear->Get(Appearance::TRANSPARENCY);

	numtexunits = (int) m_Appear->GetNumSamplers();
	if (numtexunits <= s_CurTexUnit)	// maximum number of samplers
		numtexunits = s_CurTexUnit + 1;	// we must initialize

	VX_TRACE2(Appearance::Debug, ("<appear_load appid='%p' />", this));
	/*
	 * Load device stage for all the texture samplers
	 * Default the state for the samplers used by the last appearance.
	 */
	for (i = 0; i < numtexunits; ++i)
	{
		const Sampler* smp = m_Appear->GetSampler(i);

		if (smp && UpdateSampler(smp, i))	// texture loaded for this slot?
		{
			s_CurTexUnit = i;
			if (smp->HasAlpha() && !transparency)
				((Appearance*) m_Appear)->Set(Appearance::TRANSPARENCY, 1);
		}
		else if (s_CurTexID[i])				// did we have a texture in this slot?
		{
			s_CurTexID[i] = NULL;			// unload it, no texture now
			if (i == 0)						// default to material color for slot 0
			{
				d3d->SetTexture(i, NULL);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				d3d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
				VX_TRACE2(Appearance::Debug, ("\t<textureop stage='%d' val='replace' />", i));
				VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='%d' val='diffuse' />", i));
				VX_TRACE2(Appearance::Debug, ("\t<dx_colorop stage='%d' val='selectarg2' />", i));
			}
			else							// disable this slot if no texture
			{
				d3d->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
				VX_TRACE2(Appearance::Debug, ("\t<dx_colorop stage='%d' val='disable' />", i));
				VX_TRACE2(Appearance::Debug, ("\t<textureop stage='%d' val='disable' />", i));
			}
		}
	}
	if (s_CurTexID[0] != prevtex)
		s_AppearAttrs[Appearance::TRANSPARENCY] = -1;
	/*
	 * Check all appearance attributes against the current state
	 * Only update those that have changed. If the texture
	 * has changed, transparency must be reset.
	 */
	for (i = 0; i < Appearance::NUM_ATTRS; ++i)
	{
		D3DRENDERSTATETYPE type;
		HRESULT	hr;
		int		val = m_Appear->Get(i);

		if (val == s_AppearAttrs[i])
			continue;
		s_AppearAttrs[i] = val;
		VX_TRACE2(Appearance::Debug, ("\t<%s value='0x%x' />", m_Appear->GetAttrName(i), val));
		switch (i)
		{
			case Appearance::CULLING:
			type = D3DRS_CULLMODE;
			val = val ? D3DCULL_CW : D3DCULL_NONE;
			break;

			case Appearance::LIGHTING:
			type = D3DRS_LIGHTING;
			break;

//			case Appearance::ZBUFFER:
//			type = D3DRS_ZWRITEENABLE;
//			break;

			case Appearance::TRANSPARENCY:
			SetAlphaBlend(d3d, val);
			continue;

			case Appearance::SHADING:
			switch (val)   
			{
				default:
				d3d->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
				type = D3DRS_FILLMODE;
				val = D3DFILL_SOLID;
				break;

				case Appearance::WIRE:
				type = D3DRS_FILLMODE;
				val = D3DFILL_WIREFRAME;
				break;

				case Appearance::POINTS:
				type = D3DRS_FILLMODE;
				val = D3DFILL_POINT;
				break;   
			}
			break;
		}
		if (hr = d3d->SetRenderState(type, val)!= D3D_OK)
			VX_ERROR_RETURN(("Appearance::SetDev ERROR in SetRenderState %x", hr));
	}
	/*
	 * Set material parameters. If diffuse color has alpha, enable transparency.
	 */
	const DeviceBuffer* mat = m_Appear->GetMaterial();
	if ((mat != s_CurMtl) && mat->IsClass(VX_PhongMaterial))
	{
		UpdateMaterial((PhongMaterial*) mat);
		s_CurMtl = (PhongMaterial*) mat;
	}
	VX_TRACE2(Appearance::Debug, ("</material_load>", mat));
}

void DXAppearance::SetAlphaBlend(D3DDEVICE* d3d, int blend)
{
	d3d->SetRenderState(D3DRS_ALPHABLENDENABLE, blend);
	if (blend)
	{
		d3d->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		VX_TRACE2(Appearance::Debug, ("\t<dx_alphaarg2 stage='0' val='diffuse' >"));
		if (s_CurTexID[0])
		{
			d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			VX_TRACE2(Appearance::Debug, ("\t<dx_alphaop stage='0' val='selectarg1' />"));
		}
		else
		{
			d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			VX_TRACE2(Appearance::Debug, ("\t<dx_alphaop stage='0' val='selectarg2' />"));
		}
		d3d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else
	{
		d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		VX_TRACE2(Appearance::Debug, ("\t<dx_alphaop stage='0' val='disable' />"));
	}
}

/*
 * Primitives are separated according to render state.
 * Opaque primitives are sorted by appearance.
 * Z buffering is enabled when they are rendered and alpha blending is not.
 * Transparent primitives are sorted by the Z value of their center.
 * Z buffering is disabled and alpha blending is enabled when they are rendered.
 */
void	DXRenderer::RenderState(int32 stateindex)
{
	D3DDEVICE*	d3d = GetDevice();
	if (stateindex == 0)			// if transparent, disable Z buffering
	{
		d3d->SetRenderState(D3DRS_ZWRITEENABLE, 0);
		DXAppearance::SetAlphaBlend(d3d, true);
	}
	else
	{
		d3d->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		DXAppearance::SetAlphaBlend(d3d, false);
	}
	GeoSorter::RenderState(stateindex);
}


}	// end Vixen
