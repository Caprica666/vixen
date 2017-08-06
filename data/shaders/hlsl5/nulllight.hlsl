
class NullLight : Light
{
	Radiance Illuminate(Surface s)
	{
		Radiance r;
		r.direction.xyz = float3(0.0, 0.0, 0.0);
		r.flux.xyz = float3(0, 0, 0);
		return r;
	}
};
