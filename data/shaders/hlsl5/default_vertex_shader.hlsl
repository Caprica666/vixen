cbuffer PerObjectConstants
{
	row_major float4x4 WorldMatrix;
};

cbuffer PerFrameConstants
{
	row_major float4x4	ViewMatrix;
	row_major float4x4	ViewProjection;
	row_major float4x4	InverseViewMatrix;
};

struct VOutPosNormTex
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
};

struct VOutPosNormTexTang
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 texcoord : TEXCOORD;
	float4 clipSpacePosition : SV_POSITION;
	float4 tangent : TANGENT;
	float4 bitangent : BINORMAL;
};


struct VPos
{
	float4	position : POSITION;
};

struct VPosNorm
{
	float4 position : POSITION;
	float4 normal : NORMAL;
};

struct VPosTex
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VPosNormTex
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct VPosNormTexTang
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float4 tangent : TANGENT;
	float4 bitangent : BINORMAL;
};

VOutPosNormTex VertexPos(VPos vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
 	vout.normal = float4(0, 0, 0, 0);
	vout.texcoord = float4(0, 0, 0, 0);
	vout.clipSpacePosition = mul(ViewProjection, vout.position);
	return vout;
};

VOutPosNormTex VertexPosNorm(VPosNorm vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(WorldMatrix, float4(vin.normal.xyz, 0.0));
	vout.texcoord = float4(0, 0, 0, 0);
	vout.clipSpacePosition = mul(ViewProjection, vout.position );
	return vout;
};

VOutPosNormTex VertexPosTex(VPosTex vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(ViewProjection, vout.position );
	return vout;
};

VOutPosNormTex VertexPosNormTex(VPosNormTex vin)
{
	VOutPosNormTex vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(WorldMatrix, float4(vin.normal.xyz, 0.0));
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(ViewProjection, vout.position );
	return vout;
};

VOutPosNormTexTang VertexPosNormTexTang(VPosNormTexTang vin)
{
	VOutPosNormTexTang vout;
	vout.position = mul(vin.position, WorldMatrix);
	vout.normal = mul(WorldMatrix, float4(vin.normal.xyz, 0.0));
	vout.tangent = mul(WorldMatrix, float4(vin.tangent.xyz, 0.0));
	vout.bitangent = mul(WorldMatrix, float4(vin.bitangent.xyz, 0.0));
	vout.texcoord = float4(vin.texcoord.xy, 0, 0);
	vout.clipSpacePosition = mul(ViewProjection, vout.position );
	return vout;
};

