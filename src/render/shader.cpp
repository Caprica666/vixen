/****
 *
 * Shader Class
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Shader, SharedObj, VX_Shader);

static const TCHAR* opnames[] =
	{ TEXT("SetFileName"), TEXT("SetSource"), TEXT("SetType"), TEXT("SetInputDesc"), TEXT("SetOutputDesc") };

const TCHAR** Shader::DoNames = opnames;

/*!
 * @fn Shader::Shader(int type)
 * @param type	type of this shader
 *				PIXEL	pixel shader
 *				VERTEX	vertex shader
 *				LIGHT	light shader
 * Makes a Shader which contains no source code.
 * You must specify the filename of the shader code or a string
 * which contains the source before you can compile the shader.
 *
 * @see Shader::SetFileName Shader::SetSource Shader::Compile
 */
Shader::Shader(int type)
  :	SharedObj(),
	m_InputLayout(NULL),
	m_OutputLayout(NULL),
	m_SourceCode(NULL),
	m_Status(UNINITIALIZED),
	m_Type(type),
	m_BinaryCode(NULL),
	m_BinaryLength(0)
{
}

Shader::~Shader()
{
	if (m_SourceCode)
	{
		delete[] m_SourceCode;
		m_SourceCode = NULL;
		Core::InterlockSet(&m_Status, UNINITIALIZED);
	}
	if (m_BinaryCode != NULL)
		Core::GlobalAllocator::Get()->Free(m_BinaryCode);
}

/*!
 * @fn bool Shader::operator==(const Shader& src)
 *
 * Compares the source Shader with this one
 * and returns \b true if they are the same.
 *
 * @returns  true if Shaders the same, else \b false
 */
bool Shader::operator==(const Shader& src)
{
	if (m_FileName != src.m_FileName)
		return false;
	if (m_InputDesc != src.m_InputDesc)
		return false;
	if (m_OutputDesc != src.m_OutputDesc)
		return false;
	if (m_Type != src.m_Type)
		return false;
	if (m_SourceCode == src.m_SourceCode)
		return true;
	if (m_SourceCode && src.m_SourceCode)
		return STRCMP(m_SourceCode, src.m_SourceCode) == 0;
	return false;
}

/****
 *
 * Class Shader override for SharedObj::Copy
 * 	Shader::Copy(SharedObj* src)
 *
 * Copies the contents of one Shader object into another.
 * If the source object is not a Shader, the attributes
 * which are Shader-specific are unchanged in the destination.
 *
 ****/
bool Shader::Copy(const SharedObj* src_obj)
{
	const Shader*	src = (const Shader*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!SharedObj::Copy(src_obj))
		return false;
	if (!src->IsClass(VX_Shader))
		return true;
	m_FileName = src->m_FileName;
	m_InputDesc = src->m_InputDesc;
	m_OutputDesc = src->m_OutputDesc;
	m_Type = src->m_Type;
	SetSource(src->m_SourceCode);
	return true;
}

/*!
 * @fn void Shader::SetFileName(const TCHAR* fname)
 * @param fname	new value for file name
 *
 * The file name designates the file which contains shader code.
 * If no extension is specified, it will default to the extension
 * appropriate for this version of Vixen. The directory defaults
 * to the current media directory if the file name is not fully
 * qualified.
 *
 * Setting the file name does not actually compile the shader.
 * On some versions of Vixen, it may initiate loading of the file.
 *
 * @see Shader::SetSource Shader::SetInputDesc Shader::SetOutputDesc Shader::Compile
 */
void Shader::SetFileName(const TCHAR* fname)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shader, SHADER_SetFileName) << this << fname;
	VX_STREAM_END( )
	if (m_FileName == fname)
		return;
	m_FileName = fname;
	Core::InterlockSet(&m_Status, UNINITIALIZED);
	SetChanged(true);
	GetMessenger()->Observe(this, Event::LOAD_TEXT, NULL);
	GetMessenger()->Observe(this, Event::LOAD_DATA, NULL);
	World3D::Get()->LoadAsync(fname, this);
}

/*!
 * @fn void Shader::SetSource(const TCHAR* source_code)
 * @param source_code	string containing source code for shader
 *
 * Specifies the source code for this shader. If the code is given,
 * the file name is ignored and this source code is used.
 * Setting the source does not actually compile the shader.
 * The input string is copied and stored internally within the shader.
 *
 * @see Shader::SetFileName Shader::SetInputDesc Shader::SetOutputDesc Shader::Compile
 */
void Shader::SetSource(const TCHAR* source_code)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shader, SHADER_SetSource) << this << source_code;
	VX_STREAM_END( )

	const TCHAR*	source = m_SourceCode;

	if (source)
	{
		delete[] source;
		m_SourceCode = NULL;
		Core::InterlockSet(&m_Status, UNINITIALIZED);
	}
	if (source_code != NULL)
	{
		m_SourceCode = new TCHAR[STRLEN(source_code) + 1];
		STRCPY(m_SourceCode, source_code);
		Core::InterlockSet(&m_Status, SOURCE);
	}
	SetChanged(true);
}

/*!
 * @fn void Shader::SetShaderCode(const void* codeptr, size_t codelen)
 * @param codeptr	-> binary code for shader
 * @param codelen	size of binary code
 *
 * This function is called internally when the shader is compiled to
 * associate binary code with the shader.
 *
 * @see Shader::SetSource Shader::GetSourceCode Renderer::CompileShader
 */
void Shader::SetShaderCode(const void* codeptr, size_t codelen) const
{
	if (m_BinaryCode == codeptr)
		return;
	if ((m_BinaryCode != NULL) && m_BinaryLength)
		Core::GlobalAllocator::Get()->Free(m_BinaryCode);
	Core::InterlockSet(&m_BinaryCode, (voidptr) codeptr);
	Core::InterlockSet(&m_BinaryLength, codelen);
	Core::InterlockSet(&m_Status, (int32) CODE);
}

/*!
 * @fn void Shader::SetInputDesc(const TCHAR* desc)
 * @param desc	string giving description of shader input element
 *
 * The input description specifies the format of the data to the shader which varies,
 * i.e. the vertex or the pixel. It is in the same format as the other DataLayout strings:
 * name of component, type of component and number of floats/ints
 *	"position float 4, normal float 4, texcoord float 2" or "position float 3, color float 4"
 *
 * @see Shader::SetOutputDesc Shader::GetInputDesc Shader::GetInputLayout Shader::Compile
 */
void Shader::SetInputDesc(const TCHAR* desc)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shader, SHADER_SetInputDesc) << this << desc;
	VX_STREAM_END(  )
	m_InputDesc = desc;
	SetChanged(true);
}

/*!
 * @fn void Shader::SetOutputDesc(const TCHAR* desc)
 * @param desc	string giving description of shader output element
 *
 * The input description specifies the format of the data to the shader which varies,
 * i.e. the vertex or the pixel. It is in the same format as the other DataLayout strings:
 * name of component, type of component and number of floats/ints
 *	"position float 4, normal float 4, texcoord float 2" or "position float 3, color float 4"
 *
 * @see Shader::SetOutputDesc Shader::GetInputDesc Shader::GetInputLayout Shader::Compile
 */
void Shader::SetOutputDesc(const TCHAR* desc)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shader, SHADER_SetOutputDesc) << this << desc;
	VX_STREAM_END(  )
	m_OutputDesc = desc;
	SetChanged(true);
}


const DataLayout*	Shader::GetInputLayout() const
{
	if (m_InputLayout == NULL)
	{
		if (m_InputDesc.IsEmpty())
			return NULL;
		m_InputLayout = DataLayout::FindLayout(m_InputDesc);
	}
	return m_InputLayout;
}

const DataLayout*	Shader::GetOutputLayout() const
{
	if (m_OutputLayout == NULL)
	{
		if (m_OutputDesc.IsEmpty())
			return NULL;
		m_OutputLayout = DataLayout::FindLayout(m_OutputDesc);
	}
	return m_OutputLayout;
}
/****
 *
 * Class Shader override for SharedObj::PrintInfo
 *
 * Prints an ASCII description of the Shader
 *
 ****/
DebugOut& Shader::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Type'>" << m_Type << "</attr>");
	if (!m_FileName.IsEmpty())
		endl(dbg << "\t<attr name='FileName'>" << m_FileName << "</attr>");
	if (m_SourceCode)
		endl(dbg << "\t<attr name='Source'>" << m_SourceCode << "</attr>");
	if (!m_InputDesc.IsEmpty())
		endl(dbg << "\t<attr name='InputDesc'>" << m_InputDesc << "</attr>");
	if (!m_InputDesc.IsEmpty())
		endl(dbg << "\t<attr name='OutputDesc'>" << m_OutputDesc << "</attr>");
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Shader override for SharedObj::Do
 *
 ****/
bool Shader::Do(Messenger& s, int op)
{
	int32		n;
	TCHAR		str[VX_MaxString];

	switch (op)
	{
		case SHADER_SetFileName:
		s >> str;
		SetFileName(str);
		break;

		case SHADER_SetSource:
		s >> str;
		SetSource(str);
		break;

		case SHADER_SetInputDesc:
		s >> str;
		SetInputDesc(str);
		break;

		case SHADER_SetOutputDesc:
		s >> str;
		SetOutputDesc(str);
		break;

		case SHADER_SetShaderType:
		s >> n;
		m_Type = n;
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Shader::DoNames[op - SHADER_SetFileName]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class Shader override for SharedObj::Save
 *
 ****/
int Shader::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Shader, SHADER_SetShaderType) << h << m_Type;
	if (!m_FileName.IsEmpty())
		s << OP(VX_Shader, SHADER_SetFileName) << h << m_FileName;
	else if (m_SourceCode)
		s << OP(VX_Shader, SHADER_SetSource) << h << m_SourceCode;
	if (!m_InputDesc.IsEmpty())
		s << OP(VX_Shader, SHADER_SetInputDesc) << h << m_InputDesc;
	if (!m_OutputDesc.IsEmpty())
		s << OP(VX_Shader, SHADER_SetOutputDesc) << h << m_OutputDesc;
	return h;
}

bool Shader::OnEvent(Event* ev)
{
	LoadTextEvent* lev = (LoadTextEvent*) ev;

	if (lev->FileName != m_FileName)
		return false;
	/*
	 * If a binary file has been loaded, it is the compiled code for the shader.
	 * Save it for the renderer.
	 */
	if (ev->Code == Event::LOAD_DATA)
	{
		LoadDataEvent* ldata = (LoadDataEvent*) ev;

		if ((ldata->Data == 0) ||
			(ldata->Length == 0))
			return false;
		SetShaderCode((const void*) ldata->Data, ldata->Length);
		ldata->Data = NULL;						// indicate we own the data now
		return true;
	}
	if (ev->Code != Event::LOAD_TEXT)
		return false;
	if (GetName() == NULL)						// if shader not named,
	{
		TCHAR filebase[VX_MaxPath];

		Core::Stream::ParseDirectory(m_FileName, filebase, NULL);
		SetName(filebase);						// set name from file base name
	}
	if (lev->SequenceID == 0)					// entire shader in this event
	{
		SetSource(lev->Text);
		return true;
	}
	if (lev->SequenceID == 1)					// this event this first of many
	{
		SetSource(lev->Text);					// replace current source
		Core::InterlockSet(&m_Status, UNINITIALIZED);	// indicate more to come
		return true;
	}
	intptr	n;
	intptr	m = lev->Text.GetLength();
	TCHAR*	newsource;

	VX_ASSERT(m_SourceCode);
	n = STRLEN(m_SourceCode);
	newsource = new TCHAR[n + m + 1];			// allocate larger memory area
	STRCPY(newsource, m_SourceCode);			// copy existing source code
	STRCPY(newsource + n, lev->Text);			// append new source code
	SetSource(newsource);						// replace shader source
	if (lev->SequenceID == 0)					// is this the last one?
		Core::InterlockSet(&m_Status, UNINITIALIZED);	// indicate load in progress
	return true;
}

}	// end Vixen