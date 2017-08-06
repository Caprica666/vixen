Texture2D<float4> DiffuseMap;
Texture2D<float4> SpecularMap;
Texture2D<float4> NormalMap;
SamplerState DiffuseSampler : register(s0);
SamplerState SpecularSampler : register(s1);
SamplerState NormalSampler : register(s2);

struct VertexOutput
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
	float4 tangent : TANGENT;
	float4 bitangent : BINORMAL;
};

#include "phong_common.h"

class PixelPhongBump : PhongShader
{												
	Surface ComputeColor(in VertexOutput v)
	{
		Surface s;
		s.diffuse = float4(1.0, 1.0, 1.0, 1.0);
		s.specular = float4(0.0, 0.0, 0.0, 0.0);
		s.position = float4(v.position.xyz, 1.0);
		if (HasDiffuseMap)
			s.diffuse *= DiffuseMap.Sample(DiffuseSampler, v.texcoord.xy);
		s.specular = Specular;
		if (HasSpecularMap)
			s.specular *= SpecularMap.Sample(SpecularSampler, v.texcoord.xy);
		if (HasNormalMap)
		{
			float3 n = NormalMap.Sample(NormalSampler, v.texcoord.xy).xyz * 2.0 - 1.0;
			s.normal = normalize(n.x * v.tangent + n.y * v.bitangent + n.z * v.normal);
		}
		else
			s.normal = normalize(v.normal);
		return s;
	}
};