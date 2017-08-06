#include "vertex_common.h"

struct VPos
{
	float4	position : POSITION;
};

VOutPosNormTex VertexPos(VPos vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
 	vout.normal = float4(0, 0, 1, 0);
	vout.texcoord = float4(0, 0, 0, 0);
	vout.clipSpacePosition = mul(vout.position, ViewMatrix);
	vout.clipSpacePosition = mul(vout.clipSpacePosition, ProjMatrix);
	return vout;
};
