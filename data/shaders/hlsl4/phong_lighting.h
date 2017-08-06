

cbuffer PhongMaterialConstants : register (cb1)
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
	int			LightType;
	int			Decay;
	float		InnerAngle;
	float		OuterAngle;
	float4		WorldDir;
	float4		WorldPos;
	float4		Color;
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


float3 AddLight(Surface s, Radiance r)
{
	float3 L = r.direction.xyz;	// From surface towards light, unit length, world-space
	float3 I = r.flux.xyz;		// N . L already accounted for
	float3 campos = CameraPos.xyz;

	// Vector from surface location to the camera's eye
	float3 E = normalize(campos - s.position.xyz);
	float3 H = normalize(L+E);
	float3 kD = s.diffuse.xyz * I;
	float3 kS = s.specular.xyz * I;
	if (Shine > 0)
		kS *= pow(max(dot(s.normal.xyz, H), 0.0), Shine);
	return kD + kS;
}

Radiance AmbientLight(Surface s, float3 color)
{
	Radiance r;
	r.direction.xyz = float3(0.0, -1.0, 0.0);
	r.flux.xyz = color;
	return r;
}

Radiance DirectionalLight(Surface s, float3 color, float3 worlddir)
{
	Radiance r;
	r.direction.xyz = normalize(worlddir);
	r.flux.xyz = color * max(dot(s.normal.xyz, worlddir), 0.0);
	return r;
}

/// world-space vectors, from light to surface and from light				
float ComputeFallOff(float3 LPrime, float3 D, float cosInner, float cosOuter)	
{																			
   float cosAngle = dot(LPrime, D);										
																			
   if      (cosAngle < cosOuter) return 0.0f;								
   else if (cosAngle > cosInner) return 1.0f;								
   else																	
   {																		
      float delta = (cosAngle - cosOuter) / (cosInner - cosOuter);			
      return delta*delta*(delta*delta);										
   }																		
}																																													
																	
Radiance SpotLight(Surface s, float3 color, float3 worlddir, float3 worldpos, float InnerAngle, float OuterAngle)										
{																	
   /// outerWidth is the angle, in radians, from center to the outer edge of the cone
   /// innerWidth is the angle, in radians, from center to the inner edge
																						
	float	cosInner = cos(InnerAngle);
	float	cosOuter = cos(OuterAngle);
	float3	L = normalize(worldpos - s.position.xyz);   /// Towards the light!																						
	float	falloff = ComputeFallOff(L, worlddir, cosInner, cosOuter);
	float	nDotL = max(dot(s.normal.xyz, L), 0.0f);
	Radiance r;																						
																						
	r.direction.xyz = L;
	r.flux.xyz = falloff * color * nDotL;
	return r;
};

Radiance PointLight(Surface s, float3 color, float3 worldpos)										
{																	
   /// outerWidth is the angle, in radians, from center to the outer edge of the cone
   /// innerWidth is the angle, in radians, from center to the inner edge
																						
	float3	L = normalize(worldpos - s.position.xyz);   /// Towards the light!																						
	float	nDotL = max(dot(s.normal.xyz, L), 0.0f);
	Radiance r;																						
																						
	r.direction.xyz = L;
	r.flux.xyz = color * nDotL;
	return r;
};

float shBasis0(float3 p) { return 0.282095; }
float shBasis1(float3 p) { return 0.488603 * p.z; }
float shBasis2(float3 p) { return 0.488603 * p.y; }
float shBasis3(float3 p) { return 0.488603 * p.x; }
float shBasis4(float3 p) { return 1.092548 * p.x * p.z; }
float shBasis5(float3 p) { return 1.092548 * p.y * p.z; }
float shBasis6(float3 p) { return 0.315392 * (3.0 * p.y * p.y - 1.0); }
float shBasis7(float3 p) { return 1.092548 * p.x * p.y; }
float shBasis8(float3 p) { return 0.546274 * (p.x*p.x - p.z*p.z); }

Radiance SphericalHarmonicLight(Surface s, float3 color)
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
	float intensity = length(color) / 4.0f;
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
	while (LightList[lightIndex].LightType > 0)                                                
	{                                                               				    
     [unroll] for (uint n = 0; n < 1; ++n, ++lightIndex)
		 {
			LightConstants	light = LightList[lightIndex];                         			    
			int				type = light.LightType;         				    
			Radiance		radiance;
			float3			lcolor = light.Color.xyz;

			radiance.flux = float4(0, 0, 0, 0);
			radiance.direction = float4(0, -1.0, 0, 0);
		// Light types are Vixen class IDs
		// 44 = ambient, 25 = point, 26 = directional, 27 = spot, 0 = end of list
			if (type == 26)
				radiance = DirectionalLight(surface, lcolor, light.WorldDir.xyz);
			else if (type == 25)
				radiance = PointLight(surface, lcolor, light.WorldPos.xyz);
			else if (type == 27)
				radiance = SpotLight(surface, lcolor, light.WorldDir.xyz, light.WorldPos.xyz, light.InnerAngle, light.OuterAngle);
			else if (type == 44)
				radiance = AmbientLight(surface, lcolor);
			color += AddLight(surface, radiance); 
		}
	}
	return color;
}