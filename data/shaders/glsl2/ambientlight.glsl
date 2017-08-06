
struct AmbientLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
};

Radiance AmbientLight(in Surface s, in AmbientLightData data)
{
	Radiance r;
	r.direction.xyz = vec3(0.0, -1.0, 0.0);
	r.flux.xyz = data.Color.xyz;
	return r;
}
