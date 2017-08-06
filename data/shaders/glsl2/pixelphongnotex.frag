precision mediump float;

uniform vec4 Diffuse;
uniform vec4 Specular;

varying	vec4 world_position;
varying	vec4 world_normal;
varying	vec4 texcoord0;

struct Surface
{
   vec4 position;
   vec4 normal;
   vec4 diffuse;
   vec4 specular;
};
											
Surface PixelPhongNotex()
{
	Surface s;
	s.position = vec4(world_position.xyz, 1.0);
	s.diffuse = Diffuse;
	s.specular = Specular;
	s.normal = normalize(world_normal);
	return s;
}
