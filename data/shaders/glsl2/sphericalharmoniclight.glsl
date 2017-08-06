vec3 shc0 = float3(1.674,   2.098,  2.757);
vec3 shc1 = float3(0.302,   0.23 ,  0.157);
vec3 shc2 = float3(0.001,   0.001,  0.001);
vec3 shc3 = float3(0.134,   0.102,  0.07 );
vec3 shc4 = float3(0.035,   0.027,  0.013);
vec3 shc5 = float3(0,       0    ,  0    );
vec3 shc6 = float3(-0.139, -0.123, -0.082);
vec3 shc7 = float3(0,       0    ,  0    );
vec3 shc8 = float3(-0.031, -0.024, -0.012);

float shBasis0(float3 p) { return 0.282095; }
float shBasis1(float3 p) { return 0.488603 * p.z; }
float shBasis2(float3 p) { return 0.488603 * p.y; }
float shBasis3(float3 p) { return 0.488603 * p.x; }
float shBasis4(float3 p) { return 1.092548 * p.x * p.z; }
float shBasis5(float3 p) { return 1.092548 * p.y * p.z; }
float shBasis6(float3 p) { return 0.315392 * (3.0 * p.y * p.y - 1.0); }
float shBasis7(float3 p) { return 1.092548 * p.x * p.y; }
float shBasis8(float3 p) { return 0.546274 * (p.x*p.x - p.z*p.z); }

struct SphericalHarmonicLightData
{
	float4	WorldDir;
	float4	WorldPos;
	float4	Color;
	int		Decay;
};


Radiance SphericalHaromicLight(Surface s, SphericalHarmonicLightData data)
{
	Radiance r;
	float intensity = length(data.Color) / 4.0f;
	r.direction.xyz = s.normal.xyz;
	r.flux.xyz
		 = shBasis0(s.normal.xyz) * shc0 + shBasis1(s.normal.xyz) * shc1
		 + shBasis2(s.normal.xyz) * shc2 + shBasis3(s.normal.xyz) * shc3
		 + shBasis4(s.normal.xyz) * shc4 + shBasis5(s.normal.xyz) * shc5
		 + shBasis6(s.normal.xyz) * shc6 + shBasis7(s.normal.xyz) * shc7
		 + shBasis8(s.normal.xyz) * shc8;
	r.flux.xyz *= intensity;
	return r;
}