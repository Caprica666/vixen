#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Sequencer, Engine, VX_Sequencer);

static const TCHAR* opnames[] = {
	TEXT("Load"), TEXT("Empty"), TEXT("Begin"), TEXT("Kill"), TEXT("End"),
};

const TCHAR** Sequencer::DoNames = opnames;

/*!
 * @fn void Sequencer::Empty()
 *
 * Removes all lines from the script and deactivate all animations.
 * The animator children are left in place and connected to their
 * targets so they may be reused with another script.
 */
void Sequencer::Empty()
{
// TODO: add stream logging

	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	Engine * e;
	while (e = iter.Next())
	{
		if (e->IsClass(VX_Animator))
		{
			Animator* a = (Animator*) e;
			a->Init(false);
			a->SetActive(false);
		}
	}
}

/****
 *
 * class Scriptor override for SharedObj::Do
 *
 ****/
bool Sequencer::Do(Messenger& s, int op)
{
	TCHAR			name1[VX_MaxName];
	int32			n;
	Core::String	name2;
	SharedObj*		obj;

	switch (op)
	{
		case SEQ_Load:
		s >> name1;
		name2 = name1;
		s >> name1 >> n;
		Load(name2, name1, n);
		break;

		case SEQ_Begin:
		s >> name1;
		name2 = name1;
		s >> name1 >> obj;
		Begin(name2, name1, obj);
		break;

		case SEQ_Kill:
		s >> name1;
		Kill(name1);
		break;

		case SEQ_End:
		s >> name1;
		End(name1);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Sequencer::DoNames[op - SEQ_Load]
					   << " " << this);
#endif
	return true;
}

/*!
 * @fn Animator* Sequencer::Load(const TCHAR* animfile, const TCHAR* engname, int opts, SharedObj* target)
 * @param animfile	name of animation file to load (usually animations are in .vix or .bvh files)
 * @param engname	name of animation engine to associate with target.
 *					usually the engine name begins with the name of the engine's target model
 * @param target	target hierarchy to animate
 * @param opts		animation options
 *
 * Loads a file containing one or more animation engines.
 * If an engine name is given, only that engine will be used
 * from the input file. Otherwise, the scene manager looks for an engine whose name
 * matches the Scriptor's target. Before loading or playing an animation,
 * you must set the target hierarchy for this Scriptor.
 *
 * Loading, playing and recording are independent operations.
 * By default, an animation is loaded asynchronously, does not play
 * automatically and remains available after it has stopped playing.
 * Animation options allow you to control the startup and play behavior.
 * @code
 *	Animator::LOAD_STREAM	stream part of the animation from the file each frame.
 *							default is to load the animation asynchronously and
 *							not play until completely loaded
 *	Animator::AUTO_PLAY		start playing the animation as soon as it is available
 *	Engine::CYCLE			play the animation repeatedly
 *	Engine::PING_PONG		play the animation forward and then reverse
 *	Engine::REVERSE			play the animation backwards
 *	Engine::RECORD			record the animation results
 * @endcode
 *
 * @see Sequencer::Begin Sequencer::LoadScript Sequencer::SetTarget
 */
Animator* Sequencer::Load(const TCHAR* animfile, const TCHAR* engname, int opts, SharedObj* target)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_Load) << this << animfile << engname << int32(opts) << target;
	VX_STREAM_END(  )

	TCHAR			animname[VX_MaxPath];
	TCHAR			dirbuf[VX_MaxPath];
	Animator*		eng = NULL;
	const TCHAR*	dir = dirbuf;
	size_t			n = STRLEN(animfile);

	Core::Stream::ParseDirectory(animfile, animname, dirbuf);
	Core::String	ext(Core::String(animfile).Right(4));
	if ((ext.CompareNoCase(TEXT(".vix")) != 0) &&
		(ext.CompareNoCase(TEXT(".bvh")) != 0) &&
		(ext.CompareNoCase(TEXT(".xml")) != 0) &&
		(ext.CompareNoCase(TEXT(".hkt")) != 0))
	{
		eng = FindAnim(animname);
	}
	if (target == NULL)								// default to scriptor target
		target = GetTarget();
	if (opts & Animator::LOAD_STREAM)				// streamed navigation events?
	{
		eng = (Animator*) MakeAnim(animfile, target, false);
	}
	else if (engname)								// engine name given?
	{
		if (*engname != TEXT('.'))					// don't put extra '.'
			STRCAT(animname, TEXT("."));
		STRCAT(animname, engname);
		eng = (Animator*) MakeAnim(animname, target, true);
		eng->SetEngineName(engname);
	}
	else
	{
		eng = (Animator*) FindAnim(animfile);
		if (eng == NULL)
			eng = (Animator*) MakeAnim(animname, target, true);
	}
	if (eng)
	{
		if (target && target->IsClass(VX_Skeleton))
			GetMessenger()->Observe(target, Event::LOAD_SCENE, NULL);
		eng->Load(animfile, opts);
	}
	GetMessenger()->Observe(eng, Event::LOAD_SCENE, this);
	return eng;
}

Animator* Sequencer::MakeAnim(const TCHAR* engname, SharedObj* target, bool domerge)
{
	Core::String	animname(engname);
	Animator*		anim;

	if (animname.Right(5) != TEXT(".anim"))
		animname += TEXT(".anim");	
	anim = (Animator*) Find(animname, Group::FIND_EXACT | Group::CHILDREN);
	if (anim)
		return anim;
	if (domerge)
	{
		if (STRSTR(engname, TEXT(".skeleton")))
		{
			TCHAR *p = STRCHR((TCHAR*) engname, '.');
			if (p && (anim = FindAnim(p + 1)) && anim->IsClass(VX_BoneAnimator))
				anim = new BoneAnimator(*((BoneAnimator*) anim));
			else
				anim = new BoneAnimator;
		}
		else if (target)
		{
			GroupIter<Engine> iter(this, Group::CHILDREN);
			Engine* e;
			while (e = iter.Next())
				if (e->GetTarget() != target)
					continue;
				else if (e->IsClass(VX_BoneAnimator))
				{
					anim = new BoneAnimator(*((BoneAnimator*) e));
					break;
				}
		}
	}
	if (anim == NULL)
		anim = new Animator;
	Append(anim);
	anim->SetName(animname);
	if (target)
		anim->SetTarget(target);
	anim->SetFlags(SharedObj::DOEVENTS);
	GetMessenger()->Define(animname, anim);
	return anim;
}


/*!
 * @fn void Sequencer::Kill(const TCHAR* name)
 * @param name	name of animation to delete. If NULL, all animations
 *				used by the scriptor are deleted.
 *
 * Delete the named animation and all of the resources it uses.
 * The engine and target controlled by the animation is not deleted.
 *
 *
 * @see Sequencer::Load Sequencer::End
 */
void Sequencer::Kill(const TCHAR* name)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_Kill) << this << name;
	VX_STREAM_END( )

	Animator * eng;
	if (name && *name)
	{
		eng = (Animator*) Find(name);
		if (eng && eng->IsClass(VX_Animator))
			eng->Kill();
		return;
	}
	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	while (eng = iter.Next())
	{
		if (eng->IsClass(VX_Animator))
			eng->Kill();
	}
//	Group::Empty();
}	


/*!
 * @fn void Sequencer::End(const TCHAR* name)
 *
 * Ends the specified animation, stopping the engines 
 * under its control.
 *
 * @param name	name of animation to stop.
 *
 * @see Sequencer::Load Sequencer::Kill
 */
void Sequencer::End(const TCHAR* name)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_End) << this << name;
	VX_STREAM_END( )

	Animator* eng = MakeAnim(name);
	if (eng)
	{
		eng->MakeName(name);
		eng->Stop();
		//eng->SetActive(false);
	}
}

void Sequencer::End()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_End) << this << NULL;
	VX_STREAM_END( )

	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	Engine * e;
	while (e = iter.Next())
	{
		if (e->IsClass(VX_Animator))
		{
			((Animator*) e)->Stop();
			//e->SetActive(false);
		}
	}
}

/*!
 * @fn void Sequencer::Begin(const TCHAR* animname, const TCHAR* leader_name, SharedObj* target)
 * @param animname		name of animation to start. Animations are named after the file they
 *						came from. Only those engines whose names start with this name are played.
 * @param follow_name	name of animation to follow. If non-null, times
 *						are relative to the end of the follow animation.
 *						"$ indicates this animation should follow the animation
 *						most recently added (the last one)
 * @param target		target hierarchy to animate
 * @param playopts		play options for this animation
 *						PLAY_AFTER	play after the leader
 *						PLAY_WITH	play with the leader
 *
 * Begins the specified animation at the given time. Unless the Animator::AUTO_PLAY option
 * is set, you need to call this function to start a loaded the animation.
 * Normally, the starting time of an animation is relative to the current time.
 * If an engine to follow is provided, the animation's starting time will
 * be relative to when that engine ends.
 *
 * @see Sequencer::Load Animator::Follow Animator::Kill
 */
void Sequencer::Begin(const TCHAR* animname, const TCHAR* leader_name, SharedObj* target, int playopts)
{
//	VX_STREAM_ASYNC_BEGIN(s)
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_Begin) << this << animname << target << leader_name << playopts;
	VX_STREAM_END( )
//	VX_STREAM_ASYNC_END( )

	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	const Engine*	leader = NULL;
	Animator*		eng;

	if (leader_name && *leader_name)
		leader = (const Engine*) Find(leader_name, Group::CHILDREN | Group::FIND_EXACT);
	eng = MakeAnim(animname, target, true);
	if (playopts != 0)
		eng->SetLeader(leader, playopts);
	eng->SetActive(true);
	if (playopts == 0)
	{
		if (!(eng->m_StateFlags & Animator::ISLOADING))
			eng->Init(true);
	}
	else
	{
		eng->Init(false);
		eng->SetChanged(true);
	}
	if ((GetState() & RUNNING) == 0)
		Start();
}

void Sequencer::Begin()
{
//	VX_STREAM_ASYNC_BEGIN(s)
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Sequencer, SEQ_Begin) << this << NULL << NULL << NULL << 0;
	VX_STREAM_END( )
//	VX_STREAM_ASYNC_END( )

	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	Animator*		eng;

	while (eng = iter.Next())
	{
		int state = eng->GetState();

		if (!eng->IsClass(VX_Animator))
			continue;
		if (state & Animator::WAS_KILLED)
			continue;
		eng->SetActive(true);
		if ((eng->GetLeader() == NULL) && !(eng->GetState() & Animator::PLAYOPTS))
			eng->Init(true);
		else
		{
			eng->Init(false);
			eng->SetChanged(true);
		}
	}
	if ((GetState() & RUNNING) == 0)
		Start();
}

/*!
 * @fn Engine* Sequencer::GetRootEngine(const TCHAR* name) const
 * @param name	name of animation to search for. Only an
 *				animator with this exact name will be considered.
 *
 * Returns the root of the engine hierarchy controlled by
 * the named animator.
 *
 * @return root hierarchy or NULL on error
 *
 * @see Group::Find Sequencer::FindAnim
 */
Engine* Sequencer::GetRootEngine(const TCHAR* name) const
{
	if ((name == NULL) || (*name == 0))
		return NULL;
	const Animator* anim = (const Animator*) Find(name, Group::CHILDREN | Group::FIND_EXACT);
	if (anim)
		return anim->GetRootEngine();
	return NULL;
}

/*!
 * @fn Animator* Sequencer::FindAnim(const TCHAR* name, SharedObj* targ) const
 * @param name	name of animation to search for. Any animator
 *				name containing this string will suffice.
 * @param targ	target to search for. If NULL, any target
 *				is accepted. Otherwise only animators with
 *				this target are returned.
 *
 * Searches for the animator with the given name and target.
 *
 * @return animator found or NULL on error
 *
 * @see Group::Find Sequencer::GetRootEngine
 */
Animator* Sequencer::FindAnim(const TCHAR* name, SharedObj* targ) const
{
	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	Animator* anim;

	while (anim = iter.Next())
	{
		if (!anim->IsClass(VX_Animator))
			continue;
		if (name && (STRCASECMP(anim->GetFileName(), name) != 0))
		{
			if ((STRSTR(anim->GetName(), name) == NULL) ||
				(anim->GetRootEngine() == NULL))
			continue;
		}
		if ((targ == NULL) || (anim->GetTarget() == targ))
			return anim;
	}
	return NULL;
}

/****
 *
 * Sequencer override for Engine::Eval
 * Starts, stops and kills animations based on their state and options.
 *
 ****/
bool Sequencer::Eval(float t)
{
	Animator*	anim;
	Engine*		leader;
	float		tabs = GetStartTime() + t;	// absolute time

	GroupIter<Animator> iter((Animator*) this, Group::CHILDREN);
	while (anim = iter.Next())
	{
		bool	start = false;
		int		c = anim->GetControl();
		int		state = anim->GetState();
		float	timeofs = 0.0f;

		if (!anim->IsClass(VX_Animator) || !anim->IsActive())
			continue;
		if (anim->IsRunning())
			continue;
		leader = anim->GetLeader();
		if (state & Animator::PLAYOPTS)
		{
			if (leader == NULL)
				continue;
			if (leader->IsRunning())
			{
				if (state & Animator::PLAY_WITH)
				{
					t = leader->GetStartTime();
					if (tabs > t)			// time elapsed since leader started?
						timeofs = tabs - t;
					VX_TRACE(Animator::Debug, ("Sequencer::Eval %s with %s at %f\n", anim->GetName(), leader->GetName(), t));
				}
				else if (state & Animator::PLAY_AFTER)
				{
					t = leader->GetStopTime();
					if (c & BLEND_BETWEEN)
						t -= anim->m_BlendTime;
					VX_TRACE(Animator::Debug, ("Sequencer::Eval %s follows %s at %f\n", anim->GetName(), leader->GetName(), t));
				}
			}
			else
				continue;
		}
		else if (!anim->HasChanged())
			continue;
		else if ((c & CYCLE) == 0)
			continue;
//		if ((c & BLEND_TO) &&
//			(anim->m_TimeStart > 0))
//			t -= anim->m_TimeStart;
		anim->SetTimeOfs(timeofs);
		anim->SetStartTime(t, ONLYME | RELATIVE);
		anim->Start();
		anim->SetChanged(false);
	}
	return true;
}

bool Sequencer::OnStop()
{
	End();
	return true;
}

Engine* Sequencer::FindLast(Animator* eng)
{
	Engine*	last = NULL;
	GroupIter<Animator>	iter((Animator*) this, Group::CHILDREN);

	Animator* e;
	while (e = (Animator*) iter.Next())
	{
		if (e == eng)
			continue;
		if (!e->IsClass(VX_Animator))
			continue;
		if (e->m_StateFlags & Animator::WAS_KILLED)
			continue;
		if (!e->IsRunning())
			continue;
		last = e;
	}
	return last;
}

}	// end Vixen