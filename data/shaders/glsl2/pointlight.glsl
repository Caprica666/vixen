struct PointLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
};

Radiance Illuminate(Surface s, PointLightData data)
{
	vec3	L = normalize(data.WorldPos - s.position.xyz);   /// Towards the light!																						
	vec3	nDotL = max(dot(s.normal.xyz, L), 0.0f);
	Radiance r;																						
																							
	r.direction.xyz = L;
	r.flux.xyz = data.Color.xyz * nDotL;
	return r;
}

