#include "vixen.h"
#include "dx9/lightdx.h"
#include "dx9/renderdx.h"

namespace Vixen {

#define	LIGHT_ID	0


GPULight& DXLightProp::operator=(const GPULight& src)
{
	GPULight::operator=(src);
	DXLightProp& dxsrc = (DXLightProp&) src;
	DXLight = dxsrc.DXLight;
	return *this;
}

intptr DXRenderer::AddLight(Light* light)
{
	DXLightProp* prop = (DXLightProp*) light->DevHandle;

	if (prop)
		return prop->DevIndex;
	prop = new DXLightProp(*this, *light);
	prop->DevIndex = m_LightList.Attach(light, prop);
	if ((prop->DevIndex < 0) ||
		(prop->DevIndex >= MaxLights))
	{
		VX_ERROR(("DXRenderer::AddLight ERROR cannot add light #%d", prop->DevIndex), -1);
	}
	return prop->DevIndex;
}

DXLightProp::DXLightProp(Renderer& r, const Light& l) : GPULight(r, l)
{
	ZeroMemory(&DXLight, sizeof(D3DLIGHT9));
	DXLight.Type			= D3DLIGHT_DIRECTIONAL;
	DXLight.Diffuse.r		= 1.00f;
	DXLight.Diffuse.g		= 1.00f;
	DXLight.Diffuse.b		= 1.00f;
	DXLight.Ambient.r		= 0.0f;
	DXLight.Ambient.g		= 0.0f;
	DXLight.Ambient.b		= 0.0f;
	DXLight.Specular.r		= 0.50f;
	DXLight.Specular.g		= 0.50f;
	DXLight.Specular.b		= 0.50f;
	DXLight.Attenuation0	= 1.0f;
	DXLight.Attenuation1	= 0.0f;
	DXLight.Attenuation2	= 0.0f;
	DXLight.Falloff			= 1.0f;
	DXLight.Theta			= DXLight.Phi = 0.0;
	DXLight.Direction.x		= 0.0f;
	DXLight.Direction.y		= 0.0f;
	DXLight.Direction.z		= 1.0f;
	DXLight.Range			= sqrtf(FLT_MAX); 
}

void DXLightProp::Load(bool changed)
{
	D3DDEVICE* d3d = ((DXRenderer&) Render).GetDevice();
	HRESULT hr;

	if (Type == VX_AmbientLight)
		return;
	hr = d3d->SetLight((DWORD) DevIndex, &DXLight);
	if (hr != D3D_OK)
		VX_ERROR_RETURN(("DXLightProp::Load ERROR cannot set light #%d err %x", DevIndex, hr));
}

void DXLightProp::Update(const Matrix* mtx)
{
	Light*	l = (Light*) LightModel;
	D3DCOLORVALUE black = { 0.0f, 0.0f, 0.0f, 0.0f };

	GPULight::Update(mtx);
	DXLight.Position.x = Data.WorldPos.x;
	DXLight.Position.y = Data.WorldPos.y;
	DXLight.Position.z = Data.WorldPos.z;
	DXLight.Direction.x = Data.WorldDir.x;
	DXLight.Direction.y = Data.WorldDir.y;
	DXLight.Direction.z = Data.WorldDir.z;
	DXLight.Diffuse = (D3DCOLORVALUE&) l->GetColor();
	DXLight.Specular = (D3DCOLORVALUE&) l->GetColor();
	switch (l->GetDecay())
	{
		default:
		DXLight.Attenuation0 = 1.0f;
		DXLight.Attenuation1 = 0.0f;
		DXLight.Attenuation2 = 0.0f;
		break;

		case SpotLight::INVERSE:
		DXLight.Attenuation0 = 0.0f;
		DXLight.Attenuation1 = 1.0f;
		DXLight.Attenuation2 = 0.0f;
		break;

		case SpotLight::INVERSE_SQUARE:
		DXLight.Attenuation0 = 0.0f;
		DXLight.Attenuation1 = 0.0f;
		DXLight.Attenuation2 = 1.0f;
		break;
	}
	switch (Type)
	{
		case VX_AmbientLight:
		DXLight.Type = D3DLIGHT_DIRECTIONAL;
		DXLight.Ambient = (D3DCOLORVALUE&) l->GetColor();
		DXLight.Diffuse = black;
		DXLight.Specular = black;
		break;

		case VX_DirectLight:
		DXLight.Type = D3DLIGHT_DIRECTIONAL;
		DXLight.Ambient = black;
		break;

		case VX_SpotLight:
		DXLight.Type = D3DLIGHT_SPOT;
		DXLight.Theta = ((SpotLight*) l)->GetInnerAngle();
		DXLight.Phi = ((SpotLight*) l)->GetOuterAngle();
		DXLight.Ambient = black;
		break;

		case VX_Light:
		DXLight.Ambient = black;
		DXLight.Type = D3DLIGHT_POINT;
	}
	if (l->GetRadius() == 0.0f)
		DXLight.Range = sqrtf(FLT_MAX); 
	else
		DXLight.Range = l->GetRadius(); 
}

/****
 *
 * Enable, Disable
 * turn lights on and off at device level 
 *
 ****/
void DXLightProp::Enable() const
{
	D3DDEVICE* d3d = ((DXRenderer&) Render).GetDevice();
	d3d->LightEnable((DWORD) DevIndex, TRUE);
	VX_TRACE2(Light::Debug, ("DXLightProp::Enable #%d", DevIndex));
}

void DXLightProp::Disable() const
{
	D3DDEVICE* d3d = ((DXRenderer&) Render).GetDevice();
	d3d->LightEnable((DWORD) DevIndex, FALSE);
	VX_TRACE2(Light::Debug, ("DXLightProp::Disable #%d", DevIndex));
}

void DXLightProp::Detach()
{
	delete this;
}

}	// end Vixen