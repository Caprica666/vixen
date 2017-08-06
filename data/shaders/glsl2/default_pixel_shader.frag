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

$LIGHTSOURCES

void main()
{
	Surface s;
	vec3	color = vec3(0.0, 0.0, 0.0);

	s = $SURFACESHADER();
	color = LightPixel(s);
	gl_FragColor = vec4(color, 1.0);
}
