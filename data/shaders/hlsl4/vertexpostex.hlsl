#include "vertex_common.h"

struct VPosTex
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};


VOutPosNormTex VertexPosTex(VPosTex vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = float4(0, 1, 0, 0);
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(vout.position, ViewMatrix);
	vout.clipSpacePosition = mul(vout.clipSpacePosition, ProjMatrix);
	return vout;
};

