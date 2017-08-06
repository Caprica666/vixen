
cbuffer PerFrameConstants : register(cb0)
{
	matrix	ViewMatrix;
	matrix	ProjMatrix;
	float3	CameraPos;
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