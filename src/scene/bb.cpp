#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(BillBoard, SimpleShape, VX_BillBoard);


static const TCHAR* opnames[] =
{	TEXT("SetKind"), TEXT("SetAxis"), TEXT("SetNormal") };

const TCHAR** BillBoard::DoNames = opnames;

/****
 *
 * class BillBoard override for Model::CalcMatrix
 * Calculates the total transformation matrix from
 * local to world coordinates for the billboard. The
 * transformation on input has the parent's world
 * transform. We change it to make the billboard
 * always face the viewer
 *
 ****/
bool BillBoard::CalcMatrix(Matrix* trans, Scene* scene) const
{
	Model::CalcMatrix(trans, scene);
	if (m_Normal.IsEmpty())
	{
		Box3 bbox;
		const Geometry* surf = GetGeometry();
		BillBoard* cheat = (BillBoard*) this;
		if (surf && surf->GetBound(&bbox))
			cheat->m_Normal = ComputeNormal(bbox);
		else
			cheat->m_Normal.Set(0, 0, 1);
	}
	
	if (m_Kind == RADIAL)
		RadialMatrix(trans, scene);
	else AxialMatrix(trans, scene);
	CalcImageIndex();
	return true;
}

void BillBoard::CalcImageIndex() const
{
	const Appearance*	app;
	const Sampler* smp;
	ImageSwitch*	image;

	app = GetAppearance();
	if (app == NULL)
		return;
	smp = app->GetSampler(0);
	if (smp == NULL)
		return;
	image = (ImageSwitch*) (Texture*) smp->GetTexture();
	if (image && image->IsClass(VX_ImageSwitch))
	{
		float a = m_RotateAngle / (2.0f * PI);
		if (a > 0)
			a = 1 - a;
		else
			a = -a;
		int n = (int) (a * image->GetSize());
		image->SetIndex(n);
	}
}

void BillBoard::AxialMatrix(Matrix* trans, Scene* scene) const
{
/*
 * Compute billboard normal and billboard axis in camera coordinates
 */
	Vec3	billNorm;				// billboard normal in camera coordinates
	Vec3	billAxis;				// billboard axis in camera coordinates
	Vec3	LineOfSight(0, 0, -1);	// line of sight in camera coordinates
	float	angle = 0.0f;
	float	dotprod;

	Shape::CalcMatrix(trans, scene);
	trans->TransformVector(m_Normal, billNorm);
	trans->TransformVector(m_Axis, billAxis);
/*	
 * calculate the plane containing the axis and the line of sight (negative Z axis)
 * and also find the plane containing the axis and the normal
 */
	Vec3 losPlane = billAxis.Cross(LineOfSight);
	Vec3 normPlane = billAxis.Cross(billNorm);

/*
 * calculate the angle between the two planes, which is the
 * angle that the normal needs to be rotated by to bring
 * it into the plane of the axis and lineOfSight
 */
	losPlane.Normalize();
	normPlane.Normalize();
	dotprod = losPlane.Dot(normPlane);
	angle = (float) acos(dotprod);
/*
 * check for the correct direction in which the normal needs to
 * be rotated about the axis
 */
	if ((billNorm.Cross(LineOfSight)).Dot(billAxis) < 0.0)
	    angle = PI - angle;
	else
	    angle -= PI;
	((BillBoard*) this)->m_RotateAngle = angle;
/*
 * Apply the billboard axis rotation BEFORE all other model transformations. 
 */
	Matrix mtx;
	mtx.RotationMatrix(m_Axis, angle);
	trans->PostMul(mtx);
}

void BillBoard::RadialMatrix(Matrix* trans, Scene* scene) const
{
	Vec3	LineOfSight(Vec3(0, 0, -1));  // line of sight in camera coordinates
	Vec3	billNorm;
	float	angle = 0;

	Shape::CalcMatrix(trans, scene);
	trans->TransformVector(m_Normal, billNorm);
	billNorm.Normalize();
	if (billNorm == LineOfSight)
		return;
/*
 * Compute the normal to the plane that contains the normal and line of sight
 * (Z axis). Then calculate the rotation angle about this axis that makes
 * them coincide.
 */
	Vec3	axis = billNorm.Cross(LineOfSight);
	Vec3	p;
	Matrix mtx;
	float	dotprod;

	axis.Normalize();
 	dotprod = billNorm.Dot(LineOfSight);
 	angle = (float) acos(dotprod);
	angle -= PI;
	((BillBoard*) this)->m_RotateAngle = angle;
	
	mtx.RotationMatrix(axis, angle);
	trans->PostMul(mtx);
}

/****
 *
 * Creates geometry for a BillBoard if there is none.
 * The geometry is two coplanar triangles that make
 * a 1x1 square on which to map a texture
 *
 ****/
Geometry* BillBoard::MakeImageGeometry()
{
	SetNormal(Vec3(0, 0, 1));
	return SimpleShape::MakeImageGeometry();
}

TextGeometry* BillBoard::MakeTextGeometry()
{
	TextGeometry* text = SimpleShape::MakeTextGeometry();
	if (m_Normal.IsEmpty())
		m_Normal.Set(0, 0, 1);
	if (text)
		text->SetNormal(m_Normal);
	return text;
}

/*!
 * @fn void BillBoard::SetKind(int kind)
 * @param kind kind of billboard rotation
 *
 * The billboard kind determines rotation behavior and image selection.
 * A  multi-textured billboard searches its geometry for an appearance which has
 * a ImageSwitch as the image. The switch index of that object will be
 * varied based on the angle between the billboard and the viewer.
 *	- BB_Axial	Rotates about a specified axis to face the viewer
 *	- BB_Radial	Rotates so the face normal points to viewer
 *	- BB_MultiTex Rotation angle selects one of several textures
 *				to display different views of an object.
 *
 * @see Model::CalcMatrix  BillBoard::SetAxis BillBoard::SetImageIndex
 */
void BillBoard::SetKind(int kind)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_BillBoard, BB_SetKind) << this << int32(kind);
	VX_STREAM_END(  )

	m_Kind = kind;
}

/*!
 * @fn void BillBoard::SetAxis(const Vec3& axis)
 * @param axis	unit vector giving axis of rotation
 *
 * The billboard rotation axis is in the coordinate system
 * of the billboard geometry. It is ignored by radial billboards.
 *
 * @see BillBoard::SetKind BillBoard::SetNormal Model::Rotate
 */
void BillBoard::SetAxis(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_BillBoard, BB_SetAxis) << this << v;
	VX_STREAM_END(  )

	m_Axis = v;
}

/*!
 * @fn void BillBoard::SetNormal(const Vec3& nml)
 * @param nml	unit vector giving axis of rotation
 *
 * The face normal is in the coordinate system
 * of the billboard geometry. It defaults to the
 * vector (0, 0, 1) which points directly at the viewer.
 *
 * @see BillBoard::SetKind BillBoard::SetAxis
 */
void BillBoard::SetNormal(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_BillBoard, BB_SetNormal) << this << v;
	VX_STREAM_END(  )

	m_Normal = v;
}

/*!
 * @fn Vec3 BillBoard::ComputeNormal(const Box3& bbox)
 * @param bbox bounding box to compute normal from
 *
 * The input bounding box is examined to see which is the
 * smallest dimension. The "face" of the billboard is composed
 * of the other 2 larger dimensions. The face normal is computed
 * from the cross product of the two largest dimensions.
 * It will be a unit vector.
 *
 * @return face normal for input bounds
 */
Vec3 BillBoard::ComputeNormal(const Box3& bbox)
{	
	Vec3	v1, v2;
	float	l, med_x, med_y, med_z;
	float	w = bbox.Width();
	float	h = bbox.Height();
	float	d = bbox.Depth();
	
	w = (float) fabs(w); h = (float) fabs(h); d = (float) fabs(d);
	med_x = 0.5f*(bbox.min.x + bbox.max.x);
	med_y = 0.5f*(bbox.min.y + bbox.max.y);
	med_z = 0.5f*(bbox.min.z + bbox.max.z);
	v1.Set(med_x, med_y, med_z);
	if ((d < w) && (d < h))						// depth dimension smallest
	   {
		l = (w + h) / 2;
		v2.Set(med_x, med_y, med_z + l);
	   }
	else if ((w < d) && (w < h))				// width dimension smallest
	   {
		l = (h + d) / 2;
		v2.Set(med_x + l, med_y, med_z);
	   }
	else										// height dimension smallest
	   {
		l = (w + d) / 2;
		v2.Set(med_x, med_y + l, med_z);
	   }
	v2 -= v1;
	v2.Normalize();
	return v2;
}
	
bool BillBoard::Copy(const SharedObj* src_obj)
{
	const BillBoard* src = (const BillBoard*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!SimpleShape::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_BillBoard))
	   {
		m_Kind = src->m_Kind;
		m_Axis = src->m_Axis;
		m_Normal = src->m_Normal;
	   }
	return true;
}

/****
 *
 * class BillBoard override for SharedObj::Do
 *		BB_SetKind			<int32>
 *		BB_SetImageIndex	<int32>
 *		BB_SetAxis			<Vec3>
 *		BB_SetNormal		<Vec3>
 *
 ****/
bool BillBoard::Do(Messenger& s, int op)
{
	int32		v;
	Vec3	axis;

	switch (op)
	   {
		case BB_SetAxis:
		s >> axis;
		SetAxis(axis);
		break;

		case BB_SetNormal:
		s >> axis;
		SetNormal(axis);
		break;

		case BB_SetKind:
		s >> v;
		SetKind(v);
		break;

		default:
		return SimpleShape::Do(s, op);
	   }

#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << BillBoard::DoNames[op - BB_SetKind]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class BillBoard override for SharedObj::Save
 *
 ****/
int BillBoard::Save(Messenger& s, int opts) const
{
	int32 h = SimpleShape::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_BillBoard, BB_SetNormal) << h << GetNormal();
	s << OP(VX_BillBoard, BB_SetAxis) << h << GetAxis();
	s << OP(VX_BillBoard, BB_SetKind) << h << (int32) GetKind();
	return h;
}

DebugOut& BillBoard::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SimpleShape::Print(dbg, opts);
	SimpleShape::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Kind'>" << GetKind() << "</attr>");
	endl(dbg << "\t<attr name='Axis'>" << GetAxis() << "</attr>");
	return SimpleShape::Print(dbg, opts & PRINT_Trailer);
}

}	// end Vixen


