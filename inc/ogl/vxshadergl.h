/*!
 * @class GLScene
 *
 * The scene object contains global characteristics that define how
 * models are displayed. A scene describes how objects are
 * being viewed, the environment they are viewed in and
 * drives a simulation component which controls object behaviors.
 * GLScene is a subclass of the generic PSScene that uses
 * OpenGL as a geometry and rendering engine.
 *
 * @see vixengl.h
 */
#pragma once

namespace Vixen
{

class GLShader
{
public:
	GLShader() {};

	static const Shader*	Find(const TCHAR* name);
	static const Shader*	Create(const Shader*);
	static void				Release(const Shader* shader);
	static intptr			Compile(const Shader* shader, const TCHAR* source = NULL, const TCHAR* name = NULL);
	static	void			ReleasePixelShaders();
	static	void			ReleaseAll();

	static const int		NUM_VERTEX_SHADERS = 16;
	static Ref<NameTable>	ShaderDict;		// table of compiled shaders indexed by name
	static Ref<Shader>		VertexShaders[NUM_VERTEX_SHADERS];
};

	
class GLProgram
{
public:
	GLProgram() { m_GLProgram = 0; }

	GLuint			GetProgram()	{ return (GLuint) m_GLProgram; }
	GLuint			FindProgram(const TCHAR* key);
	GLuint			NewProgram(const TCHAR* key);
	bool			LinkProgram(GLuint vertshader, GLuint pixshader);
	void			UpdateMaterial(const DeviceBuffer* mat);

	static bool		UpdateTextures(GLRenderer* render, const Appearance* app);
	static intptr	UpdateSampler(GLRenderer* render, const Sampler* smp, int texunit);
	static	void	ReleaseAll();
	static	const Shader*	SelectPixelShader(GLRenderer* render, const Appearance* app, int vertex_style, bool& compiled);

protected:
	GLuint						m_GLProgram;
	static NameDict< int32 >*	ProgramDict;	// maps vertex & pixel shader name combo into program id
};

namespace Core
{

template <> inline bool Core::Dict<NameProp, int32, Vixen::BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
{
	return String(knew).CompareNoCase(kdict) == 0;
}

template <> inline uint32 Core::Dict<NameProp, int32, Vixen::BaseDict>::HashKey(const NameProp& np) const
{
	return HashStr((const TCHAR*) np);
}

}

}	// end Vixen

