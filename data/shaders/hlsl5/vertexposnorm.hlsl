#include "vertex_common.h"

struct VPosNorm
{
	float4 position : POSITION;
	float4 normal : NORMAL;
};


VOutPosNormTex VertexPosNorm(VPosNorm vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(float4(vin.normal.xyz, 0.0), WorldMatrix);
	vout.texcoord = float4(0, 0, 0, 0);
	vout.clipSpacePosition = mul(vout.position, ViewMatrix);
	vout.clipSpacePosition = mul(vout.clipSpacePosition, ProjMatrix);
	return vout;
};

