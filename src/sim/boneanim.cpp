#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(BoneAnimator, Animator, VX_BoneAnimator);

/*
 * In a merged hierarchy, only search on the filename prefix of the animation.
 * The filename prefix identifies all engines belonging to this animator.
 * foo.hip.skeleton, foo.hip.pos, foo.hip.rot, foo.pelvis.pos and
 * foo.pelvis.rot all belong to animator foo.hip.skeleton.anim.
 * The hierarchy may also contain bar.hip.pos, bar.hip.rot, bar.pelvis.pos
 * and bar.pelvis.rot for animator bar.hip.skeleton.anim.
 * The only unique feature is the filename prefix.
 */
inline BoneAnimator::Iter::Iter(BoneAnimator* anim)
 :	GroupIter<Engine>((Engine*) anim->GetRootEngine(), Group::DEPTH_FIRST)
{
	const TCHAR* animname = anim->GetName();
	TCHAR*		afterfilebase;
	TCHAR		searchname[VX_MaxPath];
	
	if (animname == NULL)
		return;
	STRCPY(searchname, animname);
	afterfilebase = STRCHR(searchname, '.');
	if (afterfilebase)
	{
		*afterfilebase = 0;
		m_SearchString = searchname;
	}
}

Engine* BoneAnimator::Iter::Next()
{
	Engine* eng;

	if (m_SearchString.IsEmpty())
		return NULL;
	while (eng = GroupIter<Engine>::Next())
	{
		const TCHAR* name = eng->GetName();
		if ((name == NULL) || eng->IsClass(VX_Navigator) || eng->IsClass(VX_KeyBlender))
			continue;
		if (STRNCMP(name, m_SearchString, m_SearchString.GetLength()) == 0)
			return eng;
	}
	return NULL;
}

Engine*  BoneAnimator::Iter::Next(int classid)
{
	Engine* eng;

	while (eng = GroupIter<Engine>::Next())
	{
		const TCHAR* name = eng->GetName();
		if ((name == NULL) || !eng->IsClass(classid))
			continue;
		if (m_SearchString.IsEmpty())
			return eng;
		if (STRNCMP(name, m_SearchString, m_SearchString.GetLength()) == 0)
			return eng;
	}
	return NULL;
}

BoneAnimator::BoneAnimator(const TCHAR* name) : Animator(name)
{
	m_BlendTime = 0;
	Disable(CONTROL_CHILDREN);
}

BoneAnimator::BoneAnimator(const BoneAnimator& src) : Animator(src.GetName())
{
	Copy(&src);
	m_Leader = (Engine*) NULL;
	m_TimeStart = 0;
	m_BlendTime = 0;
	m_StateFlags = 0;
	SetStartTime(0);
	SetName(NULL);
	Disable(CONTROL_CHILDREN);
}

BoneAnimator::~BoneAnimator()
{
//	Kill();
}

void BoneAnimator::Kill()
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_Animator, ANIM_Kill) << this;
	VX_STREAM_ASYNC_END( )

	Engine* eng = GetRootEngine();
	m_BlendTime = 0;
	Stop();
	SetActive(false);
	SetLeader(NULL);
	if (eng && (m_StateFlags & FROM_FILE))
	{
		Iter iter(this);
		while (eng = iter.Next())
		{
			if (eng->IsClass(m_AnimClass))
				continue;
			VX_TRACE(Animator::Debug > 1, ("\tKill %s\n", eng->GetName()));
			eng->Remove(Group::UNLINK_FREE);
		}
		m_StateFlags &= ~(MERGED | ISATTACHED);
	}
	m_StateFlags &= ~(ISLOADING | PLAYOPTS);
	m_StateFlags |= WAS_KILLED;
}	

/*!
 * @fn void BoneAnimator::Blend(float time, int opts)
 * @param time	time to blend for (in seconds). If -1, the blend time
 *				will be the entire duration of the animation
 * @param opts	how to blend: BLEND_BETWEEN blends between two
 *				active animations (the default). BLEND_TO blends
 *				from the current state to this animation
 *
 * Sets up to blend between animations. The default is to blend between
 * this animation and the one it follows. You can also blend between the
 * current state to the start of this animation. Depending on the relative
 * starting times of this animation and its prececessor, you can get different effects.
 *
 * Starting this animation before it predecessor has finished
 * will smoothly transition between the animations for the specified duration.
 * If this animation starts after its predecessor finishes, no blending
 * will occur unless Engine::BLEND_TO is enabled. This blends between the current
 * position and this animation for the blend duration.
 *
 * Blending is applied uniformly across the entire hierarchy associated
 * with this animator. It is performed by linearly interpolating an alpha
 * value during the blend transition and applying it individually to
 * he animated tracks at each node.
 *	
 * @see KeyBlender::Blend Engine::SetStartTime Engine::Start
 */
void BoneAnimator::Blend(float time, int opts)
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_BoneAnimator, ANIM_Blend) << this << time << int32(opts);
	VX_STREAM_ASYNC_END( )

	Engine*	root = GetRootEngine();
	int		tmp = opts & (BLEND_TO | BLEND_BETWEEN);

	if (root == NULL)
		return;
	if (tmp)
	{
		if ((m_Flags & MERGED) == 0)
			AttachBlenders(root);
		Enable(tmp);
	}
	else
		Disable(tmp);
	m_BlendTime = time;
	if ((m_MaxDur > 0) && (m_TimeStart > 0) && (m_BlendTime > 0))
		m_Duration = m_MaxDur + m_TimeStart;
}	

void BoneAnimator::OnBlend()
{
	Engine* root = GetRootEngine();
	if (root == NULL)
		return;

	Iter		iter(root);
	KeyBlender*	eng;
	int32		opts = GetControl() & (BLEND_TO | BLEND_BETWEEN);

	VX_TRACE(Animator::Debug, ("BoneAnimator::Blend %s for %f\n", GetName(), m_BlendTime));
	while (eng = (KeyBlender*) (Engine*) iter.Next(VX_KeyBlender))
	{
		VX_ASSERT(eng->IsClass(VX_KeyBlender));
		if (opts)
			eng->Enable(opts);
		else
			eng->Disable(BLEND_TO | BLEND_BETWEEN);
		eng->SetDuration(m_BlendTime, ONLYME);
		StartEngine(eng, ONLYME, false);
	}
}	

void BoneAnimator::Stop(int who)
{
	if (!IsRunning())
		return;
	Engine::Stop(who);
	if ((Messenger*) m_Stream)
	{
		m_Stream->Ignore(NULL, 0, NULL);
		m_Stream->Close();
		m_Stream = (Messenger*) NULL;
	}
	SetChanged(true);
}	

bool BoneAnimator::OnStop()
{
	if (!Animator::OnStop())
		return false;
	Engine* eng = GetRootEngine();
	if (eng == NULL)
		return false;

	Iter	iter(this);
	while (eng = iter.Next(VX_Interpolator))
	{
		eng->SetActive(false);
		eng->Stop(ONLYME);
		VX_TRACE(Animator::Debug > 1, ("\tStop %s\n", eng->GetName()));
	}
	return true;
}

void BoneAnimator::Start(int who)
{
	Engine*	eng = GetRootEngine();
	bool	cycle = (GetControl() & CYCLE) != 0;
	bool	relative = (GetControl() & PLAY_RELATIVE) != 0;

	SetActive(true);
	Engine::Start(who);
	if (GetState() & PLAYOPTS)
		cycle = false;
	VX_TRACE(Animator::Debug, ("BoneAnimator::Start %s at %f for %f\n", GetName(), GetStartTime(), GetDuration()));
	if (eng == NULL)
		return;

	/*
	 * If PLAY_RELATIVE is set and this is a skeleton, animations should be
	 * relative to the current position of the skeleton instead of the root
	 * bone of the bind pose. We can set an extra offset for the root
	 * bone to accomplish this.
	 */
	if (eng->IsClass(VX_Skeleton))
	{
		Skeleton* skel = (Skeleton*) eng;
		if (relative)
		{
			Vec3	pos = skel->GetPosition() - skel->GetBindPose()->GetWorldPosition(0);
			skel->SetRootOffset(pos);
			VX_TRACE(Animator::Debug, ("\tRelative %s (%0.3f, %0.3f, %0.3f)\n", eng->GetName(), pos.x, pos.y, pos.z));
		}
		else
			skel->SetRootOffset(Vec3(0, 0, 0));
	}
	Engine::Iter	iter1(this);
	while (eng = (Engine*) iter1.Next())
	{
		if (eng->IsClass(m_AnimClass))
		{
			eng->SetActive(true);
			eng->Start(ONLYME);
		}
		if (eng->IsClass(VX_KeyBlender))
			StartEngine(eng, ONLYME, cycle);
	}
	Iter	iter2(this);
	while (eng = iter2.Next())
	{
		if (!eng->IsRunning())
			StartEngine(eng, ONLYME, cycle);
	}
}


void BoneAnimator::SetControl(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::SetControl(flags, who);
	if ((who & ONLYME) || (eng == NULL))
		return;

	Iter	iter(this);
	while (eng = iter.Next())
		if (!eng->IsClass(m_AnimClass))
			eng->SetControl(flags, ONLYME);
}

void BoneAnimator::Enable(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::Enable(flags, who);
	if ((who & ONLYME) || (eng == NULL))
		return;
	VX_ASSERT(eng != this);

	Iter	iter(this);
	while (eng = iter.Next())
		if (!eng->IsClass(m_AnimClass))
			eng->Enable(flags, ONLYME);
}

void BoneAnimator::Disable(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::Disable(flags, who);
	if ((who & ONLYME) || (eng == NULL))
		return;

	Iter	iter(this);
	while (eng = iter.Next())
		if (!eng->IsClass(m_AnimClass))
			eng->Disable(flags, ONLYME);
}

void BoneAnimator::StartEngine(Engine* eng, int who, bool cycle)
{
	int32 flags = GetControl() & (REVERSE | CYCLE);
	float start = GetStartTime();

	if ((GetControl() & BLEND_TO) && (m_TimeStart > 0))
		start += m_TimeStart;	
	eng->SetStartTime(start, who);
	eng->SetSpeed(GetSpeed(), who);
	eng->SetTimeOfs(GetTimeOfs(), who);
	eng->Enable(flags | ACTIVE, who);
	flags = ~GetControl() & (ACTIVE | REVERSE | CYCLE);
	eng->Disable(flags);
	eng->Start(who);
	VX_TRACE(Animator::Debug, ("\tStart %s\n", eng->GetName()));
}

/*!
 * @fn void BoneAnimator::SetTarget(SharedObj* targobj)
 * @param targobj	target model to attach to
 *
 * Attaches the animator hierarchy to the new target hierarchy.
 * Each child of the target is attached to the transformer which contains its name.
 * Only transformer targets are affected.
 *
 * @see Engine::SetTarget
 */
void BoneAnimator::SetTarget(SharedObj* targobj)
{
	if (targobj == GetTarget())			// target did not change?
		return;
	Engine::SetTarget(targobj);
	if (targobj == NULL)				// no new target?
		return;							// we are done
	Engine*	eng = GetRootEngine();
	Model*	target = (Model*) targobj;
	if (eng)
	{
		if (target->IsClass(VX_Model))	// new target is a model?
		{
			ObjectLock elock(eng);
			ObjectLock tlock(target);
			AttachTarget(eng, target);	// attach new target
		}
	}
	else if (targobj->IsClass(VX_Skeleton))
	{
		if ((m_StateFlags & ISATTACHED) == 0)	// no root engine, nothing attached?
		{
			SetRootEngine((Engine*) targobj);	// make this the root
			m_StateFlags &= ~ISATTACHED;		// no animations attached yet
		}
	}
}

/*!
 * @fn Engine* BoneAnimator::AttachEngine(Engine* src)
 *
 * Attaches the engine hierarchy to the target hierarchy by matching names.
 * The name format for transformers is filename.targetname.xform.
 * For all transformers, we search for an engine whose name contains the name of
 * the target model in the input target hierarchy and make it the transformer's target.
 *
 * If this is the first animation to attach, the new engine hierarchy is 
 * just appended to this animator. The first animation establishes the
 * structure of the animation hierarchy. Subsequent animations are
 * "patched in" by attaching the children of each transformer in the
 * new hierarchy to the parent transformers of the existing hierarchy
 * (based on name matching). This permits several sets of keyframers
 * to be active concurrently under a single transformer so animations
 * may be smoothly blended together.
 */
Engine* BoneAnimator::AttachEngine(Engine* src)
{
/*
 * There is an existing hierarchy. Because engines may still be animating and
 * we want to blend animations, we merge the children of corresponding transformers
 * from the new hierarchy into the existing one (as opposed to maintaining 2 hierarchies)
 */
	Engine*		dst = GetRootEngine();
	SharedObj*	targ = GetTarget();

	ComputeDuration(src);
	if ((m_MaxDur > 0) && (m_TimeStart > 0) && (m_BlendTime > 0))
		m_Duration = m_MaxDur + m_TimeStart;
	if (dst == src)
		return dst;
	if (src->IsClass(VX_Skeleton))
	{
		((Skeleton*) src)->FindBones();
		src = ((Skeleton*) src)->GetBone(0);
	}
	VX_ASSERT(src != NULL);
	m_StateFlags |= ISATTACHED;
	if (dst == NULL)
	{
		if (targ && targ->IsClass(VX_Skeleton))
			dst = (Engine*) targ;
	}
	if (dst)
	{
		if (dst->IsClass(VX_Skeleton))
			dst = ((Skeleton*) dst)->GetBone(0);
		if (dst && dst->IsClass(m_AnimClass) && src->IsClass(m_AnimClass)) 
		{
			VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach %s -> %s\n", dst->GetName(), src->GetName()));
			Merge(dst, src);
			return dst;
		}
	}
	if (IsParent())							// animator has child engines?
	{
		dst = (Engine*) FindTarget(targ);	// search for root below animator
		if (dst && (dst != this) && dst->IsClass(m_AnimClass))
			return dst;						// found one? we are already attached

		GroupIter<Engine> iter(this);		// try to merge with animator hierarchy
		while (dst = iter.Next())
		{
			const TCHAR* name = dst->GetName();
			VX_ASSERT(dst != this);
			if ((dst->GetTarget() == targ) && dst->IsClass(m_AnimClass))
			{
				VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach %s -> %s", src->GetName(), targ->GetName()));
				if ((dst != src) && src->IsClass(m_AnimClass))
					Merge(dst, src);
				return dst;
			}
		}
	}
	else
	{
		VX_ASSERT(src->IsClass(VX_Transformer));
		((Transformer*) src)->SetOptions(Transformer::RELATIVE);
		Append(src);
	}
	AttachTarget(src, targ);				// attach targets to animation
	return src;
}


/*!
 * @fn int BoneAnimator::AttachTarget(Engine* root, SharedObj* targ)
 * @param root	root of engine hierarchy to attach
 * @param targ	root of modle hierarchy to attach engines to
 *
 * Attaches the engines in one hierarchy to the models in another
 * based on matching the model name. For example, an engine hierarchy
 * from file foo.dsm with engine foo.bip01.pelvis.xform will be
 * attached to model bar.bip01.pelvis.
 *
 * This routine facilitates retargeting animations from one hierarchy
 * to another. Note that it does not change the names of any objects.
 */
int BoneAnimator::AttachTarget(Engine* root, SharedObj* targ)
{
	GroupIter<Engine>	iter(root ? root : this, Group::DEPTH_FIRST);
	Engine*				srceng;
	Model*				tmp;
	const TCHAR*		name;
	TCHAR				modname[VX_MaxName];
	int					depth = 0;

	if ((targ == NULL) || !targ->IsClass(VX_Model))
		return 0;
	name = targ->GetName();
	if (root->IsClass(m_AnimClass))
		++depth;
	while (srceng = iter.Next())			// examine new engine hierarchy
	{										// find name of target model
		if (srceng->IsClass(m_AnimClass))	// is a connecting engine?
		{
			VX_ASSERT(srceng->GetName());
			GetTargetName(modname, srceng->GetName());
			tmp = NULL;
			if (targ->IsClass(VX_Model))
				tmp = (Model*) ((Model*) targ)->Find(modname);
			if (tmp == NULL)
			{
				if (srceng != root)
				{
					VX_TRACE(Animator::Debug > 1, ("BoneAnimator::ERROR cannot find target for %s\n", modname));
					continue;
				}
				tmp = (Model*) targ;
				if (name)
					STRCPY(modname, name);
			}
			srceng->SetTarget(tmp);			// attach target to engine
			srceng->OnStart();
			++depth;
			VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach %s -> %s\n", srceng->GetName(), modname));
		}
	}
	VX_ASSERT(GetRootEngine() != this);
	return depth;
}

void BoneAnimator::Merge(Engine* dstroot, Engine* srcroot)
{
	Engine*			dsteng = NULL;
	Engine*			srceng = NULL;

	VX_ASSERT(dstroot != this);
	VX_ASSERT(srcroot != this);
	if ((dstroot == NULL) || (srcroot == NULL))
		return;
	ObjectLock	lock1(dstroot);
	ObjectLock	lock2(srcroot);

	VX_ASSERT(srcroot->IsClass(m_AnimClass));
	TCHAR*			tmp;
	const TCHAR*	name = dstroot->GetName();
	TCHAR			fullname[VX_MaxName];

	if (name == NULL)
		return;
	STRCPY(fullname, name);					// copy destination root engine name
	tmp = STRCHR(fullname, TEXT('.'));		// -> after first dot
	if (tmp)
		*tmp = 0;							// isolate destination prefix
	name = srcroot->GetName();
	tmp = (TCHAR*) STRCHR(name, TEXT('.'));
	if (tmp == NULL)						// -> after first dot
		tmp = (TCHAR*) name;
	STRCAT(fullname, tmp);					// exact name of source engine
	dsteng = (Engine*) (SharedObj*) dstroot->Find(fullname, Group::FIND_DESCEND | Group::FIND_EXACT);
	srceng = srcroot->First();
	if (dsteng)								// connector found?
	{
		VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach %s -> %s\n", name, dsteng->GetName()));
	}
	else
	{
		VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach CANNOT MATCH %s\n", name));
	}
	while (srceng)
	{
		Engine* next = srceng->Next();

		if (srceng->IsClass(VX_Interpolator) && dsteng)	// animation keys?
		{
			AddKeyFramer(dsteng, (Interpolator*) srceng);
			m_StateFlags |= MERGED;
		}
		else if (srceng->IsClass(m_AnimClass))	// child bone?
			Merge(dstroot, srceng);
		srceng = next;
	}
	dstroot->SetDuration(0, ONLYME);
}

void BoneAnimator::AddKeyFramer(Engine* dsteng, Engine* srceng)
{
	const TCHAR*	searchfor = srceng->GetName();
	Engine*			kfeng = NULL;
	KeyBlender*		blendeng = NULL;
	TCHAR			blendname[VX_MaxName];
	TCHAR*			tmp;

	STRCPY(blendname, dsteng->GetName());
	tmp = STRCHR(blendname, TEXT('.'));
	if (tmp)
		*tmp = 0;
	searchfor = STRCHR(searchfor, '.');
	STRCAT(blendname, searchfor);
	kfeng = (Engine*) (SharedObj*) dsteng->Find(blendname, Group::FIND_CHILD);	// find corresponding keyframer in destination
	if (kfeng)
	{
		STRCPY(blendname, kfeng->GetName());
		if (kfeng->IsClass(VX_KeyBlender))
		{
			blendeng = (KeyBlender*) kfeng;
			kfeng = NULL;				// already have a blender
		}
	}
	if (blendeng == NULL)
	{
		Interpolator* interp = (Interpolator*) srceng;
		blendeng = new KeyBlender;		// make blender for keyframes
		STRCAT(blendname, TEXT(".blend"));
		blendeng->SetName(blendname);	// name it consistently
		blendeng->SetValSize(interp->GetValSize());
		blendeng->SetDestType(interp->GetDestType());
		dsteng->Append(blendeng);		// add to parent
		dsteng->OnStart();
	}
	if (kfeng)
	{
		kfeng->Remove(Group::UNLINK_NOFREE);	// remove, don't free
		blendeng->Append(kfeng);				// add destination keyframer
		kfeng->OnStart();
	}
	if ((const SharedObj*) blendeng->Find(searchfor, Group::CHILDREN | Group::FIND_EXACT))
		return;									// this one already there?
	srceng->Remove(Group::UNLINK_NOFREE);		// remove and don't free
	if (srceng->IsClass(VX_Interpolator))		// is position interpolator?
	{											// apply animation scale factor
		Interpolator* interp = (Interpolator*) srceng;
		if (interp->GetInterpType() < Interpolator::QSTEP)
			interp->ScaleKeys(Scriptor::GetAnimationScale());
	}
	blendeng->Append(srceng);					// add source keyframer
	srceng->OnStart();
	srceng->Stop(ONLYME);
	blendeng->OnStart();
	VX_TRACE(Animator::Debug > 1, ("BoneAnimator::Attach %s -> %s\n", srceng->GetName(), blendname));
}

/*!
 * @fn void BoneAnimator::AttachBlenders(Engine* dstroot)
 * @param dstroot	root of hierarchy to get keyframe blenders
 *
 * Attaches keyframe animation blenders to the input hierarchy.
 * This permits multiple simultaneously running keyframers to be alpha blended together.
 * It is done by putting the keyframers to be blended together as children of a KeyBlender.
 * The parent transformer then has keyblenders instead of keyframers as children.
 *
 * @see Animator::Blend KeyBlender KeyFramer
 */
void BoneAnimator::AttachBlenders(Engine* dstroot)
{
	GroupIter<KeyFramer>	iter((KeyFramer*) dstroot, Group::CHILDREN);
	KeyFramer* kfeng;

	if (dstroot == NULL)
		return;
	while (kfeng = (KeyFramer*) iter.Next())	// get next source engine
	{
		Engine* parent = kfeng->Parent();
		KeyBlender* blendeng;

		if (!kfeng->IsClass(VX_Interpolator))	// is a keyframer?
			continue;
		if (parent->IsClass(VX_KeyBlender))
			continue;
		Core::String blendname(kfeng->GetName());
		blendeng = new KeyBlender;				// make blender for keyframes
		blendname += ".blend";
		blendeng->SetName(blendname);			// name it consistently
		blendeng->SetValSize(kfeng->GetValSize());
		blendeng->SetDestType(kfeng->GetDestType());
		parent->Append(blendeng);				// add to parent
		parent->OnStart();
		kfeng->Remove(Group::UNLINK_NOFREE);
		blendeng->Append(kfeng);				// add destination keyframer
		kfeng->OnStart();
		kfeng->SetActive(false);
		VX_TRACE(Animator::Debug, ("BoneAnimator::Attach %s -> %s\n", kfeng->GetName(), blendeng->GetName()));
	}
}


}	// end Vixen