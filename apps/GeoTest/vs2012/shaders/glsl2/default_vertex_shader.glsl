uniform mat4	WorldMatrix;
uniform mat4	ViewProjMatrix;

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

