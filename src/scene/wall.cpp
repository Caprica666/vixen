#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Wall, Model, VX_Wall);

static const TCHAR* opnames[] = { TEXT("SetModelPlane")  };

const TCHAR** Wall::DoNames = opnames;

/****
 *
 * Debug = 0 disables all debug printout
 * Debug = 1 prints messages about wall containment
 * Debug = 2 creates and displays geometry for walls so you can see them in the scene
 *
 ****/

/*!
 * @fn void Wall::SetModelPlane (const Plane& plane)
 *
 * Establish the model space plane defining the wall.
 * A wall’s plane defines the plane containing all of the geometry for that wall,
 * as well as the half-space that is the inside side of the wall.
 * The plane should have its positive side face into the room.
 * Applications using these functions to modify a wall’s model-space plane
 * must ensue that Update is called upon the wall or an ancestor of the wall
 * before the world data is used (e.g. for rendering, querying point-in-room, etc).
 *
 * @see Wall::GetModelPlane
 */
void Wall::SetModelPlane (const Plane& plane)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Wall, WALL_SetModelPlane) << this << plane;
	VX_STREAM_END( )

    m_modelPlane = plane;
    m_worldPlane = plane;
}


bool Wall::CalcBound(Box3* box) const
{
	if (GetBound(box, NONE))
		return true;
	return false;
}

bool Wall::CalcSphere(Sphere* bsp) const
{
	Box3 box;

	if (GetBound(&box, NONE))
	{
		Sphere sp(box);
		*bsp = sp;
		return true;
	}
	bsp->Empty();
	return false;
}

int	Wall::Cull(const Matrix* trans, Scene* scene)
{
#ifdef _DEBUG
	if (Wall::Debug > 1)
		return DISPLAY_ALL;
#endif
	return DISPLAY_NONE;
}

void Wall::RemoveMatrix()
{
	const Matrix* wallmtx = GetTransform();
	Plane		p;
	Box3		b;

	if (wallmtx->IsIdentity())
		return;
	VX_TRACE(Wall::Debug, ("Wall::RemoveMatrix %s)", GetName()));
	if (GetBound(&b, NONE))
	{
		b *= *wallmtx;
		b.Normalize();
		SetBound(&b);
	}
	Matrix inverse;

	inverse.Invert(*wallmtx);
	inverse.Transpose();
	p.Transform(inverse, m_modelPlane);
	SetModelPlane(p);
	SetTransform((Matrix*) NULL);
}

/*!
 * @fn void Wall::UpdateWorldPlane(const Matrix* mv)
 * @param mv	model/view matrix from root
 *
 * Calculates the world space plane from the model space plane
 * by applying the model/view matrix.
 * Wall is assumed not to be transformed with respect to its parent,
 * so only the parent's transform is applied to get the world plane
 * from the model plane.
 *
 * The model plane as a homogeneous vector is Pm.  The world plane is Pw.
 * If M is the homogeneous world transform, then the two planes are
 * related by Pw = M^{-t}*Pm where M^{-t} is the inverse transpose of M.
 */
void Wall::UpdateWorldPlane (const Matrix* mv)
{
	Matrix inverse;

	inverse.Invert(*mv);
	inverse.Transpose();
	m_worldPlane = m_modelPlane;
	m_worldPlane.Transform(inverse, m_modelPlane);
#ifdef _DEBUG
	if (Wall::Debug && !IsParent())
		MakeRenderGeometry();
#endif
}

bool Wall::ComputeCorrectNormal (const Vec3& point)
{
	float	dist =  m_modelPlane.Distance(point);

	if (Wall::Debug > 2)
		Print();
    if (dist < 0.0f )
    {
		VX_TRACE(Wall::Debug > 1, ("Wall::ComputeNormal[%s] reversing", GetName()));
        m_modelPlane.x = -m_modelPlane.x;
        m_modelPlane.y = -m_modelPlane.y;
        m_modelPlane.z = -m_modelPlane.z;
        m_modelPlane.w = -m_modelPlane.w;
        return true;
    }

    return false;
}

/****
 *
 * class Wall override for SharedObj::Do
 *
 ****/
bool Wall::Do(Messenger& s, int op)
{
	Vec4	p;

	switch (op)
	{
		case WALL_SetModelPlane:
		s >> p;
		SetModelPlane(Plane(p.x, p.y, p.z, p.w));
		break;

		default:
		return Model::Do(s, op);
	}
	return true;
}

/****
 *
 * class Wall override for SharedObj::Save
 *
 ****/
int Wall::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Wall, WALL_SetModelPlane) << h << m_modelPlane;
	return h;
}

/****
 *
 * class Wall override for SharedObj::Print
 *
 ****/
DebugOut&	Wall::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='ModelPlane'>" << (Vec4&) m_modelPlane << "</attr>");
	Model::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Wall override for SharedObj::Copy
 *
 ****/
bool Wall::Copy(const SharedObj* src_obj)
{
	const Wall*	src = (const Wall*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Model::Copy(src_obj))
	  return false;
	if (src_obj->IsClass(VX_Wall))
	{
		m_modelPlane = src->m_modelPlane;
		m_worldPlane = src->m_worldPlane;
	}
	return true;
}

void Wall::MakeRenderGeometry()
{
	Shape*	shape = (Shape*) (Model*) First();
	TriMesh*	mesh;
	Box3		b;

	if (shape || !GetBound(&b, NONE))
		return;
	shape = new Shape;
	Append(shape);
	mesh = new TriMesh(VertexPool::LOCATIONS);
	shape->SetGeometry(mesh);

	Vec3		size(b.Width(), b.Height(), b.Depth());
	Matrix	mtx;

	mtx.TranslationMatrix(b.Center());
	GeoUtil::Block(mesh, size);
	*mesh *= mtx;
}

}	// end Vixen