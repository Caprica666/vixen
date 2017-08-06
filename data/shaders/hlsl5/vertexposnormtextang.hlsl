#include "vertex_common.h"

struct VPosNormTexTang
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float4 tangent : TANGENT;
	float4 bitangent : BINORMAL;
};


VOutPosNormTexTang VertexPosNormTexTang(VPosNormTexTang vin)
{
	VOutPosNormTexTang vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(float4(vin.normal.xyz, 0.0), WorldMatrix);
	vout.tangent = mul(float4(vin.tangent.xyz, 0.0), WorldMatrix);
	vout.bitangent = mul(float4(vin.bitangent.xyz, 0.0), WorldMatrix);
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(vout.position, ViewMatrix);
	vout.clipSpacePosition = mul(vout.clipSpacePosition, ProjMatrix);
	return vout;
};

