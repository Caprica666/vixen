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
	vec3 kD = s.diffuse.xyz * r.flux.xyz;
	return kD;
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
