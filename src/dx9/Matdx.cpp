/****
 *
 * Material Class
 * Material members are in the same order and overlay exactly
 * onto the D3DMATERIAL structure
 *
 ****/
#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen {

bool Material::LoadDev(Scene* scene, const Appearance* appear)
{
	VX_ASSERT(IsKindOf(CLASS_(PhongMaterial)));
	D3DMATERIAL* d3dmat = (D3DMATERIAL*) DeviceBuffer::GetData();
	DXRenderer*	render = (DXRenderer*) scene->GetRenderer();
	LPD3DDEVICE	d3d = render->GetDevice();
	bool is_transparent = false;

	VX_TRACE2(Appearance::Debug, ("<material_load appid='%p'>", this));
	SetChanged(false);
	if (d3d->SetMaterial(d3dmat) != D3D_OK)
		VX_ERROR(("Material::LoadDev failed\n"), false);
	VX_TRACE2(Appearance::Debug, ("\t<dx_colorarg2 stage='0' val='diffuse' >"));
	d3d->SetRenderState(D3DRS_SPECULARENABLE, d3dmat->Power != 0);
	if ((d3dmat->Diffuse.a < 1.0f) && appear->Get(Appearance::TRANSPARENCY))
	{
		if (Texture::s_CurTexID[0] != NULL)
		{
			d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			VX_TRACE2(Appearance::Debug, ("\t<dx_alphaop stage='0' val='modulate' />"));
		}
		is_transparent = true;
	}
	VX_TRACE2(Appearance::Debug, ("</appear_load>"));
	return is_transparent;
}

void Material::DefaultDev(Scene* scene)
{
	D3DMATERIAL d3dmat;
	DXRenderer*	render = (DXRenderer*) scene->GetRenderer();
	LPD3DDEVICE	d3d = render->GetDevice();

	memset(&d3dmat, 0, sizeof(d3dmat));
	d3dmat.Diffuse.r = 1.0f;
	d3dmat.Diffuse.g = 1.0f;
	d3dmat.Diffuse.b = 1.0f;
	d3dmat.Diffuse.a = 1.0f;
	if (d3d->SetMaterial(&d3dmat) != D3D_OK)
		VX_ERROR_RETURN(("Material::LoadDev failed\n"));
	d3d->SetRenderState(D3DRS_SPECULARENABLE, 0);
}
}	// end Vixen