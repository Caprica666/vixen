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


class DirectLight : Light
{
	float4	WorldDir;
	float4	WorldPos;
	float4	Color;
	int		Decay;

	Radiance Illuminate(Surface s)										
	{																	
	   /// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	   /// InnerAngle is the angle, in radians, from center to the inner edge																							
		float3	L = normalize(WorldDir.xyz);   /// Towards the light!																						
		float	nDotL = max(dot(s.normal.xyz, L), 0.0f);
		Radiance r;																						
																							
		r.direction.xyz = L;
		r.flux.xyz = Color.xyz * nDotL;
		return r;
	}
};


cbuffer ActiveLights : register(b2)
{
	PixelPhongNotex surfaceshader;
	DirectLight	light0;
};


Light LightList[1];

SurfaceShader SurfaceColor;

float4 _PixelPhongNotex(in VertexOutput v) : SV_Target0
{
	Surface		s;
	float3		color;
	int			enabled = LightsEnabled;
	int			nlights = NumLights;
	int			i;

	s = SurfaceColor.ComputeColor(v);
	color = float3(0, 0, 0);
	for (i = 0; i < nlights; ++i)
	{ 
		if ((enabled & 0x80000000) != 0)
		{
			Radiance r;
			r = LightList[i].Illuminate(s);
			color += SurfaceColor.ApplyLight(s, r);
		}
		enabled = enabled << 1;
	}
	return float4(color, 1.0);
}