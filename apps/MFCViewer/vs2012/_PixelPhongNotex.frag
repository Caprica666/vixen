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

struct SpotLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
	float	InnerAngle;
	float	OuterAngle;
};

	
float ComputeFallOff(vec3 LPrime, vec3 D, float cosInner, float cosOuter)	
{																			
	float cosAngle = dot(LPrime, D);										
																				
	if      (cosAngle < cosOuter) return 0.0f;								
	else if (cosAngle > cosInner) return 1.0f;								
	else																	
	{																		
		float delta = (cosAngle - cosOuter) / (cosInner - cosOuter);			
		return delta*delta*(delta*delta);										
	}																		
}																																													
																	
Radiance SpotLight(Surface s, SpotLightData data)									
{																	
	// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	// InnerAngle is the angle, in radians, from center to the inner edge																							
	float	cosInner = cos(data.InnerAngle);
	float	cosOuter = cos(data.OuterAngle);
	vec3	L = normalize(s.position.xyz - data.WorldPos.xyz);   // Towards the light!																						
	float	falloff = ComputeFallOff(-L, data.WorldDir.xyz, cosInner, cosOuter);
	float	nDotL = max(dot(s.normal.xyz, L), 0.0);
	Radiance r;																						
																							
	r.direction.xyz = L;
	r.flux.xyz = data.Color.xyz * falloff * nDotL;
//	r.flux.xyz = data.Color.xyz * nDotL;
	return r;
};

struct DirectLightData
{
	vec4	WorldDir;
	vec4	WorldPos;
	vec4	Color;
	int		Decay;
};

Radiance DirectLight(Surface s, DirectLightData data)										
{																	
	/// OuterAngle is the angle, in radians, from center to the outer edge of the cone
	/// InnerAngle is the angle, in radians, from center to the inner edge																							
	vec3	L = normalize(data.WorldDir.xyz);   /// Towards the light!																						
	float	nDotL = max(dot(s.normal.xyz, L), 0.0);
	Radiance r;																						
																							
	r.direction.xyz = L;
	r.flux.xyz = data.Color.xyz * nDotL;
	return r;
}

uniform DirectLightData	light0;
uniform SpotLightData	light1;
uniform SpotLightData	light2;
vec3 LightPixel(Surface surface)				
{												
	vec3		color = vec3(0, 0, 0);			
	Radiance	r;								
	int			n = NumLights;					
	if (LightsEnabled[0] != 0)
	{
		Radiance r = DirectLight(surface, light0);
		color += AddLight(surface, r);
	}
	if (LightsEnabled[1] != 0)
	{
		Radiance r = SpotLight(surface, light1);
		color += AddLight(surface, r);
	}
	if (LightsEnabled[2] != 0)
	{
		Radiance r = SpotLight(surface, light2);
		color += AddLight(surface, r);
	}
	return color;
}


void main()
{
	Surface s;
	vec3	color = vec3(0.0, 0.0, 0.0);

	s = PixelPhongNotex();
	color = LightPixel(s);
	gl_FragColor = vec4(color, 1.0);
}
