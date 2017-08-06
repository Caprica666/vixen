#include "dx11/vixendx11.h"

namespace Vixen {
	
template <> inline bool Core::Dict<NameProp, Core::String, Vixen::BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
{
	return String(knew).CompareNoCase(kdict) == 0;
}

template <> inline uint32 Core::Dict<NameProp, Core::String, Vixen::BaseDict>::HashKey(const NameProp& np) const
{
	return HashStr((const TCHAR*) np);
}

int		DXLight::NumLights = 0;

intptr DXRenderer::AddLight(Light* light)
{ 
	DXLight*	prop = (DXLight*) light->DevHandle;

	if (prop)
		return prop->DevIndex;
	VX_ASSERT(light);
	prop = new DXLight(*this, *light);
	VX_ASSERT(prop);
	light->AddRef();
	prop->DevIndex = m_LightList.Attach(light, prop);
	if ((prop->DevIndex < 0) ||
		(prop->DevIndex >= MaxLights))
		VX_ERROR(("DXRenderer::AddLight ERROR cannot add light #%d\n", prop->DevIndex), -1);
	VX_ASSERT(prop);
	if (prop->DevIndex == LIGHT_NoDevice)
		return -1;
	VX_TRACE(Debug, ("DXRenderer::AddLight #%d %s %s\n", prop->DevIndex, light->GetName(), light->ClassName()));
	return prop->DevIndex;
}

int DXRenderer::LoadLights()
{
	int				nlights = m_LightList.GetNumLights();
	bool			listchanged = m_LightList.CheckListChanged();
	int				nbytes = 0;

	/*
	 * Generate the pixel shader source for each light
	 * and determine the total size of the light constant buffer
	 */
	if (listchanged || (nlights != DXLight::NumLights))
	{
		nbytes = DXLight::LoadLights(m_LightList, m_LightShaderSource);
		DXShader::ReleasePixelShaders();
		if (nbytes == 0)
			return 0;
	}
	if (!listchanged && !m_LightList.LightsChanged())
		return nlights;
	/*
	 * Allocate a temporary buffer to hold all of the light data
	 */
	DeviceBuffer*	lightlist = m_ConstantBuffers[CBUF_LIGHT];

	if (((lightlist == NULL) ||
		(lightlist->GetByteSize() != nbytes)) &&
		(nbytes > 0))
	{
		DXConstantBuf::Release(lightlist);
		m_ConstantBuffers[CBUF_LIGHT] = lightlist = new DeviceBuffer(NULL, nbytes);
	}
	DXLight::UpdateBuffer(m_LightList, lightlist->GetData());
	lightlist->SetChanged(true);
	return DXLight::NumLights;
}

/*
 * Generate the pixel shader source for each light
 * and determine the total size of the light constant buffer
 */
int	DXLight::LoadLights(LightList& lights, Core::String& source)
{
	bool			enabled;
	DXLight*		l;
	int				nlights = 0;
	int				nbytes = 0;
	Core::String	decls = TEXT("\ncbuffer ActiveLights : register(b2)\n{\n\t$SURFACESHADER surfaceshader;\n");
	LightList::Iter	iter(lights);
	NameDict<Core::String>	shader_sources;

	source = TEXT("\n\n");
	while (l = (DXLight*) iter.NextProp(enabled))
	{
		NameProp		shadername = l->LightModel->ClassName();
		DXShader*		tmp = DXShader::Find(shadername);
		const TCHAR*	source;

		if (tmp == NULL)
			continue;
		source = tmp->SceneShader->GetSource();
		if (source == NULL)
			continue;
		shader_sources[shadername] = source;
		decls += TEXT('\t');
		decls += l->LightModel->ClassName();
		decls += TEXT("\tlight");
		decls += Core::String(nlights);
		decls += TEXT(";\n");
		++nlights;
		nbytes += l->GetByteSize();
	}
	NumLights = nlights;
	/*
	 * If there are no light sources, declare a NullLight so
	 * the pixel shader will compile.
	 */
	if (nlights == 0)
	{
		decls += TEXT("\tNullLight light0;\n");
		nbytes = sizeof(GPULight::ShaderConstants);
		nlights = 1;
	}
	/*
	 * Add light source declarations.
	 */
	else
	{
		NameDict<Core::String>::Iter niter(&shader_sources);
		Core::String* stringentry;

		while (stringentry = niter.Next())
			source += *stringentry;
	}
	nbytes = (nbytes + 0X0F) & ~0X0F;	// multiple of 16 bytes
	VX_ASSERT(nlights < LIGHT_MaxLights);

	decls += TEXT("};\n\n");
	source = source + decls;
	source += TEXT("\nLight LightList[");
	source += Core::String(nlights);
	source += TEXT("];\n\n");
	return nbytes;
}

void DXLight::UpdateBuffer(LightList& lights, void* buffer)
{
	char*			p = (char*) buffer;
	DXLight*		l;
	bool			enabled;
	int				vectorofs = NULL;
	LightList::Iter	iter(lights);

	while (l = (DXLight*) iter.NextProp(enabled))
	{
		l->VectorOffset = vectorofs;
		vectorofs += l->GetByteSize() / sizeof(float);
		memcpy(p, l->GetData(), l->GetByteSize());
		p += l->GetByteSize();
	}
}

DXLight& DXLight::operator=(const GPULight& src)
{
	GPULight::operator=(src);
	DXLight& dxsrc = (DXLight&) src;
	if (dxsrc.LightClassInstance)
		LightClassInstance = dxsrc.LightClassInstance;
	return *this;
}

ID3D11ClassInstance*	DXLight::GetClassInstance(const TCHAR* name)
{
	ID3D11ClassInstance*	classInstance = LightClassInstance;

	VX_ASSERT(name);
	if (classInstance == NULL)
	{
		DXShader::ClassLinkage->GetClassInstance(name, 0, &classInstance);
		LightClassInstance = classInstance;
		VX_TRACE(DXRenderer::Debug, ("DXLight::GetClassInstance creating instance of class %s\n", GetName()));
	}
	return classInstance;
}

ID3D11ClassInstance*	DXLight::GetNullInstance(const TCHAR* name)
{
	ID3D11ClassInstance*	classInstance;
	DXShader*				dxshader = DXShader::Find(TEXT("NullLight"));

	VX_ASSERT(dxshader);
	classInstance = dxshader->ShaderClass;
	if (classInstance == NULL)
	{
		DXShader::ClassLinkage->GetClassInstance(name, 0, &classInstance);
		dxshader->ShaderClass = classInstance;
		VX_TRACE(DXRenderer::Debug, ("DXRenderer::LoadLights creating NullLight instance\n"));
	}
	return classInstance;
}

void DXLight::Detach()
{
	LightClassInstance = NULL;
	delete this;
}

}  // end Vixen
