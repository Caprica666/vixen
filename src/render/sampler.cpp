/*
 * All sampler attributes accessible thru the Set and Get
 * functions are kept in a device independent format.
 * For each attribute, there is a bit flag maintained to
 * indicate whether the attribute has been changed with Set.
 * If Set has never been called for the attribute, the flag
 * for it will be clear. The bit flags in m_Enabled are
 * maintained to allow device dependent code that loads the
 * render state to be optimized.
 *
 * Sampler implementations must provide functions to
 * maintain the device state for the sampler attributes.
 */
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Sampler, SharedObj, VX_Sampler);

const TCHAR*	Sampler::s_AttrNames[NUM_ATTRS] = {
	TEXT("TextureOp"),
	TEXT("TexCoord"),
	TEXT("MinFilter"),
	TEXT("MagFilter"),
	TEXT("MipMap"),
};


static const TCHAR* app_filters[] = {
	TEXT("nearest"),
	TEXT("linear"),
	};


static const TCHAR* app_texop[] = {
	TEXT("emission"),
	TEXT("diffuse"),
	TEXT("specular"),
	TEXT("bump")
	};

static	const TCHAR**	app_attr_vals[] = {
	app_texop,		/* TextureOp */
	NULL,			/* TexCoord */
	NULL,			/* Transparency */
	app_filters,	/* MinFilter */
	app_filters,	/* MagFilter */
	NULL,			/* MipMap */
};

static const TCHAR* opnames[] =
	{ TEXT("SetTexture"), TEXT("Set"), NULL  };

const TCHAR** Sampler::DoNames = opnames;

/*!
 * @fn Sampler::Sampler()
 *
 * Constructs a sampler object that describes the texture, filtering and
 * texture coordinate generation options.
 *
 * @see Texture::Texture Sampler::SetTexture Appearance::SetSampler
 */
Sampler::Sampler() : SharedObj()
{
	DevHandle = NULL;
	m_Attrs[TEXCOORD] = 0;
	m_Attrs[MINFILTER] = NEAREST;
	m_Attrs[MAGFILTER] = NEAREST;
	m_Attrs[MIPMAP] = false;
	m_Attrs[TEXTUREOP] = DIFFUSE;
}

/*!
 * @fn Sampler::Sampler(const TCHAR* filename)
 * @param filename	name of texture to associate with sampler
 *
 * Constructs a sampler object with a texture loaded from
 * the filename specified.
 *
 * @see Texture::Texture
 */
Sampler::Sampler(const TCHAR* filename) : SharedObj()
{
	Texture*	tex = new Texture();

	DevHandle = NULL;
	m_Attrs[TEXCOORD] = 0;
	m_Attrs[MINFILTER] = NEAREST;
	m_Attrs[MAGFILTER] = NEAREST;
	m_Attrs[MIPMAP] = false;
	m_Attrs[TEXTUREOP] = DIFFUSE;
	SetTexture(tex);
	tex->Load(filename);
}

/*!
 * @fn Sampler::Sampler(Texture* image)
 * @param image	Texture representing the texture to associate with sampler
 *
 * Constructs a sampler object with a texture loaded from
 * the Texture object provided.
 *
 * @see Texture::Texture
 */
Sampler::Sampler(Texture* image) : SharedObj()
{
	DevHandle = NULL;
	m_Attrs[TEXCOORD] = 0;
	m_Attrs[MINFILTER] = NEAREST;
	m_Attrs[MAGFILTER] = NEAREST;
	m_Attrs[MIPMAP] = false;
	m_Attrs[TEXTUREOP] = DIFFUSE;
	SetTexture(image);
}

/*!
 * @fn Sampler::Sampler(const Sampler& src)
 * @param src input appearance to copy
 *
 * Constructs a sampler object which is the same as the
 * input appearance.
 *
 * @see Texture::Texture Sampler::Sampler
 */
Sampler::Sampler(const Sampler& src) : SharedObj()
{
	DevHandle = NULL;
	Copy(&src);
}

/*!
 * @fn bool Sampler::operator==(const Sampler& src) const
 *
 * Compares two sampler objects and determines whether they
 * are the same. If the samplers have the same name, texture,
 * and the same attributes, they are considered the same.
 */
bool Sampler::operator==(const Sampler& src) const
{
	const TCHAR* name = GetName();
	const TCHAR* sname = src.GetName();

	if ((name != sname) && STRCMP(name, sname))
		return false;
	if (m_Texture != src.m_Texture)
		return false;
	for (int i = 0; i < NUM_ATTRS; ++i)
		if (m_Attrs[i] != src.m_Attrs[i])
			return false;
	return true;
}

/*!
 * @fn void Sampler::SetTexture(const Texture* image)
 *
 * Designates which image to be used as the texture for this sampler.
 * How the texture is mapped onto the geometry is specified by
 * supplying  texture  coordinates with the vertices. If these
 * are missing, the texture will be ignored and will appear white in the scene.
 *
 * @see Texture Sampler::SetMaterial VertexArray::SetStyle Sampler::Set
 */
void Sampler::SetTexture(const Texture* image)
{
	if (image && IsGlobal())
		GetMessenger()->Distribute(image, GLOBAL);

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sampler, SAMPLER_SetTexture) << this << image;
	VX_STREAM_END( )

	m_Texture = image;
}


/*!
 * @fn void Sampler::Set(int attr)
 * @param attr sampler attribute to set (see table below)
 * @param val integer value to give the attribute
 *
 * Set the given sampler attribute.
 * The sampler attributes control how the texture and material are used to render geometry.
 * If you do not explicitly set a sampler attribute, it will be
 * in the default state. You can \b unset an attribute again by setting
 * it's value to 0. The sampler attributes are:
 * @code
 *	Sampler::TEXTUREOP	how texture is applied
 *						EMISSION, DIFFUSE, SPECULAR, BUMP
 *	Sampler::TEXCOORD	texture coordinate set (0 based index) or texture coordinate generation  method
 *						TEXGEN_SPHERE, TEXGEN_CUBE
 *	Sampler::MAGFILTER	magnification filter: LINEAR, NEAREST
 *	Sampler::MINFILTER	minification filter: LINEAR, NEAREST
 *	Sampler::MIPMAP		enable / disable mip mapping
 * @endcode
 *
 * @see Sampler::Get Sampler::SetMaterial Sampler::SetTexture
 */
void Sampler::Set(int attr, int val)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Appearance, SAMPLER_Set) << this << int32(attr) << int32(val);
	VX_STREAM_END(  )

	if ((attr < 0) || (attr >= NUM_ATTRS))
		return;
	m_Attrs[attr] = val;
}

/*!
 * @fn int Sampler::Get(int attr) const
 *
 * Gets the value of the given sampler attribute.
 *
 * @return Value of the attribute or 0 if attribute is not set
 *
 * @see Sampler::Set Sampler::GetTexture
 */
int Sampler::Get(int attr) const
{
	if ((attr < 0) || (attr >= NUM_ATTRS))
		return NONE;
	return m_Attrs[attr];
}

/*!
 * @fn const TCHAR* Sampler::GetAttrName(int attr) const
 * @param attr	0 based index of attribute whose name to get
 *
 * Gets the name of the given sampler attribute.
 *
 * @return -> name of the attribute or  NULL if not such attribute
 *
 * @see Sampler::Get
 */
const TCHAR* Sampler::GetAttrName(int attr) const
{
	if ((attr < 0) || (attr >= NUM_ATTRS))
		return NULL;
	return s_AttrNames[attr];
}

/****
 *
 * class Sampler override for SharedObj::Copy
 *
 ****/
bool Sampler::Copy(const SharedObj* src)
{
	const Sampler*	app = (const Sampler*) src;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!SharedObj::Copy(src))
		return false;
	if (!src->IsClass(VX_Sampler))
		return false;
	m_Texture = app->m_Texture;
	for (int i = 0; i < NUM_ATTRS; ++i)
		m_Attrs[i] = app->m_Attrs[i];
	return true;
}

/****
 *
 * class Appearance override for SharedObj::Do
 *		SAMPLER_SetImage	<Texture*>
 *		SAMPLER_Set			<int32 attr> <int32 val>
 *		SAMPLER_SetMaterial	<Material*>
 *
 ****/
bool Sampler::Do(Messenger& s, int op)
{
	int32				attr, val;
	const Texture*		image;
	SharedObj*				obj;
	Opcode				o = Opcode(op);

	switch (op)
	{
		case SAMPLER_SetTexture:
		s >> obj;
		VX_ASSERT(!obj || obj->IsClass(VX_Image));
		image = (const Texture*) obj;
		SetTexture(image);
		break;

		case SAMPLER_Set:
		s >> attr >> val;
		Set(attr, val);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Sampler::DoNames[op - SAMPLER_SetTexture]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Sampler override for SharedObj::Save
 *
 ****/
int Sampler::Save(Messenger& s, int opts) const
{
	int32	h = SharedObj::Save(s, opts);
	if (h < 0)
		return h;
	const Texture* image = m_Texture;
	if (image && (image->Save(s, opts) >= 0) && h)
		s << OP(VX_Sampler, SAMPLER_SetTexture) << h << image;
	if (h == 0)
		return h;
	for (int32 i = 0; i < NUM_ATTRS; ++i)
		s << OP(VX_Sampler, SAMPLER_Set) << h << i << int32(m_Attrs[i]);
	return h;
}

DebugOut& Sampler::Print(DebugOut& dbg, int opts) const
{
	const Texture*		image = m_Texture;

	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);

	if (image)
		image->Print(dbg, opts | PRINT_Trailer);
	for (int i = 0; i < NUM_ATTRS; ++i)
		endl(dbg << "\t<attr name='" << s_AttrNames[i] << "'>" << int32(m_Attrs[i]) << "</attr>");
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}

}	// end Vixen