/*!
 * @file vxpose.h
 * @brief Describes a set of transformations on a skeleton.
 *
 * Contains the matrices for each bone in the skeleton, designating a specific pose.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxdeformer.h vxbone.h vxmorph.h
 */
#pragma once

namespace Vixen {

class Skeleton;

/*!
 * @class Pose
 * @brief Set of transformations on the bones of a skeleton.
 *
 * A pose is associated with a specific skeleton.
 * It contains a matrix for every bone in that skeleton.
 * You can view a pose in local space where each bone matrix is relative to its parent bone.
 * You can also view it in world space where each bone matrix gives the world space joint orientation
 * and the world space joint position.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * Usually the bones are in a hierarchy and transformations on a parent bone apply to the
 * child bones as well.
 *
 * Each skeleton has a current pose. Often the current pose of a skeleton is used to
 * drive a skinned animation.
 *
 * @see Transformer Skeleton Skin BodyTracker HavokPose
 * @ingroup vixen
 */
class Pose : public SharedObj
{
public:
	friend class Skeleton;
	friend class RagDoll;

	enum
	{
		WORLD = 1,				// pose angles are relative to world coordinate system
		BIND_POSE_RELATIVE = 2,	// pose angles are relative to the skeleton bind pose
		SKELETON = 4,			// pose angles are relative to the root joint of the skeleton
		BIND_POSE = 5,			// pose angles represent the bind pose
	};

	VX_DECLARE_CLASS(Pose);
	Pose(const Skeleton* skel = NULL, int space = SKELETON);

	int			GetNumBones() const;						//!< Get number of bones in skeleton.
	int			GetCoordSpace() const { return m_CoordSpace; }	//!< Get angle coordinate space.
	Skeleton*	GetSkeleton() const;						//!< Get skeleton this pose belongs to.
	void		ClearRotations();							//!< set all the rotations in a pose to empty.
	const Vec3&	GetWorldPosition(int i) const;				//!< Get world space position of a bone.
	void		GetWorldPositions(Vec3* positions) const;	//!< Get world space positions for all bones.
	void		SetWorldPositions(const Vec3* positions);	//!< Set world space positions for all bones.
	void		SetWorldRotations(const Quat* rotations);	//!< Set world space rotations for all bones.
	void		GetWorldMatrix(int i, Matrix& mtx) const;	//!< Get world matrix for a bone.
	void		SetWorldMatrix(int i, const Matrix& mtx);	//!< Set world matrix for a bone.
	void		GetWorldRotations(Quat* rotations) const;	//!< Get world space rotations for all bones.
	const Quat&	GetWorldRotation(int i) const;				//!< Get world space rotation for a bone.
	void		SetWorldRotation(int i, const Quat& rot);	//!< Set world rotation for a bone.
	void		GetLocalMatrix(int i, Matrix& mtx) const;	//!< Get local matrix for a bone.
	void		SetLocalRotations(const Quat* rotations);	//!< Set local rotations for all the bones.
	const Quat&	GetLocalRotation(int i) const;				//!< Get local space rotation for a bone.
	void		SetLocalRotation(int i, const Quat& rot);	//!< Set local rotation for a bone.
	const Vec3&	GetLocalPosition(int i) const;				//!< Get local space position of a bone.
	bool		Compare(const Pose& src) const;				//!< Compare two poses

	virtual void		SetPosition(const Vec3&);			//!< Set the world position of root and propagate to descendants.
	virtual bool		Sync() const;
	virtual int			Save(Messenger&, int) const;
	virtual bool		Do(Messenger& s, int op);
	virtual bool		Copy(const SharedObj*);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		POSE_SetSkeleton = SharedObj::OBJ_NextOp,
		POSE_SetWorldRotations,
		POSE_SetLocalRotations,
		POSE_SetLocalRotation,
		POSE_SetWorldPositions,
		POSE_SetWorldMatrix,
		POSE_SetWorldPosition,
		POSE_SetWorldRotation,
		POSE_Concat,
		POSE_SetCoordSpace,
		POSE_SetPosition,
		POSE_ClearRotations,
		POSE_Next = SharedObj::OBJ_NextOp + 20
	};

protected:
	/*!
	 * Internal structure used to maintain information about each bone.
	 */
	class Bone
	{
	public:
		int		ParentID;	//! index of parent bone
		int		Changed;	//! WORLD_ROT, LOCAL_ROT, WORLD_POS
		Quat	LocalRot;	//! local rotation
		Quat	WorldRot;	//! world rotation
		Vec3	WorldPos;	//! world position
		Vec3	LocalPos;	//! local position

		enum ChangeFlags
		{
			LOCAL_ROT = 1,
			WORLD_ROT = 2,
			WORLD_POS = 4
		};

		Bone() : ParentID(-1),
				LocalRot(0.0f, 0.0f, 0.0f, 1.0f),
				WorldRot(0.0f, 0.0f, 0.0f, 1.0f),
				LocalPos(0.0f, 0.0f, 0.0f),
				WorldPos(0.0f, 0.0f, 0.0f),
				Changed(0)
				{ }

		Bone(const Bone& src)
		{
			ParentID = src.ParentID;
			LocalRot = src.LocalRot;
			LocalPos = src.LocalPos;
			WorldRot = src.WorldRot;
			WorldPos = src.WorldPos;
			Changed = src.Changed;
		}
		Bone& operator=(const Bone& src)
		{
			ParentID = src.ParentID;
			LocalRot = src.LocalRot;
			WorldRot = src.WorldRot;
			WorldPos = src.WorldPos;
			LocalPos = src.LocalPos;
			Changed = src.Changed;
			return *this;
		}

		void GetWorldMatrix(Matrix& mtx)
		{
			mtx.RotationMatrix(WorldRot);
			mtx.SetTranslation(WorldPos);
		}

		void GetLocalMatrix(Matrix& mtx)
		{
			mtx.RotationMatrix(LocalRot);
			mtx.SetTranslation(LocalPos);
		}

		bool operator==(const Bone& src)
		{
			if ((ParentID == src.ParentID) &&
				(LocalRot == src.LocalRot) &&
				(WorldRot = src.WorldRot) &&
				(WorldPos = src.WorldPos))
				return true;
			return false;
		}
	};

	void			SetCoordSpace(int s);			//!< Set angle coordinate space.
	void			SetSkeleton(const Skeleton*);	//!< Set associated skeleton.
	Bone&			GetBone(int boneindex)			{ return m_Bones[boneindex]; }
	const Bone&		GetBone(int boneindex) const	{ return m_Bones[boneindex]; }
	void			CalcWorld(Bone& bone) const;
	void			CalcLocal(Bone& bone) const;
	void			CalcHybrid(Bone& bone) const;
	void			SetLocalPosition(const Vec3&);		//!< Set the local position of root.

	mutable WeakRef<Skeleton>	m_Skeleton;
	int					m_NumBones;
	int					m_CoordSpace;
	mutable bool		m_NeedSync;
	mutable Core::BaseArray<Bone, Core::BaseObj > m_Bones;
};


FORCE_INLINE Skeleton* Pose::GetSkeleton() const
	{ return m_Skeleton;	}


/*!
 * @fn void Pose::GetNumBones() const
 * @return number of bones in the skeleton associated with this pose.
 * If there is no skeleton associated with the pose, 0 is returned.
 */
inline  int Pose::GetNumBones() const
{
	return m_NumBones;
}

/*!
 * @fn const Quat& Pose::GetLocalRotation(int boneindex)
 * @param boneindex	zero based index of bone whose rotation is wanted.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The local bone matrix expresses the orientation and position of the bone relative
 * to it's parent. This function returns the rotation component of that matrix as a quaternion.
 *
 * @return local rotation for the designated bone as a quaternion.
 *
 * @see Pose::SetLocalRotation Pose::SetWorldRotations Pose::SetWorldMatrix Skeleton::SetBoneAxis
 */
inline const Quat& Pose::GetLocalRotation(int boneindex) const
{
	VX_ASSERT((boneindex >= 0) && (boneindex < m_NumBones));
	return m_Bones[boneindex].LocalRot;
}

/*!
 * @fn const Vec3& Pose::GetLocalPosition(int boneindex)
 * @param boneindex	zero based index of bone whose position is wanted.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The local bone matrix expresses the orientation and position of the bone relative
 * to it's parent. This function returns the translation component of that matrix.
 *
 * @return local translation for the designated bone.
 *
 * @see Pose::SetLocalRotation Pose::SetWorldRotations Pose::SetWorldMatrix Skeleton::SetBoneAxis
 */
inline const Vec3& Pose::GetLocalPosition(int boneindex) const
{
	VX_ASSERT((boneindex >= 0) && (boneindex < m_NumBones));
	return m_Bones[boneindex].LocalPos;
}

/*!
 * @fn const Quat& Pose::GetWorldRotation(int boneindex)
 * @param boneindex	zero based index of bone whose rotation is wanted.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation of the bone relative
 * to the root bone of the skeleton.
 *
 * @return world rotation for the designated bone as a quaternion.
 *
 * @see Pose::SetWorldRotation Pose::SetWorldRotations Pose::SetWorldMatrix Skeleton::SetBoneAxis
 */
inline const Quat& Pose::GetWorldRotation(int boneindex) const
{
	VX_ASSERT((boneindex >= 0) && (boneindex < m_NumBones));
	return m_Bones[boneindex].WorldRot;
}


/*!
 * @fn  const Vec3& Pose::GetWorldPosition(int i)
 * @param i		index of bone whose position is wanted.
 *
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton with respect to this initial state.
 * The world bone matrix expresses the orientation and position of the bone relative
 * to the root of the skeleton. This function gets the world space position of the designated bone.
 *
 * @return world position for the given bone.
 *
 * @see Pose::SetWorldPositions
 */
inline const Vec3& Pose::GetWorldPosition(int i) const
{
	VX_ASSERT((i >= 0) && (i < m_NumBones));

	return m_Bones[i].WorldPos;
}

} // end Vixen



