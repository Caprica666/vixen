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
precision mediump float;

// per object constants
uniform mat4 WorldMatrix;

// per frame constants
uniform mat4	ViewMatrix;
uniform mat4	ViewProjMatrix;
uniform	vec3	CameraPos;
uniform	vec2	ImageSize;
uniform int		NumLights;
uniform int		LightsEnabled[16];

struct Radiance
{
   vec4 flux;
   vec4 direction;
};

vec3 AddLight(Surface s, Radiance r)
{
	vec3 L = r.direction.xyz;	// From surface towards light, unit length, world-space
	vec3 I = r.flux.xyz;		// N . L already accounted for
	vec3 campos = CameraPos;

	// Vector from surface location to the camera's eye
	vec3 E = normalize(campos - s.position.xyz);
	vec3 H = normalize(L + E);
	vec3 kD = s.diffuse.xyz * I;
	//vec3 kS = s.specular.xyz * I * pow(max(dot(s.normal.xyz, H), 0.0), s.specular.w);
	vec3 kS = s.specular.xyz * I * pow(max(dot(s.normal.xyz, H), 0.0), 5.0);
	return kD + kS;
}

vec3 LightPixel(Surface s) { return s.diffuse.xyz; }

void main()
{
	Surface s;
	vec3	color = vec3(0.0, 0.0, 0.0);

	s = PixelPhongDiffuse();
	color = LightPixel(s);
	gl_FragColor = vec4(color, 1.0);
}
