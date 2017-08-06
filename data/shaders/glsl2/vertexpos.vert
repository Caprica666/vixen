uniform		mat4	WorldMatrix;
uniform	mat4		ViewMatrix;
uniform	mat4		ProjMatrix;

attribute	vec4	position;

varying		vec4	world_position;
varying		vec4	world_normal;
varying		vec4	texcoord0;

void main()
{
#if 0
	world_position = WorldMatrix * position;
	gl_Position = ViewMatrix * world_position;
	gl_Position = ProjMatrix * gl_Position;
 	world_normal = vec4(0, 0, 1, 0);
#else
	world_position = position * WorldMatrix;
	world_normal = vec4(normal.xyz, 0.0) * WorldMatrix;
	gl_Position = world_position * ViewPatrix;
	gl_Position = gl_Position * ProjMatrix;
#endif
	texcoord0 = vec4(0, 0, 0, 0);
}
