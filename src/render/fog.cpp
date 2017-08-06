/****
 *
 * Fog Class
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Fog, SharedObj, VX_Fog);

#ifdef _DEBUG
static const TCHAR* opnames[] =
	{ TEXT("SetDensity"), TEXT("SetColor"), TEXT("SetStart"), TEXT("SetEnd"), TEXT("SetKind") };

const TCHAR** Fog::DoNames = opnames;
#endif

/*!
 * @fn Fog::Fog()
 *
 * Default conditions for fog are:
 * @code
 *	Kind	FOG_Linear | FOG_Vertex
 *	Color	Col4(0.5, 0.5, 0.5)
 *	Density	0.1
 *	Start	0.01
 *	End		100
 * @endcode
 *
 * @see Fog::SetKind Scene::SetFog
 */
Fog::Fog() : SharedObj()
{
	m_Color.Set(0.5f, 0.5f, 0.5f, 0.0f);
	m_Kind = VERTEX | LINEAR;
	m_Density = 0.1f;
	m_Start = 0.01f;
	m_End = 100.0f;
	SetChanged(true);
}

Fog::Fog(const Fog& src) : SharedObj()
	{ Copy(&src); }

bool Fog::operator==(const Fog& src)
{
	if (m_Kind != src.m_Kind)
		return false;
	if (m_Density != src.m_Density)
		return false;
	if (m_Color != m_Color)
		return false;
	return true;
}

/****
 *
 * Class Fog override for SharedObj::Copy
 * 	Fog::Copy(SharedObj* src)
 *
 * Copies the contents of one material object into another.
 * If the source object is not a material, the attributes
 * which are material-specific are unchanged in the destination.
 *
 ****/
bool Fog::Copy(const SharedObj* src_obj)
{
	const Fog*	src = (const Fog*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!SharedObj::Copy(src_obj))
		return false;
	if (!src->IsClass(VX_Fog))
		return true;
	m_Kind = src->m_Kind;
	m_Density = src->m_Density;
	m_Start = src->m_Start;
	m_End = src->m_End;
	m_Color = src->m_Color;
	return true;
}

/*!
 * @fn void Fog::SetDensity(float density)
 * @param density	density of fog
 *
 * The density of the fog controls how thick it appears and
 * is used by all types of fog.
 *
 * @see Fog::SetColor
 */
void Fog::SetDensity(float density)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Fog, FOG_SetDensity) << this << density;
	VX_STREAM_END(  )

	SetChanged(true);
	m_Density = density;
}

/*!
 * @fn void Fog::SetColor(const Col4& c)
 * @param c	color of fog
 *
 * The fog color is the color used to blend with at each pixel.
 * For best results, the scene's background color and fog color
 * should be the same.
 *
 * @see Fog::SetDensity Fog::SetStart Fog::SetEnd Scene::SetBackColor
 */
void Fog::SetColor(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Fog, FOG_SetColor) << this << c;
	VX_STREAM_END(   )

	SetChanged(true);
	m_Color = c;
}

/*!
 * @fn void Fog::SetKind(int kind)
 * @param kind	type of fog model to use
 *
 * The fog kind controls which fog equation is used and whether
 * fogging is done at the pixel or vertex level. The three methods
 * of fogging and the equations to compute the fog blending factor are:
 *
 *	FOG_Linear			f = (end - z) / (end - start)
 *	FOG_Exponential		f = e ** -(density * z)
 *	FOG_Exponential2	f = e ** -(density * z) ** 2
 *
 *
 * where \b z is the distance from the eye to the pixel or vertex
 * and \b start and  end are near and far distances.
 *
 * Setting FOG_Pixel indicates fogging should be done at the pixel level.
 * Usually, this requires a 3D accelerator that supports this in hardware
 * to get good performance. Some displays will ignore the request for fog
 * or degenerate to vertex level fogging if the hardware does not support it.
 *
 * Setting FOG_Vertex will perform fog computations at the vertex level.
 * This gives the best performance if you do not have hardware fog support
 * but does not look as nice. This option may not be available on some
 * display cards and pixel fog is used instead.
 *
 * @see Fog::SetStart Fog::SetEnd Fog::SetDensity
 */
void Fog::SetKind(int kind)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Fog, FOG_SetKind) << this << int32(kind);
	VX_STREAM_END(   )

	SetChanged(true);
	m_Kind = kind;
}

/*!
 * @fn void Fog::SetStart(float start)
 * @param start	near distance for linear fog
 *
 * The start of the fog is the nearest distance at which fogging
 * should occur. It is only used with linear fog.
 *
 * @see Fog::SetColor Fog::SetEnd Fog::SetKind Fog::SetDensity
 */
void Fog::SetStart(float start)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Fog, FOG_SetStart) << this << start;
	VX_STREAM_END(  )

	SetChanged(true);
	m_Start = start;
}

/*!
 * @fn void Fog::SetEnd(float end)
 * @param end	far distance for linear fog
 *
 * The end of the fog is the farthest distance at which fogging
 * should occur. It is only used with linear fog.
 *
 * @see Fog::SetColor Fog::SetStart Fog::SetKind Fog::SetDensity
 */
void Fog::SetEnd(float end)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Fog, FOG_SetEnd) << this << end;
	VX_STREAM_END(  )

	SetChanged(true);
	m_End = end;
}

/****
 *
 * Class Fog override for SharedObj::Print
 *
 * Prints an ASCII description of the fog properties
 *
 ****/
DebugOut& Fog::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	dbg << "\t<attr name='Kind'>";
	switch (m_Kind & 3)
	{
		case EXPONENTIAL:	dbg << "exponential'"; break;
		default:				dbg << "linear'"; break;
	}
	if (m_Kind & PIXEL)
		dbg << "|pixel'";
	else
		dbg << "|vertex'";
	endl(dbg << " </attr>");
	endl(dbg << "\t<attr name='Density'>" << m_Density << "</attr>");
	endl(dbg << "\t<attr name='Color'>" << m_Color << "</attr>");
	endl(dbg << "\t<attr name='Start'>" << m_Start << "</attr>");
	endl(dbg << "\t<attr name='End'>" << m_End << "'</attr>");
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Fog override for SharedObj::Do
 *
 ****/
bool Fog::Do(Messenger& s, int op)
{
	float	f;
	Col4	c;
	int32	n;

	switch (op)
	   {
		case FOG_SetDensity:
		s >> f;
		SetDensity(f);
		break;

		case FOG_SetColor:
		s >> c;
		SetColor(c);
		break;

		case FOG_SetStart:
		s >> f;
		SetStart(f);
		break;

		case FOG_SetEnd:
		s >> f;
		SetEnd(f);
		break;

		case FOG_SetKind:
		s >> n;
		SetKind(n);
		break;

		default:
		return SharedObj::Do(s, op);
	   }
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Fog::DoNames[op - FOG_SetDensity]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class Fog override for SharedObj::Save
 *
 ****/
int Fog::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Fog, FOG_SetKind) << h << (int32) GetKind();
	s << OP(VX_Fog, FOG_SetDensity) << h << GetDensity();
	s << OP(VX_Fog, FOG_SetColor) << h << GetColor();
	s << OP(VX_Fog, FOG_SetStart) << h << GetStart();
	s << OP(VX_Fog, FOG_SetEnd) << h << GetEnd();
	return h;
}

} // end Vixen