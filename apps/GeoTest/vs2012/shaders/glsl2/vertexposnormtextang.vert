
uniform	mat4		WorldMatrix;
uniform	mat4		ViewMatrix;
uniform	mat4		ProjMatrix;

attribute	vec4	position;
attribute	vec4	normal;
attribute	vec2	texcoord;
attribute	vec4	tangent;
attribute	vec4	bitangent;

varying		vec4	world_position;
varying		vec4	world_normal;
varying		vec4	texcoord0;
varying		vec4	tangent0;
varying		vec4	bitangent0;

void main()
{
	world_position = position * WorldMatrix;
	world_normal = vec4(normal.xyz, 0.0) * WorldMatrix;
	tangent0 = vec4(tangent.xyz, 0.0) * WorldMatrix;
	bitangent0 = vec4(bitangent.xyz, 0.0) * WorldMatrix;
	gl_Position = world_position * ViewMatrix;
	gl_Position = gl_Position * ProjMatrix;
	texcoord0 = vec4(texcoord, 0, 0);
}
