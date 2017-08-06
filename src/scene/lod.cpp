/****
 *
 * class LevelOfDetail implementation
 *
 * Level Of Detail objects are implemented by overriding the
 * Model::Select function that selects which children of a model
 * should be displayed. We select the child based on the screen
 * space size of its bounding box. 
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(LevelOfDetail, Model, VX_LevelOfDetail);

bool	LevelOfDetail::DoRanges = true;
float	LevelOfDetail::Scale = 1.0f;

static const TCHAR* opnames[] =
{	TEXT("AddRange"), TEXT("SetRanges"), TEXT("SetOptions") };

const TCHAR** LevelOfDetail::DoNames = opnames;

void LevelOfDetail::AddRange(float range)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_LevelOfDetail, LOD_AddRange) << this << range;
	VX_STREAM_END(  )

	if (m_Ranges.IsNull())
		m_Ranges = new FloatArray();
	m_Ranges->Append(range);
}

/*++++
 *
 * @fn void LevelOfDetail::SetOptions(int opts)
 * @param opts	options to set
 *
 * Selects display and range options, flags are one or more of:
 * @code
 *	LOD_AutoRange		automatically compute ranges if no range data supplied
 *	LOD_CamDistance		range values are world space distances from camera
 *	LOD_ScreenSizeRatio	range values are ratio of object's screen size
 *						to entire viewport
 * @endcode
 *
 * @see LevelOfDetail::SetRanges
 */
void LevelOfDetail::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_LevelOfDetail, LOD_SetOptions) << this << int32(opts);
	VX_STREAM_END(  )

	m_Options = opts;
}

/*!
 * @fn void LevelOfDetail::SetRanges(FloatArray* ranges)
 * @param ranges	pointer to range array. If LOD_Automatic is passed,
 *					a new range array is computed automatically 
 *
 * The  ranges for a LevelOfDetail model define distance ranges
 * that control the display of the LODs children. You should have
 * the same number of ranges in this array as there are children
 * of the LevelOfDetail model.
 *
 * During scene graph traversal, the LevelOfDetail Cull
 * function is called to determine which child to display.
 * The scene manager computes the ratio of the screen size of the LevelOfDetail group
 * relative to the diagonal size of the entire viewport
 * and compares this against the ranges. The index of the
 * first range found whose value is greater or equal to the
 * this ratio selects the child to display.
 *
 * The numbers stored in the range array should be in ascending order
 * and the children in the LevelOfDetail group should be in order
 * of decreasing complexity so that simpler models are used when
 * the object is further from the viewer. These are dimensionless
 * quantities that describe size ratios and should be between
 * 0 and 1 (largest size, object fills entire viewport)
 *
 * @see LevelOfDetail::AddRange
 */ 
void LevelOfDetail::SetRanges(FloatArray* ranges)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_LevelOfDetail, LOD_SetRanges) << this << 0L;
		for (int i = 0; i < ranges->GetSize(); ++i)
			*s << OP(VX_LevelOfDetail, LOD_AddRange) << this << ranges->GetAt(i);
	VX_STREAM_END(  )

		m_Ranges = ranges;
}

void LevelOfDetail::ComputeRanges()
{
	int nlevs = GetSize();
	m_Ranges = new FloatArray(nlevs + 1);
	m_Ranges->Append(0);
	for (int i = 0; i < nlevs; ++i)
		m_Ranges->Append(float(i + 1) / nlevs);
}

/****
 *
 * class LevelOfDetail override for Model::Cull
 *
 * Selects which child to display based on embedded
 * ranges and the size of the rendered model in world
 * space relative to the camera viewport size.
 *
 ****/
int LevelOfDetail::Cull(const Matrix* trans, Scene* scene)
{
	if (Model::Cull(trans, scene) == DISPLAY_NONE)
		return DISPLAY_NONE;					// this node culled?
	if (trans == NULL)
		return DISPLAY_ME;
	if (m_Ranges.IsNull() && m_Options & AUTOMATIC)
	{
		((LevelOfDetail*) this)->ComputeRanges();
		if (m_Ranges.IsNull())
			return DISPLAY_ME;
	}
	Sphere	obj_bound;
	Box2		vport = scene->GetViewport();	// screen dimensions
	Camera*	cam = scene->GetCamera();
	float		v;

	GetBound(&obj_bound, NONE);					// bounding sphere in local coords
	obj_bound *= *trans;						// bounding sphere in world coords
/*
 * Compute the radius of the bounding sphere in screen space relative to
 * the largest dimension of the screen viewport. This ratio should be
 * between 0 and 1 and describes the size of the object on the screen.
 */
	if (m_Options & SCREEN_SIZE_RATIO)			// use world distance from camera?
	{
		scene->GetCamera()->GetDeviceCoords(obj_bound);	// bounding sphere in screen coords
		Vec2	vportdim(vport.Width(), vport.Height());
		v = vportdim.Length();			// length of diagonal
	}
/*
 * Compute the distance in world space between the object and the camera.
 * Use this to compare with ranges
 */
	else
		v = obj_bound.Center.Length();
/*
 * Scan all the ranges, scaling them based on the relative size
 * of this object in the world, and select the first model whose
 * scaled range is smaller than the distance from the camera
 */
	if (DoRanges && !m_Ranges.IsNull())
	{
		int nranges = m_Ranges->GetSize();
		for (int i = 0; (i + 1) < nranges; i++)
			if (v * Scale < m_Ranges->GetAt(i + 1))
			{
				Model* child = GetAt(i);
				if (child)
				{
					m_SelectedChild = child;
					return DISPLAY_ALL;
				}
			}
	}
	return DISPLAY_ME;						// display a child
}

void LevelOfDetail::Render(Scene* scene)
{
	if (!m_SelectedChild.IsNull())
		m_SelectedChild->Render(scene);
}

/****
 *
 * class LevelOfDetail override for SharedObj::Copy
 *
 ****/
bool LevelOfDetail::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Model::Copy(src_obj))
		return false;
	const LevelOfDetail* src = (const LevelOfDetail*) src_obj;
	if (src->IsClass(VX_LevelOfDetail))
		m_Ranges = src->m_Ranges;
	return true;
}

/****
 *
 * class LevelOfDetail override for SharedObj::Do
 *
 ****/
bool LevelOfDetail::Do(Messenger& s, int op)
{
	int32	n;
	float	v;
	SharedObj*	obj;
	Vec3	ctr;

	switch (op)
	{
		case LOD_SetOptions:
		s >> n;
		SetOptions(n);
		break;

		case LOD_SetRanges:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Array));
		SetRanges((FloatArray*) obj);
		break;

		case LOD_AddRange:
		s >> v;
		AddRange(v);
		break;
	
		default:
		return Model::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << LevelOfDetail::DoNames[op - LOD_AddRange]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class LevelOfDetail override for SharedObj::Save
 *
 ****/
int LevelOfDetail::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	if (m_Ranges.IsNull())
		return h;
	s << OP(VX_LevelOfDetail, LOD_SetOptions) << h << m_Options;
	for (int i = 0; i < m_Ranges->GetSize(); ++i)
		s << OP(VX_LevelOfDetail, LOD_AddRange) << h << m_Ranges->GetAt(i);
	return h;
}

/****
 *
 * class LevelOfDetail override for SharedObj::Print
 *
 ****/
DebugOut&	LevelOfDetail::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	if ((opts & PRINT_Data) && !m_Ranges.IsNull())
	{
		dbg << "<attr name'Ranges'>";
		for (int i = 0; i < m_Ranges->GetSize(); ++i)
			dbg << m_Ranges->GetAt(i) << " ";
		endl(dbg << "</attr>");
	}
	return Model::Print(dbg, opts & PRINT_Trailer);
}

}	// end Vixen
