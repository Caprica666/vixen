#include "vixen.h"


#define QUAT_TOLERANCE	0.00001f

namespace Vixen {

VX_IMPLEMENT_CLASSID(Pose, SharedObj, VX_Pose);

static const TCHAR* opnames[] = {
		TEXT("SetSkeleton"),
		TEXT("SetWorldRotations"),
		TEXT("SetLocalRotations"),
		TEXT("SetLocalRotation"),
		TEXT("SetWorldPositions"),
		TEXT("SetWorldMatrix"),
		TEXT("SETWORLDPOSITION"),
		TEXT("SetWorldRotation"),
		TEXT("Concat"),
		TEXT("SetSpace"),
};

const TCHAR** Pose::DoNames = opnames;

/*!
 * @fn Pose::Pose(Skeleton* skel, int space)
 * @param skel	skeleton associated with the pose.
 * @param space	coordinate space for the pose (SKELETON or BIND_POSE_RELATIVE)
 *
 * Constructs a pose for the given skeleton.
 * Initially all of the bone matrices are identity.
 * If the input skeleton is NULL, the pose will be empty
 * (have no matrices in it).
 *
 * @see Pose::SetSkeleton
 */
Pose::Pose(const Skeleton* skel, int space)
 :	m_NeedSync(false),
	m_NumBones(0),
	m_Skeleton(NULL),
	m_CoordSpace(space)
{
	if (skel)
		SetSkeleton(skel);
}

/*!
 * @fn bool Pose::SetSkeleton(const Skeleton* skel)
 * @param skel	skeleton to associated with the pose.
 *
 * Changes the skeleton associated with a pose.
 * If the input skeleton is set to NULL, the matrices
 * of the pose are cleared and the pose will report
 * have 0 bones.
 *
 * @return true if skeleton is actually changed, else false
 *
 * @see Pose::SetSkeleton Pose::GetNumBones
 */
void Pose::SetSkeleton(const Skeleton* skel)
{
	if (skel)
	{
		if (m_NumBones != skel->GetNumBones())
		{
			m_NumBones = skel->GetNumBones();
			m_Bones.SetSize(skel->GetNumBones());
		}
		for (int i = 0; i < m_NumBones; ++i)
		{
			int	pid = skel->GetParentBoneIndex(i);

			m_Bones[i].ParentID = pid;
		}
	}
	else
	{
		m_NumBones = 0;
		m_Bones.SetSize(0);
	}
	m_Skeleton = skel;
}

/*!
 * @fn void Pose::ClearRotations()
 *
 * Clear all the rotations in the pose.
 *
 * @see Pose::SetLocalRotation Pose::GetNumBones Skeleton::SetBoneAxis Pose::SetWorldRotations Pose::SetWorldMatrix
 */
void Pose::ClearRotations()
{
	if (m_NumBones == 0)
		return;
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose,  POSE_ClearRotations) << this;
	VX_STREAM_END(  )

	VX_TRACE2(Debug, ("Pose::ClearRotations %s\n", GetName()));
	for (int i = 0; i < m_NumBones; ++i)
	{
		Bone&	bone(m_Bones[i]);

		bone.LocalRot.Set(0, 0, 0, 1);
		bone.WorldRot.Set(0, 0, 0, 1);
		bone.Changed = 0;
	}
	SetChanged(true);
	m_NeedSync = false;
}

/*!
 * @fn void Pose::SetLocalRotation(int boneindex, const Quat& quat)
 * @param boneindex	zero based index of bone to rotate
 * @param quat		quaternion with the rotation for the named bone
 *
 * Sets the local rotation for the designated bone.
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The local bone matrix expresses the orientation and position of the bone relative
 * to it's parent. This function sets the rotation component of that matrix from a quaternion.
 * The position of the bone is unaffected.
 *
 * @see Pose::SetLocalRotations Pose::SetWorldRotations Pose::SetWorldMatrix Skeleton::SetBoneAxis
 */
void Pose::SetLocalRotation(int boneindex, const Quat& newrot)
{
	VX_ASSERT((boneindex >= 0) && (boneindex < GetNumBones()));
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose, POSE_SetLocalRotation) << this << boneindex << newrot;
	VX_STREAM_END(  )

	Bone&	bone = m_Bones[boneindex];
	Quat	q(newrot);

	if (!m_Skeleton.IsNull() && m_Skeleton->IsLocked(boneindex))
		return;
	q.Normalize();
	if (bone.LocalRot == q)
		return;
	SetChanged(true);
	bone.LocalRot = q;
	VX_TRACE2(Debug, ("Pose::SetLocalRotation %s %s (%.3f, %.3f, %.3f, %.3f)\n",
			 GetName(), m_Skeleton->GetBoneName(boneindex), q.x, q.y, q.z, q.w));
	bone.Changed |= Bone::LOCAL_ROT;
	if (m_CoordSpace == BIND_POSE_RELATIVE)
	{
		bone.WorldRot = q;
		return;
	}
	m_NeedSync = true;
	if (bone.ParentID < 0)
		bone.WorldRot = q;
}

/*!
 * @fn void Pose::SetLocalRotations(const Quat* quat)
 * @param quat		array of quaternions with the rotations for each bone.
 *					the angles are in the bone's local coordinate system.
 *
 * The local space joint rotations for each bone are copied from the
 * source array of quaterions in the order of their bone index.
 * The order of the bones in the array must follow the order in the skeleton for this pose.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The local bone matrix expresses the orientation and position of the bone relative
 * to it's parent. This function sets the local orientations of all the bones from an
 * array of quaternions. The position of the bones are unaffected.
 *
 * @see Pose::SetLocalRotation Pose::GetNumBones Skeleton::SetBoneAxis Pose::SetWorldRotations Pose::SetWorldMatrix
 */
void Pose::SetLocalRotations(const Quat* rots)
{
	if (m_NumBones == 0)
		return;
// TODO: if you have a lot of bones, this won't all fit in one buffer
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose,  POSE_SetLocalRotations) << this << m_NumBones;
		s->Output((float*) rots, m_NumBones * sizeof(Quat) / sizeof(float));
	VX_STREAM_END(  )

	VX_TRACE2(Debug, ("Pose::SetLocalRotations %s\n", GetName()));
	for (int i = 0; i < m_NumBones; ++i)
	{
		if (!m_Skeleton.IsNull() && m_Skeleton->IsLocked(i))
			continue;
		Quat	q(rots[i]);
		Bone&	bone(m_Bones[i]);

		q.Normalize();
		if (bone.LocalRot == q)
			return;
		bone.LocalRot = q;
		VX_TRACE2(Debug, ("\t%s (%.3f, %.3f, %.3f, %.3f)\n",
				 m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w));
		if (m_CoordSpace == BIND_POSE_RELATIVE)
			bone.WorldRot = q;
		bone.Changed |= Bone::LOCAL_ROT;
	}
	SetChanged(true);
	m_NeedSync = true;
}


/*!
 * @fn void Pose::GetLocalMatrix(int boneindex, Matrix& mtx) const
 * @param boneindex	zero based index of bone to get matrix for
 * @param mtx		where to store bone matrix
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The local bone matrix expresses the orientation and position of the bone relative
 * to the parent bone.
 *
 * @return local matrix for the designated bone.
 *
 * @see  Pose::GetWorldRotation Pose::GetLocalRotation Pose::SetLocalMatrix  Skeleton::SetBoneAxis
 */
void Pose::GetLocalMatrix(int boneindex, Matrix& mtx) const
{
	ObjectLock	lock(this);
	VX_ASSERT((boneindex >= 0) && (boneindex < m_NumBones));
	m_Bones[boneindex].GetLocalMatrix(mtx);
}

/*!
 * @fn void Pose::GetWorldMatrix(int boneindex, Matrix& mtx) const
 * @param boneindex	zero based index of bone to get matrix for
 * @param mtx		where to store bone matrix
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton.
 *
 * @return world matrix for the designated bone.
 *
 * @see  Pose::GetWorldRotation Pose::GetLocalRotation Pose::SetWorldMatrix  Skeleton::SetBoneAxis
 */
void Pose::GetWorldMatrix(int boneindex, Matrix& mtx) const
{
	ObjectLock	lock(this);
	VX_ASSERT((boneindex >= 0) && (boneindex < m_NumBones));
	Bone& bone = m_Bones[boneindex];
	float tiny = VX_SMALL_NUMBER;

	if (bone.WorldRot.IsEmpty() &&
		(fabs(bone.WorldPos.x) < tiny) &&
		(fabs(bone.WorldPos.y) < tiny) &&
		(fabs(bone.WorldPos.z) < tiny))
		mtx.Identity();
	else
		bone.GetWorldMatrix(mtx);
}

/*!
 * @fn void Pose::SetWorldRotation(int boneindex, const Quat& q)
 * @param boneindex	zero based index of bone to set matrix for
 * @param q			new bone world rotation
 *
 * Sets the world rotation for the designated bone.
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone rotation expresses the orientation of the bone relative
 * to the root of the skeleton.
 *
 * This function recomputes the local bone rotation and the world bone position.
 *
 * @see Pose::GetWorldRotation Pose::SetLocalRotation Pose::GetWorldMatrix Pose::GetWorldPositions
 */
void Pose::SetWorldRotation(int boneindex, const Quat& newrot)
{
	VX_ASSERT((boneindex >= 0) && (boneindex < GetNumBones()));
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose, POSE_SetWorldRotation) << this << boneindex << newrot;
	VX_STREAM_END(  )

	Bone&	bone = m_Bones[boneindex];
	int		parentid = bone.ParentID;
	Quat	q(newrot);	

	q.Normalize();
	if (bone.WorldRot == q)
		return;
	if (!m_Skeleton.IsNull() && m_Skeleton->IsLocked(boneindex))
		return;
	bone.WorldRot = q;
	VX_TRACE2(Debug, ("Pose::SetWorldRotation %s %s (%.3f, %.3f, %.3f, %.3f))\n",
			 GetName(), m_Skeleton->GetBoneName(boneindex), q.x, q.y, q.z, q.w));
	if (m_CoordSpace == BIND_POSE_RELATIVE)
	{
		bone.Changed |= Bone::LOCAL_ROT;
		bone.LocalRot = bone.WorldRot;
		return;
	}
	/*
	 * Indicate world matrix has changed for this bone
	 */
	bone.Changed |= Bone::WORLD_ROT;
	m_NeedSync = true;
	SetChanged(true);
	if (parentid < 0)
		bone.LocalRot = q;				// save new world rotation
}


/*!
 * @fn void Pose::SetWorldMatrix(int boneindex, const Matrix& mtx)
 * @param boneindex	zero based index of bone to set matrix for
 * @param mtx		new bone matrix
 *
 * Sets the world matrix for the designated bone.
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton.
 *
 * @see Pose::GetWorldRotation Pose::SetLocalRotation Pose::GetWorldMatrix Pose::GetWorldPositions Skeleton::SetBoneAxis 
 */
void Pose::SetWorldMatrix(int boneindex, const Matrix& mtx)
{
	VX_ASSERT((boneindex >= 0) && (boneindex < GetNumBones()));
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose, POSE_SetWorldMatrix) << this << boneindex << &mtx;
	VX_STREAM_END(  )

	Bone&	bone = m_Bones[boneindex];
	int		parentid = bone.ParentID;
	Vec3	p;
	Quat	rot(mtx);

	SetChanged(true);
	rot.Normalize();
	mtx.GetTranslation(p);
#if defined(_DEBUG) || (_TRACE > 1) 
	if (Debug > 1)
	{
		bool rchanged = (rot.Distance(bone.WorldRot) > QUAT_TOLERANCE);
		bool pchanged = (p.Distance(bone.WorldPos) > QUAT_TOLERANCE);
		if (pchanged | rchanged)
			VX_PRINTF(("Pose::SetWorldMatrix %s %s\n", GetName(), m_Skeleton->GetBoneName(boneindex)));
		if (pchanged)
			VX_PRINTF(("\t\twpos (%f, %f, %f)\n", p.x, p.y, p.z));
		if (rchanged)
			VX_PRINTF(("\t\twrot (%.3f, %.3f, %.3f, %.3f)\n", rot.x, rot.y, rot.z, rot.w));
	}
#endif
	bone.WorldRot = rot;
	bone.WorldPos = p;
	if (parentid < 0)		// calculate new local matrix
	{
		bone.LocalRot = bone.WorldRot;
		bone.LocalPos = bone.WorldPos;
	}
	bone.Changed |= Bone::WORLD_POS | Bone::WORLD_ROT;
	m_NeedSync = true;
}


/*!
 * @fn void Pose::GetWorldRotations(Quat* dest) const
 * @param dest	destination array to get world space joint rotations
 *
 * The world space joint rotations for each bone are copied into the
 * destination array as quaterions in the order of their bone index.
 * The array must be as large as the number of bones in the skeleton
 * (which can be obtained by calling GetNumBones).
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton. This function returns the world space bone rotations
 * as an array of quaternions.
 *
 * @see Pose::SetWorldRotatons Pose::GetWorldRotation Pose::GetWorldMatrix Pose::GetNumBones Skeleton::SetBoneAxis
 */
void Pose::GetWorldRotations(Quat* dest) const
{
	if (dest == NULL)
		return;
	ObjectLock	lock(this);
	Sync();
	for (int i = 0; i < m_NumBones; ++i)
		*dest++ = m_Bones[i].WorldRot;
}


/*!
 * @fn void Pose::SetWorldRotations(const Quat* quat)
 * @param quat		array of quaternions with the rotations in world coordinates
 *
 * The world space joint rotations for each bone are copied from the
 * source array of quaterions in the order of their bone index.
 * The order of the bones in the array must follow the order in the skeleton for this pose.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton. This function sets the world space bone rotations
 * from an array of quaternions.The bone positions are unaffected.
 *
 * @see Pose::SetLocalRotations Pose::SetWorldMatrix Pose::GetWorldRotations Pose::SetWorldPositions Skeleton::SetBoneAxis Pose::GetNumBones
 */
void Pose::SetWorldRotations(const Quat* rots)
{
	VX_TRACE(Debug, ("Pose::SetWorldRotations %s\n", GetName()));
	for (int i = 0; i < m_NumBones; ++i)
	{
		Bone&	bone(m_Bones[i]);
		Quat	q(rots[i]);

		q.Normalize();
		// WorldRot = WorldRot(par) * LocalRot
		// LocalRot = INVERSE[ WorldRot(par) ] * WorldRot
		if (bone.WorldRot == q)
			continue;
		bone.WorldRot = q;
		VX_TRACE2(Debug, ("\t%s wrot (%.3f, %.3f, %.3f, %.3f)\n",
			 m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w));
		if (m_CoordSpace == BIND_POSE_RELATIVE)
		{
			bone.LocalRot = q;
			bone.Changed |= Bone::LOCAL_ROT;
		}
		else
		{
			m_NeedSync = true;
			bone.Changed |= Bone::WORLD_ROT;
			if (bone.ParentID < 0)
				bone.LocalRot = q;
		}
	}
}

/*!
 * @fn void Pose::GetWorldPositions(Vec3* dest) const
 * @param dest	destination array to get world space joint positions
 *
 * The world space joint positions for each bone are copied into the
 * destination array as vectors in the order of their bone index.
 * The array must be as large as the number of bones in the skeleton
 * (which can be obtained by calling GetNumBones).
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton. This function returns the world space bone positions
 * as an array of vectors.
 *
 * @see Pose::SetWorldRotations Pose::SetWorldMatrix Pose::SetWorldPositions
 */
void Pose::GetWorldPositions(Vec3* dest) const
{
	if (dest == NULL)
		return;
	ObjectLock	lock(this);
	for (int i = 0; i < m_NumBones; ++i)
		*dest++ = m_Bones[i].WorldPos;
}

/*!
 * @fn void Pose::SetWorldPositions(const Vec3* vecs)
 * @param vecs	array of vectors with the positions in world coordinates
 *
 * The world space joint positions for each bone are copied from the
 * source array of vectors in the order of their bone index.
 * The array must be as large as the number of bones in the skeleton
 * (which can be obtained by calling GetNumBones).
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton. This function sets the world space bone positions
 * from an array of vectors. The bone orientations are unaffected and it is up to the
 * caller to make sure these positions are compatible with the current pose rotations.
 *
 * @see Pose::SetWorldRotations Pose::SetWorldMatrix Pose::GetWorldPositions
 */
void Pose::SetWorldPositions(const Vec3* vecs)
{
	if (m_NumBones == 0)
		return;
// TODO: if you have a lot of bones, this won't all fit in one buffer
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose,  POSE_SetWorldPositions) << this;
		s->Output((float*) vecs, m_NumBones * sizeof(Vec3) / sizeof(float));
	VX_STREAM_END(  )

	VX_ASSERT(m_CoordSpace != BIND_POSE_RELATIVE);
	VX_TRACE2(Debug, ("Pose::SetWorldPositions %s\n", GetName()));
	bool changed = false;
	for (int i = 0; i < m_NumBones; ++i)
	{
		Vec3	p(vecs[i]);
		Bone&	bone(m_Bones[i]);

		VX_TRACE2(Debug, ("\t%s wpos (%.3f, %.3f, %.3f)\n", m_Skeleton->GetBoneName(i), p.x, p.y, p.z));
		if (bone.WorldPos != p)
		{
			bone.WorldPos = p;
			CalcLocal(bone);
			changed = true;
		}
	}
	if (changed)
		SetChanged(true);
}

/*!
 * @fn void Pose::SetPosition(const Vec3& pos)
 * @param pos	new world position of root bone
 *
 * Sets the world position of the root bone and propagates to all children.
 * This has the effect of moving the overall skeleton to a new position
 * without affecting the orientation of it's bones.
 *
 * @see Pose::SetWorldPositions Pose::GetWorldPosition
 */
void Pose::SetPosition(const Vec3& pos)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Pose, POSE_SetPosition) << this << pos;
	VX_STREAM_END(  )

	VX_TRACE2(Debug, ("Pose::SetPosition %s %0.3f, %0.3f, %0.3f\n", GetName(), pos.x, pos.y, pos.z));
	Vec3	diff(pos - m_Bones[0].WorldPos);	// how much has root bone moved?
	if (diff.LengthSquared() <= VX_EPSILON)
		return;
	Sync();
	m_Bones[0].LocalPos = pos;
	for (int i = 0; i < m_NumBones; ++i)
		m_Bones[i].WorldPos += diff;		// move all the bones by the same amount
	SetChanged(true);
}

void Pose::SetCoordSpace(int space)
{
	VX_ASSERT((space >= WORLD) && (space <= SKELETON));
	m_CoordSpace = space;
}

void Pose::SetLocalPosition(const Vec3& pos)
{
	Bone&	bone(m_Bones[0]);

	bone.WorldPos = pos;
	bone.LocalPos = pos;
	VX_TRACE2(Debug, ("\t%s wpos (%.3f, %.3f, %.3f)\n", m_Skeleton->GetBoneName(0),
			bone.WorldPos.x, bone.WorldPos.y, bone.WorldPos.z));
	SetChanged(true);
}

bool Pose::Do(Messenger& s, int op)
{
	int			n = m_NumBones;
	Vec3		v;
	Quat		q;
	Quat*		rotations;
	Vec3*		positions;
	SharedObj*	obj;

	switch (op)
	{
		case POSE_SetCoordSpace:
		s >> n;
		m_CoordSpace = n;
		break;

		case POSE_SetPosition:
		s >> v;
		SetPosition(v);
		break;

		case POSE_SetSkeleton:
		s >> obj;
		SetSkeleton((Skeleton*) obj);
		break;

		case POSE_SetLocalRotation:
		s >> n;
		s >> q;
		SetLocalRotation(n, q);
		break;

		case POSE_SetLocalRotations:
		s >> n;
		rotations = (Quat*) alloca(sizeof(Quat) * n);
		s.Input((float*) rotations, n * sizeof(Quat) / sizeof(float));
		SetLocalRotations(rotations);
		break;

		case POSE_SetWorldRotations:
		s >> n;
		rotations = (Quat*) alloca(sizeof(Quat) * n);
		s.Input((float*) rotations, n * sizeof(Quat) / sizeof(float));
		SetWorldRotations(rotations);
		break;

		case POSE_SetWorldPositions:
		s >> n;
		positions = (Vec3*) alloca(sizeof(Vec3) * n);
		s.Input((float*) positions, n * sizeof(Vec3) / sizeof(float));
		SetWorldPositions(positions);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Pose::DoNames[op - POSE_SetSkeleton]
					   << " " << this);
#endif
	return true;
}

DebugOut& Pose::Print(DebugOut& dbg, int opts) const
{
	int32	n = m_NumBones;
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "<attr name='Skeleton'>" << ((SharedObj*) m_Skeleton) << "</attr>");
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='Space'>" << m_CoordSpace << "</attr>");
		endl(dbg << "<attr name='WorldPositions'>");
		for (int i = 0; i < n; ++i)
			endl(dbg << m_Bones[i].WorldPos);
		endl(dbg << "</attr>");
		endl(dbg << "<attr name='WorldRotations'>");
		for (int i = 0; i < n; ++i)
			endl(dbg << m_Bones[i].WorldRot);
		endl(dbg << "</attr>");
	}
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}

bool Pose::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!SharedObj::Copy(src_obj))
		return false;

	const Pose* src = (const Pose*) src_obj;

	if (src->IsClass(VX_Pose))
	{
		m_CoordSpace = src->m_CoordSpace;
		m_NumBones = src->m_NumBones;
		m_NeedSync = src->m_NeedSync;
		m_Bones.SetSize(m_NumBones);
		for (int i = 0; i < m_NumBones; ++i)
			m_Bones[i] = src->m_Bones[i];
		SetSkeleton(src->m_Skeleton);
		SetChanged(true);
	}
	return true;
}

int Pose::Save(Messenger& s, int opts) const
{
	int32		h;
	int32		n = m_NumBones;
	Quat*		rot;
	Vec3*		pos;
	SharedObj*	skel = m_Skeleton;

	h = SharedObj::Save(s, opts);
	if (h <= 0)
		return h;
	if (skel && (skel->Save(s, opts) >= 0) && h)
		s << OP(VX_Pose, POSE_SetSkeleton) << h << skel;
	if (n > 0)
	{
		int32 cmdsize = 4 * sizeof(int32);
		int32 maxbytes = (BufMessenger::MaxBufSize - cmdsize);

		rot = (Quat*) alloca(sizeof(Quat) * n);
		pos = (Vec3*) alloca(sizeof(Vec3) * n);
		Sync();
		GetWorldRotations(rot);
		GetWorldPositions(pos);
		s << OP(VX_Pose, POSE_SetCoordSpace) << h << m_CoordSpace;
		s << OP(VX_Pose, POSE_SetWorldRotations) << h << n;
		VX_ASSERT(maxbytes >= (n * sizeof(Quat)));
		s.Output((float*) rot, n * sizeof(Quat) / sizeof(float));
		s << OP(VX_Pose, POSE_SetWorldPositions) << h << n;
		s.Output((float*) pos, n * sizeof(Vec3) / sizeof(float));
	}
	return h;
}


/*!
 * @fn void Pose::Sync()
 * Synchronize world rotations and local rotations.
 * Positions are unaffected.
 */
bool Pose::Sync() const
{
	if (!m_NeedSync)
		return false;
	m_NeedSync = false;
	if (m_CoordSpace == BIND_POSE_RELATIVE)
		return false;
	VX_TRACE2(Debug, ("Pose::Sync %s\n", GetName()));

	for (int i = 0; i < m_NumBones; ++i)
	{
		Bone&	bone = m_Bones[i];
		int		pid = bone.ParentID;
		bool	update;
		Quat	q;
		Vec3	p;

		if (pid < 0)								// root bone?
			continue;
		update = (m_Bones[pid].Changed & (Bone::WORLD_ROT | Bone::LOCAL_ROT)) != 0;
		if (!m_Skeleton->IsLocked(i))				// bone not locked?
		{
			if (bone.Changed & Bone::WORLD_POS)	// world matrix changed?
			{
				q = bone.WorldRot;
				p = bone.WorldPos;
				VX_TRACE2(Debug, ("\t%s wrot (%0.3f, %0.3f, %0.3f, %0.3f) wpos (%0.3f, %0.3f, %0.3f)\n",
							m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w, p.x, p.y, p.z));
				CalcLocal(bone);					// calculate local rotation and position
				continue;
			}
			if (bone.Changed & Bone::WORLD_ROT)
			{										// world rotation changed?
				q = bone.WorldRot;
				VX_TRACE2(Debug, ("\t%s wrot (%0.3f, %0.3f, %0.3f, %0.3f)\n", m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w));
				CalcHybrid(bone);					// calculate local rotation, world position
				continue;
			}
		}
		if (update ||								// use local pos & rot?
			(bone.Changed & (Bone::LOCAL_ROT | Bone::WORLD_ROT)))
		{
			q = bone.LocalRot;
			VX_TRACE2(Debug, ("\t%s lrot (%0.3f, %0.3f, %0.3f, %0.3f)\n", m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w));
			bone.Changed = Bone::LOCAL_ROT;
			CalcWorld(bone);						// update world rotation & position
		}
	}
	for (int i = 0; i < m_NumBones; ++i)
		m_Bones[i].Changed = 0;
	return true;
}

/*!
 * void Pose::CalcLocal(Bone& bone)
 * @param bone	bone to update
 *
 * Calculates the local translation and rotation for a bone.
 * Assumes WorldRot and WorldPos have been calculated for the bone.
 */
void Pose::CalcLocal(Bone& bone) const
{
	Matrix	locmtx, mtx;
	Quat	q;
	Vec3	lp;

	if (bone.ParentID < 0)
	{
		bone.LocalPos = bone.WorldPos;
		bone.LocalRot = bone.WorldRot;
		return;
	}
	/*
	 * WorldMatrix = WorldMatrix(parent) * LocalMatrix
	 * LocalMatrix = INVERSE[ WorldMatrix(parent) ] * WorldMatrix
	 */
	GetWorldMatrix(bone.ParentID, locmtx);	// WorldMatrix(par)
	locmtx.Invert();						// INVERSE[ WorldMatrix(parent) ]
	bone.GetWorldMatrix(mtx);
	locmtx *= mtx;							// INVERSE[ WorldMatrix(parent) ] * WorldMatrix
	locmtx.GetTranslation(lp);
	q.Set(locmtx);
	q.Normalize();
	if (q.DistanceSquared(bone.LocalRot))
	{
		VX_TRACE2(Debug, ("\t\tlrot (%.3f, %.3f, %.3f, %.3f)\n", q.x, q.y, q.z, q.w));
		bone.LocalRot = q;						// save local rotation
	}
	if (lp.DistanceSquared(bone.LocalPos))
	{
		VX_TRACE2(Debug, ("\t\tlpos (%.3f, %.3f, %.3f)\n", lp.x, lp.y, lp.z));
		bone.LocalPos = lp;						// save local position
	}
}

/*!
 * void Pose::CalcHybrid(Bone& bone)
 * @param bone	bone to update
 *
 * Calculates the local rotation and world translation for a bone.
 * Assumes WorldRot and WorldPos have been calculated for the bone.
 */
void Pose::CalcHybrid(Bone& bone) const
{
	Matrix	tmp1, tmp2;
	Quat	q1, q2;
	Vec3	p;

	if (bone.ParentID < 0)
	{
		bone.LocalPos = bone.WorldPos;
		bone.LocalRot = bone.WorldRot;
		return;
	}
/*
	WorldPos * WorldRot = WorldMatrix(par) * LocalPos * LocalRot
	WorldRot = WorldRot(parent) * LocalRot
	LocalRot = INVERSE[ WorldRot(par) ] * WorldRot
	WorldPos * WorldRot(par) = WorldMatrix(par) * LocalPos
	WorldPos = WorldMatrix(par) * LocalPos * INVERSE[WorldRot(par)]
*/
#if 0
	q1 = GetWorldRotation(bone.ParentID);		// WorldRot(par)
	q1.Invert();								// INVERSE( WorldRot(par) ]
	q2.Mul(q1, bone.WorldRot);					// INVERSE( WorldRot(par) ] * WorldRot
	tmp2.RotationMatrix(q1);					// INVERSE( WorldRot(par) ]
#else
	GetWorldMatrix(bone.ParentID, tmp2);		// WorldMatrix(par)
	tmp2.Invert();								// INVERSE[ WorldMatrix(parent) ]
	bone.GetWorldMatrix(tmp1);
	tmp1.PreMul(tmp2);							// INVERSE[ WorldMatrix(parent) ] * WorldMatrix
	q2.Set(tmp1);
#endif
	q2.Normalize();
	bone.LocalRot = q2;							// save local rotation
	GetWorldMatrix(bone.ParentID, tmp1);		// WorldMatrix(par)
	tmp2.SetTranslation(bone.LocalPos);			// LocalPos * INVERSE( WorldRot(par) ]
	tmp1 *= tmp2;								// WorldMatrix(par) * LocalPos * INVERSE( WorldRot(par) ]
	tmp1.GetTranslation(p);						// WorldPos
	bone.WorldPos = p;
#ifdef _DEBUG
	if (q2.DistanceSquared(bone.LocalRot) > QUAT_TOLERANCE)
	{	VX_TRACE2(Debug, ("\t\tlrot (%.3f, %.3f, %.3f, %.3f)\n", q2.x, q2.y, q2.z, q2.w)); }
	if (p.DistanceSquared(bone.WorldPos) > QUAT_TOLERANCE)
	{	VX_TRACE2(Debug, ("\t\twpos (%.3f, %.3f, %.3f)\n", p.x, p.y, p.z)); }
#endif
}


/*!
 * void Pose::CalcWorld(Bone& bone)
 * @param bone	bone to update
 *
 * Calculates the world position and rotation based on the
 * local position and rotation (WorldRot, WorldPos from LocalRot, LocalPos)
 */
void Pose::CalcWorld(Bone& bone) const
{
	Matrix	locmtx, mtx;
	Quat	q;
	Vec3	wp;

	GetWorldMatrix(bone.ParentID, mtx);	// WorldMatrix(parent)
	bone.GetLocalMatrix(locmtx);
	mtx *= locmtx;						// WorldMatrix = WorldMatrix(parent) * LocalMatrix
	mtx.GetTranslation(wp);
	q = mtx;
	q.Normalize();
	if (q.DistanceSquared(bone.WorldRot) > QUAT_TOLERANCE)
	{
		VX_TRACE2(Debug, ("\t\twrot (%.3f, %.3f, %.3f, %.3f)\n", q.x, q.y, q.z, q.w));
		bone.WorldRot = q;					// save new world rotation
	}
	if (wp.DistanceSquared(bone.WorldPos) > QUAT_TOLERANCE)
	{
		VX_TRACE2(Debug, ("\t\twpos (%.3f, %.3f, %.3f)\n", wp.x, wp.y, wp.z));
		bone.WorldPos = wp;					// save new world position
	}
}



bool Pose::Compare(const Pose& src) const
{
	int		numbones = m_NumBones;
	bool	same = true;

	if (numbones != src.GetNumBones())
		return false;
	Sync();
	for (int i = 0; i < numbones; ++i)
	{
		const Bone& srcbone = src.GetBone(i);
		const Bone&	dstbone = m_Bones[i];
		Vec3		p(dstbone.WorldPos);
		Quat		q(dstbone.WorldRot);
		float		d = p.DistanceSquared(srcbone.WorldPos);
	
		if (d > QUAT_TOLERANCE)
		{
#ifdef _DEBUG
			if (Debug > 1)
				endl(vixen_debug << "Pose::Compare " << m_Skeleton->GetBoneName(i) << " world position " <<
					 p << " != " << srcbone.WorldPos);
#endif
			same = false;
		}
		p = dstbone.LocalPos;
		d = p.DistanceSquared(srcbone.WorldPos);	
		if (d > QUAT_TOLERANCE)
		{
#ifdef _DEBUG
			if (Debug > 1)
				endl(vixen_debug << "Pose::Compare " << m_Skeleton->GetBoneName(i) << " local position " <<
					 p << " != " << srcbone.WorldPos);
#endif
			same = false;
		}
		d = q.DistanceSquared(srcbone.WorldRot);
		if (d > QUAT_TOLERANCE)
		{
#ifdef _DEBUG
			if (Debug > 1)
				endl(vixen_debug << "Pose::Compare " << m_Skeleton->GetBoneName(i) << " world rotation " <<
					 q << " != " << srcbone.WorldRot);
#endif
			same = false;
		}
		q = dstbone.LocalRot;
		d = q.DistanceSquared(srcbone.LocalRot);
		if (d > QUAT_TOLERANCE)
		{
#ifdef _DEBUG
			if (Debug > 1)
				endl(vixen_debug << "Pose::Compare " << m_Skeleton->GetBoneName(i) << " local rotation " <<
					 q << " != " << srcbone.LocalRot);
#endif
			same = false;
		}
	}
	return same;
}

}	// end Vixen
