cbuffer PerObjectConstants
{
	row_major float4x4 WorldMatrix;
};

cbuffer PerFrameConstants
{
	row_major float4x4	ViewMatrix;
	row_major float4x4	ViewProjMatrix;
	row_major float4x4	InverseViewMatrix;
};

struct VOutPosNormTex
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
};

struct VOutPosNormTexTang
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
	float4 tangent : TANGENT;
	float4 bitangent : BINORMAL;
};

cbuffer PhongMaterialConstants
{
   float4 Diffuse;
   float4 Ambient;
   float4 Specular;
   float4 Emission;
   float Shine;
   int HasNormalMap;
   int HasDiffuseMap;
   int HasSpecularMap;
   int ShaderType;
};


struct LightConstants
{
	int		LightType;
	int		Decay;
	float4	Color;
	matrix	LightToWorld;
	float	InnerAngle;
	float	OuterAngle;
};

StructuredBuffer<LightConstants> LightList;

struct Surface
{
   float4 position;
   float4 normal;
   float4 diffuse;
   float4 specular;
};

struct Radiance
{
   float4 flux;
   float4 direction;
};

SamplerState LinearWrappedSampler  : register ( s0 );
SamplerState LinearClampedSampler  : register ( s1 );
SamplerState NearestWrappedSampler : register ( s2 );
SamplerState NearestClampedSampler : register ( s3 );

Texture2D<float4> DiffuseMap;
Texture2D<float4> SpecularMap;
Texture2D<float4> NormalMap;


float3 AddLight(Surface s, Radiance r)
{
	float3 L = r.direction.xyz;	// From surface towards light, unit length, world-space
	float3 I = r.flux.xyz;		// N . L already accounted for
//	float3 campos = float3(View[2][0], View[2][1], View[2][2]);
	float3 campos = float3(InverseViewMatrix[2][0], InverseViewMatrix[2][1], InverseViewMatrix[2][2]);

	// Vector from surface location to the camera's eye
	float3 E = normalize(campos - s.position.xyz);
	float3 H = normalize(L+E);
	float3 kD = s.diffuse.xyz * I;
	//float3 kS = s.specular.xyz * I * pow(max(dot(s.normal.xyz, H), 0.0), s.specular.w);
	float3 kS = s.specular.xyz * I * pow(max(dot(s.normal.xyz, H), 0.0), 5.0f);
	return kD + kS;
}

Radiance AmbientLight(float4 Color, Surface s)
{
	Radiance r;
	r.direction.xyz = float3(0.0, -1.0, 0.0);
	r.flux.xyz = Color.xyz;
	return r;
}

Radiance DirectionalLight(float4 Color, matrix LightToWorld, Surface s)
{
	float3 L = mul(LightToWorld, float4(0.0, 0.0, 1.0, 0.0)).xyz;
	Radiance r;
	r.direction.xyz = normalize(L);
	r.flux.xyz = Color.xyz * max(dot(s.normal.xyz, L), 0.0);
	return r;
}

float shBasis0(float3 p) { return 0.282095; }
float shBasis1(float3 p) { return 0.488603 * p.z; }
float shBasis2(float3 p) { return 0.488603 * p.y; }
float shBasis3(float3 p) { return 0.488603 * p.x; }
float shBasis4(float3 p) { return 1.092548 * p.x * p.z; }
float shBasis5(float3 p) { return 1.092548 * p.y * p.z; }
float shBasis6(float3 p) { return 0.315392 * (3.0 * p.y * p.y - 1.0); }
float shBasis7(float3 p) { return 1.092548 * p.x * p.y; }
float shBasis8(float3 p) { return 0.546274 * (p.x*p.x - p.z*p.z); }

Radiance SphericalHarmonicLight(float4 Color, Surface s)
{
float3 shc0 = float3(1.674,   2.098,  2.757    );
float3 shc1 = float3(0.302,   0.23 ,  0.157    );
float3 shc2 = float3(0.001,   0.001,  0.001    );
float3 shc3 = float3(0.134,   0.102,  0.07     );
float3 shc4 = float3(0.035,   0.027,  0.013    );
float3 shc5 = float3(0,       0    ,  0        );
float3 shc6 = float3(-0.139, -0.123, -0.082    );
float3 shc7 = float3(0,       0    ,  0        );
float3 shc8 = float3(-0.031, -0.024, -0.012    );

	Radiance r;
	float intensity = length(Color) / 4.0f;
	r.direction.xyz = s.normal.xyz;
	r.flux.xyz
	     = shBasis0(s.normal.xyz) * shc0 + shBasis1(s.normal.xyz) * shc1
	     + shBasis2(s.normal.xyz) * shc2 + shBasis3(s.normal.xyz) * shc3
	     + shBasis4(s.normal.xyz) * shc4 + shBasis5(s.normal.xyz) * shc5
		 + shBasis6(s.normal.xyz) * shc6 + shBasis7(s.normal.xyz) * shc7
		 + shBasis8(s.normal.xyz) * shc8;
	r.flux.xyz *= intensity;
	return r;
};


float3 LightPixel(Surface surface)													    
{																										    
	float3 color = float3(0.0f, 0.0f, 0.0f);                        				    
	uint lightIndex = 0;
	color = surface.diffuse.xyz;
	while (LightList[lightIndex].LightType > 0)                                                
	{                                                               				    
     [unroll] for (uint n = 0; n < 1; ++n, ++lightIndex)
		 {
			LightConstants	light = LightList[lightIndex];                         			    
			int				type = light.LightType;         				    
			matrix			LightToWorld = light.LightToWorld;
			Radiance		radiance;                                           				    

		// 44 = ambient, 26 = directional, 27 = spot, 0 = end of list
			if (type == 26)
				radiance = DirectionalLight(light.Color, LightToWorld, surface);
//			else if (type == 27)
//				radiance = SpotLight(light.InnerAngle, light.OuterAngle, light.Color, LightToWorld, surface);
			else if (type == 44)
				radiance = AmbientLight(light.Color, surface);
			color += AddLight(surface, radiance); 
		}
	}
	return color;
}
												
float4 PixelPhongDiffuse(in VOutPosNormTex v, in float4 PixelCoord : SV_Position) : SV_Target0
{
	Surface s;
	s.specular = Specular;
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (HasDiffuseMap)
		s.diffuse *= DiffuseMap.Sample(LinearWrappedSampler, v.texcoord.xy);
	if (s.diffuse.w < 0.4) discard;
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}

float4 PixelPhongNotex(in VOutPosNormTex v, in float4 PixelCoord : SV_Position) : SV_Target0
{
	Surface s;
	s.diffuse = float4(1.0,1.0,1.0,1.0);
	s.specular = Specular;
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (s.diffuse.w < 0.4) discard;
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}

float4 PixelPhongSpecular(in VOutPosNormTex v, in float4 PixelCoord : SV_Position) : SV_Target0
{
	Surface s;
	s.diffuse = float4(1.0,1.0,1.0,1.0);
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (HasDiffuseMap)
		s.diffuse *= DiffuseMap.Sample(LinearWrappedSampler, v.texcoord.xy);
	if (s.diffuse.w < 0.4) discard;
	s.specular = Specular;
	if (HasSpecularMap)
		s.specular *= SpecularMap.Sample(LinearWrappedSampler, v.texcoord.xy);
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}

float4 PixelPhongBump(in VOutPosNormTexTang v, in float4 PixelCoord : SV_Position) : SV_Target0
{
	Surface s;
	s.diffuse = float4(1.0, 1.0, 1.0, 1.0);
	s.specular = float4(0.0, 0.0, 0.0, 0.0);
	s.position = v.position;
	if (HasDiffuseMap)
		s.diffuse *= DiffuseMap.Sample(LinearWrappedSampler, v.texcoord.xy);
	if (s.diffuse.w < 0.4) discard;
	s.specular = Specular;
	if (HasSpecularMap)
		s.specular *= SpecularMap.Sample(LinearWrappedSampler, v.texcoord.xy);
	if (HasNormalMap)
	{
		float3 n = NormalMap.Sample(LinearWrappedSampler, v.texcoord.xy).xyz * 2.0 - 1.0;
		s.normal = normalize(n.x * v.tangent + n.y * v.bitangent + n.z * v.normal);
	}
	else
		s.normal = normalize(v.normal);
		return float4(LightPixel(s), 1.0f);
}