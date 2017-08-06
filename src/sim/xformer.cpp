#include "vixen.h"


namespace Vixen {
VX_IMPLEMENT_CLASSID(Transformer, Engine, VX_Transformer);
static const TCHAR* opnames[] =
{	TEXT("SetLocalPosition"), TEXT("SetRotation"), TEXT("SetPosition"), TEXT("SetScale"), TEXT("SetLookRotation"),
	TEXT("SetTransform"), TEXT("SetOptions"), TEXT("SetBoneIndex") };

const TCHAR** Transformer::DoNames = opnames;

/*!
 * @fn Transformer::Transformer()
 *
 * Constructs an empty Transformer with default attributes.
 * If a matrix is supplied, the Position, Rotation and
 * Scale are set from that.
 */
Transformer::Transformer()
 :	Engine(),
	m_Options(0),
	m_Position(0.0f, 0.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f, 1.0f),
	m_LookRotation(0.0f, 0.0f, 0.0f, 1.0f),
	m_Scale(1.0f, 1.0f, 1.0f),
	m_BoneIndex(-1),
	m_Flags(0)
{
	m_Transform = new Matrix;
	m_TotalTransform = new Matrix;
}

/*!
 * @fn Transformer::Transformer(const Matrix* mtx)
 * @param mtx	matrix to initialize from
 *
 * Constructs a Transformer with Position, Rotation and
 * Scale set from the input matrix
 *
 * @see Transformer::SetTransform
 */
Transformer::Transformer(const Matrix* trans)
 :	Engine(),
	m_Options(0),
	m_Position(0.0f, 0.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f, 1.0f),
	m_LookRotation(0.0f, 0.0f, 0.0f, 1.0f),
	m_BoneIndex(-1),
	m_Scale(1.0f, 1.0f, 1.0f),
	m_Flags(0)
{
	m_Transform = new Matrix;
	m_TotalTransform = new Matrix;
	SetTransform(trans);
}

/*!
 * @fn void Transformer::Reset()
 *
 * Resets the state of the transformer to initial conditions:
 * @code
 *	Position	0, 0, 0
 *	Scale		1, 1, 1
 *	Rotation	0, 0, 0, 1
 *	LookRotaton	0, 0, 0, 1
 * @endcode
 *
 * @see Transformer::SetPosition Transformer::SetScale Transformer::SetRotation
 */
void Transformer::Reset()
{
	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(Quat(0, 0, 0, 1));
	SetLookRotation(Quat(0, 0, 0, 1));
	SetScale(1.0f, 1.0f, 1.0f);
}


/*!
 * @fn void Transformer::SetPosition(float x, float y, float z)
 * @param x,y,z		coordinates of new position
 *
 * The position is given in local coordinates with respect
 * to the parent Transformer. It specifies
 * the current position of the transformer target.
 *
 * @see Transformer::SetLocalPosition Transformer::SetRotation
 */
void Transformer::SetPosition(float x, float y, float z)
	{ SetPosition(Vec3(x, y, z)); }

/*!
 * @fn void Transformer::SetPosition(const Vec3& v)
 * @param v		vector with coordinates of new position
 *
 * The position is given in local coordinates with respect
 * to the parent Transformer. It specifies
 * the current position of the transformer target.
 *
 * @see Transformer::SetLocalPosition Transformer::SetRotation
 */
void Transformer::SetPosition(const Vec3& v)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetPosition) << this << v;
   VX_STREAM_END(  )

	m_Position = v;
	if ((v.x != 0) || (v.y != 0) || (v.z != 0))
		m_Flags |= NavigateEvent::POSITION;
	else m_Flags &=~ NavigateEvent::POSITION;
	SetChanged(true);
}


/*!
 * @fn void Transformer::SetScale(float x, float y, float z)
 * @param x,y,z		coordinates of new position
 *
 * The scale specifies the final scale factors applied
 * to the target of the transformer after it has been rotated.
 *
 * @see Transformer::SetPosition Transformer::SetRotation
 */
void Transformer::SetScale(float x, float y, float z)
	{ SetScale(Vec3(x, y, z)); }

/*!
 * @fn void Transformer::SetScale(const Vec3& v)
 * @param v		vector with new scale factors
 *
 * The scale specifies the final scale factors applied
 * to the target of the transformer after it has been rotated.
 *
 * @see Transformer::SetPosition Transformer::SetRotation
 */
void Transformer::SetScale(const Vec3& v)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetScale) << this << v;
   VX_STREAM_END(  )

	m_Scale = v;
	if ((v.x != 1) || (v.y != 1) || (v.z != 1))
		m_Flags |= NavigateEvent::SCALE;
	else m_Flags &=~ NavigateEvent::SCALE;
	SetChanged(true);
}


/*!
 * @fn void Transformer::SetRotation(const Vec3& axis, float angle)
 * @param axis	rotation axis
 * @param angle	angle to rotate about axis (radians)
 *
 * Rotates the transformer target, changing the direction
 * of its movement. This rotation does not change the
 * direction the object faces, which is established by
 * calling Transformer::SetLookRotation.
 *
 * @see Transformer::SetPosition Transformer::SetLookRotation
 */
void Transformer::SetRotation(const Vec3& axis, float angle)
	{ SetRotation(Quat(axis, angle)); }

/*!
 * @fn void Transformer::SetRotation(const Quat& q)
 * @param q quaternion describing rotation
 *
 * Rotates the transformer target, changing the direction
 * of its movement. This rotation does not change the
 * direction the object faces, which is established by
 * calling Transformer::SetLookRotation.
 *
 * @see Transformer::SetPosition Transformer::SetLookRotation
 */
void	Transformer::SetRotation(const Quat& q)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetRotation) << this << q;
   VX_STREAM_END(  )

	m_Rotation = q;
	if (q.w != 1.0f)
		m_Flags |= NavigateEvent::ROTATION;
	else m_Flags &=~ NavigateEvent::ROTATION;
	SetChanged(true);
}

/*!
 *
 * @fn void Transformer::SetLocalPosition(const Vec3 &v)
 * @param v vector used to set local position
 *
 * The vector is assumed to be a position in local coordinates,
 * allowing you to move an object with respect to the direction
 * it is currently facing.
 *
 * @see Transformer::SetPosition Transformer::SetLookRotation
 */
void Transformer::SetLocalPosition(const Vec3 &v)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetLocalPosition) << this << v;
   VX_STREAM_END(  )

	if ((v.x != 0) || (v.y != 0) || (v.z != 0))
		m_Flags |= NavigateEvent::LOCAL_POS;
	else m_Flags &=~ NavigateEvent::LOCAL_POS;
	SetChanged(true);
	m_LocalPosition = v;
}

/*!
 *
 * @fn void Transformer::SetOptions(int opts)
 * @param opts	options to set
 *				LOCAL	use local matrix to transform target
 *				WORLD	use total world matrix to transform target
 *
 * Designates how to transform the target object. If the target
 * is part of an animated hierarchy, the LOCAL option should be
 * used to apply the local transformation matrix from the Transformer
 * to the object. If the target is at the root of the scene graph
 * and but the Transformer represents a bone in a hierarchy,
 * the WORLD option should be used to apply the Transformer's
 * total world matrix to the object.
 */
void Transformer::SetOptions(int opts)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetOptions) << this << opts;
   VX_STREAM_END(  )

	m_Options = opts;
}

/*!
 *
 * @fn void Transformer::SetBoneIndex(int index)
 * @param index	0-based bone index
 *
 * When the Transformer is used as a bone in a Skeleton,
 * the bone index is set to the ID of the skeleton bone
 * this Transformer represents. Skeleton bone IDs are
 * 0-based and are automatically set if the Skeleton creates
 * the Transformer. Skeleton::FindBones will use bone indices
 * if they are available.
 *
 * @see Skeleton::GetBone Skeleton::GetParentBoneIndex
 */
void Transformer::SetBoneIndex(int index)
{
	VX_STREAM_BEGIN(s)
      *s << OP(VX_Transformer, XFORM_SetBoneIndex) << this << index;
	VX_STREAM_END(  )

	m_BoneIndex = index;
}

/*!
 *
 * @fn void Transformer::SetLookRotation(const Quat& q)
 * @param q quaternion specifying look rotation
 *
 * Orients the transformer target in its local coordinate system.
 * The object will face in this direction but will move in
 * the direction established with Transformer::SetRotation.
 *
 * @see Transformer::SetLocalPosition Transformer::SetRotation
 */
void Transformer::SetLookRotation(const Quat& q)
{
	m_LookRotation = q;
	if (q.w != 1.0f)
		m_Flags |= NavigateEvent::LOOK;
	else m_Flags &=~ NavigateEvent::LOOK;
	SetChanged(true);
}

/*!
 * @fn bool Transformer::OnStart()
 *
 * If any of the children of this transformer are interpolators
 * with destination type INTRP_XFORM_xxx, hook their destination
 * to the appropriate field in this transformer.
 * @code
 *	XFORM_Position		hook to SetPosition value
 *	XFORM_Rotation		hook to SetRotation value
 *	XFORM_LookRotation	hook to SetLookRotation value
 *	XFORM_Scale			hook to SetScale value
 * @endcode
 *
 * @see Evaluator::SetDestType Evaluator::SetDest
 */
bool Transformer::OnStart()
{
	GroupIter<Engine> iter(this, Group::CHILDREN);
	Engine*	e;

	while (e = iter.Next())
	{
		if (!e->IsClass(VX_Evaluator))
			continue;
		Interpolator* intrp = (Interpolator*) e;
		SetControl(CHILDREN_FIRST | GetControl(), ONLYME);
		switch (intrp->GetDestType())
		{
			case Evaluator::POSITION:
			case XFORM_SetPosition:
			intrp->SetDest((float*) &m_Position);
			m_Flags |= NavigateEvent::POSITION;
			break;

			case Evaluator::LOCAL_POSITION:
			intrp->SetDest((float*) &m_LocalPosition);
			m_Flags |= NavigateEvent::LOCAL_POS;
			break;

			case Evaluator::ROTATION:
			case XFORM_SetRotation:
			intrp->SetDest((float*) &m_Rotation);
			intrp->SetValSize(sizeof(Quat) / sizeof(float));
			m_Flags |= NavigateEvent::ROTATION;
			break;

			case Evaluator::SCALE:
			case XFORM_SetScale:
			intrp->SetDest((float*) &m_Scale);
			m_Flags |= NavigateEvent::SCALE;
			break;

			case Evaluator::LOOK_ROTATION:
			case XFORM_SetLookRotation:
			intrp->SetValSize(5);
			intrp->SetDest((float*) &m_LookRotation);
			m_Flags |= NavigateEvent::LOOK;
			break;
		}
	}
	return true;
}

/*!
 * @fn const Matrix* Transformer::CalcMatrix()
 *
 * Calculates the local matrix for the transformer based on the
 * position, rotation, scale and look rotation values.
 *
 * @see Transformer::Eval Transformer::SetPosition Transformer::Reset Transformer::SetRotation
 */
const Matrix* Transformer::CalcMatrix()
{
	static Matrix tmp_mat;
	static Vec3 tmp_vec;

	//--- start with Identity ---
	m_Transform->Identity();
	if (m_Flags & NavigateEvent::LOOK)	/* has look? */
		m_Transform->Rotate(m_LookRotation);
	if ((m_Flags & NavigateEvent::ROTATION) && !m_Rotation.IsEmpty())	/* has rotation? */
	{
		if (m_Flags & NavigateEvent::LOCAL_POS)
		{
			tmp_mat.RotationMatrix(m_Rotation);
			m_Transform->PreMul(tmp_mat);
			tmp_mat.Transform(m_LocalPosition, tmp_vec);
			tmp_vec += m_Position;
			SetPosition(tmp_vec); // this manages the flags... but it's awkward.
			m_LocalPosition.Set(0,0,0);
		}
		else
			m_Transform->Rotate(m_Rotation);
	}
	else if (m_Flags & NavigateEvent::LOCAL_POS)
	{
		tmp_vec = m_Position + m_LocalPosition;
		SetPosition(tmp_vec);
		m_LocalPosition.Set(0,0,0);
	}
	if (m_Flags & NavigateEvent::POSITION)
		m_Transform->Translate(m_Position);	/* apply translation */
	if (m_Flags & NavigateEvent::SCALE)		/* has normal scale? */
		m_Transform->Scale(m_Scale);
/*
 * Update total transform
 */
	Transformer* par = (Transformer*) Parent();
	if (par && par->IsClass(VX_Transformer))
		m_TotalTransform->Multiply(*(par->GetTotalTransform()), *((Matrix*) m_Transform));
	else
		m_TotalTransform->Copy((Matrix*) m_Transform);
	return m_Transform;
}

void Transformer::Compute(float t)
{
	Lock();
	bool		do_kids = !(m_Control & CONTROL_CHILDREN);
	bool		kids_first = (m_Control & CHILDREN_FIRST) != 0;
	float		eval_t = ComputeTime(t);
	bool		do_eval = (eval_t >= 0);
	Unlock();

	if (!IsParent())
	{
		if (do_eval)
			DoEval(eval_t);
		return;
	}
	if (do_eval)
	{
		if (kids_first)							// evaluate children first?
		{
			ComputeChildren(t, -VX_Transformer);// do non-transformer children first
			DoEval(eval_t);						// ignore eval return code
			ComputeChildren(t, VX_Transformer);	// always do child transformers after
			return;
		}
		if (!DoEval(eval_t))
			do_kids = false;
	}
	if (do_kids)
	{
		ComputeChildren(t, -VX_Transformer);	// do non-transformer children first
		ComputeChildren(t, VX_Transformer);		// always do child transformers after
	}
}

/*!
 * @fn bool Transformer::Eval(float time)
 *
 * Computes the transformer local and total matrices and updates
 * the target. The scale, rotation, position and look rotation are
 * combined to produce the local matrix for the target model.
 * If the parent engine is also a transformer, the local matrix computed
 * is concatenated to the parent's total matrix to derive the
 * total matrix for this transformer. Thus a hierarchy of transformers
 * can be used to calculate the world matrices for all target models.
 *
 * Transformers respond to navigation events by saving the most recent
 * event and processing it during Eval. If recording is enabled,
 * the processed event is logged to the transformer's output stream.
 *
 * @see Evaluator::Eval Engine::Eval Transformer::CalcMatrix
 */
bool Transformer::Eval(float time)
{
	int debug;
/*
 * If this evaluation has changed the state of the transformer,
 * recalculate the local matrix. If there is a target model,
 * change its local matrix based on the calculated one from this frame.
 */
	if (!HasChanged())								// state did not change?
		return true;								// we are done
	CalcMatrix();									// recalculate target matrix
	debug = Engine::Debug + Transformer::Debug;
	VX_TRACE(debug, ("Transformer::Eval %s p(%f, %f, %f) r(%f, %f, %f, %f)\n",
			GetName(), m_Position.x, m_Position.y, m_Position.z, m_Rotation.x, m_Rotation.y, m_Rotation.z, m_Rotation.w));
	m_Flags &= ~NavigateEvent::EVENT_PENDING;	// clear event pending flag

	SharedObj* target = GetTarget();
	SetChanged(false);							// mark state as unchanged
	if (target == NULL)
		return true;
	if (target->IsClass(VX_Model))				// have model target?
	{
		Model* m = (Model*) target;
		if (m_Options & WORLD)
			m->SetTransform(m_TotalTransform);	// update model using world matrix
		else
			m->SetTransform(m_Transform);		// update model using local matrix
	}
	else if (target->IsClass(VX_Transformer))
	{
		Transformer* x = (Transformer*) target;
		if (m_Options & WORLD)
			x->SetTransform(m_TotalTransform);	// update bone using world matrix
		else
			x->SetTransform(m_Transform);		// update bone using local matrix
	}
	return true;
}

/****
 *
 * class Transformer override for SharedObj::Copy
 *
 ****/
bool Transformer::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Engine::Copy(srcobj))
		return false;
	const Transformer* src = (const Transformer*) srcobj;
	if (src->IsClass(VX_Transformer))
	{
		m_Options = src->m_Options;
		m_Flags = src->m_Flags;
		m_LookRotation = src->m_LookRotation;
		m_Position = src->m_Position;
		m_Rotation = src->m_Rotation;
		m_LocalPosition = src->m_LocalPosition;
		m_Scale = src->m_Scale;
		m_NavEvent = src->m_NavEvent;
		m_TotalTransform = src->m_TotalTransform;
		m_Transform = src->m_Transform;
	}
	return true;
}

/****
 *
 * class Transformer override for SharedObj::Save
 *
 ****/
int Transformer::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h <= 0)
		return h;
	if (m_Options)
		s << OP(VX_Transformer, XFORM_SetOptions) << h << m_Options;
	if (m_Flags & NavigateEvent::LOCAL_POS)
		s << OP(VX_Transformer, XFORM_SetLocalPosition) << h << m_LocalPosition;
	if (m_Flags & NavigateEvent::LOOK)
 		s << OP(VX_Transformer, XFORM_SetLookRotation) << h << m_LookRotation;
 	if (m_Flags & NavigateEvent::ROTATION)
		s << OP(VX_Transformer, XFORM_SetRotation) << h << m_Rotation;
 	if (m_Flags & NavigateEvent::POSITION)
 		s << OP(VX_Transformer, XFORM_SetPosition) << h << m_Position;
 	if (m_Flags & NavigateEvent::SCALE)
 		s << OP(VX_Transformer, XFORM_SetScale) << h << m_Scale;
	return h;
}

DebugOut& Transformer::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	if (m_Flags & NavigateEvent::LOCAL_POS)
		endl(dbg << "\t<attr name='LocalPosition'>" << m_LocalPosition << "</attr>");
	if (m_Flags & NavigateEvent::LOOK)
		endl(dbg << "\t<attr name='LookRotation'>" << m_LookRotation << "</attr>");
 	if (m_Flags & NavigateEvent::POSITION)
		endl(dbg << "\t<attr name='Position'>" << m_Position << "</attr>");
 	if (m_Flags & NavigateEvent::SCALE)
		endl(dbg << "\t<attr name='Scale'>" << m_Scale << "</attr>");
 	if (m_Flags & NavigateEvent::ROTATION)
		endl(dbg << "\t<attr name='Rotation'>" << m_Rotation << "</attr>");
	if (m_Options)
		endl(dbg << "\t<attr name='Options'>" << m_Options << "</attr>");
	Engine::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Transformer override for SharedObj::Do
 *
 ****/
bool Transformer::Do(Messenger& s, int op)
{
	Vec3	v;
	Quat	q;
	int32	i;

	switch (op)
	{
		case XFORM_SetLocalPosition:
		s >> v;
		SetLocalPosition(v);
		break;

		case XFORM_SetPosition:
		s >> v;
		SetPosition(v);
		break;

		case XFORM_SetScale:
		s >> v;
		SetScale(v);
		break;

		case XFORM_SetRotation:
		s >> q;
		SetRotation(q);
		break;

		case XFORM_SetLookRotation:
		s >> q;
		SetLookRotation(q);
		break;

		case XFORM_SetOptions:
		s >> i;
		SetOptions(i);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Transformer::DoNames[op - XFORM_SetLocalPosition]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * Transformer  SetTransform()
 * Assumes M = Translate * Rotate * Scale
 *
 ****/

void Transformer::SetTransform(const Matrix *M)
{
	Vec3	scale;
	Vec3	pos;
	Matrix	tmp(*M);

	if (M)
	{
		//--- extract scale ---
		M->GetScale(scale);
		SetScale(scale);
		tmp.Scale(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);

		//--- extract rotation ---
		Quat r(tmp);
		r.Normalize();
		SetRotation(r);

		//--- extract position ---
		tmp.GetTranslation(pos);
		SetPosition(pos);
#if defined(_DEBUG) || (_TRACE > 1)
		if (Pose::Debug > 1)
		{
			CalcMatrix();
			tmp.Copy(GetTransform());
			if (tmp != *M)
				VX_WARNING(("WARNING: Transformer::SetTransform error\n"));
		}
#endif
	}
	else
	{
		SetScale(1.0f, 1.0f, 1.0f);
		SetRotation(Quat(0.0f, 0.0f, 0.0f, 1.0f));
		SetPosition(0.0f, 0.0f, 0.0f);
	}

	SetChanged(true);
}

/****
 *
 * Transformer  SetTarget()
 *
 ****/

void Transformer::SetTarget(SharedObj *target)
{
	Model*	mod = (Model*) target;
	int		hints;

	Engine::SetTarget(target);
	if ((target == NULL) || !target->IsClass(VX_Model))
		return;
	hints = mod->GetHints();
	mod->SetHints(hints & ~Model::STATIC);
	SetTransform(mod->GetTransform());
}

/****
 *
 * Name: Transformer::OnEvent
 *
 ****/
bool Transformer::OnEvent(Event* event)
{
	if (event->Code == Event::NAVIGATE)
	{
		OnNavigate((NavigateEvent*) event);
		return true;
	}
	return false;
}

void Transformer::OnNavigate(NavigateEvent* event)
{
	if (event->Flags & NavigateEvent::LOCAL_POS)		// process local position
		SetLocalPosition(event->Pos);
	if (event->Flags & NavigateEvent::POSITION)			// process global position
		if (event->Flags & NavigateEvent::ABSOLUTE)		// absolute position?
			SetPosition(event->Pos);
		else											// relative to current
			SetPosition(event->Pos + GetPosition());
	if (event->Flags & NavigateEvent::ROTATION)			// process world rotation
		if (event->Flags & NavigateEvent::ABSOLUTE)		// absolute angle?
			SetRotation(event->Rot);
		else											// no, relative to current
		{
			Quat tmp_quat, old_quat;
			old_quat = GetRotation();
			tmp_quat.Mul(old_quat,event->Rot);
			SetRotation(tmp_quat);
		}
	if (event->Flags & NavigateEvent::LOOK)				// process look rotation
		if (event->Flags & NavigateEvent::ABSOLUTE)		// absolute angle?
			SetLookRotation(event->Look);
		else											// no, relative to current
		{
			Quat tmp_quat, old_quat;
			old_quat = GetLookRotation();
			tmp_quat.Mul(old_quat,event->Look);
			if (tmp_quat.Dot(Quat(0,0,0,1)) > 0.7071f) // constraint angle
				SetLookRotation(tmp_quat);
		}
	if (m_Flags & NavigateEvent::EVENT_PENDING)
		CalcMatrix();
	m_Flags |= NavigateEvent::EVENT_PENDING;
	SetChanged(true);									// mark state as changed
}


}	// end Vixen