SurfaceShader SurfaceColor;

float4 _$SURFACESHADER(in VertexOutput v) : SV_Target0
{
	Surface		s;
	float3		color;
	int			enabled = LightsEnabled;
	int			nlights = NumLights;
	int			i;

	s = SurfaceColor.ComputeColor(v);
	color = float3(0, 0, 0);
	for (i = 0; i < nlights; ++i)
	{ 
		if ((enabled & 0x80000000) != 0)
		{
			Radiance r;
			r = LightList[i].Illuminate(s);
			color += SurfaceColor.ApplyLight(s, r);
		}
		enabled = enabled << 1;
	}
	return float4(color, 1.0);
}