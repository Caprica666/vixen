

cbuffer PerFrameConstants : register(b0)
{
	float4x4	ViewMatrix;
	float4x4	ProjMatrix;
	float4		CameraPos;
	uint		NumLights;
	int			LightsEnabled;
	uint2		ImageSize;
};

cbuffer PerObjectConstants : register(b1)
{
	matrix WorldMatrix;
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
