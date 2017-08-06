
class PointLight : Light
{
	float4	WorldDir;
	float4	WorldPos;
	float4	Color;
	int		Decay;

	Radiance Illuminate(Surface s)
	{
		float3	L = normalize(WorldPos - s.position.xyz);   /// Towards the light!																						
		float	nDotL = max(dot(s.normal.xyz, L), 0.0f);
		Radiance r;																						
																							
		r.direction.xyz = L;
		r.flux.xyz = Color.xyz * nDotL;
		return r;
	}
};
