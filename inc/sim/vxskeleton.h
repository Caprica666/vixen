#pragma once

/*!
 * @file vxskeleton.h
 * @brief Base class for skeleton.
 *
 * Creates an animation hierarchy for controlling a skeleton and
 * a set of skinned meshes.
 *
 * @author Nola Donato
 * @ingroup vixen
 */

namespace Vixen {

class Pose;

/*!
 * @class Skeleton
 *
 * @brief Engine that animates a skeleton based on a set of bones.
 *
 * This class provides a common infrastructure for skeletal animation.
 * It can construct an animation hierarchy representing the bones of a skeleton
 * which can be used to animate a skinned character. You can set the bone names in the skeleton
 * to designate which shapes in your hierarchy represent which body parts.
 * If you set a hierarchy of shapes to be the target of the Skeleton,
 * it will construct an animation hierarchy for you. The Maya exporter also
 * constructs an animation hierarchy when you export a rigged character.
 *
 * The Skeleton relies on the Pose class to represent the position and orientation of its bones.
 * All bones in the skeleton start out at the origin oriented along the bone axis (usually 0,0,1).
 * The pose orients and positions each bone in the skeleton respect to this initial state by maintaining
 * a matrix for each bone. The root bone which is the parent of all the other bones in the
 * hierarchy should be the first bone (with index 0).
 *
 * The "bind pose" of the skeleton defines the neutral position of the bones before any
 * external transformations are applied. Usually it represents the pose of the skeleton
 * matching the meshes which will be skinned from the skeleton. The bone transformations
 * used for skinning the mesh are relative to the bind pose.
 *
 * The "current pose" of the skeleton defines the current orientation and position of the bones.
 * It uses the same conventions as the bind pose, relative to bones at the origin along the bone axis
 * (It is NOT relative to the bind pose). When the current pose is updated, the skeleton modifies the matrices
 * attached to the models in the hierarchy and the bone matrices for each child Skin. The current pose
 * can be shared with other engines.
 *
 * The Skeleton can also work with the generative body model to pose it
 * from the joint angles. If you add a Skin as a child of the Skeleton,
 * the bone angles from the skeleton will be automatically applied to the skin.
 *
 * @image html body_track.png
 *
 * KinectTracker is a body tracking which uses the Microsoft
 * Kinect controller to modify the bone matrices of the pose. It can be used to control
 * the Skeleton based on live video input.
 *
 * By default, the body tracker has 24 bones and the bone indices match
 * the convention used by Kinect. You can set the number of bones and
 * use a different skeleton if desired.
 *
 * @see  Skin Evaluator SCAPESkin KinectTracker BodyTracker RagDoll Pose
 * @ingroup vixen
 */
class Skeleton : public Engine
{
public:
	VX_DECLARE_CLASS(Skeleton);

	Skeleton(int numbones = Skeleton::NUM_BONES);
	Skeleton(const Skeleton& src);

	/*!
	 * Indices used to identify the default bones in the skeleton.
	 */
	enum
	{
		WAIST = 0,
		TORSO = 1,
		NECK = 2,
		HEAD = 3,
		RIGHT_COLLAR = 4,
		RIGHT_SHOULDER = 5,
		RIGHT_ELBOW = 6,
		RIGHT_WRIST = 7,
		RIGHT_HAND = 8,
		RIGHT_FINGER = 9,
		LEFT_COLLAR = 10,
		LEFT_SHOULDER = 11,
		LEFT_ELBOW = 12,
		LEFT_WRIST = 13,
		LEFT_HAND = 14,
		LEFT_FINGER = 15,
		RIGHT_HIP = 16,
		RIGHT_KNEE = 17,
		RIGHT_ANKLE = 18,
		RIGHT_FOOT = 19,
		LEFT_HIP = 20,
		LEFT_KNEE = 21,
		LEFT_ANKLE = 22,
		LEFT_FOOT = 23,
		NUM_BONES = 24,

		WORLD_ABSOLUTE = 1,	// world rotation relative to skeleton bones with no transform
		WORLD_BIND = 2,		// world rotation relative to skeleton bones in the bind pose

		BONE_LOCK_ROTATION = 1,	// lock bone rotation
		BONE_ANIMATE = 4,		// keyframe bone animation
		BONE_PHYSICS = 2,		// use physics to compute bone motion
	};

	//! Makes a test skeleton and attaches it to the body poser.
	Model*			MakeSkeleton(const TCHAR* name, const Quat* rotations, const Vec3* positionsL, float* bonelengthsL, float bonewidth);
	Model*			MakeSkeleton(const TCHAR* name);

	//! Get the bounds of the skeleton (bounding volume of bone mesh hierarchy if any).
	const Box3&		GetBound() const;

	//! Get world space position of skeleton (position of the root bone, index 0).
	virtual Vec3	GetPosition() const;

	//! Set world space position of skeleton (position of the root bone, index 0).
	virtual void	SetPosition(const Vec3& p);

	//! Get offset of root bone - useful for moving the starting point of an animated character.
	const Vec3&		GetRootOffset() const;

	//! Set offset of root bone. This is an additional offset only added to the root bone if it is animated.
	void			SetRootOffset(const Vec3&);

	//! Set bind pose - neutral positions and rotations for all the bones relative to skeleton root joint.
	virtual void	SetBindPose(Pose* pose);
	virtual void	SetBindPose(const Quat* rotations, const Vec3* positions);

	//! Get bind pose - neutral positions and rotations for all the bones relative to skeleton root joint.
	virtual const Pose*	GetBindPose() const;

	//! Get current pose of skeleton.
	virtual Pose*	GetPose() const;

	//! Make a pose for this skeleton in the given coordinate space.
	virtual	Pose*	MakePose(int coordspace) const;

	//! Get current pose of skeleton relative to bind pose
	virtual const Pose*	GetSkinPose() const;

	//! Set a new pose as the current pose.
	virtual void	SetPose(Pose* pose);

	//! Change the current pose of the skeleton.
	virtual void	ApplyPose(const Pose* pose);

	//! Restore the skeleton to the bind pose.
	virtual	void	RestoreBindPose();

	//! Get bone index given the bone name in the target mesh.
	int				GetBoneIndex(const TCHAR* bonename) const;

	//! Get index of parent bone. 
	int				GetParentBoneIndex(int boneindex) const;

	//! Set index of parent bone.
	bool			SetParentBoneIndex(int boneindex, int parentindex);

	//! Get bone index given the engine representing the bone.
	int				GetBoneIndex(const Engine* bone) const;

	//! Get the number of bones in the skeleton.
	int				GetNumBones() const;

	//! Get the bone axis.
	const Vec3&		GetBoneAxis() const;

	//! Set the bone axis, defines the bone coordinate system, default is (0, 0, 1)
	void			SetBoneAxis(const Vec3&);

	//! Set the name of the bone in the target mesh for the corresponding index.
	virtual void	SetBoneName(int boneindex, const TCHAR* bonename);

	//! Get the name of a bone given it's index.
	const TCHAR*	GetBoneName(int boneindex) const;

	//! Get the animation engine for the given bone.
	Transformer*	GetBone(int boneindex) const;

	//! Set rotation and physics options for this bone
	virtual void	SetBoneOptions(int boneindex, int options);

	//! Get rotation and physics options for this bone
	int				GetBoneOptions(int boneindex) const;

	//! Determine if given bone is locked or not.
	bool			IsLocked(int boneindex) const;

	//! Attach the animation engines for each bone to the target model or skin hierarchy.
	virtual	void	AttachBones(Model* root, Engine* engroot = NULL, int parentbone = -1);

	//! Scan children of the skeleton looking for new unattached bones and save their current pose.
	void			FindBones(Pose* pose = NULL);

	//! Make a limb of the test body.
	Shape*			MakeLimb(const TCHAR* name, Model* parent, const Quat& jointrot, const Vec3& jointpos, const Appearance* app, float bonelen, float bonewidth);

	//! Find indices of a set of bones based on their names.
	void			MakeBoneMap(const TCHAR** bonenames, int32* bonemap, int numbones) const;

	/*
	 * Skeleton::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SKELETON_Init = Engine::ENG_NextOp,
		SKELETON_SetBoneName,
		SKELETON_SetBoneAxis,
		SKELETON_SetPose,
		SKELETON_SetBindPose,
		SKELETON_ApplyPose,
		SKELETON_SetBoneOptions,
		SKELETON_SetPosition,
		SKELETON_SetRootOffset,
		SKELETON_NextOp = Engine::ENG_NextOp + 30
	};

//
// Overrides
//
	virtual bool		Eval(float t);
	virtual void		Compute(float);
	virtual bool		OnStart();
	virtual void		SetTarget(SharedObj* target);
	virtual bool		Do(Messenger& s, int op);
	virtual bool		Copy(const SharedObj*);
	virtual	bool		OnEvent(Event*);
	virtual	void		Empty();
	virtual int			Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	//! Table of strings containing names of default bones.
	static const TCHAR* DefaultBoneNames[Skeleton::NUM_BONES];

protected:
	//! first time initialization, subclasses should call and override.
	virtual	bool		Init(int numbones = 0, const int32* parentindices = NULL);

	//! Attach an animation engine  to a bone.
	virtual	Transformer* AttachBone(int boneindex, Engine* eroot, Model* target);

	//! update a hierarchy of joints from joint rotations in world coordinates.
	void				UpdateLocalRotations(Engine* joint, const Quat& parent_rotation);

	//! update the positions and translations of a hierarchy of joints from the bind pose.
	void				UpdateBindPose();

public:
	/*!
	 * Internal structure used to maintain information about each bone.
	 */
	class BoneInfo
	{
	public:
		Core::String		Name;		//! name of the bone
		int					ParentID;	//! index of parent bone
		int					Options;	//! BONE_LOCK_ROT = rotation locked, BONE_PHYSICS = use physics
		Matrix				InvBind;	//! inverse bind pose
		Ref<Transformer>	Bone;		//! -> engine that controls the bone matrix

		BoneInfo() : ParentID(-1),
					 Options(0),
					 Bone((const Transformer*) NULL)
					{ Name.Empty(); }
		~BoneInfo() { Name.Empty(); Bone = NULL; }

		BoneInfo(const BoneInfo& src)
		{
			Name = src.Name;
			ParentID = src.ParentID;
			Bone = src.Bone;
			Options = src.Options;
			InvBind = src.InvBind;
		}
		BoneInfo& operator=(const BoneInfo& src)
		{
			Name = src.Name;
			ParentID = src.ParentID;
			Bone = src.Bone;
			Options = src.Options;
			InvBind = src.InvBind;
			return *this;
		}

		bool operator==(const BoneInfo& src)
		{
			if ((Name == src.Name) &&
				(ParentID == src.ParentID))
				return true;
			return false;
		}
	};

protected:
	class BoneArray : public Core::BaseArray<BoneInfo, Core::BaseObj >
	{
	public:
		BoneArray(int numbones);
		~BoneArray()	{ SetSize(0); }
	};

	Vec3		m_RootOffset;			// offset for root bone animations
	Vec3		m_BoneAxis;				// axis of bone, defines bone coordinate system
	Box3		m_Bound;				// bounding box of skeleton in bind pose
	BoneArray	m_BoneInfo;				// information on each bone
	Pose		m_SkinPose;				// current skinning pose (bind pose factored out)
	Pose		m_BindPose;				// bind pose for this skeleton
	mutable Ref<Pose>	m_Pose;			// current pose for this skeleton
};

struct BVHLoader
{
	BVHLoader(Core::Stream& instream, const TCHAR* filename);
	Skeleton*	GetSkeleton()	{ return m_Skeleton; }
	Pose*		GetPose()		{ return m_Pose; }
	const TCHAR* GetFileBase()	{ return m_FileBase; }
	int			GetNumFrames()	{ return m_NumFrames; }
	bool		LoadSkel();	
	bool		LoadKeyAsPose(Skeleton* skel);
	bool		LoadKeys();
	int			NextLine(TCHAR* linebuf, int maxlen);
	int			ReadKeys(float* data, int numchannels);
	static bool ReadAnim(const TCHAR* filename, Core::Stream* instream, LoadEvent* ev);
	static	float	AnimationScale;	// position scale factor

protected:
	Core::Stream&	m_Stream;
	Core::String	m_FileName;
	Core::String	m_FileBase;
	Ref<Skeleton>	m_Skeleton;
	Ref<Pose>		m_Pose;
	Vec3			m_RootPos;
	float			m_TimeStep;
	float			m_Time;
	int				m_NumFrames;
};

inline Transformer*	Skeleton::GetBone(int boneindex) const
{
	if (m_BoneInfo.GetSize() == 0)
		return NULL;
	return m_BoneInfo[boneindex].Bone;
}

inline const TCHAR* Skeleton::GetBoneName(int boneindex) const
{
	const TCHAR* bonename;

	if ((boneindex < 0) || (boneindex >= (int) m_BoneInfo.GetSize()))
		return NULL;
	if (m_BoneInfo[boneindex].Name.IsEmpty())
		return NULL;
	bonename = m_BoneInfo[boneindex].Name;
	return bonename;
}

inline const Vec3& Skeleton::GetBoneAxis() const
{
	return m_BoneAxis;
}


inline void Skeleton::SetBoneAxis(const Vec3& v)
{
	m_BoneAxis = v;
}

inline const Box3& Skeleton::GetBound() const
{
	return m_Bound;
}

/*!
 * @fn Pose* Skeleton::GetBindPose() const
 * The bind pose is the pose the skeleton is in when no rotations are
 * applied to it's joints. This is a reference to an internal Pose object
 * which cannot be shared across multiple Skin objects.
 *
 * @see Skeleton::GetNumBones Skeleton::SetBindPose Pose::GetPose Pose::SetWorldRotations Pose::SetWorldPositions
 */
FORCE_INLINE  const Pose*	Skeleton::GetBindPose() const
{
	return &m_BindPose;
}

/*!
 * @fn Pose* Skeleton::GetPose() const
 * The current pose is the pose the skeleton is currently in. It contains the
 * location and orientation of each bone relative to the skeleton root bone.
 * The current pose is a reference to an internal Pose object that changes dynamically.
 * Modifications made to this Pose will affect the skeleton. Unlike the bind pose, which is referenced
 * when a Skeleton object is copied or cloned, the current pose is duplicated. Multiple skeletons
 * can share the same current pose if you use Skeleton::SetPose.
 *
 * @see Pose Skeleton::GetSkinPose Skeleton::SetPose Pose::SetWorldRotations Pose::SetWorldPositions
 */
FORCE_INLINE  Pose*	Skeleton::GetPose() const
{
	Pose*	pose = m_Pose;
	if (pose != NULL)
		return pose;
	pose = MakePose(Pose::SKELETON);
	m_Pose = pose;
	return pose;
}

/*!
 * @fn const Pose* Skeleton::GetSkinPose() const
 * The skin pose is the pose the skeleton is currently in relative to it's bind pose.
 * This differs from the current pose (Skeleton::GetPose) which returns a pose
 * with locations and orientations relative to the skeleton's root joint.
 * The skin pose is a reference to an internal Pose object that changes dynamically.
 * Unlike the bind pose, which is referenced when a Skeleton object is copied or cloned,
 * the skin pose is duplicated and cannot be shared across skeletons.
 *
 * @see Skeleton::SetPose Skeleton::GetPose Pose
 */
FORCE_INLINE  const Pose*	Skeleton::GetSkinPose() const
{
	return &m_SkinPose;
}


}	// end Vixen