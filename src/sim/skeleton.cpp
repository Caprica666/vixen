#include "vixen.h"


namespace Vixen {

VX_IMPLEMENT_CLASSID(Skeleton, Engine, VX_Skeleton);

static const TCHAR* opnames[] = {
		TEXT("Init"),
		TEXT("SetBoneName"),
		TEXT("SetBoneAxis"),
		TEXT("SetPose")
		TEXT("SetBindPose")
		TEXT("ApplyPose"),
		TEXT("SetBoneOptions"),
		TEXT("SetPosition"),
		TEXT("SetRootOffset"),
};

const TCHAR** Skeleton::DoNames = opnames;

const TCHAR* Skeleton::DefaultBoneNames[Skeleton::NUM_BONES] = {
	TEXT("Waist"),
	TEXT("Torso"),
	TEXT("Neck"),
	TEXT("Head"),
	TEXT("RightCollar"),
	TEXT("RightShoulder"),
	TEXT("RightElbow"),
	TEXT("RightWrist"),
	TEXT("RightHand"),
	TEXT("RightFinger"),
	TEXT("LeftCollar"),
	TEXT("LeftShoulder"),
	TEXT("LeftElbow"),
	TEXT("LeftWrist"),
	TEXT("LeftHand"),
	TEXT("LeftFinger"),
	TEXT("RightHip"),
	TEXT("RightKnee"),
	TEXT("RightAnkle"),
	TEXT("RightFoot"),
	TEXT("LeftHip"),
	TEXT("LeftKnee"),
	TEXT("LeftAnkle"),
	TEXT("LeftFoot"),
};

static int DefaultParents[Skeleton::NUM_BONES] = {
	-1,							// Waist
	Skeleton::WAIST,			// Torso
	Skeleton::TORSO,			// Neck
	Skeleton::NECK,				// Head
	Skeleton::TORSO,			// RightCollar
	Skeleton::RIGHT_COLLAR,		// RightShoulder
	Skeleton::RIGHT_SHOULDER,	// RightElbow
	Skeleton::RIGHT_ELBOW,		// RightWrist
	Skeleton::RIGHT_WRIST,		// RightHand
	Skeleton::RIGHT_HAND,		// RightFinger
	Skeleton::TORSO,			// LeftCollar
	Skeleton::LEFT_COLLAR,		// LeftShoulder
	Skeleton::LEFT_SHOULDER,	// LeftElbow
	Skeleton::LEFT_ELBOW,		// LeftWrist
	Skeleton::LEFT_WRIST,		// LeftHand
	Skeleton::LEFT_HAND,		// LeftFinger
	Skeleton::WAIST,			// RightHip
	Skeleton::RIGHT_HIP,		// RightKnee
	Skeleton::RIGHT_KNEE,		// RightAnkle
	Skeleton::RIGHT_ANKLE,		// RightFoot
	Skeleton::WAIST,			// LeftHip
	Skeleton::LEFT_HIP,			// LeftKnee
	Skeleton::LEFT_KNEE,		// LeftAnkle
	Skeleton::LEFT_ANKLE,		// LeftFoot
};


Skeleton::BoneArray::BoneArray(int numbones)
{
	SetSize(numbones);
}


/*!
 * @fn Skeleton::Skeleton(int numbones = Skeleton::NUM_BONES)
 *
 * Constructs a Skeleton animation engine with the given number of bones.
 * Initially the skeleton has no bind pose and the current pose contains
 * identity matrices. The number of bones in a skeleton is initialized
 * in the constructor and cannot be changed afterwards.
 *
 * @see Skeleton::GetNumBones Skeleton::SetNumBones Skeleton::SetBindPose Skeleton:SetTarget
 */
Skeleton::Skeleton(int numbones)
:	Engine(),
	m_BoneInfo(numbones),
	m_BoneAxis(0.0f, 0.0f, 1.0f),
	m_SkinPose(NULL, Pose::SKELETON),
	m_BindPose(NULL, Pose::SKELETON),
	m_RootOffset(0.0f, 0.0f, 0.0f)
{
	m_Bound.Empty();
	SetControl(Engine::CONTROL_CHILDREN);
	Init(numbones);
}

Skeleton::Skeleton(const Skeleton& src)
  :	Engine(),
	m_BoneInfo(src.GetNumBones()),
	m_SkinPose(NULL, Pose::SKELETON),
	m_BindPose(NULL, Pose::SKELETON),
	m_RootOffset(0.0f, 0.0f, 0.0f)
{
	m_Bound.Empty();
	SetControl(Engine::CONTROL_CHILDREN);
	Init(src.GetNumBones());
	Copy(&src);
}

/*!
 * @fn bool Skeleton::Init(int numbones, int32* parents)
 * @param numbones	number of bones in the skeleton
 * @param parents	array with parent indices for each bone
 *
 * Skeleton is completely generate and can be extended to any number of bones.
 * If you are using the "generic" skeleton with NUM_BONES bones, this
 * constructor will initialize the bone names for you.
 *
 * @see Skeleton::SetBindPose Skeleton::GetNumBones
 */
bool Skeleton::Init(int numbones, const int32* parents)
{
	m_BoneInfo.SetSize(numbones);
	if (parents)
		for (int i = 0; i < numbones; ++i)
			m_BoneInfo[i].ParentID = parents[i];
	m_SkinPose.SetSkeleton(this);
	m_BindPose.SetSkeleton(this);
	if ((numbones == NUM_BONES) &&
		m_BoneInfo[0].Name.IsEmpty())
	{
		for (int i = 0; i < NUM_BONES; ++i)
		{
			m_BoneInfo[i].Name = DefaultBoneNames[i];
			m_BoneInfo[i].ParentID = DefaultParents[i];
		}
	}
	if (m_Pose.IsNull())
		m_Pose =  MakePose(Pose::SKELETON);
	return true;
};

void Skeleton::Empty()
{
	m_Pose = NULL;
	Engine::Empty();
}

Pose*	Skeleton::MakePose(int coordspace) const
{
	Pose*	pose = new Pose(this, coordspace);
	int		nbones = GetNumBones();

	switch (coordspace)
	{
		case Pose::BIND_POSE_RELATIVE:
		pose->ClearRotations();
		break;

		case Pose::BIND_POSE:
		pose->Copy(&m_BindPose);
		break;

		default:
		if (!m_Pose.IsNull())
			pose->Copy(m_Pose);
		break;
	}
	return pose;
}


/*!
 * @fn int Skeleton::GetNumBones() const
 * Gets the number of bones in the skeleton.
 * The number of bones is established when the skeleton is first created.
 * The default number of bones is Skeleton::NUM_BONES
 *
 * Skeleton::Skeleton
 */
int Skeleton::GetNumBones() const { return (int) m_BoneInfo.GetSize(); }

/*!
 * @fn bool Skeleton::SetBoneParentIndex(int boneindex, int parentindex)
 * @param boneindex	index of bone whose parent to set.
 * @param parentindex index of parent bone.
 *
 * If either bone index is out of range or if the parent index is already set
 * this function fails.
 *
 * @return true if successful, else false
 *
 * @see Skeleton::GetParentBoneIndex Skeleton::GetBoneIndex
 */
bool Skeleton::SetParentBoneIndex(int boneindex, int parentindex)
{
	if ((boneindex < 0) || (boneindex >= GetNumBones()))
		return false;
	if ((parentindex < 0) || (parentindex >= GetNumBones()))
		return false;
	if (m_BoneInfo[boneindex].ParentID >= 0)
		return false;
	m_BoneInfo[boneindex].ParentID = parentindex;
	return true;
}

/*!
 * @fn void Skeleton::SetRootOffset(const Vec3& ofs)
 * @param ofs	vector with new root bone offset
 *
 * Sets the offset of the root bone when animated.
 * This offset is useful for moving the starting point of animations.
 * For example, you can make a zero-based animation relative to the
 * current position of the character by setting this offset to that position.
 * By default, this offset is zero.
 *
 * The root bone offset only affects animations. It does not affect any
 * updates to the current pose.
 *
 * @see Skeleton::GetRootOffset Transformer
 */
void Skeleton::SetRootOffset(const Vec3& ofs)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_SetRootOffset) << this << ofs;
	VX_STREAM_END()

	m_RootOffset = ofs;
}

const Vec3& Skeleton::GetRootOffset() const
{
	return m_RootOffset;
}


/*!
 * @fn void Skeleton::MakeBoneMap(const TCHAR** bonenames, int32* bonemap, int numbones)
 * @param bonenames	array of strings with names of bones to map
 * @param bonemap	integer array to get the indices of mapped bones
 * @param numbones	number of bones to map (size of bonenames and bonemap)
 *
 * Populates an array which maps a set of bone names to the bones in this skeleton.
 * Upon return, the bonemap array will contain an entry for each string in bonenames.
 * If a bone in the skeleton is found which contains the string, it's index is
 * stored. Otherwise the entry is -1.
 *
 * This function is useful with the BodyTracker, which needs to map the bones
 * of an arbitrary skeleton to it's internal skeleton.
 *
 * @see KinectTracker
 */
void Skeleton::MakeBoneMap(const TCHAR** bonenames, int32* bonemap, int numbones) const
{
	ObjectLock	lock(this);
	if (bonenames == NULL)
		bonenames = DefaultBoneNames;
	int skelbones = GetNumBones();
	for (int i = 0; i < numbones; ++i)
	{
		int	boneindex = -1;
		const TCHAR* searchfor = bonenames[i];

		bonemap[i] = -1;
		if (searchfor == NULL)
			continue;
		for (int b = 0; b < skelbones; ++b)
		{
			const TCHAR* bonename = GetBoneName(b);
			if (bonename && (Core::String(bonename).FindNoCase(searchfor) >= 0))
			{
				boneindex =  b;
				break;
			}
		}
		bonemap[i] = boneindex;
		if (boneindex < 0)
			{ VX_WARNING(("MakeBoneMap: cannot find bone %s\n", searchfor)); }
	}
}

/*!
 * @fn void Skeleton::SetPosition(const Vec3& pos)
 * @param pos	world space position of the body.
 *
 * The world space position of the skeleton is the translation of
 * the root bone (index 0) of the current pose. Setting the skeleton position
 * directly updates both the root bone position and the translation
 * on the target model of the skeleton.
 *
 * @see Model::SetTranslation Skeleton::SetTarget Pose::SetWorldPositions
 */
void Skeleton::SetPosition(const Vec3& p)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_SetPosition) << this << p;
	VX_STREAM_END(  )

	m_Pose->SetPosition(p);
}

/*!
 * @fn Vec3 Skeleton::GetPosition()
 *
 * The world space position of the skeleton is the translation of
 * the root bone (index 0) of the current pose. It should match the
 * translation on the target model of the skeleton (if any).
 *
 * @see Model::SetTranslation
 */
Vec3 Skeleton::GetPosition() const
{
	return m_Pose->GetWorldPosition(0);
}


/*!
 * @fn void Skeleton::SetBoneOptions(int boneindex, int boneopts)
 * @param boneindex	0 based bone index
 * @param boneopts	options to control how this bone moves
 *					BONE_PHYSICS will use physics (rigid body dynamics) to calculate
 *					the motion of the bone
 *					BONE_LOCK_ROTATION will lock the bone rotation, freezing its current
 *					local rotation
 *								
 * @see UnlockBone SetLocalRotation SetLocalRotations SetWorldRotation
 */
void	Skeleton::SetBoneOptions(int boneindex, int boneopts)
{
	if ((boneindex < 0) || (boneindex >= GetNumBones()))
		return;
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_SetBoneOptions) << this << int32(boneindex) << int32(boneopts);
	VX_STREAM_END(  )
	m_BoneInfo[boneindex].Options = boneopts;
}

int Skeleton::GetBoneOptions(int boneindex) const
{
	if ((boneindex < 0) || (boneindex >= GetNumBones()))
		return 0;
	return m_BoneInfo[boneindex].Options;
}

/*!
 * @fn bool Skeleton::IsLocked(int boneindex)
 * @param boneindex	0 based bone index
 *
 * Determines if the given bone is locked.
 *
 * @see UnlockBone SetLocalRotation LockBone IsLocked SetWorldRotation
 */
bool	Skeleton::IsLocked(int boneindex) const
{
	if ((boneindex < 0) || (boneindex >= GetNumBones()))
		return false;
	return m_BoneInfo[boneindex].Options &  BONE_LOCK_ROTATION;
}

/*!
 * @fn void Skeleton::SetPose(Pose* pose)
 * @param pose	Pose containing rotations and positions
 *
 * Sets the the current pose for this skeleton, making this skeleton the
 * owner of the pose. The pose can be shared by multiple engines which both read and write it.
 * The current pose contains the current orientation and position of the bones in the skeleton,
 * relative to bones at the origin along the bone axis (It is NOT relative to the bind pose).
 * When the current pose is updated, it causes the skeleton to modify the matrices
 * attached to the models in the target hierarchy and the bone matrices for each child Skin.
 *
 * @see Skeleton::GetPose Skeleton::RestoreBindPose Skeleton::SetBindPose Skin
 */
void Skeleton::SetPose(Pose* pose)
{
	const TCHAR*	name = GetName();
	Pose*			oldpose = m_Pose;

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_SetPose) << this << pose;
	VX_STREAM_END(  )
	VX_TRACE(Debug, ("Skeleton::SetPose %s\n", name));

	if (pose == NULL)
		return;
	if (name && (pose->GetName() == NULL))
		pose->SetName(Core::String(name) + TEXT(".curpose"));
	if (pose == oldpose)
		return;
	SetChanged(true);
	pose->SetSkeleton(this);	// pose references skeleton
	pose->Sync();
	m_Pose = pose;				// replace current pose
}

/*!
 * @fn void Skeleton::SetBindPose(Pose* pose)
 * @param pose	Pose containing rotations and positions for the bind pose
 *
 * Replaces the bind pose of the skeleton with the input Pose object.
 * The "bind pose" of the skeleton defines the neutral position of the bones before any
 * animations are applied. Usually it represents the pose that matches
 * the source vertices of the Skins driven by the skeleton.
 * You can restore the skeleton to it's bind pose with RestoreBindPose().
 *
 * The bind pose is maintained as a Pose object internal to the skeleton
 * and connot be shared across skeletons. Setting the bind pose copies the
 * value of the input pose into the skeleton's bind pose. Subsequent
 * updates to the input pose are not reflected in the skeleton's bind pose.
 * These semantics are different than elsewhere in the API.
 *
 * @see Skeleton::SetPose Skeleton::GetBindPose Skeleton::RestoreBindPose Pose
 */
void Skeleton::SetBindPose(Pose* pose)
{
	VX_ASSERT(pose != NULL);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_SetBindPose) << this << pose;
	VX_STREAM_END(  )

	VX_ASSERT(pose->GetCoordSpace() != Pose:: BIND_POSE_RELATIVE);
	m_BindPose.Copy(pose);
	UpdateBindPose();
	m_BindPose.SetChanged(false);
}

/*!
 * @fn void Skeleton::SetBindPose(const Quat* rotations, const Vec3* positions)
 * @param rotations	new bind pose rotations
 * @param positions new bind pose positions
 *
 * Updates the rotations and positions of the existing bind pose of this skeleton.
 * Both arrays must have enough entries for all the bones in the skeleton.
 * The bind pose" of the skeleton defines the neutral position of the bones before any
 * animations are applied. Usually it represents the pose that matches
 * the source vertices of the Skins driven by the skeleton.
 * You can restore the skeleton to it's bind pose with RestoreBindPose().
 *
 * @see Skeleton::SetPose Skeleton::GetBindPose Skeleton::RestoreBindPose Pose
 */
void Skeleton::SetBindPose(const Quat* rotations, const Vec3* positions)
{
	ObjectLock	lock(this);
	m_BindPose.SetWorldRotations(rotations);
	m_BindPose.SetWorldPositions(positions);
	m_BindPose.Sync();
	SetChanged(true);
	UpdateBindPose();
	m_BindPose.SetChanged(false);
}

/*!
 * @fn Skeleton::RestoreBindPose()
 * The bind pose is the default pose of the skeleton when not being animated.
 * It is initialized as the pose the skeleton is when it is first attached
 * to the Skeleton but you can change it with SetBindPose.
 *
 * @see Skeleton::SetBindPose Skeleton::GetBindPose Pose
 */
void Skeleton::RestoreBindPose()
{
	m_Pose->Copy(&m_BindPose);
	SetChanged(true);
}

/*!
 * @fn void Skeleton::ApplyPose(const Pose* newpose)
 * @param newpose	new pose to apply.
 *
 * Establishes the current pose of a skeleton - supports all pose coordinate spaces.
 * If a bone is not present in the hierarchy or skin being animated, it's input is ignored.
 * If a bone is locked, it's input is also ignored.
 *
 * The input rotations may be absolute world rotations relative to the bone axis or they can be
 * relative to the bind pose of the skeleton. If a transformation has been applied to the target model,
 * this function will reverse it before applying the pose. This function does not affect the bind pose.
 *
 * @see Skeleton::SetPose Skeleton::SetBindPose Pose
 */
void Skeleton::ApplyPose(const Pose* newpose)
{
	int				numbones = GetNumBones();
	Quat			root(0,0,0,1);
	Pose*			curpose = GetPose();
	int				space = newpose->GetCoordSpace();

	if ((numbones == 0) || (newpose == NULL))
		return;
	VX_ASSERT(curpose);
	VX_ASSERT(numbones == newpose->GetNumBones());
	VX_ASSERT(numbones == curpose->GetNumBones());
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton, SKELETON_ApplyPose) << this << newpose;
	VX_STREAM_END(  )

	ObjectLock	lock1(newpose);
	ObjectLock	lock2(curpose);

	if (space == Pose::BIND_POSE_RELATIVE)
	{
		VX_ASSERT(curpose != newpose);
		for (int i = 0; i < numbones; ++i)
		{
			const Pose::Bone&	bone = newpose->GetBone(i);
			Quat				wr(bone.WorldRot);

			if (bone.Changed == 0)
				continue;
			if (m_BoneInfo[i].Options & BONE_LOCK_ROTATION)
				continue;
			wr *= m_BindPose.GetWorldRotation(i);
			curpose->SetWorldRotation(i, wr);
		}
	}
	else if (space == Pose::WORLD)
	{
		/*
		 * Get the pose in the coordinate space of the skeleton
		 * by applying the inverse of the transform on the
		 * target hierarchy root.
		 */
		Model*		model = (Model*) GetTarget();
		if (model && model->IsClass(VX_Model))
		{
			Matrix mtx;

			model->TotalTransform(&mtx);
			root.Set(mtx);
			root.Conjugate();
			root.Normalize();
		}
		newpose->Sync();
		VX_ASSERT(curpose != newpose);
		for (int i = 0; i < numbones; ++i)
		{
			Quat	q(root);
			Quat	wr(newpose->GetWorldRotation(i));

			if ((m_BoneInfo[i].Options & BONE_LOCK_ROTATION) == 0)
			{
				q *= wr;
				curpose->SetWorldRotation(i, q);
			}
		}
	}
	else if (space & Pose::SKELETON)
	{
		if (curpose != newpose)
		{
			for (int i = 0; i < numbones; ++i)
			{
				Quat	q(newpose->GetWorldRotation(i));

				if ((m_BoneInfo[i].Options & BONE_LOCK_ROTATION) == 0)
					curpose->SetWorldRotation(i, q);	// including bind pose rotation
			}
		}
	}
	SetChanged(true);
}

/*!
 * @fn void Skeleton::SetBoneName(int boneindex, const TCHAR* bonename)
 * @param boneindex	zero based index of bone to rotate
 * @param bonename	string with name of bone
 *
 * Sets the name of the designated bone.
 *
 * @see Skeleton::GetBoneName Skeleton::Skeleton
 */
void Skeleton::SetBoneName(int boneindex, const TCHAR* bonename)
{
	if ((boneindex < 0) ||
		(boneindex >= GetNumBones()))
		VX_ERROR_RETURN(("Skeleton: ERROR bad bone index %d\n", boneindex));

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skeleton,  SKELETON_SetBoneName) << this << boneindex << bonename;
	VX_STREAM_END(  )

	if (bonename)
		m_BoneInfo[boneindex].Name = bonename;
	else
		m_BoneInfo[boneindex].Name.Empty();
}

/*!
 * @fn int Skeleton::GetBoneIndex(const TCHAR* bonename) const
 * @param bonename	string identifying the bone whose index you want
 *
 * @return 0 based bone index or -1 if bone with that name is not found.
 */
int Skeleton::GetBoneIndex(const TCHAR* bonename) const
{
	for (int i = 0; i < GetNumBones(); ++i)
		if (m_BoneInfo[i].Name.CompareNoCase(bonename) == 0)
			return i;
	return -1;
}

/*!
 * @fn int Skeleton::GetBoneParentIndex(int boneindex) const
 * @param boneindex	index of bone whose parent we need.
 *
 * @return 0 based parent bone index or -1 if no parent
 *
 * @see Skeleton::Init Skeleton::GetBoneIndex
 */
int Skeleton::GetParentBoneIndex(int boneindex) const
{
	if ((boneindex < 0) || (boneindex >= GetNumBones()))
		return -1;
	return m_BoneInfo[boneindex].ParentID;

}


/*!
 * @fn int Skeleton::GetBoneIndex(const Engine* bone) const
 * @param bone	Engine representing the bone you want the index for.
 *
 * Usually, bones are represented by Transformer engines.
 *
 * @return 0 based bone index or -1 if this engine is not a bone
 *
 * @see Skeleton::SetBoneName Skeleton::GetBone
 */
int Skeleton::GetBoneIndex(const Engine* bone) const
{
	for (int i = 0; i < GetNumBones(); ++i)
		if (m_BoneInfo[i].Bone == bone)
			return i;
	return -1;
}


bool Skeleton::Do(Messenger& s, int op)
{
	TCHAR		bonename[VX_MaxPath];
	int			n = GetNumBones();
	Vec3		v;
	Quat		q;
	SharedObj*	obj;
	ObjRef		ref;
	int32*		parentindices;

	switch (op)
	{
		case SKELETON_Init:
		s >> n;
		parentindices = (int32*) alloca(sizeof(int32) * n);
		s.Input(parentindices, n);
		Init(n, parentindices);
		break;

		case SKELETON_SetBoneName:
		s >> n;
		s >> bonename;
		SetBoneName(n, bonename);
		break;

		case SKELETON_SetPose:
		s >> obj;
		ref = obj;
		SetPose((Pose*) obj);
		break;

		case SKELETON_SetBindPose:
		s >> obj;
		ref = obj;
		SetBindPose((Pose*) obj);
		break;

		case SKELETON_SetBoneAxis:
		s >> v;
		SetBoneAxis(v);
		break;

		case SKELETON_SetRootOffset:
		s >> v;
		SetRootOffset(v);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Skeleton::DoNames[op - SKELETON_Init]
					   << " " << this);
#endif
	return true;
}

DebugOut& Skeleton::Print(DebugOut& dbg, int opts) const
{
	int32	n = GetNumBones();

	if ((opts & PRINT_Attributes) == 0)
		return Engine::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "<attr name='NumBones'>" <<n << "</attr>");
	endl(dbg << "<attr name='BoneAxis'>" << m_BoneAxis << "</attr>");
	if (!m_RootOffset.IsEmpty())
		endl(dbg << "<attr name='RootOffset'>" << m_RootOffset << "</attr>");
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='BindPose'>");
		m_BindPose.Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
		endl(dbg << "<attr name='Pose'>");
			m_Pose->Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	return Engine::Print(dbg, opts & PRINT_Trailer);
}

bool Skeleton::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;

	const Skeleton* src = (const Skeleton*) src_obj;
	if (src->IsClass(VX_Skeleton))
	{
		m_BoneAxis = src->m_BoneAxis;
		m_RootOffset = src->m_RootOffset;
		m_BindPose = src->m_BindPose;
		m_Pose->Copy(src->m_Pose);
		for (int i = 0; i < GetNumBones(); ++i)
			m_BoneInfo[i] = src->m_BoneInfo[i];
	}
	return true;
}

int Skeleton::Save(Messenger& s, int opts) const
{
	int32		h;
	int			n = GetNumBones();
	Skeleton*	cheat = (Skeleton*) this;

	h = Engine::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Skeleton, SKELETON_Init) << h << n;
	for (int i = 0; i < n; ++i)
		s << (int32) m_BoneInfo[i].ParentID;
	s << OP(VX_Skeleton, SKELETON_SetBoneAxis) << h << m_BoneAxis;
	if (!m_RootOffset.IsEmpty())
		s << OP(VX_Skeleton, SKELETON_SetRootOffset) << h << m_RootOffset;
	if (n > 0)
	{
		for (int i = 0; i < n; ++i)
			s << OP(VX_Skeleton, SKELETON_SetBoneName) << h << i << m_BoneInfo[i].Name;
		if ((m_BindPose.Save(s, opts) >= 0) && h)
			s << OP(VX_Skeleton, SKELETON_SetBindPose) << h << &m_BindPose;
		if (!m_Pose.IsNull() && (m_Pose->Save(s, opts) >= 0) && h)
			s << OP(VX_Skeleton, SKELETON_SetPose) << h << m_Pose;
	}
	return h;
}

bool Skeleton::OnEvent(Event* ev)
{
	LoadSceneEvent*	le = (LoadSceneEvent*) ev;
	Pose*		pose;
	Skeleton*	srcskel;

	if (ev->Code != Event::LOAD_SCENE)
		return false;
	pose = (Pose*) (SharedObj*) le->Object;
	if ((pose == NULL) || !pose->IsClass(VX_Pose))
		return false;
	VX_PRINTF(("Skeleton::OnEvent LOAD_SCENE %s\n", (const TCHAR*) le->FileName));
	if (pose->GetNumBones() != GetNumBones())
	{	VX_WARNING(("Skeleton::OnEvent BVH animation %s incompatible with Skeleton %s\n", (const TCHAR*) le->FileName, GetName())); }
	srcskel = pose->GetSkeleton();
	if (srcskel != this)
	{
		PoseMapper	mapper;
		mapper.SetSource(pose->GetSkeleton());
		mapper.SetTarget(this);
		mapper.MapLocalToTarget();
	}
	else
		SetPose(pose);
	return true;
}

/*
 * @fn void Skeleton::Compute(float t)
 * @param t	time
 *
 * Unlike most other engines, the Skeleton engine evaluates some of its children
 * before Skeleton::Eval and some of them after.
 * - Animation engines are evaluated first to compute the next animated position
 *	 for the skeleton bones (which are Transformer children of the Skeleton).
 * - Skeleton::Eval is then called to combine updates to the bones with updates
 *	 to the current pose.
 * - Then the bones are evaluated, calculating a final matrix for each bone.
 */
void Skeleton::Compute(float t)
{
	float		eval_t = ComputeTime(t);
	bool		do_eval = (eval_t >= 0);

	/*
	 * Evaluate child engines of all transformers below this skeleton.
	 * These are the interpolators that compute animation results.
	 */
	GroupIterNotSafe<Engine> iter(this, Group::DEPTH_FIRST);
	Engine*	g;

	while (g = iter.Next())
	{
		if (g->IsClass(VX_Transformer))
			g->ComputeChildren(t, -VX_Transformer);
	}
	/*
	 * Apply animation to the bones and calculate skin matrices
	 */
	if (do_eval)
		Eval(eval_t);
	/*
	 * Evaluate all child engines which are not bones
	 */
	ComputeChildren(t, -VX_Transformer);
}

bool Skeleton::OnStart()
{
	if (m_BoneInfo[0].Bone == NULL)
		FindBones();
	return Engine::OnStart();
}

/*!
 * bool Skeleton::Eval(float t)
 *
 * Applies the matrices computed from the current pose to update the
 * target hierarchy and the child skins.
 *
 * @see Skeleton::SetTarget Skin
 */
bool Skeleton::Eval(float t)
{
	Pose*			pose = m_Pose;
	bool			changed;
	Transformer*	xform;
	Matrix			mpose;

	/* 
	 * Update the pose based on bone animations.
	 * If any of the bones are animated, they will be marked as changed.
	 */
	if (pose == NULL)
		return false;
	pose->Lock();
	changed = HasChanged() || pose->HasChanged();
	for (int i = 0; i < GetNumBones(); ++i)
	{
		xform = m_BoneInfo[i].Bone;
		if (xform == NULL)
			continue;
		ObjectLock lock2(xform);
		if (xform->HasChanged())
		{
			Quat	q(xform->GetRotation());

			q.Normalize();
			pose->SetLocalRotation(i, q);
			if (i == 0)
			{
				Vec3 pos = xform->GetPosition() + m_RootOffset;
				pose->SetPosition(pos);
			}
		}
	}
	/*
	 * Now we update the skin matrix from the current pose
	 */
	pose->Sync();
	for (int i = 0; i < GetNumBones(); ++i)
	{
		xform = m_BoneInfo[i].Bone;
		if (xform)
		{
			ObjectLock lock3(xform);
			xform->SetRotation(pose->GetLocalRotation(i));
			if (i == 0)
			{
				Vec3 v = pose->GetLocalPosition(i);
				xform->SetPosition(v);
			}
			xform->Eval(t);
		}
		pose->GetWorldMatrix(i, mpose);
		mpose *= m_BoneInfo[i].InvBind;
		m_SkinPose.SetWorldMatrix(i, mpose);
	}
	pose->SetChanged(false);
	m_SkinPose.SetChanged(false);
	SetChanged(false);
	pose->Unlock();
	return true;
}


/*
 * Updates the transformations on the bone shapes in the target hierarchy
 * associated with the skeleton.
 */
void Skeleton::UpdateBindPose()
{
	VX_TRACE(Debug, ("Skeleton::SetBindPose %s\n", GetName()));
	const TCHAR*	name = GetName();
	int		numbones = GetNumBones();

	if (name)
	{
		if (m_BindPose.GetName() == NULL)
			m_BindPose.SetName(Core::String(name) + TEXT(".bindpose"));
		if (m_SkinPose.GetName() == NULL)
			m_SkinPose.SetName(Core::String(name) + TEXT(".skinpose"));
	}
	m_BindPose.Sync();
	for (int i = 0; i < numbones; ++i)
	{
		BoneInfo&		boneinfo = m_BoneInfo[i];
		Pose::Bone&		posebone = m_BindPose.GetBone(i);
		Matrix			localmtx;
		Matrix			worldmtx;
		Transformer*	joint = boneinfo.Bone;
		Vec3			p;
		Quat			q;
		
		if (posebone.ParentID < 0)
			posebone.ParentID = boneinfo.ParentID;
		VX_ASSERT(posebone.ParentID == boneinfo.ParentID);
		m_BindPose.GetWorldMatrix(i, worldmtx);
		boneinfo.InvBind.Invert(worldmtx);
		if (boneinfo.ParentID >= 0)
		{
			m_BindPose.GetWorldMatrix(posebone.ParentID, localmtx);
			localmtx.Invert();				// invert total matrix from parent to root
			localmtx.PostMul(worldmtx);		// isolate local matrix
		}
		else
			localmtx = worldmtx;
		if (joint && joint->IsClass(VX_Transformer))
		{
			joint->SetTransform(&localmtx);
			joint->CalcMatrix();
		}

#if defined(_DEBUG) || (defined(_TRACE) &&  (TRACE > 1))
		if (Debug > 1)
		{
			q.Set(worldmtx);
			q.Normalize();
			worldmtx.GetTranslation(p);
			VX_PRINTF(("\t%d %s wrot (%.3f, %.3f, %.3f, %.3f) wpos (%f, %f, %f)  ",
				 i, GetBoneName(i), q.x, q.y, q.z, q.w, p.x, p.y, p.z));
			q.Set(localmtx);				// local rotation for bind pose
			q.Normalize();
			localmtx.GetTranslation(p);		// local translation for bind pose
			VX_PRINTF(("lrot (%.3f, %.3f, %.3f, %.3f) lpos (%f, %f, %f)\n",
				 q.x, q.y, q.z, q.w, p.x, p.y, p.z));
			if (p.DistanceSquared(posebone.LocalPos) > 0.0001f)
				VX_WARNING(("WARNING: joint translation does not match bone (%.3f, %.3f, %.3f)\n",
							posebone.LocalPos.x, posebone.LocalPos.y, posebone.LocalPos.z));
			if (q.DistanceSquared(posebone.LocalRot) > 0.0001f)
				VX_WARNING(("WARNING: joint rotation does not match bone (%.3f, %.3f, %.3f, %.3f)\n",
							posebone.LocalRot.x, posebone.LocalRot.y, posebone.LocalRot.z, posebone.LocalRot.w));
		}
#endif
	}
}


/*!
 * @fn void Skeleton::SetTarget(SharedObj* target)
 * @param target	root of target hierarchy
 *
 * Sets the target bone hierarchy for the skeleton.
 * This function constructs Transformers for all the models in the target
 * skeleton hierarchy whose names match the bone names defined in
 * the skeleton. If a model is encountered that does not have
 * a matching bone name in the skeleton, no Transformer is created.
 * The bone Transformers are attached to the skeleton in the same hierarchy as the target models.
 *
 * The transformations on the target hierarchy become the current pose of the skeleton.
 * They also define the bind pose if one has not already been established.
 *
 * @see Skeleton::AttachBones Skeleton::SetBindPose Skeleton::SetPose
 */
void Skeleton::SetTarget(SharedObj* target)
{
	Engine::SetTarget(target);
	if (target && target->IsClass(VX_Model))
	{
		ObjectLock	lock2(target);
		AttachBones((Model*) target, this);	// attach new ones
		FindBones();
		SetPose(m_Pose);
		SetChanged(true);
		if (target->IsClass(VX_Model))		// does target have bounds?
			((Model*) target)->GetBound(&m_Bound);
	}
}

/*!
 * @fn void Skeleton::AttachBones(Model* target, Engine* joint, int parentbone)
 * @param target	root of target skeleton models
 * @param joint		root of joint hierarchy, joints created are attached to this engine
 * @param parentbone 0-based bone index of the parent bone, -1 for the root
 *
 * Thee target is a hierarchy of models defining a skeleton. This function
 * creates Transformers for all the models in the target hierarchy
 * whose names match the bone names defined in the skeleton.
 * If a model is encountered that does not have a matching bone, no Transformer is created for it.
 * The Transformer are attached to this skeleton in the same hierarchy as
 * the target models.
 *
 * @see Skeleton::AttachBones Skeleton::AttachBone Skeleton::SetTarget
 */
void Skeleton::AttachBones(Model* mroot, Engine* eroot, int parentbone)
{
	Core::String	name(mroot->GetName());
	Core::String	basename(name);
	Core::String	bonename(name);
	Transformer*	xform; 
	int				boneindex = -1;

	if (eroot == NULL)
		eroot = this;
	if (!name.IsEmpty())
	{
		size_t	p = name.ReverseFind(TEXT('.'));// strip off file base name prefix

		if (p > 0)
			basename = name.Mid(p + 1);
		boneindex = GetBoneIndex(basename);
		if (boneindex >= 0)
		{
			Pose::Bone& bone = m_Pose->GetBone(boneindex);
			if (m_BoneInfo[boneindex].ParentID < 0)
			{
				m_BoneInfo[boneindex].ParentID = parentbone;
				bone.ParentID = parentbone;
			}
			if (bone.ParentID < 0)
				bone.ParentID = m_BoneInfo[boneindex].ParentID;
			xform = AttachBone(boneindex, eroot, mroot);
			if (xform)
			{
				eroot = xform;
				parentbone = boneindex;
			}
		}
	}
	if (mroot->IsParent())						// should we descend?
	{
		GroupIterNotSafe<Model> iter(mroot, Group::CHILDREN);
		Model*		m;

		while (m = iter.Next())
		{
			ObjectLock lock1(m);
			ObjectLock lock2(eroot);
			AttachBones(m, eroot, parentbone);
		}
	}
}


/*!
 * @fn void Skeleton::AttachBone(int boneindex, Engine* eroot, Group* target)
 * @param boneindex	index of bone to attach
 * @param target	root of target skeleton
 * @param eroot		root of engine hierarchy
 *
 * This function attaches a Transformer to the input target object.
 * If the name of the target does not match one of the bone names of the
 * skeleton, no Transformer is created for it.
 *
 * @see Skeleton::AttachBones Skeleton::SetTarget Transformer
 */
Transformer* Skeleton::AttachBone(int boneindex, Engine* eroot, Model* target)
{
	Core::String	bonename(GetBoneName(boneindex));
	Transformer*	xform = (Transformer*) eroot->Find(bonename, Group::FIND_DESCEND);

	if (target == NULL)
		return xform;
	bonename += TEXT(".xform");
	if (!target->IsClass(VX_Model))
		return xform;
	if (xform && xform->IsClass(VX_Skeleton))
		xform = (Transformer*) xform->Find(bonename, Group::FIND_DESCEND);
	if (xform == NULL)
	{
		xform = new Transformer();		// make one and attach it
		xform->SetName(bonename);
		if (eroot == this)
			eroot->PutFirst(xform);
		else
			eroot->Append(xform);
		xform->Enable(Engine::CONTROL_CHILDREN, Engine::ONLYME);
		VX_TRACE(Debug, ("Skeleton: %s creating joint engine #%d %s\n", GetName(), boneindex, (const TCHAR*) bonename));
	}
	VX_ASSERT(xform->IsClass(VX_Transformer));
	m_BoneInfo[boneindex].Bone = xform;
	xform->SetBoneIndex(boneindex);
	xform->SetTarget(target);			// make this model the target of the transformer
	xform->CalcMatrix();
	VX_TRACE(Debug, ("Skeleton: %s %d %s -> model %s\n", GetName(), boneindex, (const TCHAR*) bonename, target->GetName()));
	m_Pose->SetWorldMatrix(boneindex, *xform->GetTotalTransform());
	return xform;
}

/*!
 * @fn FindBones(Pose* savepose)
 * Examines the child engines for the skeleton looking for Transformers which match the
 * bones in the skeleton. Use this function instead of AttachBones if you have already created
 * the bone Transformer hierarchy and want to attach it to the skeleton.
 *
 * @see AttachBones SetTarget
 */
void Skeleton::FindBones(Pose* savepose)
{
	GroupIter<Engine> iter(this, Group::DEPTH_FIRST);
	Transformer*	newbone;

	if (savepose)
		savepose->Copy(m_Pose);
	while (newbone = (Transformer*) iter.Next())
	{
		Core::String	name(newbone->GetName());
		Core::String	bonename(name);
		size_t			p;

		if (!newbone->IsClass(VX_Transformer))	// ignore anything but bones
			continue;
		if (name.IsEmpty())					// ignore models without names
			continue;
		if (name.Right(6) == Core::String(TEXT(".xform")))
			bonename = name.Left(-6);		// strip off ".xform" suffix
		p = bonename.Find(TEXT('.'));		// strip off leading filename prefix
		if (p > 0)
			 bonename = bonename.Mid(p + 1);
		int boneindex = GetBoneIndex(bonename);
		if (boneindex >= 0)					// a bone we recognize?
		{
			Transformer* curbone = m_BoneInfo[boneindex].Bone;
			VX_ASSERT((curbone == NULL) || (curbone == newbone));
			VX_TRACE(Debug, ("Skeleton: %s found bone #%d %s\n", GetName(), boneindex, (const TCHAR*) bonename));
			m_BoneInfo[boneindex].Bone = newbone;
			newbone->CalcMatrix();
			if (savepose)
				savepose->SetWorldMatrix(boneindex, *newbone->GetTotalTransform());
			newbone->Enable(Engine::CONTROL_CHILDREN, Engine::ONLYME);
			VX_ASSERT((newbone->GetBoneIndex() < 0) || (newbone->GetBoneIndex() == boneindex));
			newbone->SetBoneIndex(boneindex);
			if (m_BoneInfo[boneindex].ParentID < 0)
			{
				Transformer* parent = (Transformer*) newbone->Parent();

				if (parent && parent->IsClass(VX_Transformer))
				{
					m_BoneInfo[boneindex].ParentID = parent->GetBoneIndex();
					VX_TRACE(Debug, ("Skeleton: %s -> %s\n", parent->GetName(), newbone->GetName()));
				}
			}
		}
	}
	m_BindPose.SetSkeleton(this);		// copy skeleton bone parent IDs to current pose
	m_SkinPose.SetSkeleton(this);
	m_Pose->SetSkeleton(this);
}

/*!
 * @fn Shape* Skeleton::MakeLimb(const TCHAR* name, Model* parent, const Quat& jointrot, const Vec3& jointpos, const Appearance* app, float bonelen, float bonewidth)
 * Makes a proxy shape for a skeleton bone. It is a box oriented along the bone axis.
 * @param name		name of new shape.
 * @param parent	parent model to attach the limb to.
 * @param jointrot	world rotation of bone.
 * @param jointpos	world position of bone.
 * @param app		Appearance for new shape.
 * @param bonewidth	width of bone shape box.
 */
Shape* Skeleton::MakeLimb(const TCHAR* name, Model* parent, const Quat& jointrot, const Vec3& jointpos, const Appearance* app, float bonelen, float bonewidth)
{
	Shape*		shape = new Shape();
	TriMesh*	mesh = new TriMesh(VertexArray::NORMALS);
	Matrix		bonemtx;
	Vec3		trans;
	Vec3		worldpos;
	Vec3		bonesize(bonewidth, bonewidth, bonewidth);
	Matrix		mtx1, mtx2;

	if (bonelen <= 0.0f)
		return NULL;
	VX_ASSERT(bonewidth > 0.0f);
	bonesize += m_BoneAxis * (bonelen - bonewidth);
	shape->SetName(name);
	shape->SetAppearance(app);
	GeoUtil::Block(mesh, bonesize);
	bonemtx.Translate(m_BoneAxis * bonelen / 2.0f);
	*mesh *= bonemtx;
	shape->SetGeometry(mesh);
	if (0)
	{
		Shape*	upvec = new Shape();
		Vec3	upsize(bonewidth / 4.0f, bonewidth * 10.0f, bonewidth / 4.0f);
		Matrix	mtmp;

		mtmp.Translate(Model::YAXIS * upsize.y / 2.0f);
		upvec->SetName(Core::String(name) + "-up");
		mesh = new TriMesh(VertexArray::NORMALS);
		GeoUtil::Block(mesh, upsize);
		*mesh *= mtmp;
		upvec->SetGeometry(mesh);
		upvec->SetAppearance(app);
		shape->Append(upvec);
	}
	mtx2.Rotate(jointrot);
	mtx2.Translate(jointpos);
	if (parent)
	{
		parent->Append(shape);
		parent->TotalTransform(&mtx1);
		mtx1.Invert();
	}
	mtx1.PostMul(mtx2);
	shape->SetTransform(&mtx1);
	VX_TRACE(Debug, ("Skeleton::MakeLimb %s pos(%f,%f,%f) rot(%f,%f,%f,%f)\n",
			 name, jointpos.x, jointpos.y, jointpos.z, jointrot.x, jointrot.y, jointrot.z, jointrot.w));
	return shape;
}

/*!
 * @fn Model* MakeSkeleton(const TCHAR* name)
 * Constructs hierarchy of skeleton shapes in the default bind pose.
 *
 * @return root model of skeleton created
 */
Model* Skeleton::MakeSkeleton(const TCHAR* name)
{
	Core::String bodyname(name);
	Vec3		JointPos[NUM_BONES];
	Quat		JointDir[NUM_BONES];
	float		bonelengths[NUM_BONES];
	Vec3		vecs[3];

	JointPos[WAIST].Set(0.0f, 0.0f, 0.0f);
	JointPos[TORSO].Set(0.0f, 0.0f, 0.0f);
	JointPos[NECK].Set(0.0f, 8.0f, 0.0f);
	JointPos[HEAD].Set(0.0f, 9.0f, 0.0f);
	JointPos[RIGHT_HIP].Set(-2.0f, -1.0f, 0.0f);
	JointPos[RIGHT_KNEE].Set(-2.0f, -7.0f, 0.0f);
	JointPos[RIGHT_ANKLE].Set(-2.0f, -13.0f, 0.0f);	
	JointPos[RIGHT_FOOT].Set(-3.0f, -13.0f, 0.0f);	
	JointPos[RIGHT_COLLAR].Set(0.0f, 7.0f, 0.0f);
	JointPos[RIGHT_SHOULDER].Set(-3.5f, 7.0f, 0.0f);
	JointPos[RIGHT_ELBOW].Set(-8.5f, 7.0f, 0.0f);
	JointPos[RIGHT_WRIST].Set(-13.5f, 7.0f, 0.0f);	
	JointPos[RIGHT_HAND].Set(-14.5f, 7.0f, 0.0f);	
	JointPos[RIGHT_FINGER].Set(-15.0f, 7.0f, 0.0f);	
	JointPos[LEFT_HIP].Set(2.0f, -1.0f, 0.0f);
	JointPos[LEFT_KNEE].Set(2.0f, -7.0f, 0.0f);
	JointPos[LEFT_ANKLE].Set(2.0f, -13.0f, 0.0f);
	JointPos[LEFT_FOOT].Set(3.0f, -13.0f, 0.0f);
	JointPos[LEFT_COLLAR].Set(0.0f, 7.0f, 0.0f);
	JointPos[LEFT_SHOULDER].Set(3.5f, 7.0f, 0.0f);
	JointPos[LEFT_ELBOW].Set(8.5f, 7.0f, 0.0f);
	JointPos[LEFT_WRIST].Set(13.5f, 7.0f, 0.0f);
	JointPos[LEFT_HAND].Set(14.5f, 7.0f, 0.0f);	
	JointPos[LEFT_FINGER].Set(15.0f, 7.0f, 0.0f);	

	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	JointDir[NECK].Set(vecs);
	bonelengths[NECK] = 1.0f;
	JointDir[HEAD].Set(vecs);
	bonelengths[HEAD] = 3.0f;
	JointDir[TORSO].Set(vecs);
	bonelengths[TORSO] = 8.0f;
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	JointDir[LEFT_COLLAR].Set(vecs);
	bonelengths[LEFT_COLLAR] = 2.5f;
	JointDir[LEFT_SHOULDER].Set(vecs);
	bonelengths[LEFT_SHOULDER] = 5.0f;
	JointDir[LEFT_ELBOW].Set(vecs);
	bonelengths[LEFT_ELBOW] = 5.0f;
	JointDir[LEFT_WRIST].Set(vecs);
	bonelengths[LEFT_WRIST] = 1.0f;
	JointDir[LEFT_HAND].Set(vecs);
	bonelengths[LEFT_HAND] = 1.0f;
	JointDir[LEFT_FINGER].Set(vecs);
	bonelengths[LEFT_FINGER] = 0.5f;
	JointDir[LEFT_HAND].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	JointDir[RIGHT_COLLAR].Set(vecs);
	bonelengths[RIGHT_COLLAR] = 2.5f;
	JointDir[RIGHT_SHOULDER].Set(vecs);
	bonelengths[RIGHT_SHOULDER] = 5.0f;
	JointDir[RIGHT_ELBOW].Set(vecs);
	bonelengths[RIGHT_ELBOW] = 5.0f;
	JointDir[RIGHT_WRIST].Set(vecs);
	bonelengths[RIGHT_WRIST] = 1.0f;
	JointDir[RIGHT_HAND].Set(vecs);
	bonelengths[RIGHT_HAND] = 1.0f;
	JointDir[RIGHT_FINGER].Set(vecs);
	bonelengths[RIGHT_FINGER] = 0.5f;

	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	JointDir[WAIST].Set(vecs);
	bonelengths[WAIST] = 2.0f;
	JointDir[LEFT_HIP].Set(vecs);
	bonelengths[LEFT_HIP] = 6.0f;
	JointDir[LEFT_KNEE].Set(vecs);
	bonelengths[LEFT_KNEE] = 6.0f;
	JointDir[RIGHT_HIP].Set(vecs);
	bonelengths[RIGHT_HIP] = 6.0f;
	JointDir[RIGHT_KNEE].Set(vecs);
	bonelengths[RIGHT_KNEE] = 6.0f;
	JointDir[RIGHT_ANKLE].Set(0,0,0,1);	
	bonelengths[RIGHT_ANKLE] = 2.0f;
	JointDir[RIGHT_FOOT].Set(0,0,0,1);	
	bonelengths[RIGHT_FOOT] = 1.0f;
	JointDir[LEFT_ANKLE].Set(0,0,0,1);
	bonelengths[LEFT_ANKLE] = 2.0f;
	JointDir[LEFT_FOOT].Set(0,0,0,1);	
	bonelengths[LEFT_FOOT] = 1.0f;
	return MakeSkeleton(name, JointDir, JointPos, bonelengths, 1.0f);
}

/*!
 * @fn Model* MakeSkeleton(const TCHAR* name, const Quat* rotations, const Vec3* positions, float* bonelengths, float bonewidth)
 * @param name		base name for created skeleton models
 * @param rotations	world coordinate rotations for the skeleton joints
 * @param positions	world space positions for the skeleton joints
 * @param bonelengths lengths of the skeleton bones
 * @param bonewidth	 width of the bones
 *
 * Constructs hierarchy of skeleton shapes in the bind pose designated by the input rotations and positions.
 * Bone rotations are in world coordinates with respect to the root of the skeleton. Positions are
 * also in world space. Each joint in the skeleton is represented by a box along the bone axis.
 *
 * @return root model of skeleton created
 */
Model* Skeleton::MakeSkeleton(const TCHAR* name, const Quat* rotations, const Vec3* positions, float* bonelengths, float bonewidth)
{
	Core::String bodyname(name);
	Vec3			Dir[Skeleton::NUM_BONES];
	Model*			mainroot = new Model();
	Ref<Appearance>	PINK = new Appearance(Col4(1.0f, 0.0f, 1.0f));
	Ref<Appearance>	RED = new Appearance(Col4(1.0f, 0.0f, 0.0f));
	Ref<Appearance>	BLUE = new Appearance(Col4(0.0f, 0.0f, 1.0f));
	Ref<Appearance>	GREEN = new Appearance(Col4(0.0f, 1.0f, 0.0f));
	Ref<Appearance>	CYAN = new Appearance(Col4(0.0f, 1.0f, 1.0f));
	Vec3			zero(0.0f, 0.0f, 0.0f);
	Shape*			temp1;
	Shape*			temp2;
	Shape*			waist;
	Shape*			torso;
	Shape*			collar;
	int32			parents[Skeleton::NUM_BONES];

	VX_ASSERT(rotations);
	VX_ASSERT(positions);
	VX_ASSERT(bonelengths);

	parents[WAIST] = -1;
	parents[TORSO] = -1;
	parents[RIGHT_HIP] = WAIST;
	parents[RIGHT_KNEE] = RIGHT_HIP;
	parents[RIGHT_ANKLE] = RIGHT_KNEE;
	parents[RIGHT_FOOT] = RIGHT_ANKLE;
	parents[LEFT_HIP] = WAIST;
	parents[LEFT_KNEE] = LEFT_HIP;
	parents[LEFT_ANKLE] = LEFT_KNEE;
	parents[LEFT_FOOT] = LEFT_ANKLE;
	parents[NECK] = TORSO;
	parents[HEAD] = NECK;
	parents[RIGHT_COLLAR] = TORSO;
	parents[RIGHT_SHOULDER] = RIGHT_COLLAR;
	parents[RIGHT_ELBOW] = RIGHT_SHOULDER;
	parents[RIGHT_WRIST] = RIGHT_ELBOW;
	parents[RIGHT_HAND] = RIGHT_WRIST;
	parents[RIGHT_FINGER] = RIGHT_HAND;
	parents[LEFT_COLLAR] = TORSO;
	parents[LEFT_SHOULDER] = LEFT_COLLAR;
	parents[LEFT_ELBOW] = LEFT_SHOULDER;
	parents[LEFT_WRIST] = LEFT_ELBOW;
	parents[LEFT_HAND] = LEFT_WRIST;
	parents[LEFT_FINGER] = LEFT_HAND;

	mainroot->SetName(bodyname + ".skeleton");
	waist = MakeLimb(bodyname + ".Waist", mainroot, rotations[WAIST], positions[WAIST], PINK, bonelengths[WAIST], bonewidth);
	torso = MakeLimb(bodyname + ".Torso", mainroot, rotations[TORSO], positions[TORSO], GREEN, bonelengths[TORSO], bonewidth);
	temp1 = MakeLimb(bodyname + ".RightHip", waist, rotations[RIGHT_HIP], positions[RIGHT_HIP], RED, bonelengths[RIGHT_HIP], bonewidth);
	temp2 = MakeLimb(bodyname + ".RightKnee", temp1, rotations[RIGHT_KNEE], positions[RIGHT_KNEE], PINK, bonelengths[RIGHT_KNEE], bonewidth);
	temp1 = MakeLimb(bodyname + ".RightAnkle", temp2, rotations[RIGHT_ANKLE], positions[RIGHT_ANKLE], RED, bonelengths[RIGHT_ANKLE], bonewidth);	
	temp1 = MakeLimb(bodyname + ".LeftHip", waist, rotations[LEFT_HIP], positions[LEFT_HIP], BLUE, bonelengths[LEFT_HIP], bonewidth);
	temp2 = MakeLimb(bodyname + ".LeftKnee", temp1, rotations[LEFT_KNEE], positions[LEFT_KNEE], CYAN, bonelengths[LEFT_KNEE], bonewidth);
	temp1 = MakeLimb(bodyname + ".LeftAnkle", temp2, rotations[LEFT_ANKLE], positions[LEFT_ANKLE], BLUE, bonelengths[LEFT_ANKLE], bonewidth);
	if (bonelengths[RIGHT_COLLAR] > 0.0f)
		collar = MakeLimb(bodyname + ".RightCollar", torso, rotations[RIGHT_COLLAR], positions[RIGHT_COLLAR], PINK, bonelengths[RIGHT_COLLAR], bonewidth);
	else
	{
		parents[RIGHT_SHOULDER] = TORSO;
		collar = torso;
	}
	temp1 = MakeLimb(bodyname + ".RightShoulder", collar, rotations[RIGHT_SHOULDER], positions[RIGHT_SHOULDER], RED, bonelengths[RIGHT_SHOULDER], bonewidth);
	temp2 = MakeLimb(bodyname + ".RightElbow", temp1, rotations[RIGHT_ELBOW], positions[RIGHT_ELBOW], PINK, bonelengths[RIGHT_ELBOW], bonewidth);
	if (bonelengths[RIGHT_WRIST] > 0.0f)
	{
		temp1 = MakeLimb(bodyname + ".RightWrist", temp2, rotations[RIGHT_WRIST], positions[RIGHT_WRIST], GREEN, bonelengths[RIGHT_WRIST], bonewidth);	
		if (bonelengths[RIGHT_HAND] > 0.0f)
		{
			temp2 = MakeLimb(bodyname + ".RightHand", temp1, rotations[RIGHT_HAND], positions[RIGHT_HAND], RED, bonelengths[RIGHT_HAND], bonewidth);
			if (bonelengths[RIGHT_FINGER] > 0.0f)
				temp1 = MakeLimb(bodyname + ".RightFinger", temp2, rotations[RIGHT_FINGER], positions[RIGHT_FINGER], PINK, bonelengths[RIGHT_FINGER], bonewidth);
		}

	}
	if (bonelengths[LEFT_COLLAR] > 0.0f)
		collar = MakeLimb(bodyname + ".LeftCollar", torso, rotations[LEFT_COLLAR], positions[LEFT_COLLAR], CYAN, bonelengths[LEFT_COLLAR], bonewidth);
	else
	{
		collar = torso;
		parents[LEFT_SHOULDER] = TORSO;
	}
	temp1 = MakeLimb(bodyname + ".LeftShoulder", collar, rotations[LEFT_SHOULDER], positions[LEFT_SHOULDER], BLUE, bonelengths[LEFT_SHOULDER], bonewidth);
	temp2 = MakeLimb(bodyname + ".LeftElbow", temp1, rotations[LEFT_ELBOW], positions[LEFT_ELBOW], CYAN, bonelengths[LEFT_ELBOW], bonewidth);
	if (bonelengths[LEFT_WRIST] > 0.0f)
	{
		temp1 = MakeLimb(bodyname + ".LeftWrist", temp2, rotations[LEFT_WRIST], positions[LEFT_WRIST], GREEN, bonelengths[LEFT_WRIST], bonewidth);
		if (bonelengths[LEFT_HAND] > 0.0f)
		{
			temp2 = MakeLimb(bodyname + ".LeftHand", temp1, rotations[LEFT_HAND], positions[LEFT_HAND], BLUE, bonelengths[LEFT_HAND], bonewidth);
			if (bonelengths[LEFT_FINGER] > 0.0f)
				temp1 = MakeLimb(bodyname + ".LeftFinger", temp2, rotations[LEFT_FINGER], positions[LEFT_FINGER], CYAN, bonelengths[LEFT_FINGER], bonewidth);
		}
	}
	if (bonelengths[NECK] > 0.0f)
	{
		temp1 = MakeLimb(bodyname + ".Neck", torso, rotations[NECK], positions[NECK], RED, bonelengths[NECK], bonewidth);
		if (bonelengths[HEAD] > 0.0f)
			temp2 = MakeLimb(bodyname + ".Head", temp1, rotations[HEAD], positions[HEAD], PINK, bonelengths[HEAD], bonewidth);
	}
	Init(Skeleton::NUM_BONES, parents);
	SetBindPose(rotations, positions);
	mainroot->GetBound(&m_Bound);
	return mainroot;
}


}	// end Vixen