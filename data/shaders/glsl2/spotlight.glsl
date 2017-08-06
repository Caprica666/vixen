struct SpotLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
	float	InnerAngle;
	float	OuterAngle;
};

	
float ComputeFallOff(vec3 LPrime, vec3 D, float cosInner, float cosOuter)	
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
																	
Radiance SpotLight(Surface s, SpotLightData data)									
{																	
	// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	// InnerAngle is the angle, in radians, from center to the inner edge																							
	float	cosInner = cos(data.InnerAngle);
	float	cosOuter = cos(data.OuterAngle);
	vec3	L = normalize(s.position.xyz - data.WorldPos.xyz);   // Towards the light!																						
	float	falloff = ComputeFallOff(-L, data.WorldDir.xyz, cosInner, cosOuter);
	float	nDotL = max(dot(s.normal.xyz, L), 0.0);
	Radiance r;																						
																							
	r.direction.xyz = L;
	r.flux.xyz = data.Color.xyz * falloff * nDotL;
//	r.flux.xyz = data.Color.xyz * nDotL;
	return r;
};

