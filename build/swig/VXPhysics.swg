/*
 * SWIG interface file for animation engines
 */
typedef void	hkpWorld;
typedef void	hkaSkeleton;
typedef void	hclWorld;
typedef void	hkxScene;

namespace Vixen
{
	%ignore	RigidBody;
	%ignore HavokPose;
	%ignore Physics::Physics(hkpPhysicsData*);
	%ignore Physics::GetThreadPool;
	%ignore Physics::GetMemory;
	%ignore Physics::GetWorld;
	%ignore Physics::GetPhysData;
	%ignore	RagDoll::RagDoll(hkaRagdollInstance* hkdoll);
	%ignore RagDoll::GetHavokSkeleton;
	%ignore RagDoll::GetHavokRagdoll;
	%ignore ClothSim::GetThreadPool;
	%ignore ClothSim::GetWorld;
	%ignore ClothSim::GetHavokScene;
	%ignore ClothSim::GetGroundCollide;
	%ignore ClothSkin::CothSkin(hclClothData*);
	%ignore ClothSkin::ClothSkin(hclClothData* data);
	%ignore SetupCloth;
	%apply	SharedObj*	{ Physics* };
	%apply	SharedObj*	{ Cloth* };
	%apply	SharedObj*	{ ClothSkin* };
	%apply	SharedObj*	{ RagDoll* };
}

ATTR_DEFINE(Vixen::Physics, const char*, FileName);
ATTR_DEFINE(Vixen::SkeletonMapper, Vixen::RagDoll, Source);
ATTR_DEFINE(Vixen::ClothSim, Vec4, GroundPlane);

%include "physics/havok/vxphysics.h"
%include "physics/havok/vxcloth.h"




