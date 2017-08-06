// per object constants
uniform mat4 WorldMatrix;

// per frame constants
uniform mat4	ViewMatrix;
uniform mat4	ViewProjMatrix;
uniform mat4	InverseViewMatrix;

struct VOutPosNormTex
{
	vec4 position;
	vec4 normal;
	vec4 texcoord;
};

struct VOutPosNormTexTang
{
	vec4 normal;
	vec4 texcoord;
	vec4 tangent;
	vec4 bitangent;
};