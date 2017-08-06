#include "vixen.h"
#include "physics/havok/vxphysics.h"

#include <Common/Base/KeyCode.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/Config/hkConfigVersion.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Math/QsTransform/hkQsTransform.h>
#include <Common/Base/Container/LocalArray/hkLocalBuffer.h>
#if HAVOK_SDK_VERSION_NUMBER >= 20130000
#include <Physics2012/Dynamics/hkpDynamics.h>
#include <Physics2012/Dynamics/World/hkpWorld.h>
#include <Physics2012/Dynamics/Entity/hkpEntity.h>
#include <Physics2012/Dynamics/Entity/hkpRigidBody.h>
#include <Physics2012/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics2012/Collide/Shape/hkpShape.h>
#include <Physics2012/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
#include <Physics2012/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics2012/Collide/Shape/Convex/ConvexTranslate/hkpConvexTranslateShape.h>
#include <Physics2012/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics2012/Utilities/Constraint/Bilateral/hkpConstraintUtils.h>
#include <Animation/Physics2012Bridge/Instance/hkaRagdollInstance.h>
#include <Animation/Physics2012Bridge/Controller/RigidBody/hkaRagdollRigidBodyController.h>
#include <Animation/Physics2012Bridge/Controller/PoweredConstraint/hkaRagdollPoweredConstraintController.h>
#include <Animation/Physics2012Bridge/Controller/RigidBody/hkaKeyFrameHierarchyUtility.h>
#else
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpEntity.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/ConvexTranslate/hkpConvexTranslateShape.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Utilities/Constraint/Bilateral/hkpConstraintUtils.h>
#include <Animation/Ragdoll/Instance/hkaRagdollInstance.h>
#include <Animation/Ragdoll/Controller/RigidBody/hkaRagdollRigidBodyController.h>
#include <Animation/Ragdoll/Controller/PoweredConstraint/hkaRagdollPoweredConstraintController.h>
#include <Animation/Ragdoll/Controller/RigidBody/hkaKeyFrameHierarchyUtility.h>
#endif


#include <Animation/Animation/Rig/hkaSkeleton.h>
#include <Animation/Animation/Rig/hkaPose.h>
#include <Animation/Animation/Mapper/hkaSkeletonMapper.h>
#include <Cloth/Cloth/TransformSet/hclTransformSet.h>

#if HAVOK_SDK_VERSION_NUMBER >= 20120200
#include <Physics/Constraint/Data/LimitedHinge/hkpLimitedHingeConstraintData.h>
#include <Physics/Constraint/Data/Ragdoll/hkpRagdollConstraintData.h>
#else
#include <Physics/Dynamics/Constraint/Bilateral/LimitedHinge/hkpLimitedHingeConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/Ragdoll/hkpRagdollConstraintData.h>
#endif

#include "Physics/havok/math_interop.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(RagDoll, Skeleton, VX_RagDoll);
VX_IMPLEMENT_CLASSID(SkeletonMapper, PoseMapper, VX_SkeletonMapper);
VX_IMPLEMENT_CLASSID(RigidBody, Transformer, VX_RigidBody);
VX_IMPLEMENT_CLASS(HavokPose, Pose);

#ifdef _DEBUG
/*
 * Local debug printout of Havok pose data
 */
static void PrintPose(const hkaPose* pose, const hkaSkeleton* skel)
{
	for (int j = 0; j < skel->m_bones.getSize(); ++j)
	{
		hkQsTransform hkt(pose->getBoneModelSpace(j));
		QuatH q(hkt.getRotation());
		Vec3H t(hkt.getTranslation());
		int lock = skel->m_bones[j].m_lockTranslation;
		Core::DebugPrint("%s\t(%0.3f, %0.3f, %0.3f, %0.3f)  (%0.3f, %0.3f, %0.3f) %s\n",
						skel->m_bones[j].m_name, q.x, q.y, q.z, q.w, t.x, t.y, t.z, lock ? "LOCKED" : "");
	}
}
#endif

RigidBody::RigidBody(hkpRigidBody* body)
{
	m_RigidBody = body;
	body->addReference();
}

RigidBody::~RigidBody()
{
	if (m_RigidBody && Physics::IsRunning())
		m_RigidBody->removeReference();
}

void RigidBody::Empty()
{
	if (m_RigidBody && Physics::IsRunning())
	{
		m_RigidBody->removeReference();
		m_RigidBody = NULL;
	}
	Transformer::Empty();
}

RigidBody::RigidBody(const Box3& box, bool fixed, float mass)
{
	hkVector4	halfextents(box.Width() / 2.0f, box.Height() / 2.0f, box.Depth() / 2.0f);
	Vec3		center(box.Center());
	hkpBoxShape* boxshape = new hkpBoxShape(halfextents, 0);
	hkpRigidBodyCinfo rigidBodyInfo;

	if (mass == 0.0f)
		mass = box.Width() * box.Height() * box.Depth();
	rigidBodyInfo.m_position.set(center.x, center.y, center.z);
	rigidBodyInfo.m_angularDamping = 0.0f;
	rigidBodyInfo.m_linearDamping = 0.0f;
	rigidBodyInfo.m_shape = boxshape;
	rigidBodyInfo.m_motionType = fixed ? hkpMotion::MOTION_FIXED : hkpMotion::MOTION_DYNAMIC;
	hkpInertiaTensorComputer::setShapeVolumeMassProperties(rigidBodyInfo.m_shape, mass, rigidBodyInfo);
	m_RigidBody = new hkpRigidBody(rigidBodyInfo);	// Create a rigid body (using the template above).
	boxshape->removeReference();	// Remove reference since the body now "owns" the Shape.
}

void	RigidBody::SetActive(bool active)
{
	Transformer::SetActive(active);
	if (m_RigidBody == NULL)
		return;
	m_RigidBody->lockAll();
	if (active)
		m_RigidBody->activate();
	else
		m_RigidBody->requestDeactivation();
	m_RigidBody->unlockAll();
}

bool RigidBody::OnStart()
{
	if (m_RigidBody && !m_RigidBody->isActive())
		m_RigidBody->activate();
	return Transformer::OnStart();
}

bool RigidBody::Eval(float t)
{
	SharedObj* parent = Parent();

	if (!parent->IsActive() ||
		(!parent->IsClass(VX_Skeleton) &&
		 !parent->IsClass(VX_Transformer)))
	{
		MatrixH		mtx(m_RigidBody->getTransform());

		SetTransform(&mtx);
	}
	return Transformer::Eval(t);
}

#if 0
void RigidBody::CalcBoneMatrix(const hkpRigidBody* body, Quat& orient, Vec3& pivot)
{
	hkpRigidBodyCinfo cinfo;
	const hkpShape*	shape;
	Vec3H	v[3];

	v[0].Set(1, 0, 0);
	v[1].Set(0, 1, 0);
	v[2].Set(0, 0, 1);
	body->getCinfo(cinfo);
	shape = cinfo.m_shape;
	orient.Set(0, 0, 0, 1);
	pivot.Set(0, 0, 0);
	while (shape)
	{
		switch (shape->getType())
		{
			default:
			return;

			case hkpShapeType::HK_SHAPE_CAPSULE:
			{
				const hkpCapsuleShape* s = (const hkpCapsuleShape*) shape;
				
				v[0] = s->getVertex(1);
				v[0] -= Vec3H(s->getVertex(0));
				VX_TRACE(Physics::Debug > 1, ("\tSHAPE_CAPSULE boneaxis(%f, %f, %f)", v[2].x, v[2].y, v[2].z));
				v[0].Normalize();
				v[1].Set(0, 0, 1);
				v[2] = v[0].Cross(v[1]);
				orient.Set(v);
				orient.Normalize();
			}
			return;

			case hkpShapeType::HK_SHAPE_BOX:
			{
				const hkpBoxShape* s = (const hkpBoxShape*) shape;
				Vec3H	d(s->getHalfExtents());
			}
			return;

			case hkpShapeType::HK_SHAPE_CONVEX_TRANSLATE:
			{
				const hkpConvexTranslateShape* s = (const hkpConvexTranslateShape*) shape;

				pivot = Vec3H(s->getTranslation());
				shape = s->getChildShape();
				VX_TRACE(Physics::Debug > 1, ("\tSHAPE_CONVEX_TRANSLATE (%f, %f, %f)", pivot.x, pivot.y, pivot.z));
			}
			break;

			case hkpShapeType::HK_SHAPE_CONVEX_TRANSFORM:
			{
				const hkpConvexTransformShape* s = (const hkpConvexTransformShape*) shape;
				MatrixH tmp(s->getTransform());

				tmp.GetTranslation(pivot);
				orient.Set(tmp);
				orient.Normalize();
				shape = s->getChildShape();
				VX_TRACE(Physics::Debug > 1, ("\tSHAPE_CONVEX_TRANSFORM (%f, %f, %f)", pivot.x, pivot.y, pivot.z));
			}
			break;
		}
	}
}
#endif

RagDoll::RagDoll(hkaRagdollInstance* doll)
:	m_Skeleton((hkaSkeleton*) (const hkaSkeleton*) doll->m_skeleton),
	m_RagDoll(doll),
	m_RigidBodyController(NULL),
	m_BoneChanged(false),
	m_PoseChanged(false),
	Skeleton(doll->m_rigidBodies.getSize())
{
	int numbones = doll->m_skeleton->m_bones.getSize();
	VX_ASSERT(numbones == GetNumBones());
	doll->addReference();
	SetName(m_Skeleton->m_name);
	Init(numbones);
	SetBoneAxis(Model::XAXIS);
	SetControl(CONTROL_CHILDREN);
}

RagDoll::RagDoll(const hkaSkeleton* skel)
:	m_Skeleton(skel),
	m_RagDoll(NULL),
	m_BoneChanged(false),
	m_PoseChanged(false),
	m_RigidBodyController(NULL),
	Skeleton(skel->m_bones.getSize())
{
	SetName(skel->m_name);
	m_Skeleton->addReference();
	Init(skel->m_bones.getSize());
	SetBoneAxis(Model::XAXIS);
	SetControl(CONTROL_CHILDREN);
}

bool RagDoll::Init(int numbones, const int32* parentindices)
{
	VX_TRACE(Debug > 1 || Skeleton::Debug > 1, ("RagDoll::Init %s %d bones", GetName(), numbones));
	m_BoneInfo.SetSize(numbones);
	for (int i = 0; i < numbones; ++i)
	{
		int parentid = m_Skeleton->m_parentIndices[i];
		m_BoneInfo[i].ParentID = parentid;
		SetBoneName(i, m_Skeleton->m_bones[i].m_name);
	}
	m_Pose = MakePose(Pose::SKELETON);
	Skeleton::Init(numbones, NULL);
	return true;
}

RagDoll::~RagDoll()
{
	RagDoll::Empty();
}
/*!
 * @fn void RagDoll::Empty()
 * Dereference the Haovk data structures attached to this skeleton
 * and dereference all child engines.
 *
 * @see Physics::Clear Skeleton::Empty HavokPose::Clear
 */

void RagDoll::Empty()
{
	if (Physics::IsRunning())
	{
		if (m_RigidBodyController)
			delete m_RigidBodyController;
		if (m_RagDoll)
			m_RagDoll->removeReference();
		else if (m_Skeleton)
			m_Skeleton->removeReference();
		HavokPose* pose = (HavokPose*) (Pose*) m_Pose;
		if (pose)
			pose->Clear();
	}
	m_RigidBodyController = NULL;
	m_RagDoll = NULL;
	m_Skeleton = NULL;
	Skeleton::Empty();
}

void RagDoll::SetBoneOptions(int b, int options)
{
	Skeleton::SetBoneOptions(b, options);
	m_BoneChanged = true;
}

void RagDoll::SetFriction(float friction)
{
	for (int c = 1; c < m_RagDoll->getNumBones(); c++)
	{
		hkpConstraintData* constraintData = m_RagDoll->getConstraintOfBone(c)->getDataRw();
		switch (constraintData->getType())
		{
			case hkpConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
			{
				hkpLimitedHingeConstraintData* hinge = static_cast<hkpLimitedHingeConstraintData*> (constraintData);
				hinge->setMaxFrictionTorque(friction);
				break;
			}
			case hkpConstraintData::CONSTRAINT_TYPE_RAGDOLL:
			{
				hkpRagdollConstraintData* ragdoll = static_cast<hkpRagdollConstraintData*> (constraintData);
				ragdoll->setMaxFrictionTorque(friction);
				break;
			}
		}
	}
	const hkArray<hkpConstraintInstance*>& constraints = m_RagDoll->getConstraintArray();
	hkpInertiaTensorComputer::optimizeInertiasOfConstraintTree(constraints.begin(), constraints.getSize(), m_RagDoll->getRigidBodyOfBone(0));
}

void RagDoll::MakeDynamic(hkpWorld* world, int layer)
{
	if (!(m_Control & POWERED) && (m_RigidBodyController == NULL))
		m_RigidBodyController = new hkaRagdollRigidBodyController(m_RagDoll);
	for (int b = 0; b < m_RagDoll->getNumBones(); b++)
	{
		int				opts = GetBoneOptions(b);
		hkpRigidBody*	rb = m_RagDoll->getRigidBodyOfBone(b);
		int				parentId = m_RagDoll->getParentOfBone(b);
		hkUint32		filter = hkpGroupFilter::calcFilterInfo(layer, 1, b + 1, parentId + 1);

		if (rb == NULL)
			continue;
		if ((opts & BONE_PHYSICS) || (opts == 0))
		{
			rb->setMotionType(hkpMotion::MOTION_DYNAMIC);
			rb->setQualityType(HK_COLLIDABLE_QUALITY_MOVING);
		}
		else
		{
			rb->setMotionType(hkpMotion::MOTION_KEYFRAMED);
			rb->setQualityType(HK_COLLIDABLE_QUALITY_KEYFRAMED);
		}
		rb->setCollisionFilterInfo(filter);
		world->updateCollisionFilterOnEntity(rb, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}
}

void RagDoll::MakeKeyframed(hkpWorld* world, int layer)
{
	for (int b = 0; b < m_RagDoll->getNumBones(); b++)
	{
		int				opts = GetBoneOptions(b);
		hkpRigidBody*	rb = m_RagDoll->getRigidBodyOfBone(b);
		int				parentId = m_RagDoll->getParentOfBone(b);
		hkUint32		filter = hkpGroupFilter::calcFilterInfo(layer, 1, b + 1, parentId + 1);

		if (rb == NULL)
			continue;
		if (opts & BONE_PHYSICS)
		{
			rb->setMotionType(hkpMotion::MOTION_DYNAMIC);
			rb->setQualityType(HK_COLLIDABLE_QUALITY_MOVING);
		}
		else
		{
			rb->setMotionType(hkpMotion::MOTION_KEYFRAMED);
			rb->setQualityType(HK_COLLIDABLE_QUALITY_KEYFRAMED);
		}
		rb->setCollisionFilterInfo(filter);
		world->updateCollisionFilterOnEntity(rb, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}
}

Pose*	RagDoll::MakePose(int coordspace) const
{
	Pose*	pose = new HavokPose(this, coordspace);
	int		nbones = GetNumBones();

	if (coordspace == Pose::BIND_POSE_RELATIVE)
		pose->ClearRotations();
	else
		pose->Sync();
	return pose;
}

void	RagDoll::SetControl(int val, int who)
{
	Skeleton::SetControl(val, who);
	m_BoneChanged = true;
}

void	RagDoll::OnBoneChanged()
{
	if (!m_BoneChanged)
		return;
	if (m_RagDoll)
	{
		hkpWorld*	world = m_RagDoll->getWorld();
		int			layer = 2;

		if (world == NULL)
			return;
		if (m_Control & RagDoll::DYNAMIC)
		{
			SetFriction(0.0f);
			MakeDynamic(world, layer);
			if (m_RigidBodyController)
				m_RigidBodyController->reinitialize();
		}
		else
		{
			MakeKeyframed(world, layer);
//			hkaRagdollPoweredConstraintController::stopMotors(m_RagDoll);
		}
	}
	m_BoneChanged = false;
}

bool	RagDoll::OnStart()
{
	UpdatePose();
	if (m_RigidBodyController)
		m_RigidBodyController->reinitialize();
	return true;
}


#if 1


void RagDoll::ApplyPose(const Pose* newVXpose)
{
//	return Skeleton::ApplyPose(newVXpose);

	HavokPose*		curVXpose = (HavokPose*) GetPose();
	ObjectLock		lock1(newVXpose);
	ObjectLock		lock2(curVXpose);
	int				numbones = GetNumBones();
	int				space = newVXpose->GetCoordSpace();
	hkQsTransform	worldFromModel;
	hkReal			deltaTime = 0.01f;

	const hkaPose&	curHKpose = *curVXpose;

	if (m_DrivingPose == NULL)
		m_DrivingPose = new hkaPose(m_Skeleton);
	*m_DrivingPose = curHKpose;
	VX_ASSERT(curVXpose->GetCoordSpace() == Pose::SKELETON);
	if (space == Pose::BIND_POSE_RELATIVE)
	{
		hkaPose	refHKpose(hkaPose::LOCAL_SPACE, m_Skeleton, m_Skeleton->m_referencePose);

		VX_TRACE(Debug > 1 || Skeleton::Debug > 1, ("Ragdoll::ApplyPose: %s BIND_POSE", GetName()));
		for (int i = 0; i < numbones; ++i)
		{
			const Pose::Bone&	bone = newVXpose->GetBone(i);
			QuatH				q(bone.WorldRot);

			if (bone.Changed == 0)
				continue;
			if ((m_BoneInfo[i].Options & BONE_LOCK_ROTATION) == 0)
			{
				hkQuaternion	newRot(q);
				hkQsTransform	hkt(curHKpose.getBoneModelSpace(i));
				hkQsTransform	ref(refHKpose.getBoneModelSpace(i));

				newRot.setMul(newRot, ref.getRotation());
				hkt.setRotation(newRot);
				VX_TRACE(Debug > 1 || Skeleton::Debug > 1, ("\t%s (%0.3f, %0.3f, %0.3f, %0.3f)",
						GetBoneName(i), newRot(0), newRot(1), newRot(2), newRot(3)));
				m_DrivingPose->setBoneModelSpace(i, hkt, hkaPose::PROPAGATE);
			}
		}
		m_DrivingPose->syncLocalSpace();
		for (int i = 0; i < numbones; ++i)
		{
			if (m_BoneInfo[i].Options & BONE_LOCK_ROTATION)
				m_DrivingPose->setBoneLocalSpace(i, curHKpose.getBoneLocalSpace(i));
		}
	}
	else if (space & Pose::SKELETON)
	{
		VX_TRACE(Debug > 1 || Skeleton::Debug > 1, ("Ragdoll::ApplyPose: %s SKELETON", GetName()));
		curVXpose->SetPosition(newVXpose->GetWorldPosition(0));
		for (int i = 0; i < numbones; ++i)
		{
			if ((m_BoneInfo[i].Options & BONE_LOCK_ROTATION) == 0)
			{
				QuatH			q(newVXpose->GetLocalRotation(i));
				hkQsTransform	hkt(curHKpose.getBoneLocalSpace(i));

				VX_TRACE(Debug > 1 || Skeleton::Debug > 1, ("\t%s (%0.3f, %0.3f, %0.3f, %0.3f)",
						GetBoneName(i), q.x, q.y, q.z, q.w));
				hkt.setRotation(q);
				m_DrivingPose->setBoneLocalSpace(i, hkt);
			}
		}
	}
	m_DrivingPose->syncAll();
	*curVXpose = *m_DrivingPose;
	curVXpose->SetChanged(true);
	m_PoseChanged = true;
}
#else
void RagDoll::ApplyPose(const Pose* pose)
{
	HavokPose&		curpose = *((HavokPose*) GetPose());

	Skeleton::ApplyPose(pose);
	if (m_RigidBodyController && m_UsePhysics)
	{
		hkQsTransform	worldFromModel;
		hkReal			deltaTime = 0.01f;
		const hkaPose&	newPose(curpose);

		const hkQsTransform* localPose = newPose.getSyncedPoseLocalSpace().begin();
		m_RigidBodyController->driveToPose(deltaTime, localPose, worldFromModel, NULL);
	}
}
#endif

bool	RagDoll::Eval(float t)
{
	if (m_RagDoll && (m_Control & DYNAMIC))
		UpdatePose();
	return Skeleton::Eval(t);
}

void	RagDoll::DrivePhysics()
{
	if (m_DrivingPose == NULL)
		return;

	static hkReal	deltaTime = 0.015f;
	hkQsTransform	worldFromModel(hkQsTransform::IDENTITY);

	if ((m_Control & DRIVEN) && (m_RigidBodyController == NULL))
		m_RigidBodyController = new hkaRagdollRigidBodyController(m_RagDoll);
	if (m_RigidBodyController)
	{
		hkLocalBuffer<hkaKeyFrameHierarchyUtility::Output> stressout(GetNumBones());
//		if (m_PoseChanged)
//			m_RigidBodyController->reinitialize();
		m_RigidBodyController->driveToPose(deltaTime, m_DrivingPose->getSyncedPoseLocalSpace().begin(), worldFromModel, stressout.begin());
	}
	else if (m_Control & POWERED)
	{
		hkaRagdollPoweredConstraintController::startMotors(m_RagDoll);
		hkaRagdollPoweredConstraintController::driveToPose(m_RagDoll, m_DrivingPose->getSyncedPoseLocalSpace().begin());
	}
	m_PoseChanged = false;
}

/*
 * Update the pose of the skeleton from the Ragdoll physics.
 * Only update those bones that are not marked as BONE_ANIMATE.
 */
void	RagDoll::UpdatePose()
{
	HavokPose*	pose = (HavokPose*) GetPose();
	int			nbones = m_Skeleton->m_bones.getSize();
	hkLocalBuffer<hkQsTransform> modelPose(nbones);
	ObjectLock	lock(pose);

	if (m_RagDoll == NULL)
		return;
	m_RagDoll->getPoseModelSpace(modelPose.begin(), hkQsTransform::IDENTITY);
	for (int i = 0; i < nbones; ++i)
		if ((GetBoneOptions(i) & BONE_ANIMATE) == 0)
		{
			const hkQsTransform& hktrans = modelPose[i];
			MatrixH mtx(hktrans);

			pose->SetWorldMatrix(i, mtx);
		}
}

HavokPose::HavokPose(const RagDoll* skel, int space)
	: Pose(skel, space), m_HavokPose(NULL)
{
	const hkaSkeleton* hkskel = (const hkaSkeleton*) skel->GetHavokSkeleton();
	hkaPose refpose(hkaPose::LOCAL_SPACE, hkskel, hkskel->m_referencePose);
	const hkArray<hkQsTransform>& curpose = refpose.getSyncedPoseModelSpace();
	int	numbones = skel->GetNumBones();

	for (int i = 0; i < numbones; ++i)
	{
		MatrixH mtx(curpose[i]);
		m_Bones[i].ParentID = hkskel->m_parentIndices[i];
		SetWorldMatrix(i, mtx);
	}
	if (space == BIND_POSE_RELATIVE)
	{
		for (int i = 0; i < numbones; ++i)
			SetWorldRotation(i, Quat(0.0f, 0.0f, 0.0f, 1.0f));
	}
	SetChanged(true);
}

HavokPose::HavokPose(const Pose& src)
	: Pose(src), m_HavokPose(NULL)
{ }

HavokPose::~HavokPose()
{
	Clear();
}

void HavokPose::Clear()
{
	if (m_HavokPose)
	{
		if (Physics::IsRunning())
			delete m_HavokPose;
		m_HavokPose = NULL;
	}
}

HavokPose::operator const hkaPose&()
{
	if (m_HavokPose == NULL)
	{
		const RagDoll* skel = (const RagDoll*) (const Skeleton*) m_Skeleton;
		const hkaSkeleton* hkskel = skel->GetHavokSkeleton();
		m_HavokPose = new hkaPose(hkaPose::LOCAL_SPACE, hkskel, hkskel->m_referencePose);
		Sync();
	}
	return *m_HavokPose;
}

HavokPose::operator hkaPose*()
{
	if (m_HavokPose == NULL)
	{
		const RagDoll* skel = (const RagDoll*) (const Skeleton*) m_Skeleton;
		const hkaSkeleton* hkskel = skel->GetHavokSkeleton();
		m_HavokPose = new hkaPose(hkaPose::LOCAL_SPACE, hkskel, hkskel->m_referencePose);
		Sync();
	}
	return m_HavokPose;
}

void HavokPose::SetPosition(const Vec3& pos)
{
	hkQsTransform hkt;

	Pose::SetPosition(pos);
	if (m_HavokPose)
	{
		m_HavokPose->syncAll();
		hkt = m_HavokPose->getBoneModelSpace(0);
		hkt.setTranslation(hkVector4(pos.x, pos.y, pos.z));
		m_HavokPose->setBoneModelSpace(0, hkt, hkaPose::PROPAGATE);
		m_HavokPose->syncAll();
	}
}

HavokPose&	HavokPose::operator=(const hkaPose& pose)
{
	ObjectLock	lock(this);
	bool		posechanged = false;
	int			n = GetNumBones();
	hkaPose*	havokpose = *this;

	Sync();
	pose.syncAll();
	if (m_CoordSpace == BIND_POSE_RELATIVE)
	{
		hkaPose	refHKpose(hkaPose::LOCAL_SPACE, havokpose->getSkeleton(), havokpose->getSkeleton()->m_referencePose);
		refHKpose.syncModelSpace();
		for (int i = 0; i < n; ++i)
		{
			const hkQsTransform& curpose = havokpose->getBoneModelSpace(i);
			const hkQsTransform& nextpose = pose.getBoneModelSpace(i);
			if (!curpose.isApproximatelyEqual(nextpose))
			{
				QuatH q;
				hkQsTransform hkt;
				hkt.setMulInverseMul(refHKpose.getBoneModelSpace(i), nextpose);
				q = hkt.getRotation();
				SetWorldRotation(i,	q);		// rotation with respect to bind pose
				posechanged = true;
			}
		}
	}
	else
		for (int i = 0; i < n; ++i)
		{
			const hkQsTransform& curpose = havokpose->getBoneModelSpace(i);
			const hkQsTransform& nextpose = pose.getBoneModelSpace(i);
			if (!curpose.isApproximatelyEqual(nextpose))
			{
				MatrixH mtx(nextpose);
				SetWorldMatrix(i, mtx);
				posechanged = true;
			}
		}
	if (posechanged)
	{
		*havokpose = pose;
		SetChanged(true);
		Pose::Sync();
	}
#if 0
	// GetWorldMatrix does not give back the right Havok matrix
	m_HavokPose->syncModelSpace();
	for (int i = 0; i < curpose.getSize(); ++i)
	{
		MatrixH	mtx;
		hkQsTransform newtrans;
		hkQsTransform oldtrans = m_HavokPose->getBoneModelSpace(i);
		
		GetWorldMatrix(i, mtx);
		newtrans = mtx;
		m_HavokPose->setBoneModelSpace(i, newtrans, hkaPose::DONT_PROPAGATE);
	}
#endif
	return *this;
}

HavokPose&	HavokPose::operator=(const hclTransformSet& src)
{
	SetTransforms(&src);
	return *this;
}

void	HavokPose::GetTransforms(hclTransformSet* tset) const
{
	ObjectLock	lock(this);
	int			numbones = GetNumBones();

	tset->m_transforms.setSize(numbones);
	tset->m_inverseTransposes.setSize(numbones);
	for (int i = 0; i < numbones; ++i)
	{
		MatrixH mtx;
		GetWorldMatrix(i, mtx);
		tset->m_transforms[i] = mtx;
	}
	tset->updateInverseTransposesGeneric();
}

void	HavokPose::SetTransforms(const hclTransformSet* pose)
{
	ObjectLock	lock(this);
	int			numbones = GetNumBones();

	VX_ASSERT(pose->m_transforms.getSize() == numbones);
	Sync();
	for (int i = 0; i < numbones; ++i)
	{
		MatrixH mtx(pose->m_transforms[i]);
		SetWorldMatrix(i, mtx);
		if (m_HavokPose)
			m_HavokPose->setBoneModelSpace(i, mtx, hkaPose::DONT_PROPAGATE);
	}
}

bool HavokPose::Sync() const
{
	HavokPose*	cheat = (HavokPose*) this;
	int			n = GetNumBones();
	bool		haschanged = HasChanged();
	bool		didsync = Pose::Sync();

	if (m_HavokPose == NULL)
		return didsync;
	if (haschanged)
	{
		MatrixH		mtx;
		hkQsTransform hkt;

		for (int i = 0; i < n; ++i)
		{
			const Bone&	bone = GetBone(i);
			MatrixH		mtx;
			hkQsTransform hkt;
		
			GetWorldMatrix(i, mtx);
			hkt = mtx;
			m_HavokPose->setBoneModelSpace(i, hkt, hkaPose::PROPAGATE);
		}
	}
	m_HavokPose->syncAll();
	return didsync;
}


bool HavokPose::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	const HavokPose* src = (const HavokPose*) src_obj;
	if (src->IsKindOf(&HavokPose::ClassInfo))
    {
		const RagDoll*	skeleton = (const RagDoll*) (const Skeleton*) src->m_Skeleton;
		const hkaSkeleton* hkskel = (const hkaSkeleton*) skeleton->GetHavokSkeleton();

		if (m_HavokPose)
		{
			delete m_HavokPose;
			m_HavokPose = NULL;
		}
		src->Sync();
		if (src->m_HavokPose)
		{
			m_HavokPose = new hkaPose(hkskel);
			*m_HavokPose = *(src->m_HavokPose);
		}
		return Pose::Copy(src_obj);
    }
	return Pose::Copy(src_obj);
}

SkeletonMapper::SkeletonMapper(const hkaSkeletonMapper* mapper)
	:	m_Mapper(mapper)
{
	if (mapper)
		mapper->addReference();
}

SkeletonMapper::~SkeletonMapper()
{
	if (m_Mapper && Physics::IsRunning())
		m_Mapper->removeReference();
	m_Mapper = NULL;
}

void SkeletonMapper::Empty()
{
	if (m_Mapper && Physics::IsRunning())
		m_Mapper->removeReference();
	m_Mapper = NULL;
	Engine::Empty();
}

bool SkeletonMapper::MapLocalToTarget()
{
	RagDoll*	skelA = (RagDoll*) (Skeleton*) m_Source;
	RagDoll*	skelB = (RagDoll*) GetDest();

	if ((skelA == NULL) || !skelA->IsClass(VX_RagDoll))
		return false;
	if ((skelB == NULL) || !skelB->IsClass(VX_RagDoll))
		return false;

	HavokPose*	poseA = (HavokPose*) skelA->GetPose();
	HavokPose*	poseB = (HavokPose*) skelB->GetPose();
	const hkaSkeleton*	hkskelA = (const hkaSkeleton*) skelA->GetHavokSkeleton();
	const hkaSkeleton*	hkskelB = (const hkaSkeleton*) skelB->GetHavokSkeleton();

	if ((poseA == NULL) || (poseB == NULL))
		return false;
	ObjectLock			lock1(poseA);
	ObjectLock			lock2(poseB);
	const hkaPose&		hkposeA = (const hkaPose&) *poseA;
	hkaPose				poseOut(hkaPose::LOCAL_SPACE, hkskelB, hkskelB->m_referencePose);

	VX_ASSERT(hkskelA == m_Mapper->m_mapping.m_skeletonA);
	VX_ASSERT(hkskelB == m_Mapper->m_mapping.m_skeletonB);
	poseOut.syncAll();
	m_Mapper->mapPose(hkposeA, poseOut, hkaSkeletonMapper::CURRENT_POSE);
	poseOut.syncAll();
	*poseB = poseOut;
	return true;
}

}	// end Vixen