precision mediump float;

// per object constants
uniform mat4 WorldMatrix;

// per frame constants
uniform mat4	ViewMatrix;
uniform mat4	ViewProjMatrix;
uniform	vec3	CameraPos;
uniform	vec2	ImageSize;


void main()
{
	Surface s;

	s = $SURFACESHADER();
	gl_FragColor = vec4(s.Diffuse.xyz, 1.0);
}
