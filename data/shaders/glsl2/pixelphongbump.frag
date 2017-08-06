precision mediump float;

// Phong material constants
uniform sampler2D NormalMap;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform bool HasSpecularMap = true;
uniform bool HasNormalMap   = true;
uniform vec4 Diffuse;
uniform vec4 Specular;


varying		vec4	world_position;
varying		vec4	world_normal;
varying		vec4	texcoord0;
varying		vec4	tangent0;
varying		vec4	bitangent0;

struct Surface
{
   vec4 position;
   vec4 normal;
   vec4 diffuse;
   vec4 specular;
};

Surface PixelPhongBump()															
{																								
	/// We assume here that all geometric quantities are in world-space
	Surface s;																					
	s.diffuse = vec4(1.0);
	s.specular = vec4(0.0);
	s.position = world_position;
	if (HasDiffuseMap)
		s.diffuse *= texture2D(DiffuseMap, texcoord0.xy);
	if (HasSpecularMap)  s.Specular *= texture2D(SpecularMap, texcoord0);
	if (HasNormalMap)
	{
		vec3 n = (texture2D(NormalMap, texcoord0.xy).xyz) * 2.0 - 1.0;
		s.normal = normalize(n.x * tangent0 + n.y * bitangent0 + n.z * world_normal);
	}
	else																							
		s.normal = normalize(world_normal);
	return s;
}