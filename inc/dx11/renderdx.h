#pragma once

namespace Vixen
{
	class DXRenderer : public GeoSorter
	{
	public:
		enum CBufSlot
		{
			CBUF_PERFRAME = 0,
			CBUF_PEROBJECT = 1,
			CBUF_LIGHT = 2,
			CBUF_MATERIAL = 3,
			CBUF_MAX = CBUF_MATERIAL
		};

		DXRenderer();
		DXRenderer&	operator=(const DXRenderer& src);

		D3DDEVICE*		GetDevice() const	{ return m_Device; }
		D3DCONTEXT*		GetContext() const	{ return m_Context; }
		virtual	bool	Init(Scene* scene, Vixen::Window win, const TCHAR* options = NULL);
		virtual	void	Begin(int changed, int frame);
		virtual	intptr	AddLight(Light* light);
		virtual	void	Flip(int frame);
		virtual void	RenderState(int32 stateindex);
		virtual	void	RenderMesh(const Geometry* geo, const Appearance* app, const Matrix* mtx);
		virtual	void	Exit(bool shutdown = true);
		virtual	void	SetViewport(float left, float top, float right, float bottom);
		virtual	intptr	Compile(const Shader* shader);
		virtual	SharedObj*		Clone() const;
		virtual	const Shader*	InstallShader(const Shader*);
		virtual	const Shader*	FindShader(const TCHAR* name);

		intptr			CompilePixelShader(const Shader* shader);
		intptr			UpdateTexture(Texture* texture, int texunit, bool mipmap);
		DeviceBuffer*	GetBuffer(int type)	{ return m_ConstantBuffers[type]; }

	protected:
		void			UpdateAppearance(const Appearance* app, const VertexArray* verts);
		bool			UpdateMaterial(const Material* mtl, const VertexArray* verts);
		void			MakeDefaultShaders();
		int				MakeLightDesc();
		int				LoadLights();
		void			FreeResources();
		void			FreeBuffers();
		bool			CreateBuffers(int w, int h);
		bool			CreateDevice(HWND win, int width, int height);
		ID3D11View*		ChangeRenderTarget(Bitmap* bmap, DXGI_FORMAT format, D3D11_BIND_FLAG type);

		Ref<DeviceBuffer>		m_ConstantBuffers[CBUF_MAX + 1]; // constant buffer list
		Core::String			m_LightShaderSource;
		D3DDEVICE*				m_Device;				// The D3D rendering device
		D3DCONTEXT*				m_Context;
		DXGI_FORMAT				m_ColorFormat;			// format of color buffer
		DXGI_FORMAT				m_DepthFormat;
		ID3D11RenderTargetView*	m_ColorSurface;			// current color buffer
		ID3D11DepthStencilView*	m_DepthSurface;			// current depth buffer
		D3DTEXTURE2D*			m_DepthTexture;			// current depth texture resource
		IDXGISwapChain*			m_SwapChain;			// The DX swap chain
		Ref<Appearance>			m_DefaultAppear;		// default appearance

	public:
		static DXGI_FORMAT		FontFormat;
		ID3D11BlendState*		BlendState;			// current blend state
		ID3D11RasterizerState*	RasterState;		// current rasterizer state
		int						RenderStateIndex;	// 0 = rendering transparent, else opaque

		static Ref<Shader>		VertexShaders[16];
		static Ref<Shader>		PixelShaderTemplate;
		static Ref<Appearance>	CurrentAppear;

	};


	class DXShader
	{
	public:
		DXShader() {};
		~DXShader() { Release(); }

		static DXShader*	Find(const TCHAR* name);
		static DXShader&	Create(const Shader* source);
		const TCHAR*		GetSource();
		void				Release();
		void				LinkLights(D3DCONTEXT* ctx, LightList& lightlist);
		ID3D11DeviceChild*	Compile(DXRenderer* render, const TCHAR* source = NULL, const TCHAR* name = NULL);
		static	void		ReleasePixelShaders();

		Ref<Shader>						SceneShader;		// first shader in scene with this name
		DXRef<ID3D11DeviceChild>		ShaderObject;		// -> DX11 shader object
		DXRef<ID3D11ClassInstance>		ShaderClass;		// -> surface shader class
		DXRef<ID3D11ShaderReflection>	Reflector;			// -> allows shader introspection

		static DXRef<ID3D11ClassLinkage>	ClassLinkage;
		static NameDict<DXShader>			ShaderDict;		// table of compiled shaders indexed by name
	};

	class DXLight : public GPULight
	{
	public:
		DXLight(DXRenderer& render, const Light& sourcelight) : GPULight(render, sourcelight)
		{
			SetName(sourcelight.ClassName());
			LocalDir.Set(0, 0, 1);
		}
		DXLight&				operator=(const GPULight& src);
		virtual	void			Detach();
		ID3D11ClassInstance*	GetClassInstance(const TCHAR* name);

		static	void					UpdateBuffer(LightList& lights, void* buffer);
		static	Core::String			NullLight();
		static	ID3D11ClassInstance*	GetNullInstance(const TCHAR* name);
		static	int						LoadLights(LightList& lights, Core::String& source);

		int							VectorOffset;
		DXRef<ID3D11ClassInstance>	LightClassInstance;
		static	int					NumLights;
	};

	template <> inline bool Core::Dict<NameProp, DXShader, Vixen::BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
	{
	//	return STRCASECMP(knew, kdict) == 0;
		return String(knew).CompareNoCase(kdict) == 0;
	}

	template <> inline uint32 Core::Dict<NameProp, DXShader, Vixen::BaseDict>::HashKey(const NameProp& np) const
	{
		return HashStr((const TCHAR*) np);
	}
}  // end Vixen
