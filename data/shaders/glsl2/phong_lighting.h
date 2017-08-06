
uniform sampler2D NormalMap;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform int DiffuseType  = 1;
uniform bool HasSpecularMap = true;
uniform bool HasNormalMap   = true;
uniform vec4 Diffuse;
uniform vec4 Specular;

struct Surface
{
   float4 position;
   float4 normal;
   float4 diffuse;
   float4 specular;
};

struct Radiance
{
   float4 flux;
   float4 direction;
};

float3 AddLight(Surface s, Radiance r)
{
	vec3 L = r.direction.xyz;	// From surface towards light, unit length, world-space
	vec3 I = r.flux.xyz;		// N . L already accounted for
	vec3 campos = CameraPos.xyz;

	// Vector from surface location to the camera's eye
	vec3 E = normalize(campos - s.position.xyz);
	vec3 H = normalize(L+E);
	vec3 kD = s.diffuse.xyz * I;
	vec3 kS = s.specular.xyz * I;
	if (Shine > 0)
		kS *= pow(max(dot(s.normal.xyz, H), 0.0), Shine);
	return kD + kS;
}
