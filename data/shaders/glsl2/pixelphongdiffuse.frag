precision mediump float;

// Phong material constants
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;

uniform bool HasSpecularMap;
uniform bool HasDiffuseMap;
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

Surface PixelPhongDiffuse()															
{																								
	/// We assume here that all geometric quantities are in world-space
	Surface s;																					
	s.diffuse = Diffuse;
	s.specular = Specular;
	s.position = world_position;
	if (HasDiffuseMap)
		s.diffuse *= texture2D(DiffuseMap, texcoord0.xy);
	s.normal = normalize(world_normal);
	return s;
}
