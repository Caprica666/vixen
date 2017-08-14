#include "vixen.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxshadergl.h"

namespace Vixen {

Ref<Shader>		GLShader::VertexShaders[GLShader::NUM_VERTEX_SHADERS];
Ref<NameTable>	GLShader::ShaderDict;		// table of compiled shaders indexed by name

bool	GLRenderer::GeneratePixelShader(Core::String& name, Core::String& source, bool dolighting)
{
	if (name.IsEmpty())
		VX_ERROR(("GLRenderer::Compile: ERROR cannot compile shader - name of entry point missing\n"), false);

	const Shader*	pixeltemplate = FindShader(TEXT("DefaultPixelShader"));
	const TCHAR*	templatesource = NULL;

	if (source.IsEmpty())
		VX_ERROR(("GLRenderer::CompilePixelShader cannot find pixel shader %s\n", (const char*) name), false);
	if (pixeltemplate)
		templatesource = pixeltemplate->GetSource();
	if (!templatesource || (*templatesource == 0))
		VX_ERROR(("GLRenderer::CompilePixelShader cannot find pixel shader template\n"), false);
	source += templatesource;
	source.Replace(TEXT("$SURFACESHADER"), name);

	if (dolighting && (NumLights > 0))
		source.Replace(TEXT("$LIGHTSOURCES"), m_LightShaderSource);
	else
		source.Replace(TEXT("$LIGHTSOURCES"), m_NoLightSource);
	name = TEXT('_') + name;
#ifdef _DEBUG
	{
		Core::FileStream shaderout;
		Core::String	fname(name);
		
		fname += TEXT(".frag");
		if (shaderout.Open(fname, Core::FileStream::OPEN_WRITE))
			shaderout.Write(source, source.GetLength());
		shaderout.Close();
//		VX_PRINTF(("\n%s:===========================\n%s\n", (const TCHAR*) fname, (const TCHAR*) source));
	}
#endif
	return true;
}

intptr		GLRenderer::Compile(const Shader* shader)
{
	const TCHAR*	name = shader->GetName();
	const void*		codeptr;
	size_t			codelen;
	GLuint			shaderobj = 0;

	shader = InstallShader(shader);
	if (name == NULL)
		VX_ERROR(("GLRenderer::Compile: ERROR cannot compile shader - name of entry point missing\n"), 0);
	if (shader->GetInputLayout() == NULL)
		VX_ERROR(("GLRenderer::Compile: ERROR cannot compile shader - input layout missing\n"), 0);
	shader->GetShaderCode(&codeptr, &codelen);
	if ((codeptr == NULL) || shader->HasChanged())
	{
		shaderobj = (GLuint) GLShader::Compile(shader);
		if (shaderobj)
			shader->SetChanged(false);
	}
	return (intptr) shaderobj;
}

#ifdef VIXEN_EMSCRIPTEN
#define SHADER_PATH "#data/shaders/glsl2/"
#else
#define SHADER_PATH "D:\\Projects\\Vixen\\data\\shaders\\glsl2\\"
#endif

void GLRenderer::MakeDefaultShaders()
{
	Shader*			s;
	const TCHAR*	psout = TEXT("float4 position, float4 normal, float4 diffuse, float4 specular");
	const TCHAR*	vstex = TEXT("float4 position, float4 normal, float4 texcoord");
	const TCHAR*	vstextan = TEXT("float4 position, float4 normal, float4 texcoord, float4 tangent, float4 bitangent");
	const TCHAR*	radiance = TEXT("float4 flux, float4 radiance");
	const TCHAR*	light = TEXT("float4 WorldDir, float4 WorldPos, float4 Color, int Decay");

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("NullLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("AmbientLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "ambientlight.glsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("DirectLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "directlight.glsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("PointLight"));
	s->SetInputDesc(light);
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "pointlight.glsl"));
	InstallShader(s);

	s = new Shader(Shader::LIGHT | Shader::CLASS);
	s->SetName(TEXT("SpotLight"));
	s->SetInputDesc(TEXT("float4 WorldDir, float4 WorldPos, float4 Color, int Decay, float InnerAngle, float OuterAngle"));
	s->SetOutputDesc(radiance);
	s->SetFileName(TEXT(SHADER_PATH "spotlight.glsl"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("DefaultPixelShader"));
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "default_pixel_shader.frag"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongNotex"));
	s->SetInputDesc(TEXT("float4 position, float4 normal"));
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongnotex.frag"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongDiffuse"));
	s->SetInputDesc(vstex);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongdiffuse.frag"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongSpecular"));
	s->SetInputDesc(vstex);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongspecular.frag"));
	InstallShader(s);

	// Input is position, normal and output is diffuse, specular, position, normal
	s = new Shader(Shader::PIXEL | Shader::CLASS);
	s->SetName(TEXT("PixelPhongBump"));
	s->SetInputDesc(vstextan);
	s->SetOutputDesc(psout);
	s->SetFileName(TEXT(SHADER_PATH "pixelphongbump.frag"));
	InstallShader(s);

	// Vertex has positions only
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPos"));
	s->SetInputDesc(TEXT("float4 position"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexpos.vert"));
	GLShader::VertexShaders[VertexPool::LOCATIONS] = s;
	InstallShader(s);

	// Vertex has positions and normals
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNorm"));
	s->SetInputDesc(TEXT("float4 position, float4 normal"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnorm.vert"));
	GLShader::VertexShaders[VertexPool::NORMALS] = s;
	InstallShader(s);

	// Vertex has positions and texcoords
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosTex"));
	s->SetInputDesc(TEXT("float4 position, float2 texcoord"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexpostex.vert"));
	GLShader::VertexShaders[VertexPool::TEXCOORDS] = s;
	InstallShader(s);

	// Vertex has positions, normals and texcoords
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNormTex"));
	s->SetInputDesc(TEXT("float4 position, float4 normal, float2 texcoord"));
	s->SetOutputDesc(vstex);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnormtex.vert"));
	GLShader::VertexShaders[VertexPool::NORMALS | VertexPool::TEXCOORDS] = s;
	InstallShader(s);


	// Vertex has positions, normals, texcoords, tangents & bitangents
	s = new Shader(Shader::VERTEX);
	s->SetName(TEXT("VertexPosNormTexTang"));
	s->SetInputDesc(TEXT("float4 position, float4 normal, float2 texcoord, float4 tangent, float4 bitangent"));
	s->SetOutputDesc(vstextan);
	s->SetFileName(TEXT(SHADER_PATH "vertexposnormtextang.vert"));
	GLShader::VertexShaders[VertexPool::NORMALS | VertexPool::TEXCOORDS | VertexPool::TANGENTS] = s;
	InstallShader(s);
};

const Shader*	GLRenderer::FindShader(const TCHAR* name)
{
	return GLShader::Find(name);
}

const Shader*	GLRenderer::InstallShader(const Shader* srcshader)
{
	const Shader *shader = GLShader::Create(srcshader);

	if (shader->GetShaderType() & Shader::LIGHT) 
		m_LightShaderChanged = true;
	return shader;
}

const Shader*	GLShader::Find(const TCHAR* name)
{
	ObjRef*		ref;
	SharedObj*	obj;

	if (ShaderDict.IsNull())
		return NULL;
	ref = ShaderDict->Find(name);
	if (ref == NULL)
		return NULL;
	obj = *ref;
	return (const Shader*) obj;
}

const Shader*	GLShader::Create(const Shader* srcshader)
{
	const TCHAR*	name = srcshader->GetName();
	const Shader*	shader = Find(name);

	if (shader == NULL)
	{
		if (ShaderDict.IsNull())
			ShaderDict = new NameTable;
		ShaderDict->Set(name, srcshader);
		srcshader->SetFlags(SharedObj::GLOBAL);
		return srcshader;
	}
	return shader;
}

intptr	GLShader::Compile(const Shader* shader, const TCHAR* source, const TCHAR* entry)
{
	int					shadertype = shader->GetShaderType( ) & ~Shader::CLASS;
	Core::String		filename(shader->GetFileName());
	GLuint				glshader = 0;
	GLint				compiled = false;
	const void*			codeptr = NULL;
	int					codelen = 0;

	/*
	 * If the shader code has not already been loaded, compile the shader
	 * from source or from the file
	 */
	if (source == NULL)
		source = shader->GetSource();
	if (entry == NULL)
		entry = shader->GetName();
	if ((source == NULL) || (*source == 0))
		VX_ERROR(("ShaderCompile: WARNING cannot compile shader - file %s still loading\n", (const char*) filename), 0);
	Release(shader);
	switch (shadertype)
	{
		case	Shader::PIXEL: glshader = glCreateShader(GL_FRAGMENT_SHADER); break;
		case	Shader::VERTEX:	glshader = glCreateShader(GL_VERTEX_SHADER); break;
		default:	VX_ERROR(("Shader::Compile:: invalid shader type %d in shader %s\n", shadertype, entry), 0);
	}
	if (glshader)
	{

		glShaderSource(glshader, 1, &source, NULL);
		glCompileShader(glshader);
		glGetShaderiv(glshader, GL_COMPILE_STATUS, &compiled);
	}
	if (!glshader || !compiled)
	{
		GLint	infoLen = 0;

		glGetShaderiv(glshader, GL_INFO_LOG_LENGTH, &infoLen);
		glDeleteShader(glshader);
		if (infoLen > 1)
		{
			char* infolog = (char*) alloca(infoLen + 2);
			*infolog = 0;
			glGetShaderInfoLog(glshader, infoLen, &infoLen, infolog);
			VX_ERROR(("Shader::Compile cannot compile shader %s %s\n", entry, infolog), 0);
		}
		else
			VX_ERROR(("Shader::Compile cannot compile shader %s\n", entry), 0);
	}
	VX_TRACE(GLRenderer::Debug, ("GLRenderer::Compiled shader %s #%d\n", entry, glshader));
	codeptr = (const void*) glshader;
	shader->SetShaderCode(codeptr, 0);
	return glshader;
}

void	GLShader::Release(const Shader* shader)
{
	const void*	codeptr = NULL;
	size_t		codelen = 0;

	if (shader && shader->GetShaderCode(&codeptr, &codelen))
	{
		GLuint glshader = (GLuint) codeptr;
		if (glshader)
		{
			VX_TRACE(GLRenderer::Debug, ("GLShader::Release deleting shader %s #%d\n", shader->GetName(), glshader));
			glDeleteShader(glshader);
		}
		shader->SetShaderCode(0, 0);
	}
}

void GLShader::ReleasePixelShaders()
{
	 NameTable::Iter iter(ShaderDict);
	 ObjRef* ref;

	 while (ref = iter.Next())
	 {
		 Shader*	shader = (Shader*) (SharedObj*) *ref;

		 if (shader && (shader->GetShaderType() == (Shader::PIXEL | Shader::CLASS)))
			 shader->SetShaderCode(NULL, 0);
	 }
}

void GLShader::ReleaseAll()
{
	for (int i = 0; i < NUM_VERTEX_SHADERS; ++i)
		VertexShaders[i] = NULL;
	if (ShaderDict.IsNull())
		return;

	 NameTable::Iter iter(ShaderDict);
	 ObjRef* ref;

	 while (ref = iter.Next())
	 {
		 Shader*	shader = (Shader*) (SharedObj*) *ref;

		 if (shader)
			 GLShader::Release(shader);
	 }
	 ShaderDict = NULL;
}

}  // end Vixen


