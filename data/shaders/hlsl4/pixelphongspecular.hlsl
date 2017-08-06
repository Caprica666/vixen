#include "common.h"
#include "phong_lighting.h"

Texture2D<float4> DiffuseMap;
Texture2D<float4> SpecularMap;
SamplerState DiffuseSampler : register(s0);
SamplerState SpecularSampler : register(s1);
												
float4 PixelPhongSpecular(in VOutPosNormTex v) : SV_Target0
{
	Surface s;
	s.diffuse = float4(1.0,1.0,1.0,1.0);
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (HasDiffuseMap)
		s.diffuse *= DiffuseMap.Sample(DiffuseSampler, v.texcoord.xy);
	if (s.diffuse.w < 0.4) discard;
	s.specular = Specular;
	if (HasSpecularMap)
		s.specular *= SpecularMap.Sample(SpecularSampler, v.texcoord.xy);
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}
