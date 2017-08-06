/*!
 * @file vxappear.h
 * @brief base class to encapsulate rendering state.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vximage.h vxmaterial.h vxsampler.h
 */
#pragma once

namespace Vixen {


/*!
 * @class Shader
 * @brief encapsulates compiled code for a graphics accelerator.
 *
 * A shader is a blob of code that is executed by an external
 * graphics subsystem, typically not the main CPU. Usually,
 * shaders are written in a special shading language like HLSL or GLSL.
 * Vixen will load shader files and compile them automatically for you.
 *
 * A shader may be attached dynamically to the appearance of a mesh
 * to control how it is rendered. The code in the shader uses
 * attributes from the Material and Samplers associate with the Appearance
 * as it's input data. The shader code can also use pixels from the Texture
 * objects attached to each Sampler as input.
 *
 * @see Appearance::SetShader Material Sampler Texture
 * @ingroup vixen
 */
class Shader : public SharedObj
{
public:
	VX_DECLARE_CLASS(Shader);
	Shader(int type = Shader::PIXEL);
	~Shader();
	
	//! Returns true if shader source is present or loaded
	bool	HasSource() const					{ return m_SourceCode && (m_Status >= 0); }

	//! Get the format for the shader input data
	const DataLayout*	GetInputLayout() const;

	//! Get the format for the shader output data
	const DataLayout*	GetOutputLayout() const;

	//! Get the format for the shader input data as a string
	const TCHAR*	GetInputDesc() const		{ return m_InputDesc; }

	//! Set the format for shader input data
	void			SetInputDesc(const TCHAR* desc);

	//! Get the format for the shader output data as a string
	const TCHAR*	GetOutputDesc() const		{ return m_OutputDesc; }

	//! Set the format for shader output data
	void			SetOutputDesc(const TCHAR* desc);

	//! Get the name of the file containing shader source code
	const TCHAR*	GetFileName() const			{ return m_FileName; }

	//! Set the name of the file containing shader source code
	void			SetFileName(const TCHAR* file_name);

	//! Get the source code for the shader
	const TCHAR*	GetSource() const			{ return m_SourceCode; }

	//! Set the name of the shader function
	void			SetSource(const TCHAR* source_string);

	//! Get the type of the shader (Shader::VERTEX or Shader::PIXEL)
	int				GetShaderType() const		{ return m_Type; }

	//! Get the binary code for a shader if it is available.
	bool			GetShaderCode(const void** data, size_t* length) const;

	//! Set the binary code for a shader.
	void			SetShaderCode(const void* data, size_t length) const;

//	Overrides
	bool			operator==(const Shader& src);
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger&, int);
	virtual bool	OnEvent(Event*);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Shader types. If the CLASS modifier is used, the Shader
	 * is used by other shaders and may be dynamically linked.
	 */
	enum
	{
		PIXEL = 0,		//!< GetShaderType designates a pixel shader
		VERTEX = 1,		//!< GetShaderType designates a vertex shader
		LIGHT = 2,		//!< GetShaderType designates a light shader
		CLASS = 16		//!< GetShaderType designates a dynamically linked shader
	};


	enum Opcode
	{
		SHADER_SetShaderName = SharedObj::OBJ_NextOp,
		SHADER_SetFileName,
		SHADER_SetSource,
		SHADER_SetShaderType,
		SHADER_SetInputDesc,
		SHADER_SetOutputDesc,
		SHADER_LastOp = SharedObj::OBJ_NextOp + 20
	};

	mutable voidptr	DevHandle;			// device handle for shader

protected:
	
	/*!
	 * Shader status.
	 */
	enum Status
	{
		UNINITIALIZED = -1,	// shader source not available
		SOURCE = 0,			// source loaded, needs compile
		CODE = 1,			// shader compiled
	};

	int32			m_Type;				// type of shader
	Core::String	m_FileName;			// file name of shader source
	Core::String	m_InputDesc;		// input layout description
	Core::String	m_OutputDesc;		// output layout description
	TCHAR*			m_SourceCode;		// string with source code for shader
	mutable voidptr				m_BinaryCode;		// -> binary code for shader
	mutable intptr 				m_BinaryLength;		// byte length of code
	mutable vint32				m_Status;			// UNINITIALIZED, SOURCE or CODE
	mutable const DataLayout*	m_InputLayout;		// input layout
	mutable const DataLayout*	m_OutputLayout;		// output layout
};

/*!
 * @class Appearance
 * @brief Encapsulates the rendering, lighting and texturing attributes
 * that can be applied to geometry.
 *
 * Each Appearance may contain a Material to define lighting
 * properties and one or more Samplers that identify Textures
 * used to light or color the mesh it is used with. The pixel Shader
 * code which executes to compute the final pixel color for the mesh
 * uses these objects as input.
 *
 * The rendering thread is responsible for updating any objects which
 * may be cached on the remote GPU with changes made to the Appearance
 * and it's dependents every frame.
 *
 * @image html appearance.png
 *
 * @ingroup vixen
 * @see Texture Material Scene
 */
class Appearance : public ObjArray
{
	VX_DECLARE_CLASS(Appearance);
	friend class World3D;		// so default appearance can be deleted
public:

	/*!
	 * @brief Index for appearance attributes.
	 * @see Set Get
	 */
	enum
	{
		NONE = 0,
		CULLING = 0,		//!< enable/disable backface culling
		LIGHTING = 1,		//!< enable/disable 3D lighting
		SHADING = 2,		//!< type of surface shading (smooth, wireframe)
		ZBUFFER = 3,		//!< enable/disable hidden surface removal
		TRANSPARENCY = 4,	//!< enable/disable alpha blending
		NUM_ATTRS = 5
	};

	/*!
	 * @brief value for Appearance::SHADING attribute
	 * @see Set Get
	 */
	enum
	{
		SMOOTH = 1,	//!< smooth shading
		WIRE = 2,	//!< wireframe
		POINTS = 3	//!<points
	};

//! Constructs a default appearance.
	Appearance(int numsamplers = 0);
	Appearance(const TCHAR* filename);
	Appearance(const Appearance&);
	Appearance(const Col4&);

//! Compare two appearances to see if they are equal.
	virtual bool	operator==(const Appearance& src) const;
//! Compare two appearances to see if they differ.
	bool			operator!=(const Appearance& src) const	{ return !(*this == src); }
//! Return the appearanced used for lighting.
	DeviceBuffer*	GetMaterial();
	const DeviceBuffer*	GetMaterial() const;
//! Establish the material to use when lighting geometry using this appearance.
	virtual	void	SetMaterial(DeviceBuffer*);
//! Update the texture sampler at the given index
	virtual	bool	SetSampler(int index, Sampler* app);
//! Retrieve texture sampler at the given index
	Sampler*		GetSampler(int index);
	const Sampler*	GetSampler(int index) const;
	virtual bool	SetAt(int i, SharedObj* obj);
//! Return the number of samplers
	int				GetNumSamplers() const	{ return (int) GetSize(); }
//!	Establish the shader to use to transform vertices.
	virtual	void	SetVertexShader(const Shader*);
//!	Return the shader used for transforming vertices.
	const Shader*	GetVertexShader() const;
//!	Establish the shader to use to color pixels
	virtual	void	SetPixelShader(const Shader*);
//!	Return the shader used for shading pixels.
	const Shader*	GetPixelShader() const;
//! Return the value of the specified appearance attribute.
	int				Get(int attr) const;
//! Update the value of an appearance attribute.
	virtual	void	Set(int attr, int val);
//! Get appearance index (for internal use)
	int				GetAppIndex() const	{ return m_AppIndex; }
//! Return the name of the attribute given its index.
	const TCHAR*	GetAttrName(int attr) const;

//! Set an appearance attribute for all nodes in a hierarchy.
	static 	void	Apply(Model* root, int attr, int val);

//	Overrides
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger&, int);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	mutable voidptr	DevHandle;			//!< device-specific handle
	static	bool	DoLighting;			//!< globally enable/disable lighting

	/*
	 * Appearance::Do opcodes (for binary file format)
	 */
	enum Opcode
	{
		APPEAR_Set = ObjArray::ARRAY_NextOp,
		APPEAR_SetSampler,
		APPEAR_SetMaterial,
		APPEAR_SetPixelShader,
		APPEAR_SetVertexShader,
		APPEAR_LastOp = ObjArray::ARRAY_NextOp + 20
	};

protected:
// Internal
	Ref<Material>		m_Material;			// material to use for lighting
	Ref<Shader>			m_PixelShader;		// shader use for pixels
	Ref<Shader>			m_VertexShader;		// shader use for vertices
	int32				m_AppIndex;			// appearance index
	mutable int32		m_Attrs[NUM_ATTRS];

	static	vint32			s_MaxAppIndex;	// maximum appearance index
	static	const TCHAR*	s_AttrNames[NUM_ATTRS]; // names of appearance attributes
};

} // end Vixen
