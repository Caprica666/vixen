class SpotLight : Light
{
	float4	WorldDir;
	float4	WorldPos;
	float4	Color;
	int		Decay;

	float	InnerAngle;
	float	OuterAngle;
	
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
																	
	Radiance Illuminate(Surface s)										
	{																	
	   /// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	   /// InnerAngle is the angle, in radians, from center to the inner edge																							
		float	cosInner = cos(InnerAngle);
		float	cosOuter = cos(OuterAngle);
		float3	L = normalize(WorldPos - s.position.xyz);   /// Towards the light!																						
		float	falloff = ComputeFallOff(L, WorldDir, cosInner, cosOuter);
		float	nDotL = max(dot(s.normal.xyz, L), 0.0f);
		Radiance r;																						
																							
		r.direction.xyz = L;
		r.flux.xyz = falloff * Color.xyz * nDotL;
		return r;
	}
};

