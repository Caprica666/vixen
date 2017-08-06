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
class GLRenderer : public GeoSorter
{
	class PBuffer
	{
	public:
		PBuffer()	{ m_DevInfo = NULL; m_Buffer = 0; }
		~PBuffer()	{ Free(); }

		intptr	GetBuffer()		{ return m_Buffer; }
		void	SetDev(DeviceInfo* d);
		bool	SetSize(int width, int height);
		void	Free();
	private:
		intptr		m_Buffer;		// pbuffer for this scene
		DeviceInfo*	m_DevInfo;
		int			m_Width;
		int			m_Height;
	};

public:
	VX_DECLARE_CLASS(GLRenderer);
//	Initializers
	GLRenderer();
	GLRenderer&		operator=(const GLRenderer& src);

// Scene overrides
	virtual intptr			AddLight(Light* light);
	virtual void			SetViewport(float l, float t, float r, float b);
	virtual void			Begin(int changed, int frame);
	virtual void			End(int frame);
	virtual void			Flip(int frame);
	virtual	void			Exit(bool shutdown = true);
	virtual	void			RenderState(int32 stateindex);
	virtual	bool			Init(Scene* scene, Vixen::Window win, const TCHAR* opts);
	virtual	SharedObj*		Clone() const;
	virtual	void			RenderMesh(const Geometry* geo, const Appearance* app, const Matrix* mtx);
	virtual intptr			Compile(const Shader* shader);
	virtual const Shader*	FindShader(const TCHAR* name);
	virtual const Shader*	InstallShader(const Shader*);
	intptr					UpdateTexture(Texture* texture, int texunit, bool mipmap);
	GLuint					GetProgram() { return m_CurProgram; }
	bool					GeneratePixelShader(Core::String& name, Core::String& source, bool dolighting);

	static int				NumTexUnits;
	int						NumLights;
	int						LightsEnabled[LIGHT_MaxLights];
	Ref<PhongMaterial>		DefaultMaterial;

protected:
	int				UpdateAppearance(const Appearance* app, int vertex_style);
	bool			MakeGLContext();
	void			SelectGLContext(int changed);
	void			ChangeRenderTarget(Bitmap*);
	void			DeleteTextures();
	virtual	void	ReleaseGL();
	void			MakeDefaultShaders();
	int				LoadLights();
	void			InitPerFrame(intptr program, int changed);

#if defined(VIXEN_GLES2) && !defined(VIXEN_GLFW)
	EGLDisplay		m_Display;
	EGLSurface		m_Surface;
#else
	intptr			CreateDDSTexture(Bitmap* bmap);
#endif
	int				m_Changed;
	bool			m_LightShaderChanged;
	GLuint			m_CurProgram;			// current GL program
	Core::String	m_LightShaderSource;	// source for light shaders
	Core::String	m_NoLightSource;		// shader code for no light sources

	Matrix			m_ProjMatrix;
	Matrix			m_ViewMatrix;
	PBuffer			m_PBuffer;			// pbuffer for this scene (only on Windows currently)
	bool			b_Transpose;		// need to transpose matrix?

	static int					s_MaxTexID;
	static const Appearance*	s_CurAppear;
	static Ref<Appearance>		s_DefaultAppear;
};

}	// end Vixen

