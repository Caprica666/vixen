uniform	mat4		WorldMatrix;
uniform	mat4		ViewMatrix;
uniform	mat4		ProjMatrix;

attribute	vec4	position;
attribute	vec4	normal;
attribute	vec2	texcoord;

varying		vec4	world_position;
varying		vec4	world_normal;
varying		vec4	texcoord0;

void main()
{
#if 0
	world_position = WorldMatrix * position;
	world_normal = WorldMatrix * vec4(normal.xyz, 0.0);
	gl_Position = ViewMatrix * world_position;
	gl_Position = ProjMatrix * gl_Position;
#else
	world_position = position * WorldMatrix;
	world_normal = vec4(normal.xyz, 0.0) * WorldMatrix;
	gl_Position = world_position * ViewMatrix;
	gl_Position = gl_Position * ProjMatrix;
#endif
	texcoord0 = vec4(texcoord, 0, 0);
}


