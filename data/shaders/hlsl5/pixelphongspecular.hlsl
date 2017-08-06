struct VertexOutput
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
};

Texture2D<float4> DiffuseMap;
Texture2D<float4> SpecularMap;
Texture2D<float4> NormalMap;
SamplerState DiffuseSampler : register(s0);
SamplerState SpecularSampler : register(s1);
SamplerState NormalSampler : register(s2);

#include "phong_common.h"

class PixelPhongSpecular : PhongShader
{												
	Surface ComputeColor(in VertexOutput v)
	{
		Surface s;
		s.position = float4(v.position.xyz, 1.0);
		s.diffuse = Diffuse;
		if (HasDiffuseMap)
			s.diffuse *= DiffuseMap.Sample(DiffuseSampler, v.texcoord.xy);
		s.specular = Specular;
		if (HasSpecularMap)
			s.specular *= SpecularMap.Sample(SpecularSampler, v.texcoord.xy);
		s.emission = Emission;
		s.normal = float4(normalize(v.normal.xyz), 0.0);
		return s;
	}
};
