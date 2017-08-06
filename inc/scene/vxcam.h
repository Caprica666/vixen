/*!
 * @file vxcam.h
 * @brief Camera for viewing a 3D scene.
 *
 * A camera specifies the location of a viewer and how the
 * 3D scene is projected onto the 2D viewport.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmodel.h vxscene.h
 */

#pragma once

namespace Vixen {


/*!
 * @class Camera
 * @brief Determines how a 3D scene is viewed, the location of the viewer and the
 * viewport characteristics.
 *
 * A camera is a model and can be placed anywhere in a scene.
 * It can be moved, oriented and treated as part of the scene graph.
 * For example, a camera can be part of an airplane and can be positioned
 * in the cockpit. Then, as the airplane moves, the camera moves along
 * with it, providing a view of the scene from the cockpit.
 * By default, the camera is located at the origin
 * and looks down the negative Z axis.
 *
 * Although you can define any number of cameras, a camera is used
 * for viewing only when it is the current camera of the scene
 * being displayed.
 *
 * There are two kinds of cameras:  perspective and  orthographic.
 * A perspective camera defines a tetrahedral viewing volume with a
 * field of view angle defining the angle of the tetrahedron.
 * An orthographic camera defines a rectangular view volume
 * and uses a parallel projection.
 *
 * You can define either kind of camera by specifying its view volume.
 * You can also define a perspective camera in terms of its  hither
 * and  yon clipping planes and its field of view.
 *
 * Internally, the scene manager does view-volume culling. This causes shapes that
 * are outside the camera's view volume to be discarded and not displayed.
 * You may supply additional culling planes for the camera if desired.
 * These can sometimes be useful in higher level visibility algorithms,
 * such as portals.
 *
 * @image html camexpo.gif
 *
 * @ingroup vixen
 * @see Scene Model Scene::SetCamera Portal
 */
class Camera : public Model
{
	friend class Scene;
public:

	/*!
	 * @brief Camera types and visibility return codes.
	 * @see Camera::SetKind Camera::IsVisible
	 */
	enum
	{
		PERSPECTIVE = 0,		//!< SetType: use perspective camera
		ORTHOGRAPHIC =	1,		//!< SetType: use orthographic camera
		LEFT_PERSP = 0,			//!< StereoPerspProj: left eye stereo camera
		RIGHT_PERSP = 1,		//!< StereoPerspProj: right eye stereo camera
		VISIBLE = 1,			//!< IsVisible: partially within camera viewport
		ALL_VISIBLE = 2			//!< IsVisible: entirely within camera viewport
	};

	VX_DECLARE_CLASS(Camera);
//! Construct camera at origin, looking down -Z axis.
	Camera();

//! Construct camera like the given one.
	Camera(const Camera&);

//! @name Properties
//@{
	virtual	void	SetViewVol(const Box3&);	//!< Set 3D view volume.
	virtual	void	SetCullVol(const Box3&);	//!< Set 3D cull volume.
	virtual	void	SetType(int);				//!< Set camera type.
	virtual	void	SetHeight(float);			//!< Set view volume height.
	virtual	void	SetAspect(float);			//!< Set view volume width.
	virtual	void	SetHither(float);			//!< Set near clipping distance.
	virtual	void	SetYon(float);				//!< Set far clipping distance.
	virtual	void	SetFOV(float);				//!< Set field of view.
	virtual	void	SetEyeSep(float);			//!< Set stereo eye separation.
	virtual	void	SetFocal(float);			//!< Set focal length (distance from user).
	
	virtual	float	GetAspect() const;			//!< Get aspect ratio (view volume width/height).
	virtual	float	GetHeight() const;			//!< Get height of view volume.
	virtual	float	GetHither() const;			//!< Get near clipping plane distance.
	virtual	float	GetYon() const;				//!< Get far clipping plan distance.
	virtual	float	GetFOV() const;				//!< Get field of view angle (in radians).
	virtual	float	GetEyeSep() const;			//!< Get stereo eye separation.
	virtual	float	GetFocal() const;			//!< Get focal length.
	virtual	void	GetViewVol(Box3*) const;	//!< Get 3D view volume.
	virtual	int		GetType() const;			//!< Get camera type (PERSPECTIVE, ORTHOGRAPHIC).
	virtual	const Box3&	GetViewVol() const;	//!< Return 3D view volume.
	virtual	const Box3&	GetCullVol() const;	//!< Return 3D cull volume.
//@}

//! @name Projection and Viewing
//@{
//! Determine if camera-space sphere is visible to this camera.
	virtual	int		IsVisible(const Sphere&) const;

//! Determine if world-space box is visible to this camera.
	virtual int		IsVisible(const Box3& box) const;

 //! Return culling planes
	virtual const Plane* GetCullPlanes() const;

//! Return number of pushed clip planes.
	int				NumClipPlanes() const;

//! Push additional clip plane.
	void			PushClipPlane(const Plane&);

//! Ppop clip plane on top of stack.
	void			PopClipPlane();

//! Calculate new view planes for culling
	void			CalcViewPlanes();

//! Convert sphere from world to screen coordinates.
	virtual	void	GetDeviceCoords(Sphere&) const;

//! Convert list of points from world to screen coordinates.
	virtual	void	GetDeviceCoords(Vec3* pts, int n) const;

//! Return camera view matrix (internal).
	virtual const Matrix* GetViewTrans() const;

	//! Calculate stereo perspective view volume for left or right eye.
	void			GetStereoView(int which_eye, Box3& viewvol) const;

//! Calculate view clipping planes for the given view volume.
	static void		CalcViewPlanes(const Box3& viewvol, Plane* planes);

//! Calculate perspective projection matrix for given view volume
	static void		ProjPerspective(const Box3& vv, Matrix& m);

//! Calculate orthographic projection matrix for given view volume
	static void		ProjOrtho(const Box3& vv, Matrix& m);

//@}


//	Implementation Overrides
	virtual	Vec3	GetDirection() const;
	virtual	Vec3	GetCenter(int transformtype = WORLD) const;
	virtual	bool	GetCenter(Vec3* p, int transformtype = WORLD) const;
	virtual	bool	Do(Messenger& s, int opcode);
	virtual	int		Save(Messenger&, int) const;
	virtual	bool	Copy(const SharedObj*);
	virtual	void	NotifyParents(uint32);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	//! Set device camera view matrix (internal).
	virtual void	SetViewTrans(Scene*, Matrix* = NULL);

	/*
	 * Camera::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		CAM_SetType = Model::MOD_NextOp,
		CAM_SetFOV,
		CAM_SetHither,
		CAM_SetYon,
		CAM_SetAspect,
		CAM_SetViewVol,
		CAM_SetCullVol,
		CAM_SetHeight,
		CAM_SetEyeSep,
		CAM_SetFocal,
		CAM_NextOp = Model::MOD_NextOp + 20
	};

protected:

	//! Recompute camera culling planes.

	void			WorldPlane(Plane& newplane, const Plane& oldplane) const;

//	Data members
	int32		m_Type;
	int32		m_CullChanged;
	Box3		m_ViewVol;			// view volume
	Box3		m_CullVol;			// volume for culling
	Matrix		m_ViewTrans;		// camera view transform (inverse of m_WorldTrans)
	float		m_FOV;				// field of view in Radian
	float		m_Aspect;			// aspect ratio
	float		m_EyeSep;			// stereo eye separation
	float		m_Focal;			// focal length
	Plane		m_CullPlanes[6];	// clip planes in camera space (camera at 0,0,0)
	Plane		m_WorldPlanes[6];	// clip planes in world space
	Array<Plane> m_ExtraPlanes;
};

/*!
 * @fn void Camera::PushClipPlane(const Plane& plane)
 * @param plane	defines the new culling plane
 *
 * By default, the six view planes defining the view volume are used
 * to cull geometry. Extra planes are used by the portal culler to
 * eliminate geometry that cannot be seen thru windows.
 * Some display devices also supports these extra planes.
 *
 * @see Camera::PopClipPlane Portal Camera::NumClipPlanes
 */
inline void Camera::PushClipPlane (const Plane& plane)
	{ m_ExtraPlanes.Append(plane); }

inline const Plane* Camera::GetCullPlanes() const
	{ return &m_CullPlanes[0]; }

inline int	Camera::GetType() const
	{ return m_Type; }

inline float	Camera::GetFOV() const
	{ return m_FOV; }

inline float Camera::GetFocal() const
	{ return m_Focal; }

inline const Box3& Camera::GetViewVol() const
	{ return m_ViewVol; }

inline void Camera::GetViewVol(Box3* b) const
	{ *b = m_ViewVol; }

inline const Box3& Camera::GetCullVol() const
	{ return m_CullVol; }

inline float	Camera::GetHither() const
	{ return m_ViewVol.min.z; }

inline float	Camera::GetYon() const
	{ return m_ViewVol.max.z; }

inline float	Camera::GetHeight() const
	{ return m_ViewVol.Height(); }

inline float	Camera::GetAspect() const
	{ return m_ViewVol.Width() / m_ViewVol.Height(); }

inline int Camera::NumClipPlanes() const
	{ return (int) m_ExtraPlanes.GetSize(); }

inline Vec3 Camera::GetCenter(int f) const
	{ return Model::GetCenter(f); }

inline float Camera::GetEyeSep() const
	{ return m_EyeSep; }
} // end Vixen