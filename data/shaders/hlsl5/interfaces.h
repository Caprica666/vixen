
interface SurfaceShader
{
	Surface ComputeColor(in VertexOutput v);
	float3	ApplyLight(Surface s, Radiance r);
};

interface PostProcessor
{
	float4	Process(uint2 PixelCoord, float2 ImageCoord, float3 color);
};

interface Light
{
	Radiance	Illuminate(Surface s);
};

