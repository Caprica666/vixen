#include "common.h"
#include "phong_lighting.h"

												
float4 PixelPhongNotex(in VOutPosNormTex v) : SV_Target0
{
	Surface s;
	s.diffuse = float4(1.0,1.0,1.0,1.0);
	s.specular = Specular;
	s.position = float4(v.position.xyz, 1.0);
	s.diffuse = Diffuse;
	if (s.diffuse.w < 0.4) discard;
	s.normal = float4(normalize(v.normal.xyz), 0.0);
	return float4(LightPixel(s), 1.0f);
}
