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

