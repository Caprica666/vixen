/****
 *
 * OpenGL Appearance implementation. These functions are
 * only the OpenGL specific Appearance methods. The core
 * APP.CPP file contains the common code.
 *
 *
 ****/
#include "vixen.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxshadergl.h"
#include "ogl/vbufgl.h"

namespace Vixen {

const Appearance*	GLRenderer::s_CurAppear = NULL;
Ref<Appearance>		GLRenderer::s_DefaultAppear;
int					GLRenderer::NumTexUnits = 0;

NameDict< int32 >*	GLProgram::ProgramDict = NULL;

int	GLRenderer::UpdateAppearance(const Appearance* app, int vertex_style)
{
	ObjectLock		lock(app);
	GLuint			glpixshader;
	GLuint			glvtxshader;
	const Shader*	vertexshader;
	const Shader*	pixelshader;
	GLProgram		glstate;
	GLuint			program;
	bool			compiled;
	bool			needslink = false;
	const void*		codeptr = NULL;
	size_t			codelen = 0;
	TCHAR			key[256];
	const DataLayout*	layout;

	/*
	 * Select the OpenGL vertex and pixel shader for this appearance
	 * and make sure they are compiled
	 */
	vertexshader = app->GetVertexShader();
	if (vertexshader == NULL)
		vertexshader = GLShader::VertexShaders[vertex_style];
	VX_ASSERT(vertexshader);
	compiled = Compile(vertexshader) != 0;
	vertexshader->GetShaderCode(&codeptr, &codelen);
	if (codeptr == NULL)
		return false;
	glvtxshader = (GLuint) codeptr;
	layout = vertexshader->GetInputLayout();
	VX_ASSERT(layout);
	needslink = compiled;
	pixelshader = glstate.SelectPixelShader(this, app, vertex_style, compiled);
	VX_ASSERT(pixelshader);
	pixelshader->GetShaderCode(&codeptr, &codelen);
	if (codeptr == NULL)
		return false;
	glpixshader = (GLuint) codeptr;
	needslink |= compiled;
	/*
	 * Find the OpenGL program for this pixel / vertex shader compilation
	 * and link it if the shaders have changed.
	 */
	STRCPY(key, pixelshader->GetName());
	STRCAT(key, "-");
	STRCAT(key, vertexshader->GetName());
	program = glstate.FindProgram(key);
	if ((program == 0) || needslink)
	{
		program = glstate.NewProgram(key);
		GLVertexBuf::BindAttribs(layout, program);
		if (!glstate.LinkProgram(glvtxshader, glpixshader))
			return false;
	}
	/*
	 * Update the textures and materials associated with this program
	 */
	m_CurProgram = program;
	glUseProgram(program);
	glstate.UpdateTextures(this, app);
	glstate.UpdateMaterial(app->GetMaterial());
	return needslink;
}

void GLProgram::ReleaseAll()
{
	if (ProgramDict == NULL)
		return;

	NameDict< int32 >::Iter iter(ProgramDict);
	int32* entry;

	while (entry = iter.Next())
	{
		GLuint program = *entry;

		if (program > 0)
			glDeleteProgram(program);
		*entry = 0;
	}
	ProgramDict->Empty();
	ProgramDict->Delete();
	ProgramDict = NULL;
}

GLuint GLProgram::NewProgram(const TCHAR* key)
{
	int32*	programref;
	GLuint	programid;

	programid = glCreateProgram();
	if (programid == 0)
	{
		GLuint err = glGetError();
		VX_ERROR(("GLProgram::NewProgram failed to make OpenGL program %s %x\n",key, err), 0);
	}
	VX_TRACE(GLRenderer::Debug, ("GLProgram::NewProgram creating program #%d %s\n", programid, key));
	if (ProgramDict == NULL)
		ProgramDict = new NameDict< int32 >();
	else
	{
		programref = ProgramDict->Find(key);
		if (programref)
		{
			GLuint oldid = *programref;
			if (oldid)
				glDeleteProgram(oldid);
			VX_TRACE(GLRenderer::Debug, ("GLProgram::NewProgram deleting program #%d %s\n", oldid, key));
		}
	}
	ProgramDict->Set(key, programid);
	m_GLProgram = programid;
	return programid;
}

GLuint GLProgram::FindProgram(const TCHAR* key)
{
	int32*	programref;

	if (ProgramDict == NULL)
		return 0;
	programref = ProgramDict->Find(key);
	if (programref == NULL)
		return 0;
	m_GLProgram = *programref;
	return (GLuint) m_GLProgram;
}

bool GLProgram::LinkProgram(GLuint vertshader, GLuint pixshader)
{
	GLint linked = false;

	glAttachShader(m_GLProgram, vertshader);
	glAttachShader(m_GLProgram, pixshader);
	glLinkProgram(m_GLProgram);
	glGetProgramiv(m_GLProgram, GL_LINK_STATUS, &linked);
	if (linked && m_GLProgram)
	{
		glValidateProgram(m_GLProgram);
		glGetProgramiv(m_GLProgram, GL_VALIDATE_STATUS, &linked);
	}
	if (!linked)
	{
		GLint	infolen = 0;

		glGetProgramiv(m_GLProgram, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1)
		{
			char* infolog = (char*) alloca(infolen + 2);
			*infolog = 0;
			glGetProgramInfoLog(m_GLProgram, infolen, &infolen, infolog);
			VX_ERROR(("GLRenderer::UpdateAppearance cannot link program #%d %x %s\n", m_GLProgram, glGetError(), infolog), false);
		}
		VX_ERROR(("GLRenderer::UpdateAppearance cannot link program #%d %x\n", m_GLProgram, glGetError()), false);
	}
	VX_TRACE(GLRenderer::Debug, ("GLRenderer::UpdateAppearance linked program #%d\n", m_GLProgram));
	return true;
}

/*
 * Set material parameters. If not texturing, we force D3D to use the diffuse
 * material color instead of the texture color.
 */
void GLProgram::UpdateMaterial(const DeviceBuffer* mat)
{
	if (mat)
		GLBuffer::Update(mat, m_GLProgram);
}

intptr GLProgram::UpdateSampler(GLRenderer* render, const Sampler* smp, int texunit)
{
	Texture*	image = (Texture*) smp->GetTexture();
	int			index = 0;
	intptr		texhandle = 0;
	bool		mipmap = smp->IsMipMap();

	/*
	 * Load the texture being used if necessary. If the sampler requests
	 * mip-mapping but the texture does not have mip-maps, we turn off mipmapping
	 */
	glActiveTexture(GL_TEXTURE0 + texunit);
	if (image)
	{
		texhandle = render->UpdateTexture(image, texunit, mipmap);
		if (mipmap && !image->IsMipMap())
			mipmap = false;
	}
	if (mipmap)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (smp->Get(Sampler::MINFILTER) == Sampler::LINEAR) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (smp->Get(Sampler::MINFILTER) == Sampler::LINEAR) ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (smp->Get(Sampler::MAGFILTER) ==  Sampler::LINEAR) ? GL_LINEAR : GL_NEAREST);
	smp->SetChanged(false);
	return texhandle;
}

bool GLProgram::UpdateTextures(GLRenderer* render, const Appearance* app)
{
	int				numtexunits = (int) app->GetNumSamplers();
	const TCHAR*	shadername = NULL;

	if (app->Get(Appearance::CULLING))
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	/*
	 * Load device stage for all the texture samplers
	 * Default the state for the samplers used by the last
	 * appearance but not this one
	 */
	for (int i = 0; i < numtexunits; ++i)
	{
		const Sampler* smp = app->GetSampler(i);

		if (!smp || !UpdateSampler(render, smp, i))
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	return true;
}

const Shader* 	GLProgram::SelectPixelShader(GLRenderer* render, const Appearance* app, int vertex_style, bool& compiled)
{
	Core::String	shadername = TEXT("PixelPhongNotex");
	const Shader*	shader = app->GetPixelShader();
	GLuint			pixshader;
	bool			normalmap = false;
	bool			diffusemap = false;
	bool			specularmap = false;

	compiled = false;
	pixshader = 0;
	if ((shader == NULL) &&
		(vertex_style & VertexPool::TEXCOORDS))
	{
		int	numtexunits = (int) app->GetNumSamplers();

		for (int i = 0; i < numtexunits; ++i)
		{
			const Sampler* smp = app->GetSampler(i);

			if (smp == NULL)
				continue;
			switch (smp->Get(Sampler::TEXTUREOP))
			{
				case Sampler::DIFFUSE:	shadername = TEXT("PixelPhongDiffuse"); diffusemap = true; break;
				case Sampler::SPECULAR:	shadername = TEXT("PixelPhongSpecular"); specularmap = true; break;
				case Sampler::BUMP:		shadername = TEXT("PixelPhongBump"); normalmap = true; break;
			}
		}
	}
	bool		dolighting = Appearance::DoLighting && (app->Get(Appearance::LIGHTING) != 0);
	bool		shaderchanged = false;
	const void*	codeptr = NULL;
	size_t		codelen = 0;
	const DeviceBuffer*	mtl = app->GetMaterial();


	if (shader != NULL)
	{
		shader = render->InstallShader(shader);
		shaderchanged = shader->HasChanged();
		shader->SetChanged(false);
	}
	else
	{
		shader = render->FindShader(shadername);
		if (mtl == NULL)
		{
			mtl = render->DefaultMaterial;
			((Appearance*) app)->SetMaterial((DeviceBuffer*) mtl);
		}
	}
	if (shader == NULL)
		VX_ERROR(("GLProgram::SelectPixelShader ERROR cannot find or create shader %s\n", (const char*) shadername), NULL);
	if (mtl && mtl->IsClass(VX_PhongMaterial))
	{
		PhongMaterial::ShaderConstants* mtldata = (PhongMaterial::ShaderConstants*) mtl->GetData();
		VX_ASSERT(mtldata);
		mtldata->HasDiffuseMap = diffusemap;
		mtldata->HasSpecularMap = specularmap;
		mtldata->HasNormalMap = normalmap;
	}
	shaderchanged |= shader->HasChanged();
	shader->GetShaderCode(&codeptr, &codelen);
	compiled = shaderchanged;
	if ((codeptr == NULL) || shaderchanged)
	{
		Core::String source = shader->GetSource();
		if (shader->GetShaderType() == (Shader::PIXEL | Shader::CLASS))
			render->GeneratePixelShader(shadername, source, dolighting);
		if (GLShader::Compile(shader, source, shadername))
			compiled = true;
		shader->SetChanged(false);
	}
	return shader;
}

}	// end Vixen