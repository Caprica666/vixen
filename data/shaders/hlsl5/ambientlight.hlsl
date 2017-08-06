
class AmbientLight : Light
{
	float4	WorldDir;
	float4	WorldPos;
	float4	Color;
	int		Decay;

	Radiance Illuminate(Surface s)
	{
		Radiance r;
		r.direction.xyz = float3(0.0, -1.0, 0.0);
		r.flux.xyz = Color.xyz;
		return r;
	}
};
