#include "common.h"
#include "phong_lighting.h"

Texture2D<float4> DiffuseMap;
SamplerState DiffuseSampler : register(s0);
												
float4 PixelPhongDiffuse(in VOutPosNormTex v) : SV_Target0
{
	Surface s;
	s.specular = Specular;
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (HasDiffuseMap)
		s.diffuse *= DiffuseMap.Sample(DiffuseSampler, v.texcoord.xy);
	if (s.diffuse.w < 0.4) discard;
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}