/*
 * SWIG interface file for animation engines
 */
namespace Vixen
{
	%ignore ScriptParser;
	%ignore BVHLoader;
	%ignore Skeleton::MakeBoneMap;
	%ignore Skeleton::DefaultBoneNames;
	%ignore PoseMapper::DAZBoneNames;
	%ignore PoseMapper::AMYBoneNames;
	%ignore Skeleton::GetSkinPose;
	%ignore Skeleton::SetBindPose(const Quat*, const Vec3*);
	%ignore ComputeThread;
	%ignore ComputeThreadPool;
	%ignore Engine::Eval;
	%ignore Engine::OnStart;
	%ignore Engine::OnStop;
	%ignore Engine::OnReset;
	%ignore Evaluator::GetDest;
	%ignore Evaluator::SetDest;
	%ignore Evaluator::SetResult;
	%ignore Evaluator::GetResult;
	%ignore Interpolator::GetKey;
	%ignore Interpolator::SelectKeys;
	%ignore ColorInterp::Eval;
	%ignore TimeInterp::Compute;
	%ignore TimeInterp::OnStart;
	%ignore TimeInterp::ComputeChildren;
	%ignore Skin::GetBoneWeights;
	%ignore Skin::GetBoneIndices;
	%ignore	KeyBlender;
	%ignore BoneAnimator::Iter;
	%ignore Deformer::Init;
	%ignore Deformer::MakeVertexCache;
	%ignore Deformer::ComputeVertexMap;
	%ignore	Pose::SetWorldPositions(const Vec3*);
	%ignore	Pose::GetWorldPositions(Vec3*);
	%ignore	Pose::SetWorldRotations(const Quat*);
	%ignore	Pose::GetWorldRotations(Quat*);
	%ignore	Pose::SetLocalRotations(const Quat*);

	%apply	SharedObj*	{ Engine* };
	%apply	SharedObj*	{ Pose* };
	%apply	SharedObj*	{ Skeleton* };
	%apply	SharedObj*	{ Skin* };
	%apply	SharedObj*	{ Animator* };
	%apply	SharedObj*	{ BoneAnimator* };
	%apply	SharedObj*	{ Interpolator* };
	%apply	SharedObj*	{ KeyFramer* };
	%apply	SharedObj*	{ TimeInterp* };
	%apply	SharedObj*	{ Evaluator* };
	%apply	SharedObj*	{ MeshAnimator* };
	%apply	SharedObj*	{ Trigger* };
	%apply	SharedObj*	{ Sequencer* }
	%apply	SharedObj*	{ Scriptor* }

	%extend Pose {
		void SetWorldPositions(const float* floatArray)
		{
			return $self->SetWorldPositions((const Vec3*) floatArray);
		}
		void GetWorldPositions(float* floatArray)
		{
			return $self->GetWorldPositions((Vec3*) floatArray);
		}
		void SetWorldRotations(const float* floatArray)
		{
			return $self->SetWorldRotations((const Quat*) floatArray);
		}
		void GetWorldRotations(float* floatArray)
		{
			return $self->GetWorldRotations((Quat*) floatArray);
		}
		void SetLocalRotations(const float* floatArray)
		{
			return $self->SetLocalRotations((const Quat*) floatArray);
		}
	}
}

ATTR_READONLY(Vixen::Engine, int, State);
ATTR_READONLY(Vixen::Engine, float, Elapsed,);
ATTR_READONLY(Vixen::Engine, float, StopTime);
ATTR_DEFINE(Vixen::Engine, Vixen::SharedObj, Target);
ATTR_DEFINE(Vixen::Engine, float, StartTime);
ATTR_DEFINE(Vixen::Engine, int, Control);
ATTR_DEFINE(Vixen::Engine, float, Speed);
ATTR_DEFINE(Vixen::Engine, float, Duration);
ATTR_DEFINE(Vixen::Engine, float, TimeOfs);
ATTR_DEFINE(Vixen::Evaluator, int, DestType);
ATTR_DEFINE(Vixen::Evaluator, int, ValSize);
ATTR_DEFINE(Vixen::Evaluator, float, Alpha);
ATTR_DEFINE(Vixen::Interpolator, int, InterpType);
ATTR_DEFINE(Vixen::Interpolator, Vixen::FloatArray, Keys);
ATTR_DEFINE(Vixen::KeyFramer, int, Part);
ATTR_DEFINE(Vixen::Transformer, Vixen::Vec3, Position);
ATTR_DEFINE(Vixen::Transformer, Vixen::Vec3, Scale);
ATTR_DEFINE(Vixen::Transformer, Vixen::Quat, Rotation);
ATTR_DEFINE(Vixen::Transformer, Vixen::Quat, LookRotation);
ATTR_DEFINE(Vixen::Transformer, Vixen::Matrix, Transform);
ATTR_DEFINE(Vixen::Transformer, int, BoneIndex);
ATTR_DEFINE(Vixen::Trigger, int, Options);
ATTR_DEFINE(Vixen::Trigger, Vixen::Model, Geometry);
ATTR_DEFINE(Vixen::Trigger, Vixen::SharedObj, Colliders);
ATTR_DEFINE(Vixen::Skeleton, Vixen::Vec3, BoneAxis);
ATTR_READONLY(Vixen::Skeleton, Vixen::Box3, Bound);
ATTR_READONLY(Vixen::Skeleton, int, NumBones);
ATTR_DEFINE(Vixen::Skeleton, Vixen::Vec3, Position);
ATTR_DEFINE(Vixen::Skeleton, Vixen::Pose, Pose);
ATTR_DEFINE(Vixen::Skeleton, Vixen::Pose, BindPose);
ATTR_DEFINE(Vixen::Deformer, Vixen::VertexArray, RestLocs);
ATTR_DEFINE(Vixen::Skin, Vixen::Skeleton, Skeleton);
ATTR_DEFINE(Vixen::Skin, Vixen::IntArray, VertexMap);
ATTR_DEFINE(Vixen::PoseMapper, Vixen::Skeleton, Source);
ATTR_DEFINE(Vixen::Scriptor, const char*, Directory);
ATTR_READONLY(Vixen::Morph, Vixen::ObjArray, Sources);
ATTR_READONLY(Vixen::Morph, Vixen::FloatArray, Weights);
ATTR_DEFINE(Vixen::MeshAnimator, float, TimeInc);
ATTR_READONLY(Vixen::MeshAnimator, int, NumSources);

%include "sim/vxengine.h"
%include "sim/vxtrigger.h"
%include "sim/vxinterpolator.h"
%include "sim/vxkeyframe.h"
%include "sim/vxtransformer.h"
%include "sim/vxdeformer.h"
%include "sim/vxmorph.h"
%include "sim/vxpose.h"
%include "sim/vxskeleton.h"
%include "sim/vxskin.h"
%include "sim/vxtrigger.h"
%include "sim/vxposemapper.h"
%include "sim/vxanimator.h"
%include "sim/vxsequencer.h"
%include "sim/vxscriptor.h"
%include "sim/vxmeshanim.h"




