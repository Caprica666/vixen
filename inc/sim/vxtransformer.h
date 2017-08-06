
/*!
 * @file vxtransformer.h
 * @brief Model matrix controller.
 *
 * Transformers control the matrix of a model and let you manipulate
 * position, rotation and scale components independently.
 * Interpolator children can be added to perform key-frame animations
 * of these tracks.
 *
 * @author Nola Donato
 * @ingroup vixen
 * @see vxinterpolator.h vxkeyframe.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Transformer
 * @brief Engine that modifies the transformation matrix of
 * its target  Model> based on its current  Position,
 *  Rotation and  Scale.
 *
 * The Transformer also maintains a  LookRotation, allowing an object to move in a direction
 * different than it is facing. The rotations for the object
 * are specified as quaternions so that rotations will interpolate nicely.
 * You can more easily think of it as specifying an axis
 * of rotation and an angle about that axis.
 *
 * <B>Animation</B>
 * Usually a transformer is used in conjunction with several
 * child engines that control position, rotation or scale.
 * It takes the output from these engines and constructs a single
 * transformation matrix to control the movement
 * of its target object within the scene.
 *
 * The  Rotation and  Position attributes
 * are typically what is used to animate the object.
 * These properties are usually controlled by individual KeyFramer
 * engines that are children of the Transformer.
 *
 * Animations from Maya are converted to scene manager
 * engines by the Vixen Maya exporter. Each Maya controller matrix
 * controller becomes a Transformer with several KeyFramer children.
 * Animations are exported by sampling the matrices produced by Maya
 * as the animation progresses.
 *
 * Articulated character animation is handled by arranging transformers
 * the same hierarchy as their target models. Each transformer computes and
 * saves a world transformation matrix which concatenates all the transformations
 * for the target from the root of the hierarchy. This capability is used
 * to implement skinned characters in the scene manager.
 *
 * @image html transform.gif
 *
 * <B>Navigation</B>
 *
 * In addition to animation, the Transformer can process navigation
 * events (Event::NAVIGATE) to control its movement. A navigation
 * event provides a generic way for input engines such as mice
 * and joysticks to communicate movement. The flags indicate
 * which fields are meaningful and can be none or more of
 * (NavigateEvent::LOCAL_POS, NavigateEvent::POSITION, NavigateEvent::ROTATION, NavigateEvent::LOOK).
 * @code
 *	struct NavigateEvent : public Event
 *	{
 *	 int32	Flags;	// which fields are set
 *	 Vec3	Pos;	// amount to change position
 *	 Quat	Rot;	// amount to change rotation
 *	 Quat	Look;	// amount to change look rotation
 *	};
 * @endcode
 * A Transformer can also work with a TerrainCollider, an engine
 * which detects collisions with 2D terrain to limit navigation
 * and animation with certain boundaries.
 *
 * <B>Recording</B>
 *
 * The behavior of a transformer over time can be recorded in a file and
 * later played back. You can record the navigation events produced by
 * input to a transformer. This feature is used by the 3D Viewer to
 * create script files to record and play back the camera movement
 * when navigating with the mouse or joystick.
 *
 * A transformer is also capable of producing a set of keyframes for
 * position, scale and rotation by observing the matrix of its target
 * model. In this mode, the transformer inspects the target matrix every
 * frame and decomposes it to compute the keyframes. The keyframer children
 * of the transformer are populated with this data.
 *
 * @see @htmlonly <A><HREF="oview/animoview.htm">Vixen Animation Overview</A> @endhtmlonly
 * @see Quat KeyFramer Model TerrainCollider NavigateEvent Navigator
 */
class Transformer : public Engine
{
public:
	VX_DECLARE_CLASS(Transformer);

	Transformer();
	Transformer(const Matrix* trans);

//	Accessors
	const Vec3&		GetPosition() const;
	const Quat&		GetRotation() const;
	const Vec3&		GetScale() const;

	//! Get local matrix computed by transformer.
	const Matrix*	GetTransform() const;

	const Quat&		GetLookRotation() const;

	//! Get total matrix concatenated from root transformer.
	const Matrix*	GetTotalTransform() const;

	//! Set options
	void			SetOptions(int opts);
	int				GetOptions() const;
	
	//! Set bone index
	void			SetBoneIndex(int i);
	int				GetBoneIndex() const;

	//! Set position in parent coordinate frame.
	void			SetPosition(const Vec3&);
	void			SetPosition(float x, float y, float z);

	//! Set scaling of target.
	void			SetScale(const Vec3&);
	void			SetScale(float x, float y, float z);

	//! Set rotation of target (where it moves to).
	void			SetRotation(const Quat&);
	void			SetRotation(const Vec3& axis, float angle);

	//! Set position in object coordinate frame.
	void			SetLocalPosition(const Vec3&);

	//! Set orientation of target (where it faces).
	void			SetLookRotation(const Quat&);
	void			Reset();

	//! Set properties from transformation matrix
	void			SetTransform(const Matrix* trans);

	//! Calculate transformation matrix from properties
	const Matrix*	CalcMatrix();

//	Internal overrides
	virtual bool	OnStart();
	virtual void	Compute(float time);
	virtual bool	Eval(float t);
	virtual bool	OnEvent(Event* event);
	virtual void	SetTarget(SharedObj *target);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& dbg = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum
	{
		LOCAL = 0,			// updates local transform of target
		WORLD = 1,			// updates world transform of target
	};

	/*
	 * Transformer::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		XFORM_SetLocalPosition = Engine::ENG_NextOp,
		XFORM_SetRotation,
		XFORM_SetPosition,
		XFORM_SetScale,
		XFORM_SetLookRotation,
		XFORM_SetTransform,
		XFORM_SetOptions,
		XFORM_SetBoneIndex,
		XFORM_NextOp = Engine::ENG_NextOp + 20,
	};

protected:
	virtual void	OnNavigate(NavigateEvent*);
		
	int32				m_BoneIndex;
	uint32				m_Flags;
	int32				m_Options;
	Vec3				m_Position;
	Vec3				m_Scale;
	Vec3				m_LocalPosition;
	Interpolator::WQuat	m_Rotation;
	Interpolator::WQuat	m_LookRotation;

	NavigateEvent		m_NavEvent;
	Ref<Matrix>			m_TotalTransform;
	Ref<Matrix>			m_Transform;
};


inline const Vec3& Transformer::GetPosition() const
	{ return m_Position; }

inline const Vec3& Transformer::GetScale() const
	{ return m_Scale; }

inline const Quat& Transformer::GetRotation() const
	{ return m_Rotation; }

inline const Quat& Transformer::GetLookRotation() const
	{ return m_LookRotation; }

/*!
 *	Gets the local transform calculated from the fields of
 *	this transformer. This becomes the local matrix for the target model.
 *
 * @see Transformer::GetTotalTransform
 */
inline const Matrix* Transformer::GetTransform() const
{
	if (m_Transform.IsNull())
		return Matrix::GetIdentity();
	else return m_Transform;
}

/*!
 *
 *	Gets the total transform for the target model from the root
 *	of the transformer hierarchy.
 *
 * @see Transformer::GetTransform
 */
inline const Matrix* Transformer::GetTotalTransform() const
{
	if (m_TotalTransform.IsNull())
		return Matrix::GetIdentity();
	else return m_TotalTransform;
}

inline int Transformer::GetOptions() const
{
	return m_Options;
}

inline int Transformer::GetBoneIndex() const
{
	return m_BoneIndex;
}

} // end Vixen