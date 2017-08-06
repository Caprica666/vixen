#include "vixen.h"

namespace Vixen {

static const TCHAR* opnames[] =
{	TEXT("SetType"), TEXT("SetFOV"), TEXT("SetHither"), TEXT("SetYon"),
	TEXT("SetAspect"), TEXT("SetViewVol"), TEXT("SetEyeSep"), TEXT("SetFocal")
};

const TCHAR** Camera::DoNames = opnames;

VX_IMPLEMENT_CLASSID(Camera, Model, VX_Camera);

/*!
 * @fn Camera::Camera()
 *
 * Initializes the camera to its default state.
 * The default view volume is a unit cube with hither at 2 and yon at 100.
 * The camera is positioned at the origin and looks down the -Z axis.
 *
 * @see Model
 */
Camera::Camera() : Model()
{
	m_Type = PERSPECTIVE;
	m_CullChanged = false;
	m_CullVol.Empty();
	m_Aspect = 1.0f;
	SetViewVol(Box3(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 100.f));
	m_NoBounds = true;
	m_EyeSep = 0;
	m_Focal = 0;
	ClearFlags(MOD_BVinvalid);
	m_NoCull = true;
}

Camera::Camera(const Camera& src) : Model()
{
	ClearFlags(MOD_BVinvalid);
	m_NoBounds = true;
	m_NoCull = true;
	Copy(&src);
}

void Camera::NotifyParents(uint32 flags)
{
	if (flags & MOD_BVinvalid)	// don't propagate bounding volume changes
		return;
	Model::NotifyParents(flags);
}

bool Camera::GetCenter(Vec3* p, int flags) const
{
	Sphere	sp;
	Matrix	t;

    VX_ASSERT(p);
	p->Set(0,0,0);
	switch (flags)
	{
		case LOCAL:
		*p = GetTranslation();
		break;

		case WORLD:
	    TotalTransform(&t);
		p->x = t.Get(0, 3);
		p->y = t.Get(1, 3);
		p->z = t.Get(2, 3);
		break;

		default:
		p->Set(0,0,0);
	}
	return true;
}

/****
 *
 * class Camera override for SharedObj::Print
 *
 ****/
DebugOut& Camera::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	if (m_Focal > 0)
		endl(dbg << "\t<attr name='Focal'>" << m_Focal << "</attr>");
	if (m_EyeSep > 0)
		endl(dbg << "\t<attr name='EyeSep'>" << m_EyeSep << "</attr>");
	endl(dbg << "\t<attr name='ViewVolume'>" << m_ViewVol << "</attr>");
	if (m_CullChanged)
		endl(dbg << "\t<attr name='CullVolume'>" << m_CullVol << "</attr>");
	return Model::Print(dbg, opts & PRINT_Trailer);
}

/****
 *
 * class Camera override for SharedObj::Copy
 * Copy the contents of one camera into another. If the source class
 * is not a camera, the camera attributes are not copied.
 *
 ****/
bool Camera::Copy(const SharedObj* src_obj)
{
	Camera* src = (Camera*) src_obj;
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Model::Copy(src))
		return false;
	if (!src->IsClass(VX_Camera))
		return true;
	m_Type =  src->m_Type;
	m_ViewVol =  src->m_ViewVol;
	m_CullVol =  src->m_CullVol;
	m_FOV = src->m_FOV;
	m_Aspect = src->m_Aspect;
	m_EyeSep = src->m_EyeSep;
	m_Focal = src->m_Focal;
	m_CullChanged = src->m_CullChanged;
	SetChanged(true);
	return true;
}

/*!
 * @fn void  Camera::SetType(int type)
 * @param type	which type of camera projection to use
 *
 * Two kinds of camera projection are supported:
 * @code
 *	Camera::PERSPECTIVE		perspective projection
 *	Camera::ORTHOGRAPHIC	orthographic projection
 * @endcode
 *
 * @see Camera::SetHither Camera::SetAspect
 */
void 	Camera::SetType(int v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetType) << this << int32(v);
	VX_STREAM_END( )

	m_Type = v;
	SetChanged(true);
}

/*!
 *
 * @fn void	Camera::SetFOV(float fov)
 * @param fov field of view angle in radians
 *
 * The field of view angle defines the width of the camera's view.
 * FOV, Hither and Aspect together define the view volume.
 *
 * @see Camera::SetHither Camera::SetAspect Camera::SetViewVol
 */
void	Camera::SetFOV(float fov)
{
	// range check
	VX_ASSERT((fov > 0.0) && (fov <= PI));

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetFOV) << this << fov;
	VX_STREAM_END( )

	if (m_Type & ORTHOGRAPHIC)
	{
		float	mid  = (m_ViewVol.min.z + m_ViewVol.max.z);
		float	aspect = m_ViewVol.Width() / m_ViewVol.Height();
		float	w = 2.0f * mid * tanf(fov / 2);
		float	h = w / aspect;
		SetViewVol(Box3(-w / 2, -h / 2, m_ViewVol.min.z, w / 2, h / 2, m_ViewVol.max.z));
	}
	else	// recalculate perspective view volume
	{
#if 0
		float	hither  = m_ViewVol.min.z;
		float	aspect = m_ViewVol.Width() / m_ViewVol.Height();
		float	w = 2.0f * hither * tanf(fov / 2.0f);
		float	h = w / aspect;
		SetViewVol(Box3(-w / 2.0f, -h / 2.0f, hither, w / 2.0f, h / 2.0f, m_ViewVol.max.z));
#else
		m_ViewVol.min.z = m_ViewVol.Width() / (2.0f * tanf(fov / 2.0f));
		m_Aspect = m_ViewVol.Width() / m_ViewVol.Height();
		m_FOV = fov;
#endif
	}
//	m_FOV should equal fov here
}

/*!
 * @fn void	Camera::SetViewVol(const Box3& b)
 * @param b	box defining view volume
 *
 * During scene graph traversal, the system maintains bounding spheres for all models
 * in the hierarchy and uses them to discard those objects outside the view volume of the camera.
 * Changing the view volume redefines Hither, Yon, Aspect and FOV.
 *
 * The width and height of the input box define the width and height of the
 * camera viewport. The minimum Z value defines the distance of the hither
 * clipping plane and the maximum Z value defines the yon clipping plane.
 *
 * @see Camera::SetHither Camera::SetAspect Camera::SetFOV Camera::SetHeight Camera::IsVisible
 */
void	Camera::SetViewVol(const Box3& b)
{
	VX_ASSERT(b.Width() != 0);
	VX_ASSERT(b.Height() != 0);

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetViewVol) << this << b.min << b.max;
	VX_STREAM_END(  )

	if (m_CullChanged)
	{
		Vec2 scl(	m_CullVol.Width() / m_ViewVol.Width(),
					m_CullVol.Height() / m_ViewVol.Height());
		Vec2 ofs((m_CullVol.min.x - m_ViewVol.min.x) / m_ViewVol.Width(),
				   (m_CullVol.min.y - m_ViewVol.min.y) / m_ViewVol.Height());
		Box3 tmp;
		tmp.min.x = b.min.x + b.Width() * ofs.x;
		tmp.min.y = b.min.y + b.Height() * ofs.y;
		tmp.max.x = tmp.min.x + b.Width() * scl.x;
		tmp.max.y = tmp.min.y + b.Height() * scl.y;
		tmp.min.z = b.min.z;
		tmp.max.z = b.max.z;
		m_CullVol = tmp;
	}
	else
		m_CullVol = b;
	m_ViewVol = b;
	// this is the only case in which the FOV can change implicitly

	if (m_Type & ORTHOGRAPHIC)
		m_FOV = 2.0f * atanf(m_ViewVol.Width() / (m_ViewVol.min.z + m_ViewVol.max.z));
	else
		m_FOV = 2.0f * atanf(m_ViewVol.Width() / (2.0f * m_ViewVol.min.z));
//		m_FOV = 2.0f * atanf(m_ViewVol.Height() / (2.0f * m_ViewVol.min.z));
		m_Aspect = m_ViewVol.Width() / m_ViewVol.Height();
	// we have changed the view volume
	SetChanged(true);
}


/*!
 * @fn void	Camera::SetCullVol(const Box3& b)
 * @param b box defining cull volume
 *
 * This defines the volume outside of which objects are culled during scene traversal.
 * If not specified, the cull volume is the view volume. When
 * the view volume is changed, the cull volume is proportionately
 * changed to match. The cull volume is useful if you wish to further restrict
 * viewport culling without changing the camera viewing characteristics.
 *
 * @see Camera::IsVisible Camera::PushClipPlane Camera::SetViewVol
 */
void	Camera::SetCullVol(const Box3& b)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetCullVol) << this << b.min << b.max;
	VX_STREAM_END(  )

	if (b.IsEmpty())
	{
		m_CullChanged = false;
		m_CullVol = m_ViewVol;
	}
	else
	{
		m_CullChanged = true;
		m_CullVol = b;
	}
}

/*!
 * @fn void	Camera::SetHither(float hither)
 * @param hither distance of near clipping plane from viewer
 *
 * The hither plane causes objects behind the viewer to be clipped.
 * The hither plane distance is the negative of the
 * minimum Z value of the view volume. Changing the hither distance
 * also affects the width and height of the view volume.
 *
 * @see Camera::SetYon Camera::SetAspect Camera::SetViewVol Camera::SetFOV
 */
void	Camera::SetHither(float hither)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetHither) << this << hither;
	VX_STREAM_END(  )

	m_CullVol.min.z = hither;
	// recalculate new view volume
	if (m_Type & ORTHOGRAPHIC)
	{
		float	aspect = m_ViewVol.Width() / m_ViewVol.Height();
		float	w = (hither + m_ViewVol.max.z) * tanf(m_FOV / 2);
		float	h = w / aspect;
		SetViewVol(Box3(-w / 2, -h / 2, hither, w / 2, h / 2, m_ViewVol.max.z));
	}
	else
	{
		float	aspect = m_ViewVol.Width() / m_ViewVol.Height();
		float	w = 2.0f * hither * tanf(m_FOV / 2);
		float	h = w / aspect;
		SetViewVol(Box3(-w / 2, -h / 2, hither, w / 2, h / 2, m_ViewVol.max.z));
	}
}

/*!
 * @fn void	Camera::SetYon(float yon)
 * @param yon distance of far clipping plane from viewer
 *
 * The yon plane causes objects far from the viewer to be clipped.
 * The yon plane distance is the negative of the
 * maximum Z value of the view volume. Changing the yon distance
 * also affects the width and height of the view volume.
 *
 *@see Camera::SetHither Camera::SetAspect Camera::SetViewVol Camera::SetFOV
 */
void	Camera::SetYon(float yon)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetYon) << this << yon;
	VX_STREAM_END(  )

	m_CullVol.max.z = yon;
	if (m_Type & ORTHOGRAPHIC)
	{
		float	aspect = m_ViewVol.Width() / m_ViewVol.Height();
		float	w = (yon + m_ViewVol.min.z) * tanf(m_FOV / 2);
		float	h = w / aspect;
		SetViewVol(Box3(-w / 2, -h / 2, m_ViewVol.min.z, w / 2, h / 2, yon));
	}
	else
		m_ViewVol.max.z = yon;
	SetChanged(true);
}

void	Camera::SetHeight(float height)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetHeight) << this << height;
	VX_STREAM_END(  )

	float	h = (float) fabs(height);
	float	w = h * m_ViewVol.Width() / m_ViewVol.Height();
	SetViewVol(Box3(-w / 2, -h / 2, m_ViewVol.min.z, w / 2, h / 2, m_ViewVol.max.z));
}


void	Camera::SetAspect(float v)
{
	// no need to do anything if the aspect hasn't changed
	if (v == m_Aspect)
		return;

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetAspect) << this << v;
	VX_STREAM_END(  )

	float	h = (float) fabs(m_ViewVol.Height());
	if ((fabs(m_ViewVol.Width()) / h) == v)
		return;
	float	w = h * v;
	Vec3	ctr = m_ViewVol.Center();
	SetViewVol(Box3(ctr.x - w / 2, ctr.y - h / 2, m_ViewVol.min.z,
					ctr.x + w / 2, ctr.y + h / 2, m_ViewVol.max.z));

	// store value for future reference
	m_Aspect = v;
}

/*!
 * @fn void Camera::SetEyeSep(float eyesep)
 * @param eyesep	Distance between the eyes for stereo viewing.
 *
 * If a non-zero value is supplied it is the distance between the
 * user's eyes in world coordinates. The underlying renderer implements
 * stereo in a device specific way. If zero is supplied, stereo
 * viewing is disabled.
 *
 * @see Camera Camera::SetAspect Scene::SetViewport
 */
void Camera::SetEyeSep(float eyesep)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Camera, CAM_SetEyeSep) << this << eyesep;
	VX_STREAM_END( )

	m_EyeSep = eyesep;
	SetChanged(true);
}

/*!
 *
 * @fn void	Camera::SetFocal(float focal)
 * @param focal	focal length in world units.
 *
 * The focal length determines the distance of the viewer from the projection plane.
 * It is used for stereo perspective projection.
  *
 * @see Camera::SetEyeSep
 */
void	Camera::SetFocal(float focal)
{
	m_Focal = focal;
	SetChanged(true);
}

bool Camera::Do(Messenger& s, int op)
{
	int32	n;
	float	f;
	Box3	b;
	Opcode	o = Opcode(op);

	switch (op)
	{
		case CAM_SetType:
		s >> n;
		SetType(n);
		break;

		case CAM_SetFOV:
		s >> f;
		SetFOV(f);
		break;

		case CAM_SetFocal:
		s >> f;
		SetFocal(f);
		break;

		case CAM_SetHither:
		s >> f;
		SetHither(f);
		break;

		case CAM_SetYon:
		s >> f;
		SetYon(f);
		break;

		case CAM_SetViewVol:
		s >> b.min >> b.max;
		SetViewVol(b);
		break;

		case CAM_SetCullVol:
		s >> b.min >> b.max;
		SetCullVol(b);
		break;

		case CAM_SetAspect:
		s >> f;
		SetAspect(f);
		break;

		case CAM_SetEyeSep:
		s >> f;
		SetEyeSep(f);
		break;

		default:
		return Model::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Camera::DoNames[op - CAM_SetType]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Camera override for SharedObj::Save
 *
 ****/
int Camera::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	Box3	vv(GetViewVol());
	s << OP(VX_Camera, CAM_SetType) << h << (int32) GetType();
	if (m_EyeSep > 0)
		s << OP(VX_Camera, CAM_SetEyeSep) << h << GetEyeSep();
	if (m_Focal > 0)
		s << OP(VX_Camera, CAM_SetFocal) << h << GetFocal();
	if (!vv.IsEmpty())
		s << OP(VX_Camera, CAM_SetViewVol) << h << vv.min << vv.max;
	if (m_CullChanged)
	{
		vv = GetCullVol();
		s << OP(VX_Camera, CAM_SetCullVol) << h << vv.min << vv.max;
	}
	return h;
}

/*!
 * @fn const Matrix* Camera::GetViewTrans() const
 *
 * A camera can be positioned and oriented anywhere in the 3D scene.
 * The display device supports a camera positioned at the origin looking down the Z axis.
 * The view matrix transforms the scene objects into this camera space.
 * The matrix is computed by taking the inverse of the total transformation matrix of the camera.
 *
 * This routine is used internally when the scene manager concurrently runs
 * separate traversal and rendering threads. The camera view matrix may be
 * different across these threads.
 *
 * @return pointer to camera view matrix
 */
const Matrix* Camera::GetViewTrans() const
{
	return &m_ViewTrans;
}

Vec3 Camera::GetDirection() const
{
	Vec3	v;
	Matrix	tmp;

	TotalTransform(&tmp);
	tmp.TransformVector(Vec3(0, 0, -1), v);
	return v;
}


/*!
 * @fn const float* Camera::ProjPerspective(const Box3& vv, Matrix& mtx)
 * @param vv	view volume to use
 * @param mtx	matrix to initialize
 *
 * Computes a mono perspective projection matrix from a view volume.
 * This is the same matrix the glFrustum calls uses (so it matches
 * the OpenGL perspective matrix). It describes a right-handed
 * coordinate system with positive X to the right, positive Y up
 * and positive Z pointed at the viewer (the camera looks down the
 * negative Z axis).
 *
 * @see Camera::ProjOrtho Camera::GetProjection Camera::SetProjection Camera::SetEyeSep
 */
void Camera::ProjPerspective(const Box3& vv, Matrix& mtx)
{
	float	m[4][4];
	float	hither = vv.min.z;
	float	yon = vv.max.z;
	float	width = vv.max.x - vv.min.x;
	float	height = vv.max.y - vv.min.y;
	float	depth = hither - yon;

	m[0][0] = 2.0f * hither / width;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 2.0f * hither / height;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = (vv.min.x + vv.max.x) / width;
	m[2][1] = (vv.min.y + vv.max.y) / height;
	m[2][2] = yon / depth;						// DirectX perspectivee
//	m[2][2] = (yon + hither) / depth;			// OpenGL perspective
	m[2][3] = -1.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = (hither * yon) / depth;			// DirectX perspective
//	m[3][2] = (2.0f * hither * yon) / depth;	// OpenGL perspective
	m[3][3] = 0.0f;
	mtx.SetMatrix((const float*) &m[0][0]);
}

/*!
 * @fn void GetStereoView(int which_eye, Box3& viewvol) const
 * @param which_eye	Camera::LEFT_PERSP for left eye stereo,
 *					Camera::RIGHT_PERSP for right eye
 * @param viewvol	Box to get the view volume for the specified eye.
 *
 * Computes a stereo perspective view volume for a right-handed
 * coordinate system with positive X to the right, positive Y up
 * and positive Z pointed at the viewer (the camera looks down the
 * negative Z axis).
 *
 * The camera stereo eye separation must not be zero or you will get the
 * same matrix for both eyes. You can set it with Camera::SetEyeSep().
 * You can also set the focal distance of the camera with Camera::SetFocal().
 * If not set, the focal distance defaults to 10 times the eye separation
 * (about arm's length for a human).
 *
 * @see Camera::ProjOrtho Camera::GetProjection Camera::SetProjection Camera::SetEyeSep
 */
void Camera::GetStereoView(int which_eye, Box3& viewvol) const
{
	float	focal = m_Focal;
	float	offset;

	viewvol = m_ViewVol;
	if (focal <= 0)
		focal = m_EyeSep * 10.0f;
	offset = (m_EyeSep * m_ViewVol.min.x) / focal;
	if (which_eye == RIGHT_PERSP)
	{
		viewvol.min.x -= offset;
		viewvol.max.x -= offset;
	}
	else
	{
		viewvol.min.x += offset;
		viewvol.max.x += offset;
	}
}


/*!
 * @fn const float* Camera::ProjOrtho(const Box3& vv, Matrix& mtx)
 * @param vv view volume
 *
 * This is the same matrix the glOrtho calls uses (so it matches
 * the OpenGL orthographic matrix).
 *
 * @see Camera::ProjPerspective Camera::GetProjection Camera::SetProjection
 */
void Camera::ProjOrtho(const Box3& vv, Matrix& mtx)
{
	float m[4][4];
	float w = vv.Width();
	float h = vv.Height();
	float znear = vv.min.z;
	float zfar = vv.max.z;

	m[0][0] = 2.0f / w;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 2.0f / h;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
#if defined(VIXEN_DX11) || defined(VIXEN_DX9)
	m[2][2] = 1.0f  / (znear - zfar);
#else
	m[2][2] = 2.0f  / (znear - zfar);
#endif
	m[2][3] = 0.0f;

	m[3][0] = -(vv.min.x + vv.max.x) / w;
	m[3][1] = -(vv.min.y + vv.max.y) / h;
#if defined(VIXEN_DX11) || defined(VIXEN_DX9)
	m[3][2] = znear / (znear - zfar);
#else
	m[3][2] = (znear + zfar) / (znear - zfar);
#endif
	m[3][3] = 1.0f;
	mtx.SetMatrix(&m[0][0]);
}


/*!
 * @fn int Camera::IsVisible(const Sphere& sphere) const
 * @param sphere	Sphere to check for visibility.
 *
 * Hit tests a sphere against the camera cull planes.
 * The input sphere must be in world coordinates.
 * Normally, this routine is called internally during
 * scene graph traversal and the bounding spheres are
 * transformed to world coordinates by the scene manager.
 * If extra culling planes have been provided, they are culled
 * against also.
 *
 * @return The return code indicates visiblity status:
 * @code
 *		0 = sphere not within view volume
 *		CAM_Visible = sphere within view volume
 *		CAM_AllVisible = sphere completely within view volume
 * @endcode
 *
 * @see Camera::SetViewVol Camera::PushClipPlane Sphere::Hit
 */
int Camera::IsVisible(const Sphere& insphere) const
{
	uint32		vis(0);
	const Box3&	vv(m_ViewVol);
	Sphere		sphere(insphere);
		

	// test for invalid bounding sphere
    if (sphere.Radius <= 0.0f)
        return VISIBLE;
	// clip against orthographic view volume
	if (m_Type == ORTHOGRAPHIC)
	{
		sphere *= m_ViewTrans;
		if (((sphere.Center.z + sphere.Radius) < -vv.max.z) ||
			((sphere.Center.z - sphere.Radius) > -vv.min.z))
			return 0;				// behind or beyond us
		if (((sphere.Center.x + sphere.Radius) < vv.min.x) ||
			((sphere.Center.x - sphere.Radius) > vv.max.x))
			return 0;			// off the edge in X 
		if (((sphere.Center.y + sphere.Radius) < vv.min.y) ||
			((sphere.Center.y - sphere.Radius) > vv.max.y))
			return 0;			// off the edge in Y 
		return VISIBLE; 
	}

	// cull against standard frustum
	int32 i;
	for (i = 0; i < 4; i++)
	{
		float dist = sphere.Hit(m_WorldPlanes[i]);
		if (dist > 0)			// outside
			return 0;
		if (dist < 0)			// completely inside
			vis = (vis << 1) | 1;
	}
	// Cull against extra planes.  Most recently pushed planes are processed
	// first as they are likely to be more restrictive.
	int last = (int) m_ExtraPlanes.GetSize() - 1;
	for (i = last; i >= 0; i--)
		if (sphere.Hit(m_ExtraPlanes.GetAt(i)) > 0)
			return 0;

	// Notify renderer that object is entirely within standard frustum, so
	// no clipping is necessary.
	if (vis == 0x3F)
		return ALL_VISIBLE;
	return VISIBLE;
}

/*!
 * @fn int Camera::IsVisible(const Box3& box) const
 * @param box	Axially aligned box to check for visibility.
 *
 * Hit tests a box against the camera cull planes.
 * The input box must be in world coordinates.
 * This differs from sphere hit testing which is in camera coordinates.
 * This routine is called internally during traversal
 * of octree hierarchies which maintain world-space bounding volumes.
 * If extra culling planes have been provided, they are culled
 * against also.
 *
 * @return The return code indicates visiblity status:
 * @code
 *		0 = box not within view volume
 *		VISIBLE = box within view volume
 *		ALL_VISIBLE = box completely within view volume
 * @endcode
 *
 * @see Camera::SetViewVol Camera::PushClipPlane Box3::Hit
 */
int Camera::IsVisible(const Box3& inbox) const
{
	uint32			vis(0);
	int				i;
	const Box3&	vv(m_ViewVol);
	Box3			box(inbox);
	
	box *= m_ViewTrans;
	// clip against orthographic view volume
	if (m_Type == ORTHOGRAPHIC)
	{
		if ((box.max.z < -vv.min.z) ||
			(box.min.z > -vv.max.z))
			return 0;				// behind or beyond us
		if ((box.max.x < vv.min.x) ||
			(box.min.x > vv.max.x))
			return 0;				// off the edge in X 
		if ((box.max.y < vv.min.y) ||
			(box.min.z > vv.max.y))
			return 0;				// off the edge in Y 
		return VISIBLE; 
	}

	// cull against standard frustum
	for (i = 0; i < 6; i++)
	{
		float dist = box.Hit(m_WorldPlanes[i]);
		if (dist > 0)
			return 0;
		if (dist < 0)
			vis = (vis << 1) | 1;
	}
	// Cull against extra planes.  Most recently pushed planes are processed
	// first as they are likely to be more restrictive.
	// extra planes must be transformed to world space
	int last = (int) m_ExtraPlanes.GetSize()-1;
	for (i = last; i >= 0; i--)
	{
		Plane plane;

		WorldPlane(plane, m_ExtraPlanes.GetAt(i));
		if (box.Hit(plane) > 0)
			return 0;
	}
	// Notify renderer that object is entirely within standard frustum, so
	// no clipping is necessary.
	if (vis == 0x3F)
		return ALL_VISIBLE;
	return VISIBLE;
}

/***
 *
 * Camera_CalcViewPlanes
 * Calculates the view plane normals for view volume clipping.
 *
 ***/
void Camera::CalcViewPlanes()
{
	CalcViewPlanes(m_CullVol, m_CullPlanes);
}

void Camera::CalcViewPlanes(const Box3& vv, Plane* planes)
{
	Vec3 loc(0, 0, 0);				// viewer location
	Vec3 dir(0, 0, -1.0f);			// viewer forward direction
	Vec3 up(0, 1.0f, 0.0f);			// viewer up direction
	Vec3 wvpr(0.0f, 0.0f, -vv.min.z);
	Vec3 wvpn(0.0f, 0.0f, 1.0f);
	Vec3 wvpu = up;
	Vec3 uxn = wvpu.Cross(wvpn);
	Vec3 p0, pa, tmp;
	float scll = vv.min.x;
	float sclr = vv.max.x;
   
    tmp.x = (wvpr.x + scll * uxn.x) - loc.x;		// vector to left edge
    tmp.y = (wvpr.y + scll * uxn.y) - loc.y;
    tmp.z = (wvpr.z + scll * uxn.z) - loc.z;
    pa = wvpu.Cross(tmp);
    pa.Normalize();
    planes[0].Set(loc, pa);							// left plane
    tmp.x = (wvpr.x + sclr * uxn.x) - loc.x;        // vector to right edge
    tmp.y = (wvpr.y + sclr * uxn.y) - loc.y;
    tmp.z = (wvpr.z + sclr * uxn.z) - loc.z;
    pa = tmp.Cross(wvpu);
    pa.Normalize();
    planes[1].Set(loc, pa);							// right plane
    tmp.x = (wvpr.x + vv.max.y * wvpu.x) - loc.x;	// vector to top edge
    tmp.y = (wvpr.y + vv.max.y * wvpu.y) - loc.y;
    tmp.z = (wvpr.z + vv.max.y * wvpu.z) - loc.z;
    pa = uxn.Cross(tmp);
    pa.Normalize( );
    planes[2].Set(loc, pa);							// top plane
    tmp.x = (wvpr.x + vv.min.y * wvpu.x) - loc.x;	// vector to bot edge
    tmp.y = (wvpr.y + vv.min.y * wvpu.y) - loc.y;
    tmp.z = (wvpr.z + vv.min.y * wvpu.z) - loc.z;
    pa = tmp.Cross(uxn);
    pa.Normalize();
    planes[3].Set(loc, pa);							// bottom plane

    planes[4].Set(wvpr, wvpn);						// near plane
    p0.Set(0, 0, -vv.max.z);
	tmp = -wvpn;
    planes[5].Set(p0, tmp);							// far plane
}
		
/*!
 * @fn void Camera::PopClipPlane()
 *
 * Pops the most recently added cull plane. Does nothing if no
 * extra planes have been pushed. This function will \b not remove
 * any of the default view volume culling planes.
 *
 * @see Camera::PushClipPlane Camera::NumClipPlanes Portal
 */
void Camera::PopClipPlane()
{
	int size = (int) m_ExtraPlanes.GetSize();
	if (size > 0)
		m_ExtraPlanes.RemoveAt(size-1);
}
	
	
/*!
 * @fn void Camera::GetDeviceCoords(Vec3* points, int count) const
 * @param points	point list to map to device coordinates
 * @param count		number of points in point list
 *
 * Applies the total camera transformation and then the camera projection
 * matrix to bring the points from world to screen (device) space.
 * This applies a 3D affine matrix followed by a perspective transformation
 * to produce 2D points from 3D points.
 *
 * @see Model::GetBound Camera::GetProjection Camera::GetViewTrans 
 */
void Camera::GetDeviceCoords(Vec3* points, int count) const
{
	Matrix	proj, camtrans;
 	Vec4		p1, p2;

	TotalTransform(&camtrans);
	while (--count >= 0)
	{
		p1 = *points;
		camtrans.Transform(p1, p2);
		proj.Transform(p2, p1);
		points->z = 0.0f;
		points->x = p1.x / p1.w;
		points->y = p1.y / p1.w;
		++points;
	}
}

/*!
 * @fn void Camera::GetDeviceCoords(Sphere& sph) const
 * @param sph	sphere to map to device coordinates
 *
 * Applies the total camera transformation and then the camera projection
 * matrix to bring the sphere from world to screen (device) space.
 * This applies a 3D affine matrix followed by a perspective transformation
 * to produce 2D points from 3D points.
 *
 * @see Model::GetBound Camera::GetProjection Camera::GetViewTrans 
 */
void Camera::GetDeviceCoords(Sphere& sph) const
{
	Vec3	p[2];

	p[0] = sph.Center;
	p[1].x = sph.Center.x + sph.Radius;
	p[1].y = sph.Center.y + sph.Radius;
	p[1].z = sph.Center.z + sph.Radius;
	GetDeviceCoords(&p[0], 2);
	sph.Center = p[0];
	sph.Radius = p[0].Distance(p[1]);	
}


void Camera::WorldPlane(Plane& newplane, const Plane& oldplane) const
{
	Matrix mtx(m_ViewTrans);
	mtx.Transpose();
	newplane.Transform(mtx, oldplane);
}

/*!
 * @fn void Camera::SetViewTrans(Scene* scene, Matrix* mtx)
 * @param mtx matrix to pre-multiply, may be NULL
 *
 * Computes the camera view matrix for the scene. This is
 * the matrix which takes world coordinates to camera coordinates.
 * The input matrix can be used to modify the view matrix for
 * stereo viewing.
 *
 * @internal
 * @see Camera::GetViewTrans
 */
void Camera::SetViewTrans(Scene* scene, Matrix* mtx)
{
	Matrix worldmtx;

	TotalTransform(&worldmtx, scene->GetModels());
	m_ViewTrans.Invert(worldmtx);
	if (mtx)
		m_ViewTrans.PreMul(*mtx);
	Matrix tmp(m_ViewTrans);
	tmp.Transpose();
	for (int i = 0; i < 6; ++i)
		m_WorldPlanes[i].Transform(tmp, m_CullPlanes[i]);
	ClearFlags(MOD_BVinvalid);
}

}	// end Vixen