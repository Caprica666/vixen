/****
 *
 * OpenGL specific lighting code:
 *	- Light overrides
 *	- GLLight implementation
 *
 ****/
#include "vixen.h"
#include "ogl/vxlightgl.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxshadergl.h"
#include "ogl/vbufgl.h"

namespace Vixen {

	namespace Core
	{
		template <> inline bool Core::Dict<NameProp, Core::String, Vixen::BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
		{
			return String(knew).CompareNoCase(kdict) == 0;
		}

		template <> inline uint32 Core::Dict<NameProp, Core::String, Vixen::BaseDict>::HashKey(const NameProp& np) const
		{
			return HashStr((const TCHAR*) np);
		}

	}

#define	LIGHT_ID	GL_LIGHT0
	
intptr GLRenderer::AddLight(Light* light)
{
	GLLight* prop = (GLLight*) light->DevHandle;

	if (prop)
		return prop->DevIndex;
	prop = new GLLight(*this, *light);
	prop->DevIndex = m_LightList.Attach(light, prop);
	if ((prop->DevIndex < 0) ||
		((int) prop->DevIndex >= MaxLights))
	{
		VX_ERROR(("AddLight ERROR cannot add light #%d\n", prop->DevIndex), -1);
	}
	return prop->DevIndex;
}

GLLight::GLLight(Renderer& r, const Light& l) : GPULight(r, l)
{
	LocalDir.Set(0, 0, 1);
}


GPULight*	GLLight::Clone() const
{
	GLLight* lnew = new GLLight(Render, *LightModel);
	*lnew = *this;
	return lnew;
}


/*!
 * @fn void GLLight::Load(Renderer& render)
 *
 * Loads the OpenGL light state from these light properties.
 * This function does not affect the position or direction of the light.
 *
 * @see GLLight::Update
 */
void GLLight::Load(bool changed)
{
//	GLRenderer& render = (GLRenderer&) Render;
//	GLBuffer::Update(this, render.GetProgram());
}


void GLLight::Detach()
{
	delete this;
}

/*
 * Generate the pixel shader source for each light
 * and determine the total size of the light constant buffer
 */
int	GLRenderer::LoadLights()
{
	LightList&		lights(m_LightList);
	bool			enabled;
	GLLight*		l;
	int				nlights = 0;
	int				nbytes = 0;
	int				numshaders = 0;
	Core::String	decls;
	Core::String	source;
	LightList::Iter	iter(lights);
	NameDict<Core::String>	shader_sources;
	Core::String	lightpixel =
		"vec3 LightPixel(Surface surface)				\n"													    
		"{												\n"																									    
		"	vec3		color = vec3(0, 0, 0);			\n"
		"	Radiance	r;								\n"
		"	int			n = NumLights;					\n";

	while (l = (GLLight*) iter.NextProp(enabled))
	{
		NameProp		shadername = l->LightModel->ClassName();
		const Shader*	shader = FindShader(shadername);
		const TCHAR*	shadersrc = NULL;
		Core::String	lightnum(nlights);

		LightsEnabled[l->ID] = enabled;
		if ((shader == NULL) || ((shadersrc = shader->GetSource()) == NULL))
		{
			VX_TRACE(GLRenderer::Debug, ("GLRenderer::LoadLights ERROR no shader found for %s\n", (const TCHAR*) shadername));
			++nlights;
			continue;
		}
		++numshaders;
		shader_sources[shadername] = shadersrc;
		decls += TEXT("uniform ");
		decls += l->LightModel->ClassName();
		decls += TEXT("Data\tlight") + lightnum + TEXT(";\n");
		lightpixel += TEXT("\tif (LightsEnabled[") + lightnum;
		lightpixel += TEXT("] != 0)\n");
		lightpixel += TEXT("\t{\n\t\tRadiance r = ");
		lightpixel += l->LightModel->ClassName();
		lightpixel += TEXT("(surface, light") + lightnum + TEXT(");\n");
		lightpixel += TEXT("\t\tcolor += AddLight(surface, r);\n\t}\n";
		++nlights;
	}
	lightpixel += "\treturn color;\n}\n");

	/*
	 * Add light source declarations.
	 */
	{
		NameDict<Core::String>::Iter niter(&shader_sources);
		Core::String* stringentry;

		while (stringentry = niter.Next())
			source += *stringentry;
	}
	VX_ASSERT(nlights < LIGHT_MaxLights);
	source = source + decls + lightpixel;
	m_LightShaderSource = source;
	if ((nlights > 0) && (numshaders == nlights))
		m_LightShaderChanged = false;
	return nlights;
}

void GLLight::Enable() const
{
	GLRenderer&	render = (GLRenderer&) Render;
	render.LightsEnabled[ID] = 1;
	VX_TRACE2(Light::Debug, ("GLLight::Enable #%d", DevIndex));
}

void GLLight::Disable() const
{
	GLRenderer&	render = (GLRenderer&) Render;
	render.LightsEnabled[ID] = 0;
	VX_TRACE2(Light::Debug, ("GLLight::Disable #%d", DevIndex));
}
}	// end Vixen