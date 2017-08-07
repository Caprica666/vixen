#include "dx11/vixendx11.h"

#pragma comment(lib,"d3dcompiler.lib")


namespace Vixen {

Ref<Shader>		DXRenderer::PixelShaderTemplate;
Ref<Shader>		DXRenderer::VertexShaders[16];

NameDict<DXShader>			DXShader::ShaderDict;		// table of compiled shaders indexed by name
DXRef<ID3D11ClassLinkage>	DXShader::ClassLinkage;

//! Compile the shader into binary code and return a device-specific handle to the code
#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

/*
 * Compiles a DX11 pixel shader based on a Vixen Shader.
 * @param shader	Vixen Shader to compile
 *
 * A Vixen pixel shader can replace the entire DX11 pixel shader or
 * only the surface shader. In the first case, the Vixen lights are
 * ignored and the Material attached to the Vixen Appearance can have
 * a user-defined format. These Vixen shaders are of type Shader::PIXEL
 * and are compiled directly from source to DX11 shader objects.
 *
 * If the Vixen pixel shader has type Shader::PIXEL | Shader::CLASS,
 * it is a surface shader only and computes the surface color of the
 * object being rendered before any lighting is applied.
 * The user-supplied surface shader will be wrapped by the Vixen
 * pixel shader template which uses a Phong lighting model to light
 * the pixels after the color is computed with user code.
 * The user-coded shader must accept the Vixen vertex shader output
 * structure and produce the Vixen surface output format
 * to compile and work with the default Phong lighting model.
 *
 * Note that it is entirely normal for this function to return 0
 * until the shader sources have been loaded and are available for compilation.
 * 
 * @return -> DX11 pixel shader or NULL on failure
 */
intptr		DXRenderer::CompilePixelShader(const Shader* shader)
{
	const TCHAR*	name = shader->GetName();

	if (name == NULL)
		VX_ERROR(("DXRenderer::Compile: ERROR cannot compile shader - name of entry point missing\n"), NULL);
	if (shader->GetInputLayout() == NULL)
		VX_ERROR(("DXRenderer::Compile: ERROR cannot compile shader - input layout missing\n"), NULL);

	DXShader*			dxshader = &DXShader::Create(shader);
	ID3D11PixelShader*	pixshader = (ID3D11PixelShader*) *(dxshader->ShaderObject);
	const TCHAR*		psrc = NULL;
	int					shadertype = shader->GetShaderType();
	Core::String		entryname(name);

	if ((pixshader == NULL) || shader->HasChanged())
	{
		ID3D11ClassInstance*	classInstance;
		Core::String			source;

		if (shadertype == (Shader::PIXEL | Shader::CLASS))
		{
			DXShader*		pixeltemplate = DXShader::Find(TEXT("DefaultPixelShader"));
			const TCHAR*	templatesource = NULL;

			if (!pixeltemplate || !(templatesource = pixeltemplate->GetSource()))
				VX_ERROR(("DXRenderer::CompilePixelShader cannot find pixel shader template\n"), NULL);
			source = shader->GetSource();
			source += m_LightShaderSource;
			source += templatesource;
			source.Replace(TEXT("$SURFACESHADER"), name);
			entryname = TEXT('_');
			entryname += name;
			psrc = source;
#ifdef _DEBUG
			{
				Core::FileStream shaderout;
				TCHAR	filebuf[VX_MaxPath];
		
				Core::Stream::ParseDirectory(shader->GetFileName(), NULL, filebuf);
				STRCAT(filebuf, TEXT("\\"));
				STRCAT(filebuf, entryname);
				STRCAT(filebuf, TEXT(".hlsl"));
				if (shaderout.Open(filebuf, Core::FileStream::OPEN_WRITE))
					shaderout.Write(psrc, strlen(psrc));
				shaderout.Close();
			}
#endif
		}
		pixshader = (ID3D11PixelShader*) dxshader->Compile(this, psrc, entryname);
		classInstance = dxshader->ShaderClass;
		if (classInstance == NULL)
		{
			dxshader->ClassLinkage->CreateClassInstance(name, DXRenderer::CBUF_LIGHT, 0, 0, 0, &classInstance);
			dxshader->ShaderClass = classInstance;
			VX_TRACE(Debug, ("DXShader::CompilePixelShader creating instance of class %s\n", name));
		}
	}
	if (pixshader)
		dxshader->LinkLights(m_Context, m_LightList);
	shader->SetChanged(false);
	return (intptr) pixshader;
}

/*
 * Compiles a Vixen vertex or pixel shader
 * @param shader	Vixen shader to compile
 *
 * If the Vixen Shader has already been compiled, the DX11 shader
 * associated with it is returned. Otherwise, the DX11 shader is
 * compiled first. Some DX11 shaders may be precompiled as part of
 * the Vixen build while others will be compiled from source.
 *
 * Note that it is entirely normal for this function to return 0
 * until the shader sources have been loaded and are available for compilation.
 * 
 * @return -> DX11 vertex or pixel shader, 0 if shader is not available
 *			(file has not been loaded, shader is missing, compilation failed)
 */
intptr		DXRenderer::Compile(const Shader* shader)
{
	const TCHAR*	name = shader->GetName();
	if (name == NULL)
		VX_ERROR(("DXRenderer::Compile: ERROR cannot compile shader - name of entry point missing\n"), NULL);
	if (shader->GetInputLayout() == NULL)
		VX_ERROR(("DXRenderer::Compile: ERROR cannot compile shader - input layout missing\n"), NULL);

	DXShader&			dxshader = DXShader::Create(shader);
	ID3D11DeviceChild*	shaderobj = dxshader.ShaderObject;

	if ((shaderobj == NULL) || shader->HasChanged())
	{
		shaderobj = dxshader.Compile(this);
		shader->SetChanged(false);
	}
	return (intptr) shaderobj;
}

#define SHADER_PATH "C:\\projects\\vixen\\data\\shaders\\hlsl5\\"

/*
 * Makes and installs the default vertex and pixel shaders.
 */
void DXRenderer::MakeDefaultShaders()
{
	Shader*			s;
	const TCHAR*	psout = TEXT("float4 position, float4 normal, float4 diffuse, float4 specular");
	const TCHAR*	vstex = TEXT("float4 position, float4 normal, float4 texcoord, float4 sv_position");
	const TCHAR*	vstextan = TEXT("float4 position, float4 normal, float4 texcoord, float4 sv_position, float4 tangent, float4 bitangent");
	const TCHAR*	radiance = TEXT("float4 flux, float4 radiance");
	const TCHAR*	light = TEXT("float4 WorldDir, float4 WorldPos, float4 Color, int Decay");
	ID3D11ClassLinkage*	linkage;

	m_Device->CreateClassLinkage(&linkage);
	DXShader::ClassLinkage = linkage;

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("NullLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("AmbientLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "ambientlight.hlsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("DirectLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "directlight.hlsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("PointLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "pointlight.hlsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("SpotLight"));
	s->SetInputDesc(TEXT("float4 WorldDir, float4 WorldPos, float4 Color, int Decay, float InnerAngle, float OuterAngle"));
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "spotlight.hlsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("DefaultPixelShader"));
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "default_pixel_shader.hlsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongNotex"));
	s->SetInputDesc(TEXT("float4 position, float4 normal"));
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongnotex.hlsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongDiffuse"));
	s->SetInputDesc(vstex);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongdiffuse.hlsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongSpecular"));
	s->SetInputDesc(vstex);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongspecular.hlsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongBump"));
	s->SetInputDesc(vstextan);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongbump.cso"));
	InstallShader(s);

	// Vertex has positions only
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPos"));
	s->SetInputDesc(TEXT("float4 position"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexpos.cso"));
	VertexShaders[VertexPool::LOCATIONS] = s;
	InstallShader(s);

	// Vertex has positions and normals
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNorm"));
	s->SetInputDesc(TEXT("float4 position, float4 normal"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnorm.cso"));
	InstallShader(s);
	VertexShaders[VertexPool::NORMALS] = s;

	// Vertex has positions and texcoords
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosTex"));
	s->SetInputDesc(TEXT("float4 position, float2 texcoord"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexpostex.cso"));
	InstallShader(s);
	VertexShaders[VertexPool::TEXCOORDS] = s;

	// Vertex has positions, normals and texcoords
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNormTex"));
	s->SetInputDesc(TEXT("float4 position, float4 normal, float2 texcoord"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnormtex.cso"));
	InstallShader(s);
	VertexShaders[VertexPool::NORMALS | VertexPool::TEXCOORDS] = s;

	// Vertex has positions, normals, tangents & bitangents
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNormTang"));
	s->SetInputDesc(TEXT("float4 position, float4 normal, float2 texcoord, float4 tangent, float4 bitangent"));
	s->SetOutputDesc(vstextan);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnormtang.cso"));
	InstallShader(s);
	VertexShaders[VertexPool::NORMALS | VertexPool::TANGENTS] = s;

	// Vertex has positions, normals, texcoords, tangents & bitangents
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNormTexTang"));
	s->SetInputDesc(TEXT("float4 position, float4 normal, float2 texcoord, float4 tangent, float4 bitangent"));
	s->SetOutputDesc(vstextan);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnormtextang.cso"));
	InstallShader(s);
	VertexShaders[VertexPool::NORMALS | VertexPool::TEXCOORDS | VertexPool::TANGENTS] = s;
};

const Shader*	DXRenderer::InstallShader(const Shader* shader)
{
	const TCHAR*	name = shader->GetName();
	DXShader*		dxs = DXShader::Find(name);
	const Shader*	outshader;

	if (dxs == NULL)
	{
		DXShader::Create(shader);
		return shader;
	}
	outshader = dxs->SceneShader;
	VX_ASSERT(shader->GetShaderType() == outshader->GetShaderType());
	return outshader;
}

const Shader*	DXRenderer::FindShader(const TCHAR* name)
{
	DXShader*		dxs = DXShader::Find(name);
	const Shader*	outshader = NULL;

	if (dxs != NULL)
		outshader = dxs->SceneShader;
	return outshader;
}

DXShader*	DXShader::Find(const TCHAR* name)
{
	return ShaderDict.Find(name);
}

DXShader&	DXShader::Create(const Shader* shader)
{
	NameDict<DXShader>::Entry& entry = ShaderDict[shader->GetName()];
	if (entry.Value.SceneShader.IsNull())
		entry.Value.SceneShader = shader;
	return entry.Value;
}

const TCHAR* DXShader::GetSource()
{
	if (!SceneShader.IsNull())
		return SceneShader->GetSource();
	return NULL;
}

/*
 * Links the DX11 light shaders into the DX11 pixel shader.
 * @param ctx		DX11 context
 * @param lightlist	Vixen light list
 *
 * Vixen uses the DX11 shader linking feature to dynamically
 * link the code which implements the various light types to
 * the pixel shader at run time. This function performs the
 * "link" operation, designating to DX11 which shader class implements
 * each shader interface.
 */
void DXShader::LinkLights(D3DCONTEXT* ctx, LightList& lightlist)
{
	ID3D11ShaderReflection*	reflector = Reflector;
	int						numinstances = reflector->GetNumInterfaceSlots();
	ID3D11ClassInstance**	instances;
	ID3D11PixelShader*		pxshader = (ID3D11PixelShader*) *ShaderObject;
	ID3D11ShaderReflectionVariable* var =  reflector->GetVariableByName(TEXT("SurfaceColor"));
	int						offset = var->GetInterfaceSlot(0);

	if (numinstances == 0)
	{
		ctx->PSSetShader(pxshader, NULL, 0);
		return;
	}
	instances = (ID3D11ClassInstance**) alloca(numinstances * sizeof(ID3D11ClassInstance*));
	memset(&instances[0], sizeof(ID3D11ClassInstance*) * numinstances, 0);
	VX_ASSERT(offset >= 0);
	instances[offset] = ShaderClass;
	var =  reflector->GetVariableByName(TEXT("LightList"));

	LightList::Iter	iter(lightlist);
	DXLight*		l;
	int				i = 0;
	bool			enabled;
	ID3D11ClassInstance*	classInstance;

	while (l = (DXLight*) iter.NextProp(enabled))
	{
		Core::String lname(TEXT("light"));
		
		lname += Core::String(i);
		classInstance = l->GetClassInstance(lname);
		offset = var->GetInterfaceSlot(i);
		if (offset >= 0)
		{
			instances[offset] = classInstance;
			++i;
		}
	}
	if (i == 0)
	{
		offset = var->GetInterfaceSlot(0);
		VX_ASSERT(offset >= 0);
		instances[offset] = DXLight::GetNullInstance(TEXT("light0"));
	}
	ctx->PSSetShader(pxshader, instances, numinstances);
}

/*
 * Compiles a Vixen vertex or pixel shader
 * @param render	DX11 renderer
 * @param source	String with source of shader, if NULL use existing source
 * @param entry		string with entry name of shader, if NULL use existing name
 *
 * If the Vixen Shader has already been compiled, the binary code for
 * the DX11 shader will be available (Some DX11 shaders may be precompiled as part of
 * the Vixen build). In this case, the DX11 vertex or pixel shader is created from
 * the shader code and returned.
 *
 * If the DX11 shader source is available, it is compiled and the result DX11
 * shader is attached to the Vixen Shader and returned.
 *
 * If the shader compilation fails or the source is not available for some reason,
 * (the file has not been loaded, the user did not supply the source),
 * NULL is returned and no shader is set.
 *
 * The input source and entry arguments allow the DX11 shader to have an entry name
 * and source that is different from the Vixen Shader. This is useful when the user
 * specifies a surface shader (Vixen Shader of type Shader::PIXEL | Shader::CLASS)
 * indicating they want to replace only the surface color calculation part of
 * the pixel shader. In this case, the Vixen Shader source is wrapped in a default
 * pixel shader framework which incorporates the Vixen lights.
 *
 * @return -> DX11 vertex or pixel shader, 0 if shader is not available
 */
ID3D11DeviceChild*	DXShader::Compile(DXRenderer* render, const TCHAR* source, const TCHAR* entry)
{
	const Shader*		shader = SceneShader;
	D3DDEVICE*			device = render->GetDevice();
	ID3DBlob*			code = NULL;
	int					shadertype = shader->GetShaderType() & ~Shader::CLASS;
	const TCHAR*		features = (shadertype == Shader::VERTEX) ? TEXT("vs_5_0") : TEXT("ps_5_0");
	Core::String		filename(shader->GetFileName());
	WCHAR				dxfilename[VX_MaxPath];
	HRESULT				hr;
	ID3DBlob*			errors = NULL;
	const void*			code_ptr;
	size_t				code_len;

	/*
	 * If the shader code has not already been loaded, compile the shader
	 * from source or from the file
	 */
	if (!shader->GetShaderCode(&code_ptr, &code_len))
	{
		UINT flags = 0;

#ifdef _DEBUG
		flags = D3DCOMPILE_DEBUG;
#endif
		if (source == NULL)
			source = GetSource();
		filename.AsWide(dxfilename, VX_MaxPath);
		if (entry == NULL)
			entry = shader->GetName();
		if ((source != NULL) && *source)
			hr = D3DCompile(source, STRLEN(source), filename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, features, flags, 0, &code, &errors);
		else
			if ((filename.Right(5).CompareNoCase(TEXT(".hlsl"))) == 0)
				hr = D3DCompileFromFile(dxfilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, features, flags, 0, &code, &errors);
		else
		{	
			VX_WARNING(("ShaderCompile: WARNING cannot compile shader - file %s still loading\n", shader->GetFileName()));
			return NULL;
		}
		if (FAILED(hr) || (code == NULL))
		{
			if (errors)
			{
				Core::String tmp((const char*) errors->GetBufferPointer(), errors->GetBufferSize());
				errors->Release();
				VX_ERROR(("Shader::Compile cannot compile shader %s: %s\n", entry, tmp), NULL);
			}
			else
				VX_ERROR(("Shader::Compile cannot compile shader %s\n", entry), NULL);
		}
		VX_TRACE(DXRenderer::Debug, ("DXRenderer::Compiled shader %s\n", entry));
		code_ptr = code->GetBufferPointer();
		code_len = code->GetBufferSize();
	}
	/*
	 * Construct the pixel or vertex shader and retain reflection information
	 */
	const DataLayout*	layout = shader->GetInputLayout();
	DXLayout&			dxlayout = (DXLayout&) layout->DevHandle;
	ID3D11VertexShader*	vs;
	ID3D11PixelShader*	ps;
	ID3D11ClassLinkage*	classlinkage = NULL;
	ID3D11ShaderReflection* reflector = NULL; 

	dxlayout.Update(render, shader, code_ptr, code_len);
	D3DReflect(code_ptr, code_len, IID_ID3D11ShaderReflection, (void**) &reflector);
	Reflector = reflector;
	switch (shadertype)
	{
		case Shader::PIXEL:
		hr = device->CreatePixelShader(code_ptr, code_len, ClassLinkage, &ps);
		VX_TRACE(Appearance::Debug || DXRenderer::Debug, ("DXRenderer::Compile creating pixel shader %s\n", entry));
		if (code)
			code->Release();
		shader->SetShaderCode(NULL, 0);
		ShaderObject = ps;
		return (ID3D11DeviceChild*) ps;

		case Shader::VERTEX:
		hr = device->CreateVertexShader(code_ptr, code_len, ClassLinkage, &vs);
		VX_TRACE(DXRenderer::Debug, ("DXRenderer::Compile creating vertex shader %s\n", entry));
		ShaderObject = vs;
		shader->SetShaderCode(NULL, 0);
		if (code)
			code->Release();
		return (ID3D11DeviceChild*) vs;

		default:
		VX_ERROR(("Shader::Compile:: invalid shader type %d in shader %s\n", shadertype, entry), NULL);
	}
	return NULL;
}

void	DXShader::Release()
{
	ShaderObject = NULL;
	Reflector = NULL;
	ShaderClass = NULL;
}

/*
 * Releases the resources used to the dynamically generated
 * pixel shaders to force them to be recompiled.
 * This occurs when a light is added to the scene.
 */
void DXShader::ReleasePixelShaders()
{
	 NameDict<DXShader>::Iter iter(&ShaderDict);
	 DXShader*	s;

	 while (s = iter.Next())
	 {
		 Shader*	shader = s->SceneShader;

		 if (shader && (shader->GetShaderType() == (Shader::PIXEL | Shader::CLASS)))
			 s->Release();
	 }
}

}  // end Vixen


