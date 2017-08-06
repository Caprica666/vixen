struct VertexOutput
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
};

#include "phong_common.h"

class PixelPhongNotex : PhongShader
{												
	Surface ComputeColor(in VertexOutput v)
	{
		Surface s;
		s.specular = Specular;
		s.position = float4(v.position.xyz, 1.0);
		s.diffuse = Diffuse;
		s.emission = Emission;
		s.normal = float4(normalize(v.normal.xyz), 0.0);
		return s;
	}
};
