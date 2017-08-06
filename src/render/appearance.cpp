
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Appearance, ObjArray, VX_Appearance);

vint32			Appearance::s_MaxAppIndex = 0;


const TCHAR*	Appearance::s_AttrNames[NUM_ATTRS] = {
	TEXT("Culling"),
	TEXT("Lighting"),
	TEXT("Shading"),
	TEXT("ZBuffer"),
	TEXT("Transparency")
};

static const TCHAR* app_shade[] = {
	TEXT("smooth"),
	TEXT("wire"),
	TEXT("points")
};

static const TCHAR* app_filters[] = {
	TEXT("nearest"),
	TEXT("linear"),
	TEXT("mipmap"),
	TEXT("bilinear"),
	TEXT("(nearest | interpolate)"),
	TEXT("(linear | interpolate)"),
	TEXT("(mipmap | interpolate)"),
	TEXT("trilinear"),
	};

static const TCHAR* app_texop[] = {
	TEXT("emission"),
	TEXT("diffuse"),
	TEXT("specular"),
	TEXT("bump")
	};

bool	Appearance::DoLighting = true;

static const TCHAR* opnames[] = {
	TEXT("Set"),
	TEXT("SetSampler"),
	TEXT("SetMaterial"),
	TEXT("SetPixelShader")
};

const TCHAR** Appearance::DoNames = opnames;


/*!
 * @fn Appearance::Appearance()
 *
 * Constructs an Appearance object that describes the texture,
 * material, and attributes used to control how geometry
 * appears when rendered. The appearance is in a default state in which
 * there is no texture or material, texturing and lighting are disabled.
 *
 * @see Material::Material Texture::Texture Appearance::SetSampler Appearance::SetMaterial
 */
Appearance::Appearance(int numsamplers) : ObjArray()
{
	DevHandle = NULL;
	m_Attrs[CULLING] = false;
	m_Attrs[LIGHTING] = true;
	m_Attrs[ZBUFFER] = true;
	m_Attrs[TRANSPARENCY] = false;
	m_Attrs[SHADING] = SMOOTH;
	Core::InterlockInc(&s_MaxAppIndex);
	m_AppIndex = s_MaxAppIndex;
	for (int i = 0; i < numsamplers; ++i)
		SetSampler(i, new Sampler());
}

/*!
 * @fn Appearance::Appearance(const TCHAR* filename)
 * @param filename	name of texture to associate with Appearance
 *
 * Constructs an Appearance object with a texture loaded from
 * the filename specified. The material color of the new
 * appearance will be white. Texturing is enabled for the appearance.
 *
 * @see Sampler::Sampler
 */
Appearance::Appearance(const TCHAR* filename) : ObjArray()
{
	Sampler* sampler = new Sampler(filename);

	DevHandle = NULL;
	m_Attrs[CULLING] = false;
	m_Attrs[LIGHTING] = true;
	m_Attrs[ZBUFFER] = true;
	m_Attrs[TRANSPARENCY] = false;
	m_Attrs[SHADING] = SMOOTH;
	sampler->SetName(TEXT("Diffuse"));
	SetSampler(0, sampler);
	Core::InterlockInc(&s_MaxAppIndex);
	m_AppIndex = s_MaxAppIndex;
}

/*!
 * @fn Appearance::Appearance(const Col4& col)
 * @param col		diffuse color of Material associated with Appearance
 *
 * Constructs an Appearance object with a material of the given color.
 * The appearance will not have or lighting texturing enabled.
 *
 * @see Material::Material
 */
Appearance::Appearance(const Col4& col) : ObjArray()
{
	DevHandle = NULL;
	m_Attrs[CULLING] = false;
	m_Attrs[LIGHTING] = true;
	m_Attrs[ZBUFFER] = true;
	m_Attrs[TRANSPARENCY] = false;
	m_Attrs[SHADING] = SMOOTH;
	SetMaterial(new PhongMaterial(col));
	Core::InterlockInc(&s_MaxAppIndex);
	m_AppIndex = s_MaxAppIndex;
}

/*!
 * @fn Appearance::Appearance(const Appearance& src)
 * @param src input appearance to copy
 *
 * Constructs an Appearance object which is the same as the
 * input appearance.
 *
 * @see Material::Material Sampler::Sampler
 */
Appearance::Appearance(const Appearance& app) : ObjArray()
{
	DevHandle = NULL;
	Copy(&app);
}

/*!
 * @fn bool Appearance::operator==(const Appearance& src) const
 *
 * Compares two Appearance objects and determines whether they
 * are the same. If the Appearances have the same name, texture,
 * material, and the same attributes, they are considered the same.
 */
bool Appearance::operator==(const Appearance& src) const
{
	const TCHAR* name = GetName();
	const TCHAR* sname = src.GetName();

	if ((name != sname) && STRCMP(name, sname))
		return false;
	if (GetSize() != src.GetSize())
		return false;
	if (m_Material != src.m_Material)
		return false;
	for (int i = 0; i < NUM_ATTRS; ++i)
		if (m_Attrs[i] != src.m_Attrs[i])
			return false;
	for (intptr i = 0; i < GetSize(); ++i)
	{
		Sampler* smp1 = (Sampler*) GetAt(i);
		Sampler* smp2 = (Sampler*) src.GetAt(i);
		if (smp1 == smp2)
			continue;
		if (*smp1 != *smp2)
			return false;
	}
	return true;
}

/*!
 * @fn void Appearance::SetSampler(int stage, const Sampler* sampler)
 * @param stage	0 based index of texture stage
 * @param sampler sampler to use for this stage
 *
 * Designates the sampler to be used with the given texture stage.
 * There is usually a texture stage for each set of texture coordinates.
 * The sampler links a  particular texture coordinate set of the vertex
 * buffer with a texture and it's associated filtering options.
 *
 * @see Texture Appearance::SetMaterial VSampler:Set Appearance::Set
 */
bool Appearance::SetSampler(int stage, Sampler* sampler)
{
	if (sampler && IsGlobal())
		GetMessenger()->Distribute(sampler, GLOBAL);

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sampler, APPEAR_SetSampler) << this << int32(stage) << sampler;
	VX_STREAM_END( )
	
	return ObjArray::SetAt(stage, sampler);
}

bool Appearance::SetAt(int stage, SharedObj* obj)
{
	if (!obj->IsClass(VX_Sampler))
		return false;
	return SetSampler(stage, (Sampler*) obj);
}

/*!
 * @fn void Appearance::SetMaterial(DeviceBuffer* mat)
 *
 * Designates the material used to light objects using this Appearance.
 * The material controls how the geometry is lit, specifying how the
 * object reacts to different types of light. The material of an
 * appearance is ignored if vertex colors are provided in the geometry.
 *
 * @see Material VertexArray::SetStyle Appearance::SetTexture Appearance::Set
 */
void Appearance::SetMaterial(DeviceBuffer* mat)
{
	if (mat && IsGlobal())
		GetMessenger()->Distribute(mat, GLOBAL);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Appearance, APPEAR_SetMaterial) << this << mat;
	VX_STREAM_END(  )

	m_Material = mat;
}

/*!
 * @fn void Appearance::SetVertexShader(const Shader* shader)
 *
 * Designates the shader used to transform vertices rendered using this Appearance.
 * The shader controls how the geometry is transformed but does not affect
 * the color or lighting of the mesh.
 *
 * @see Shader VertexArray::SetStyle Appearance::GetVertexShader Appearance::SetPixelShader
 */
void Appearance::SetVertexShader(const Shader* shader)
{
	if (shader && IsGlobal())
		GetMessenger()->Distribute(shader, GLOBAL);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Appearance, APPEAR_SetVertexShader) << this << shader;
	VX_STREAM_END(  )

	m_VertexShader = shader;
}

/*!
 * @fn void Appearance::SetPixelShader(const Shader* shader)
 *
 * Designates the shader used to color and light geometry rendered with this Appearance.
 * The shader controls how the geometry is colored, textured and lit but does not affect
 * the shape or position of the mesh.
 *
 * @see Shader Sampler Appearance::GetPixelShader Appearance::SetVertexShader Appearance::SetSampler
 */
void Appearance::SetPixelShader(const Shader* shader)
{
	if (shader && IsGlobal())
		GetMessenger()->Distribute(shader, GLOBAL);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Appearance, APPEAR_SetPixelShader) << this << shader;
	VX_STREAM_END(  )

	m_PixelShader = shader;
}

/*!
 * @fn void Appearance::Set(int attr, int val)
 * @param attr appearance attribute to set (see table below)
 * @param val integer value to give the attribute
 *
 * Set the given Appearance attribute.
 * The Appearance attributes control how the texture and material are used to render geometry.
 * If you do not explicitly set an Appearance attribute, it will be
 * in the default state. You can \b unset an attribute again by setting
 * it's value to NONE The Appearance attributes are:
 * @code
 *	Apperarance::SHADING		type of shading
 *								SMOOTH, WIRE, POINTS
 *	Apperarance::CULLING		true to enable backface culling
 *	Apperarance::LIGHTING		true to enable lighting
 *	Apperarance::TRANSPARENCY	enable alpha blending
 *	Apperarance::ZBUFFER		enable hiddens surface removal
 * @endcode
 *
 * @see Appearance::Get Appearance::SetMaterial Appearance::SetTexture
 */
void Appearance::Set(int attr, int val)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Appearance, APPEAR_Set) << this << int32(attr) << int32(val);
	VX_STREAM_END(  )

	m_Attrs[attr] = val;
}

/*!
 * @fn int Appearance::GetAt(int attr) const
 *
 * Gets the value of the given appearance attribute.
 *
 * @return Value of the attribute or APPEAR_None if attribute is not set
 *
 * @see Appearance::Set Appearance::GetTexture Appearance::GetMaterial
 */
int Appearance::Get(int attr) const
{
	if ((attr < 0) || (attr >= NUM_ATTRS))
		return NONE;
	return m_Attrs[attr];
}

/*!
 * @fn const TCHAR* Appearance::GetAttrName(int attr) const
 * @param attr	0 based index of attribute whose name to get
 *
 * Gets the name of the given appearance attribute.
 *
 * @return -> name of the attribute or  NULL if not such attribute
 *
 * @see Sampler::Get
 */
const TCHAR* Appearance::GetAttrName(int attr) const
{
	if ((attr < 0) || (attr >= NUM_ATTRS))
		return NULL;
	return s_AttrNames[attr];
}

/*!
 * @fn Appearance* Appearance::GetSampler(int stage)
 * @param stage	0 based texture stage index
 * Retrieves the sampler associated with this texture stage.
 *
 * @return sampler referenced by index, NULL if index out of range
 *
 * @see Appearance::SetSampler Sampler
 */
Sampler* Appearance::GetSampler(int index)
{
	return (Sampler*) ObjArray::GetAt(index);
}

const Sampler* Appearance::GetSampler(int index) const
{
	return (const Sampler*) ObjArray::GetAt(index);
}

/****
 *
 * class Appearance override for SharedObj::Copy
 *
 ****/
bool Appearance::Copy(const SharedObj* src)
{
	const Appearance*	app = (const Appearance*) src;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!ObjArray::Copy(src))
		return false;
	if (!src->IsClass(VX_Appearance))
		return false;
	m_Material = app->m_Material;
	m_PixelShader = app->m_PixelShader;
	for (int i = 0; i < NUM_ATTRS; ++i)
		m_Attrs[i] = app->m_Attrs[i];
	return true;
}

/****
 *
 * class Appearance override for SharedObj::Do
 *		APPEAR_SetImage	<Texture*>
 *		APPEAR_Set			<int32 attr> <int32 val>
 *		APPEAR_SetMaterial	<Material*>
 *
 ****/
bool Appearance::Do(Messenger& s, int op)
{
	int32		n, val;
	SharedObj*		obj;
	Opcode		o = Opcode(op);

	switch (op)
	{
		case APPEAR_SetSampler:
		s >> n >> obj;
		VX_ASSERT(!obj || obj->IsClass(VX_Sampler));
		SetSampler(n, (Sampler*) obj);
		break;

		case APPEAR_SetMaterial:
		s >> obj;
		VX_ASSERT(!obj || obj->IsClass(VX_Material));
		SetMaterial((Material*) obj);
		break;

		case APPEAR_SetPixelShader:
		s >> obj;
		VX_ASSERT(!obj || obj->IsClass(VX_Shader));
		SetPixelShader((Shader*) obj);
		break;

		case APPEAR_Set:
		s >> n >> val;
		Set(n, val);
		break;

		default:
		return ObjArray::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Appearance::DoNames[op - APPEAR_Set]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Appearance override for SharedObj::Save
 *
 ****/
int Appearance::Save(Messenger& s, int opts) const
{
	int32	h = ObjArray::Save(s, opts);
	if (h < 0)
		return h;
	const Material* mat = m_Material;
	if (mat && (mat->Save(s, opts) >= 0) && h)
		s << OP(VX_Appearance, APPEAR_SetMaterial) << h << mat;
	if (h <= 0)
		return h;
	for (int32 attr = 0; attr < NUM_ATTRS; ++attr)
		s << OP(VX_Appearance, APPEAR_Set) << h << attr << m_Attrs[attr];
	return h;
}

DebugOut& Appearance::Print(DebugOut& dbg, int opts) const
{
	const Material*	mat = m_Material;

	if ((opts & (PRINT_Attributes | PRINT_Children)) == 0)
		return SharedObj::Print(dbg, opts);
	ObjArray::Print(dbg, opts & ~PRINT_Trailer);
	if (mat)
	{
		endl(dbg << "<attr name='material' >");
		mat->Print(dbg, opts | PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	for (int attr = 0; attr < NUM_ATTRS; ++attr)
		endl(dbg << "\t<attr name='" << s_AttrNames[attr] << "'>" << int32(m_Attrs[attr]) << "</attr>");
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}


/*!
 * @fn void Appearance::Apply(Model* root, int attr, int val)
 * @param root	root of hierarchy to apply to
 * @param attr	attribute to apply
 * @param val	new value to give attribute
 *
 * The appearance attribute is changed in all appearances associated with all
 * Shape nodes in the hierarchy (although the root node need not be a Shape).
 *
 * @par Example
 * @code
 *	// Displays the current dynamic scene in wireframe
 *	Appearance::Apply(GetMainScene()->GetModels(), Appearance::SHADING, Appearance::WIRE);
 * @endcode
 *
 * @see Appearance::Set Shape Shape::SetAppearance
 */
void Appearance::Apply(Model* root, int attr, int val)
{
	GroupIter<Shape>	iter((Shape*) root, Group::DEPTH_FIRST);
	Shape*			shape;
	Appearance*		app;

	while (shape = iter.Next())
	{
		if (!shape->IsClass(VX_Shape))
			continue;
		app = shape->GetAppearance();
		if (app)
			app->Set(attr, val);
	}
}
 }	// end Vixen