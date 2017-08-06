/*!
 * @file vxsampler.h
 * @brief base class to encapsulate texture sampler state.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vximage.h vxmaterial.h Sampler.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Sampler
 * @brief Encapsulates texture sampling attributes.
 *
 * A Sampler is attached to an Appearance object.
 * Each Sampler contains a Texture to be sampled.
 * It's attributes control how the texture is sampled by the Shader
 * code when it computes the color of a pixel.
 *
 * @ingroup vixen
 * @see Texture  Appearance Shader
 */
class Sampler : public SharedObj
{
	VX_DECLARE_CLASS(Sampler);

public:

	/*!
	 * @brief Index for sampler attributes.
	 * @see Sampler::Set Sampler::Get
	 */
	enum
	{
		NONE =		0,
		TEXTUREOP = 0,	//!< texturing operation (selects a built-in pixel shader)
		TEXCOORD  = 1,	//!< texture coordinate offset
		MINFILTER = 2,	//!< minifcation filter
		MAGFILTER = 3,	//!< magnification filter
		MIPMAP = 4,		//!< enable mipmapping
		NUM_ATTRS = 5
	};

	/*!
	 * @ brief Values for sampler attributes.
	 * @see Sampler::Set Sampler::Get
	 */
	enum
	{
		TEXGEN_SPHERE =	-2,	//!< TEXCOORD: spherical texture coordinate generation
		TEXGEN_CUBE = -3,	//!< TEXCOORD: cubemap texture coordinate generation

		NEAREST = 0,		//!< MINFILTER, MAGFILTER: nearest pixel
		LINEAR = 1,			//!< MINFILTER, MAGFILTER: average 4 nearest pixels

		EMISSION = 0,		//!< TEXTUREOP: use texture as emission
		DIFFUSE = 1,		//!< TEXTUREOP: use texture as diffuse map
		SPECULAR = 2,		//!< TEXTUREOP: add source and destination pixels
		BUMP = 3,			//!< TEXTUREOP: use texture as bump map
		SHADER = -1,		//!< TEXTUREOP: shader defines texture usage
	};

//! Constructs a default sampler.
	Sampler();
	Sampler(const Sampler& src);
	Sampler(const TCHAR* filename);
	Sampler(Texture*);

//! Compare two samplers to see if they are equal.
	virtual bool	operator==(const Sampler& src) const;
//! Compare two samplers to see if they differ.
	bool			operator!=(const Sampler& src) const	{ return !(*this == src); }
	Texture*		GetTexture()							{ return m_Texture; }
	const Texture*	GetTexture() const						{ return m_Texture; }
//! Establish the image to use when texturing geometry using this appearance.
	virtual	void	SetTexture(const Texture*);
//! Return the name of the attribute given its index.
	const TCHAR*	GetAttrName(int attr) const;
//! Return the value of the specified sampler attribute.
	int				Get(int attr) const;
//! Update the value of a sampler attribute.
	virtual	void	Set(int attr, int val);
//! True if mip-mapping enabled
	bool			IsMipMap() const						{ return Get(Sampler::MIPMAP) != 0; }
//! True if texture is loaded and has an alpha channel
	bool			HasAlpha() const						{ const Texture* t = m_Texture; return (t && t->HasAlpha()); }

//	Overrides
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger&, int);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Appearance::Do opcodes (for binary file format)
	 */
	enum Opcode
	{
		SAMPLER_SetTexture = ObjArray::ARRAY_NextOp,
		SAMPLER_Set,
		SAMPLER_LastOp = ObjArray::ARRAY_NextOp + 20
	};
	mutable voidptr	DevHandle;			// device dependent handle

protected:
// Internal
	Ref<Texture>	m_Texture;			// image to use for texturing
	 int			m_Attrs[NUM_ATTRS];

	static	const TCHAR* s_AttrNames[NUM_ATTRS];	// names of appearance attributes
};

} // end Vixen