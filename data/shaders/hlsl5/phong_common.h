

struct Surface
{
   float4 position;
   float4 normal;
   float4 diffuse;
   float4 specular;
   float4 emission;
};

struct Radiance
{
   float4 flux;
   float4 direction;
};

#include "interfaces.h"

#include "vertex_common.h"

cbuffer PhongMaterialConstants : register(b3)
{
   float4	Diffuse;
   float4	Ambient;
   float4	Specular;
   float4	Emission;
   float	Shine;
   int		HasDiffuseMap;
   int		HasSpecularMap;
   int		HasNormalMap;
};

class NullLight : Light
{
	float4	WorldDir;
	float4	WorldPos;

	Radiance Illuminate(Surface s)
	{
		Radiance r;
		r.direction.xyz = float3(0.0, 0.0, 0.0);
		r.flux.xyz = float3(0, 0, 0);
		return r;
	}
};

class PhongShader : SurfaceShader
{
	Surface ComputeColor(in VertexOutput v)
	{
		Surface s;
		s.diffuse = float4(1.0,1.0,1.0,1.0);
		s.emission = float4(0,0,0,0);
		s.position = float4(v.position.xyz, 1.0);
		s.normal = float4(normalize(v.normal.xyz), 0.0);
		return s;
	}

	float3 ApplyLight(Surface s, Radiance r)
	{
		float3 L = r.direction.xyz;	// From surface towards light, unit length, world-space
		float3 I = r.flux.xyz;		// N . L already accounted for
		float3 campos = CameraPos.xyz;
		float  shine = s.specular.w;
		// Vector from surface location to the camera's eye
		float3 E = normalize(campos - s.position.xyz);
		float3 H = normalize(L + E);
		float3 kD = s.diffuse.xyz * I;
		float3 kS = s.specular.xyz * I;
		if (shine > 0)
			kS *= pow(max(dot(s.normal.xyz, H), 0.0), shine);
		return kD + kS;
	}
};