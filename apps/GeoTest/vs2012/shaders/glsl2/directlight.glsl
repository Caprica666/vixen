struct DirectLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
};

Radiance DirectLight(Surface s, DirectLightData data)										
{																	
	/// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	/// InnerAngle is the angle, in radians, from center to the inner edge																							
	vec3	L = normalize(data.WorldDir.xyz);   /// Towards the light!																						
	float	nDotL = max(dot(s.normal.xyz, L), 0.0);
	Radiance r;																						
																							
	r.direction.xyz = L;
	r.flux.xyz = data.Color.xyz * nDotL;
	return r;
}

