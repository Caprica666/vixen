#pragma once
#pragma managed(push, off)
/*!
 * @file vxphysics.h
 * @brief Container for physics engines.
 *
 * Encapsulates physics related engines
 *
 * @author Nola Donato
 * @ingroup vixen
 */

class hkMemoryRouter;
class hkLoader;
class hkpWorld;
class hkpRigidBody;
class hkaRagdollInstance;
class hkaSkeleton;
class hkaSkeletonMapper;
class hkaRagdollRigidBodyController;
class hkaPose;
class hclWorld;
class hclNamedSetupMesh;
class hclTransformSet;
class hkpPhysicsData;
class hkCpuJobThreadPool;
class hkJobQueue;
class hkpPhysicsSystem;

namespace Vixen {


/*!
 * @class Physics
 *
 * @brief Engine which contains one or more related physics systems.
 *
 * This class controls the overall properties of the physics systems,
 * loads physics contents and handles multithreading and load balancing
 * for physics. All of the Vixen engines which relate to Havok physics
 * must be children of the single Physics engine.
 *
 * @see Ragdoll RigidBody
 * @ingroup vixen
 */
class Physics : public Engine
{
public:
	VX_DECLARE_CLASS(Physics);

	Physics(hkpWorld* world = NULL);
	Physics(hkpPhysicsData* physdata);
	~Physics();

	//! Set the name of the Havok physics file to load.
	void				SetFileName(const TCHAR* name);

	//! Get the name of the Havok physics file to load.
	const TCHAR*		GetFileName() const			{ return m_FileName; }

	//! Get the number of threads for Havok physics.
	int					GetNumThreads() const		{ return m_NumThreads; }

	//! Set the number of threads to use for Havok physics.
	void				SetNumThreads(int n)		{ m_NumThreads = n; }

	//! Get the Havok physics time step in seconds.
	float				GetTimeStep() const			{ return m_TimeStep; }

	//! Get the Havok physics world.
	hkpWorld*			GetWorld()	const			{ return m_World; }

	//! Get the Havok physics data from which Havok world is constructed.

	hkpPhysicsData*		GetPhysData() const			{ return m_PhysData; }

	//! Get Havok physics thread pool.
	hkCpuJobThreadPool*	GetThreadPool()				{ return m_Threads; }

	//! Get Havok memory router.
	static	hkMemoryRouter*	GetMemory();

	//! Clear physics world and free Havok resources.
	void				Clear();

	/*
	 * Physics::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		PHYSICS_SetFileName = Engine::ENG_NextOp,
		PHYSICS_NextOp = Engine::ENG_NextOp + 20
	};

//
// Overrides
//
	virtual	void		Compute(float);
	virtual float		ComputeTime(float t);
	virtual bool		Eval(float);
	virtual bool		Do(Messenger& s, int op);
	virtual bool		Copy(const SharedObj*);
	virtual int			Save(Messenger&, int) const;
	virtual bool		OnEvent(Event* ev);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	static	bool		Startup();
	static	void		Shutdown(int threadtype = SCENE_MainThread);
	static	Physics*	Get()			{ return s_OnlyOne; }
	static	bool		IsRunning()		{ return s_IsRunning; }

protected:
	//! Load a Havok physics file.
	static bool			ReadScene(const TCHAR* filename, Core::Stream* instream, LoadEvent* event);

	//! Initialize Havok physics.
	void				Init();

	//! Create Havok physics world.
	hkpWorld*			CreateWorld(hkpPhysicsData* physdata = NULL);

	//! Add objects from input Havok world to current one.
	void				AddToWorld(hkpWorld* world);
	void				AddToWorld(hkpPhysicsData* physdata);

	Core::String		m_FileName;
	hkpWorld*			m_World;
	hkCpuJobThreadPool*	m_Threads;
	hkJobQueue*			m_JobQueue;
	hkpPhysicsData*		m_PhysData;
	float				m_TimeStep;
	int					m_NumThreads;

	static	bool		s_IsRunning;
	static Ref<Physics>	s_OnlyOne;
	THREAD_LOCAL hkMemoryRouter*	t_Memory;
};

/*!
 * @class RigidBody
 * Encapsulates a Havok rigid body object.
 * A rigid body has a transformation matrix, a shape and physical properties (like mass).
 * For each rigid body in the Havok world, Vixen creates a corresponding
 * RigidBody object when the Havok file is loaded. This objects is linked to
 * the Havok object and allows access to some of it's properties when running.
 *
 * @see RagDoll Physics Transformer
 * @ingroup vixen
 */
class RigidBody : public Transformer
{
public:
	VX_DECLARE_CLASS(RigidBody);

	RigidBody(hkpRigidBody* body = NULL);
	RigidBody(const Box3& box, bool fixed = true, float mass = 0.0f);
	~RigidBody();
	//static void	CalcBoneMatrix(const hkpRigidBody* body, Quat& orient, Vec3& pivot);

//
// Overrides
//
	virtual void		SetActive(bool);
	virtual bool		Eval(float);
	virtual	bool		OnStart();
	virtual	void		Empty();
	//virtual bool		Do(Messenger& s, int op);
	//virtual bool		Copy(const SharedObj*);
	//virtual int		Save(Messenger&, int) const;
	//virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;


protected:
	hkpRigidBody*	m_RigidBody;
};

/*!
 * @class RagDoll
 * Skeleton animated by Havok physics.
 * A RagDoll is a skeleton whose bones are driven by rigid bodies.
 * The physics simulation updates the transformation matrices of the rigid
 * bodies each frame and they control the deformation of a skin.
 * You can choose whether you want each bone of the ragdoll to be
 * controlled by animation or by physics.
 *
 * A RagDoll can control a Skin object to use Vixen skinning or it
 * can drive a ClothSkin object to use Havok cloth skinning.
 * All RagDolls must have a Physics engine as parent.
 *
 * @ingroup vixen
 * @see Skeleton Skin ClothSkin
 */
class RagDoll : public Skeleton
{
public:
	VX_DECLARE_CLASS(RagDoll);

	RagDoll(hkaRagdollInstance* hkdoll);
	RagDoll(const hkaSkeleton* hkskel = NULL);
	~RagDoll();

	enum
	{
		DYNAMIC = (USER_AVAILABLE),
		POWERED = (USER_AVAILABLE << 1),
		DRIVEN = (USER_AVAILABLE << 2)
	};

	void				DrivePhysics();
	void				OnBoneChanged();
	void				UpdatePose();
	const hkaSkeleton*	GetHavokSkeleton() const	{ return m_Skeleton; }
	hkaRagdollInstance*	GetHavokRagdoll() const		{ return m_RagDoll; }

//
// Overrides
//
	virtual void	SetControl(int val, int who = 0);
	virtual void	SetBoneOptions(int boneindex, int options);
	virtual bool	Eval(float);
	virtual bool	OnStart();
	virtual Pose*	MakePose(int coordspace) const;
	virtual void	ApplyPose(const Pose* pose);
	virtual bool	Init(int numbones = 0, const int32* parentindices = NULL);
	virtual	void	Empty();
	//virtual bool		Do(Messenger& s, int op);
	//virtual bool		Copy(const SharedObj*);
	//virtual int		Save(Messenger&, int) const;
	//virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

protected:
	void		MakeDynamic(hkpWorld* world, int layer);
	void		MakeKeyframed(hkpWorld* world, int layer);
	void		SetFriction(float friction);

	hkaPose*						m_DrivingPose;
	hkaRagdollInstance*				m_RagDoll;
	const hkaSkeleton*				m_Skeleton;
	hkaRagdollRigidBodyController*	m_RigidBodyController;
	bool							m_BoneChanged;
	bool							m_PoseChanged;
};

/*!
 * @class HavokPose
 * Set of matrices representing the pose of a Havok ragdoll.
 * The bones of a RagDoll skeleton are driven by rigid bodies.
 * The physics simulation updates the transformation matrices of these rigid
 * bodies and represents them as a Havok hkaPose object.
 * HavokPose is a subclass of Pose which contains the hkaPose object
 * and uses Havok for pose calculations. It also converts to a
 * hclTransformSet so it can be used with Havok cloth as well.
 * 
 *
 * @see Pose RagDoll Skeleton
 * @ingroup vixen
 */
class HavokPose : public Pose
{
public:
	VX_DECLARE_CLASS(HavokPose);

	HavokPose(const RagDoll* skel = NULL, int space = SKELETON);
	HavokPose(const Pose& src);
	~HavokPose();

	operator const hkaPose&();
	operator hkaPose*();
	HavokPose&		operator=(const hkaPose& src);
	HavokPose&		operator=(const hclTransformSet& src);
	void			SetTransforms(const hclTransformSet*);
	void			GetTransforms(hclTransformSet*) const;
	virtual void	Clear();
	virtual bool	Copy(const SharedObj* src_obj);
	virtual void	SetPosition(const Vec3& pos);
	virtual bool	Sync() const;

protected:
	hkaPose*		m_HavokPose;
};

/*!
 * @class SkeletonMapper
 * Provides access to Havok animation retargeting capabilities.
 * The SkeletonMapper can map a pose from one skeleton onto another.
 * It can be used to drive a high resolution skeleton from a
 * simpler one (or vice versa).
 *
 * @see Physics Skeleton RagDoll
 * @ingroup vixen
 */
class SkeletonMapper : public PoseMapper
{
public:
	VX_DECLARE_CLASS(SkeletonMapper);

	SkeletonMapper(const hkaSkeletonMapper* mapper = NULL);
	~SkeletonMapper();
	virtual	void Empty();

protected:
	virtual	bool	MapLocalToTarget();

	const hkaSkeletonMapper*	m_Mapper;
};


}	// end Vixen

#pragma managed(pop)