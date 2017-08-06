#include "vertex_common.h"

struct VPosNormTex
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

VOutPosNormTex VertexPosNormTex(VPosNormTex vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(float4(vin.normal.xyz, 0.0), WorldMatrix);
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(vout.position, ViewMatrix);
	vout.clipSpacePosition = mul(vout.clipSpacePosition, ProjMatrix);
	return vout;
};


