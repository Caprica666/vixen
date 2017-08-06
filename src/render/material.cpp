/****
 *
 * Material Class
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Material, DeviceBuffer, VX_Material);
VX_IMPLEMENT_CLASSID(PhongMaterial, Material, VX_PhongMaterial);

static const TCHAR* opnames1[] =
	{ TEXT("SetDescriptor"), TEXT("SetData"), };

static const TCHAR* opnames2[] =
	{ TEXT("SetDiffuse"), TEXT("SetSpecular"), TEXT("SetAmbient"), TEXT("SetEmission"), TEXT("SetShine") };

const TCHAR** Material::DoNames = opnames1;

const TCHAR** PhongMaterial::DoNames = opnames2;

Material::Material(const TCHAR* desc, int nbytes)
  : DeviceBuffer(desc, nbytes)
{
}


DebugOut& Material::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	if (m_Data && m_Layout && (m_Layout->Size > 0))
		for (int i = 0; i < m_Layout->Size; ++i)
		{
			const LayoutSlot& slot = m_Layout->Slot[i];
			float*	fval = ((float*) m_Data) + slot.Offset;
			int32*	ival = ((int32*) m_Data) + slot.Offset;

			dbg << "\t<attr name='" << slot.Name << "'>";
			for (int j = 0; j < slot.Size; ++j)
			{
				if (j > 0)
					dbg << ", ";
				if (slot.Style & DataLayout::INTEGER)
					dbg << ival[j];
				else
					dbg << fval[j];
			}
			endl(dbg << "</attr>");
		}
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}


bool Material::Do(Messenger& s, int op)
{
	int32	n;
	TCHAR	desc[1024];

	switch (op)
	{
		case MAT_SetDesc:
		s >> desc;
		m_Layout = DataLayout::FindLayout(desc);
		break;

		case MAT_SetData:
		s >> n;
		VX_ASSERT(m_Layout);
		if (m_Data == NULL)
			m_Data = Core::GlobalAllocator::Get()->Alloc(n * sizeof(int32));
		VX_ASSERT(n == m_Layout->Size);
		s.Input((int32*) m_Data, n);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Material::DoNames[op - MAT_SetDesc]
					   << " " << this);
#endif
	return true;
}


int Material::Save(Messenger& s, int opts) const
{
	int32	h = SharedObj::Save(s, opts);
	int		n;

	if (h <= 0)
		return h;
	if (m_Layout)
	{
		n = m_Layout->Size;
		s << OP(VX_Material, MAT_SetDesc) << m_Layout->Descriptor;
		if (m_Data && n)
		{
			s << OP(VX_Material, MAT_SetData) << n;
			s.Output((int32*) m_Data, n);
		}
	}
	return h;
}

bool Material::Copy(const SharedObj* src_obj)
{
	const Material*	src = (const Material*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);

	if (!DeviceBuffer::Copy(src_obj))
		return false;
	if (!src->IsKindOf(CLASS_(Material)))
		return true;
	if (m_Data && src->m_Data && (src->m_Layout == m_Layout))
		memcpy(m_Data, src->m_Data, m_Layout->Size * sizeof(int32));
	SetChanged(true);
	return true;
}

/*!
 * @fn PhongMaterial::PhongMaterial(const Col4* diffuse)
 * @param diffuse	initial value of diffuse color, default to white if omitted
 *
 * Makes a material with the given diffuse color.
 * Specular, emissive and ambient color are set to black
 * and shine is set to zero, disabling specular reflections initially.
 *
 * @see PhongMaterial::SetDiffuse
 */
PhongMaterial::PhongMaterial(const Col4* c)
  : Material(TEXT("float4 Diffuse, float4 Ambient, float4 Specular, float4 Emission, float Shine, int HasDiffuseMap, int HasSpecularMap, int HasNormalMap"))
{
	SetData(&m_ShaderData.Diffuse);
	if (c)
	{
		m_ShaderData.Diffuse = *c;
		m_ShaderData.Ambient.Set(0.0f, 0.0f, 0.0f);
	}
	else
	{
		m_ShaderData.Diffuse.Set(0.5f, 0.5f, 0.5f);
		m_ShaderData.Ambient.Set(0.5f, 0.5f, 0.5f);
	}
	m_ShaderData.Specular.Set(0.0f, 0.0f, 0.0f);
	m_ShaderData.Emission.Set(0.0f, 0.0f, 0.0f);
	m_ShaderData.Shine = 0.0f;
}

/*!
 * @fn PhongMaterial::PhongMaterial(const Col4& diffuse)
 * @param diffuse	initial value of diffuse color, default to white if omitted
 *
 * Makes a material with the given diffuse color.
 * Specular, emissive and ambient color are set to black
 * and shine is set to zero, disabling specular reflections initially.
 *
 * @see PhongMaterial::SetDiffuse
 */
PhongMaterial::PhongMaterial(const Col4& col)
  : Material(TEXT("float4 Diffuse, float4 Ambient, float4 Specular, float4 Emission, float Shine, int HasDiffuseMap, int HasSpecularMap, int HasNormalMap"))
{
	SetData(&m_ShaderData);
	m_ShaderData.Diffuse = col;
	m_ShaderData.Ambient.Set(0.0f, 0.0f, 0.0f);
	m_ShaderData.Specular.Set(0.0f, 0.0f, 0.0f);
	m_ShaderData.Emission.Set(0.0f, 0.0f, 0.0f);
	m_ShaderData.Shine = 0.0f;
}

PhongMaterial::PhongMaterial(const PhongMaterial& src)
  : Material(TEXT("float4 Diffuse, float4 Ambient, float4 Specular, float4 Emission, float Shine, int HasDiffuseMap, int HasSpecularMap, int HasNormalMap"))
{
	SetData(&m_ShaderData);
	m_ShaderData.Diffuse = src.m_ShaderData.Diffuse;
	m_ShaderData.Ambient = src.m_ShaderData.Ambient;
	m_ShaderData.Specular = src.m_ShaderData.Specular;
	m_ShaderData.Emission = src.m_ShaderData.Emission;
	m_ShaderData.Shine = src.m_ShaderData.Shine;
}

PhongMaterial::~PhongMaterial()
{
	m_Data = NULL;
}

/*!
 * @fn bool PhongMaterial::operator==(const PhongMaterial& src)
 *
 * Compares the source material with this one
 * and returns \b true if the diffuse, specular,
 * ambient and emission colors and the shine are the same.
 *
 * @returns  true if materials the same, else \b false
 */
bool PhongMaterial::operator==(const PhongMaterial& src)
{
	if (m_ShaderData.Diffuse != src.m_ShaderData.Diffuse)
		return false;
	if (m_ShaderData.Specular != src.m_ShaderData.Specular)
		return false;
	if (m_ShaderData.Shine != src.m_ShaderData.Shine)
		return false;
	if (m_ShaderData.Ambient != src.m_ShaderData.Ambient)
		return false;
	if (m_ShaderData.Emission != src.m_ShaderData.Emission)
		return false;
	return true;
}

/****
 *
 * Class PhongMaterial override for SharedObj::Copy
 * 	PhongMaterial::Copy(SharedObj* src)
 *
 * Copies the contents of one material object into another.
 * If the source object is not a material, the attributes
 * which are material-specific are unchanged in the destination.
 *
 ****/
bool PhongMaterial::Copy(const SharedObj* src_obj)
{
	const PhongMaterial*	src = (const PhongMaterial*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!DeviceBuffer::Copy(src_obj))
		return false;
	if (!src->IsClass(VX_Material))
		return true;
	m_ShaderData.Ambient = src->m_ShaderData.Ambient;
	m_ShaderData.Diffuse = src->m_ShaderData.Diffuse;
	m_ShaderData.Specular = src->m_ShaderData.Specular;
	m_ShaderData.Emission = src->m_ShaderData.Emission;
	m_ShaderData.Shine = src->m_ShaderData.Shine;
	SetChanged(true);
	return true;
}

/*!
 * @fn void PhongMaterial::SetDiffuse(const Col4& c)
 * @param c	new value for diffuse color
 *
 * The diffuse material color is the base color of the material,
 * what color diffuse light is when reflected from the object.
 *
 * @see PhongMaterial::SetSpecular PhongMaterial::SetAmbient PhongMaterial::PhongMaterial
 */

void PhongMaterial::SetDiffuse(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PhongMaterial, MAT_SetDiffuse) << this << c;
	VX_STREAM_END( )
	m_ShaderData.Diffuse = c;
	SetChanged(true);
}


/*!
 * @fn void PhongMaterial::SetSpecular(const Col4& c)
 * @param c	new value for specular color
 *
 * The specular material color is the shine color,
 * what color specular highlights are when reflected.
 *
 * @see PhongMaterial::SetDiffuse PhongMaterial::SetAmbient PhongMaterial::PhongMaterial
 */
void PhongMaterial::SetSpecular(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PhongMaterial, MAT_SetSpecular) << this << c;
	VX_STREAM_END(  )
	m_ShaderData.Specular = c;
	SetChanged(true);
}

/*!
 * @fn void PhongMaterial::SetAmbient(const Col4& c)
 * @param c	new value for ambient color
 *
 * The ambient material color is the amount this material
 * contributes to overall ambient light when it is encountered.
 *
 * @see PhongMaterial::SetDiffuse PhongMaterial::SetSpecular PhongMaterial::PhongMaterial
 */
void PhongMaterial::SetAmbient(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PhongMaterial, MAT_SetAmbient) << this << c;
	VX_STREAM_END(  )
	m_ShaderData.Ambient = c;
	SetChanged(true);
}

/*!
 * @fn void PhongMaterial::SetEmission(const Col4& c)
 * @param c	new value for emission color
 *
 * The emissive material color is the color of light emitted
 * by the material (not reflected light)
 *
 * @see PhongMaterial::SetDiffuse PhongMaterial::SetSpecular PhongMaterial::PhongMaterial
 */
void	PhongMaterial::SetEmission(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PhongMaterial, MAT_SetEmission) << this << c;
	VX_STREAM_END(  )

	m_ShaderData.Emission = c;
	SetChanged(true);
}

/*!
 * @fn void PhongMaterial::SetShine(float shine)
 * @param shine new value for shine
 *
 * The shininess of the material (this is the
 * specular power in the lighting equation)
 *
 * @see PhongMaterial::SetDiffuse PhongMaterial::SetSpecular PhongMaterial::PhongMaterial
 */
void	PhongMaterial::SetShine(float shine)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PhongMaterial, MAT_SetShine) << this << shine;
	VX_STREAM_END(  )

	m_ShaderData.Shine = shine;
	SetChanged(true);
}

/****
 *
 * Class PhongMaterial override for SharedObj::PrintInfo
 *
 * Prints an ASCII description of the material
 *
 ****/
DebugOut& PhongMaterial::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Diffuse'>" << m_ShaderData.Diffuse << "</attr>");
	endl(dbg << "\t<attr name='Specular'>" << m_ShaderData.Specular << "</attr>");
	endl(dbg << "\t<attr name='Ambient'>" << m_ShaderData.Ambient << "</attr>");
	endl(dbg << "\t<attr name='Emission'>" << m_ShaderData.Emission << "</attr>");
	endl(dbg << "\t<attr name='Shine'>" << m_ShaderData.Shine << "</attr>");
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class PhongMaterial override for SharedObj::Do
 *		MAT_SetDiffuse	<Col4>
 *		MAT_SetSpecular	<Col4>
 *		MAT_SetEmission	<Col4>
 *		MAT_SetAmbient	<Col4>
 *		MAT_SetShine	<float>
 *
 ****/
bool PhongMaterial::Do(Messenger& s, int op)
{
	float	f;
	Col4	c;

	switch (op)
	{
		case MAT_SetDiffuse:
		s >> c;
		SetDiffuse(c);
		break;

		case MAT_SetAmbient:
		s >> c;
		SetAmbient(c);
		break;

		case MAT_SetSpecular:
		s >> c;
		SetSpecular(c);
		break;

		case MAT_SetEmission:
		s >> c;
		SetEmission(c);
		break;

		case MAT_SetShine:
		s >> f;
		if (f <= 1.0f)
			f *= 128.0f;	// TODO: make Maya exporter scale up power
		SetShine(f);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << PhongMaterial::DoNames[op - MAT_SetDiffuse]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class PhongMaterial override for SharedObj::Save
 *
 ****/
int PhongMaterial::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_PhongMaterial, MAT_SetAmbient) << h << GetAmbient();
	s << OP(VX_PhongMaterial, MAT_SetDiffuse) << h << GetDiffuse();
	s << OP(VX_PhongMaterial, MAT_SetSpecular) << h << GetSpecular();
	s << OP(VX_PhongMaterial, MAT_SetEmission) << h << GetEmission();
	s << OP(VX_PhongMaterial, MAT_SetShine) << h << GetShine();
	return h;
}

}	// end Vixen