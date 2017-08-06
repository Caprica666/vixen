/*
 * SWIG interface file for Appearance, Sampler, Material
 */

namespace Vixen
{
	%ignore Appearance::LoadDev;
	%ignore Appearance::DefaultDev;
	%ignore Appearance::GetAppIndex;
	%ignore Appearance::SetAt;
	%ignore Appearance::GetAt;
	%ignore Appearance::GetSampler(int i) const;
	%ignore Appearance::GetMaterial() const;
	%ignore Appearance::DevHandle;
	%ignore Material::LoadDev;
	%ignore Material::DefaultDev;
	%ignore PhongMaterial::PhongMaterial(const Col4*);
	%ignore Sampler::GetTexture() const;
	%ignore Sampler::LoadDev;
	%ignore Sampler::DefaultDev;
	%ignore Sampler::DevHandle;
	%ignore Shader::DevHandle;
	%ignore Shader::GetInputLayout;
	%ignore Shader::GetOutputLayout;
	%ignore Shader::GetShaderCode;
	%ignore LayoutSlot;
	%ignore DataLayout;
	%ignore DeviceBuffer::GetLayout;
	%ignore DeviceBuffer::Set(const TCHAR* name, const float* src);
	%ignore DeviceBuffer::Get(const TCHAR* name, float* dst) const;

	%rename(Equal)		Sampler::operator==;
	%rename(Equal)		Appearance::operator==;
	%rename(Equal)		Material::operator==;
	%rename(NotEqual)	Sampler::operator!=;
	%rename(NotEqual)	Appearance::operator!=;
	%rename(NotEqual)	Material::operator!=;

	%apply	SharedObj*	{ Appearance* };
	%apply	SharedObj*	{ Sampler* };
	%apply	SharedObj*	{ Material* };
	%apply	SharedObj*	{ DataLayout* };
}

ATTR_READONLY(Vixen::DeviceBuffer, const char*, Layout);
ATTR_READONLY(Vixen::DeviceBuffer, int, ByteSize);
ATTR_DEFINE(Vixen::Shader, const char*, FileName);
ATTR_DEFINE(Vixen::Shader, const char*, Source);
ATTR_DEFINE(Vixen::Shader, const char*, InputDesc);
ATTR_DEFINE(Vixen::Shader, const char*, OutputDesc);
ATTR_DEFINE(Vixen::Sampler, Vixen::Texture, Texture);
ATTR_DEFINE(Vixen::Appearance, Vixen::Material, Material);
ATTR_DEFINE(Vixen::Appearance, Vixen::Shader, PixelShader);
ATTR_READONLY(Vixen::Appearance, int, NumSamplers);
ATTR_DEFINE_NEW(Vixen::PhongMaterial, Vixen::Col4, Emission);
ATTR_DEFINE(Vixen::PhongMaterial, float, Shine);
ATTR_DEFINE_NEW(Vixen::PhongMaterial, Vixen::Col4, Ambient);
ATTR_DEFINE_NEW(Vixen::PhongMaterial, Vixen::Col4, Diffuse);
ATTR_DEFINE_NEW(Vixen::PhongMaterial, Vixen::Col4, Specular);
ATTR_DEFINE_NEW(Vixen::PhongMaterial, Vixen::Col4, Emission);

%include "render/vxdevbuf.h"
%include "render/vxmaterial.h"
%include "render/vxsampler.h"
%include "render/vxappear.h"



