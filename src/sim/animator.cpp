#include "vixen.h"

namespace Vixen {

#define	ENG_FlagMask	 (Engine::CYCLE | Engine::PING_PONG | Engine::REVERSE)

VX_IMPLEMENT_CLASSID(Animator, Engine, VX_Animator);

static const TCHAR* opnames[] =
{	TEXT("Load"), TEXT("Kill"), TEXT("SetOptions"), TEXT("Record"), TEXT("SetFileName"), TEXT("UNUSED"),
	TEXT("SetRootEngine"), TEXT("SetLeader"), TEXT("Blend"), TEXT("SetSrcRoot"), TEXT("SetDstRoot"), TEXT("SetState") };

const TCHAR** Animator::DoNames = opnames;

Animator::Animator(const TCHAR* name) : Engine()
{
	SetName(name);
	m_Options = 0;
	m_AnimClass = VX_Transformer;
	m_Leader = (Engine*) NULL;
	m_TimeStart = 0;
	m_StateFlags = 0;
	m_MaxDur = 0;
}

Animator::~Animator()
{
	Kill();
}

/*!
 * @fn void Animator::Load(const TCHAR* animfile, int opts)
 * @param animfile	name of animation file to load (usually animations are in .vix or .bvh files)
 * @param opts		animation options (see above)
 *
 * Loads a file containing one or more animation engines.
 * If an engine name is given, only that engine and its children
 * will be used from the input file. Otherwise, the scene manager looks for an engine whose
 * name matches the animator's target.
 *
 * Loading, playing and recording are independent operations.
 * By default, an animation is loaded asynchronously, does not play
 * automatically and remains available after it has stopped playing.
 * Animation options allow you to control the startup and play behavior.
 * @code
 *	Animator::LOAD_STREAM	stream part of the animation from the file each frame.
 *							default is to load the animation asynchronously and
 *							not play until completely loaded
 *	Animator::KILL_ON_STOP	unload and destroy the animation when it finishes playing
 *	Animator::AUTO_PLAY		start playing the animation as soon as it is available
 *	Engine::CYCLE			play the animation repeatedly
 * @endcode
 *
 * @see  Animator::Start Animator::SetTarget Animator::Stop Animator::Kill Animator::SetRootEngine
 */
void Animator::Load(const TCHAR* animfile, int opts)
{
//	VX_STREAM_ASYNC_BEGIN(s)
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Animator, ANIM_Load) << this << animfile << int32(opts);
//	VX_STREAM_ASYNC_END( )
	VX_STREAM_END( )

	World3D*	app = World3D::Get();

	SetFileName(animfile);
	MakeName(animfile);
	if (opts)								// new load options
		SetOptions(opts);
/*
 * Handle streaming load. Typically, streamed animations control camera movement so
 * we set up the stream to observe navigation events and dispatch them.
 * The animator keeps the stream open to read a little from the file each frame,
 * up until the STREAM_End token
 */ 
  	if (animfile && (opts & LOAD_STREAM))
  	{
  		m_Stream = new FileMessenger;
		m_Stream->SetInStream(app->GetLoader()->OpenStream(animfile));
  		m_Stream->Observe(this, Event::NAVIGATE, NULL);
  		Start();
  		return;
  	}
/*
 * If the animator file has already been loaded, grab the engines from memory.
 * Otherwise, asynchronously load the file and observe the scene load event,
 * which is handled by Animator::OnEvent
 */
	Engine* root = GetRootEngine();
	SharedObj* target = GetTarget();

	m_StateFlags &= ~WAS_KILLED;
	if (root && (m_StateFlags & ISATTACHED)) // animation still attached?
	{
		root->SetActive(true);			// reactivate it
		SetActive(true);
	}
	if (Init((m_Options & AUTO_PLAY) != 0))
		return;							// animation was already loaded
	if (animfile == NULL)
		return;
	if (GetState() & Engine::RUNNING)	// stop anything currently being animated
		Stop();
	m_StateFlags |= ISLOADING;			// mark animation as being loaded
	if (target && target->IsClass(VX_Skeleton))
		GetMessenger()->Observe(target, Event::LOAD_SCENE, target);
	GetMessenger()->Observe(this, Event::LOAD_SCENE, this);
	app->LoadAsync(animfile, this);		// load file asynchronously
	VX_TRACE(Animator::Debug, ("Animator::Load %s\n", animfile));
}

/*!
 * @fn void Animator::Kill()
 *
 * Deletes this animation. Does not delete the engines
 * it controls or the target objects.
 *
 * @see Animator::Load Animator::Stop Animator::Start Scene::Suspend
 */
void Animator::Kill()
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_Animator, ANIM_Kill) << this;
	VX_STREAM_ASYNC_END( )

	Engine*	root = GetRootEngine();

	VX_TRACE(Animator::Debug, ("Animator::Kill %s\n", GetName()));
	m_StateFlags |= WAS_KILLED;		// mark as killed
	if (m_StateFlags & FROM_FILE)	// can only kill animations from files
	{
		if (root)
			root->Remove(Group::UNLINK_FREE);// free root engine if we own it
		SetTarget(NULL);				// unlink from target
		m_DstRoot = (Engine*) NULL;
		m_Stream = (Messenger*) NULL;
		m_StateFlags &= ~ISATTACHED;
	}
	else if (root)
		root->SetActive(false);
	SetActive(false);
	m_Leader = (Engine*) NULL;			// detach from leader
	m_StateFlags &= ~(PLAYOPTS | ISLOADING | KILL_ON_STOP);
}	

/*
 * Called every frame to load more from streamed animations.
 * When the stream is empty, the animator automatically stops.
 */
bool Animator::Eval(float t)
{
	if (m_Options & LOAD_STREAM)	// handle streaming case
	{
		Messenger* stream = m_Stream;
		if (stream == NULL)
			return true;
		if (stream->IsEmpty())
			Stop();
		else
			stream->Load();
	}
	return true;
}

/*!
 * @fn const Engine* Animator::Init(bool start)
 * @param start	 true to start animation,  false to stop it
 *
 * Initializes the animation engine(s) associated with the input
 * animation and attaches them to the appropriate targets.
 * Animation engines loaded are in a hierarchy and the root of
 * this tree is returned. If this hierarchy has already been loaded,
 * those engines will be attached to the current target. If the animator
 * target is a hierarchy, each  engine in the input
 * hierarchy is attached to the corresponding node in the target hierarchy.
 * Attachment is done based on name matching.
 *
 * @return root of engine tree controlled by animator, NULL if not available
 *
 * @see Animator::Load
 */
const Engine* Animator::Init(bool start)
{
	Ref<Engine>	root = GetRootEngine();
	bool		isattached = (m_StateFlags & ISATTACHED) != 0;

	VX_TRACE(Animator::Debug, ("Animator::Init %s %d\n", GetName(), start));
/*
 * If root engine is NULL and the animation has not been attached to a target yet,
 * try to find the source engines for the animation within the scene.
 * If these are found, determine the destination engine for this animation
 * (the root of the engine hierarchy controlled by the animator).
 * If an appropriate root cannot be found, return NULL.
 */
	if (!isattached || (root == NULL))
	{
		Engine* srcroot = FindSrcRoot(this);

		if (srcroot == NULL)
			return NULL;
		VX_ASSERT(srcroot != this);
		SetRootEngine(srcroot);
		root = GetRootEngine();
		if (root == NULL)
			return NULL;
	}
/*
 * Root of controlled engine hierarchy is found. If the animation
 * should be started but is still loading, mark it as auto-play so
 * it will start automatically when it has finished loading.
 * If the animation is ready, start it now.
 */
	if (start)
	{
		m_StateFlags &= ~WAS_KILLED;
		if (m_StateFlags & ISLOADING)
			m_Options |= AUTO_PLAY;
		else
		{
			Start();
			return root;
		}
	}
/*
 * Animation has been requested to initialize in a stopped state.
 * If it is running, stop it now.
 */
	else
		Stop();
	return root;
}

/*!
 * @fn void Animator::SetLeader(const Engine* leader)
 * @param leader	Engine this animator should follow.
 *					if NULL, it will follow the last scheduled animation.
 * @param playopts	play options for this animation
 *					PLAY_AFTER	play after the leader
 *					PLAY_WITH	play with the leader
 * Establishes an engine for this animation to follow
 * If an animator follows another engine, it's starting times
 * are relative to when this engine starts or finishes. Positive start
 * times will delay the start of the animator by that amount. A negative time will
 * start this animator before it's leader finishes if PLAY_AFTER is selected.
 *
  * @see Animator::Load Animator::SetTarget Animator::Start
 */
void Animator::SetLeader(const Engine* leader, int playopts)
{
//	VX_STREAM_BEGIN(s)
//		*s << OP(VX_Animator, ANIM_SetLeader) << this << leader;
//	VX_STREAM_END( )

	playopts &= PLAYOPTS;
	m_StateFlags &= ~PLAYOPTS;
	m_StateFlags |= playopts;
	m_Leader = (Engine*) leader;
	VX_TRACE(Debug, ("Animator::SetLeader %s = %s\n", GetName(), leader ? leader->GetName() : TEXT("NONE")));
}

Engine* Animator::GetLeader() const
{
	return m_Leader;
}

/*!
 * @fn Engine* Animator::SetRootEngine(Engine* root)
 * @param root	root of hierarchy to control with this animator
 *
 * Establishes the root of the engine hierarchy to be controlled
 * by this animator. These engines need not be children of the
 * animator - they can be anywhere in the scene.
 *
 * In the default implementation, the input root hierarchy is
 * directly manipulated. In this case, it is the value returned.
 *
 * The BoneAnimator subclass merges multiple source hierarchies
 * into a single tree in the scene. Several animators will have the
 * same root engine even though they started out as separate trees.
 * Subengines are taken from the input hierarchy and grafted into
 * the controlled hierarchy. For skeletal animation the return value
 * will not necessarily be the same as the input. 
 *
 * This routine is not safe to call from anywhere but the display thread
 * unless the engines being killed are not running. To call from elsewhere
 * scene processing must be suspended.
 *
 * @see Animator::Load Animator::SetTarget Animator::Start
 */
void Animator::SetRootEngine(Engine* root)
{
	if ((root == NULL) || !root->IsClass(VX_Engine))
		VX_ERROR_RETURN(("Animator::SetRootEngine ERROR: bad engine pointer %p\n", root));

	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_Animator, ANIM_SetRootEngine) << this << root;
	VX_STREAM_ASYNC_END( )

	float maxdur = ComputeDuration(root);
/*
 * Attach the controlled engine to the target. For normal animations,
 * this is the same as the source root passed in as input.
 * For bone animations, this could be a different hierarchy which
 * the input source root is blended into by AttachEngine.
 */
	Engine* nroot = AttachEngine(root);
	if (nroot)								// found appropriate root?
	{
		SharedObj* tar = GetTarget();

		VX_ASSERT(nroot != this);
		m_DstRoot = nroot;
		if (tar)							// if this animator has a target,
			nroot->SetTarget(tar);			// make it the target of the root
		else								// use the root engine target for the animator
			Engine::SetTarget(root->GetTarget());
		VX_TRACE(Animator::Debug, ("Animator::SetRootEngine %s\n", root->GetName()));
		if (root->IsClass(VX_Skeleton))
			m_AnimClass = VX_Transformer;
		else
			m_AnimClass = root->ClassID();	// save class of root engine
		m_StateFlags |= ISATTACHED;			// mark animator as being attached
	}
	else
		{ VX_TRACE(Animator::Debug, ("Animator::SetRootEngine ERROR %s\n", GetName())); }

	SetChanged(false);						// suppress scriptor examination
}

float Animator::ComputeDuration(Engine* sroot)
{
/*
 * Find the maximum duration for all child engines.
 * This becomes the duration of the animator.
 */
	GroupIter<Engine>	iter(sroot, Group::DEPTH_FIRST);
	float				maxdur = m_MaxDur;
	const Engine*		e;

	while (e = iter.Next())					// compute maximum duration
		if ((e->GetDuration() > 0) &&		// for any single child
			(e->GetDuration() > maxdur))
			maxdur = e->GetDuration();
	if (maxdur > m_MaxDur)
	{
		m_MaxDur = maxdur;
		if (GetDuration() == 0)
			SetDuration(maxdur);
	}
	return maxdur;
}

Engine* Animator::AttachEngine(Engine* sroot)
{
	if (m_StateFlags & FROM_FILE)		// animation came from file?
	{
		sroot->Remove(Group::UNLINK_NOFREE);	// remove source root
		Append(sroot);					// put under this animator
	}
	return sroot;
}

/*!
 * @fn void	Animator::SetOptions(int opts)
 * @param opts animation options
 *
 * Establishes the animation options for the named animation.
 * These options control the behavior of this animation for
 * loading and playing.
 * @code
 *	Animator::LOAD_STREAM	stream part of the animation from the file each frame.
 *							default is to load the animation asynchronously and
 *							not play until completely loaded
 *	Animator::KILL_ON_STOP	unload and destroy the animation when it finishes playing
 *	Animator::AUTO_PLAY		start playing the animation as soon as it is available
 *	Engine::CYCLE			play the animation repeatedly
 *	Engine::PING_PONG		play the animation forward and then reverse
 *	Engine::REVERSE			play the animation backwards
 *	Engine::RECORD			record the animation results
 * @endcode
 *
 * @see Animator::Load Animator::Start Engine::SetControl
 */
void	Animator::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Animator, ANIM_SetOptions) << this << int32(opts);
	VX_STREAM_END( )

	m_Options = opts;
	opts &= ENG_FlagMask;
	if (opts == 0)
		Disable(ENG_FlagMask);
	else
		Enable(opts);
}

void Animator::SetControl(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::SetControl(flags, who);
	if (who & ONLYME)
		return;
	if (eng)
		eng->SetControl(flags, who);
}

void Animator::Enable(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::Enable(flags, who);
	if (who & ONLYME)
		return;
	if (eng)
		eng->Enable(flags, who);
}

void Animator::Disable(int flags, int who)
{
	Engine*	eng = GetRootEngine();

	Engine::Disable(flags, who);
	if (who & ONLYME)
		return;
	if (eng)
		eng->Disable(flags, who);
}

/*!
 * @fn Engine* Animator::FindSrcRoot(const Engine* root)
 * @param root	root of hierarchy to search for source in
 *
 * Searches for the root of the source engine hierarchy within
 * the input hierarchy (which comes from a scene file that was loaded).
 * The source engine is found by name matching - it has the same name
 * as this animator without the ".anim" suffix)
 *
 * @return Engine whose name matches the animator (without the ".anim")
 */
Engine* Animator::FindSrcRoot(const Engine* root)
{
	TCHAR	fullname[VX_MaxPath];
	Engine*	eng = NULL;
	TCHAR*	p;

	if (m_EngineName.IsEmpty())
	{
		STRCPY(fullname, GetName());		// get animation name
		p = STRRCHR(fullname, TEXT('.'));	// look for last dot
		if (p && (STRCASECMP(p, TEXT(".anim")) == 0))
			*p = 0;							// suppress ".anim"
	}
	else
	{
		STRCPY(fullname, GetName());		// get animation name
		p = STRCHR(fullname, TEXT('.'));	// look for first dot
		if (p)
		{
			if (*((const TCHAR*) m_EngineName) != TEXT('.'))
				++p;
			*p = 0;							// isolate filename prefix
		}
		STRCAT(fullname, m_EngineName);
	}
	eng = (Engine*) root->Find(fullname, Group::FIND_DESCEND | Group::FIND_EXACT);
	m_StateFlags |= FROM_FILE;				// indicate animation came from a file
	return eng;
}

/*!
 * @fn void Animator::SetEngineName(const TCHAR* name)
 * @param name name of engine associated with this animator
 *
 * Designates the name of the source engine to associate with
 * this animator. When a scene file is loaded, this engine
 * will be selected as the one to animate.
 *
 *
 * @see Animator::GetEngineName
 */
void Animator::SetEngineName(const TCHAR* name)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Animator, ANIM_SetEngineName) << this << name;
	VX_STREAM_END( )

	m_EngineName = name;
}


/*!
 * @fn bool Animator::OnEvent(Event* e)
 *
 * Responds to the load events generated when an animation
 * or script is finished loading.
 *
 * The scene load event contains a pointer to the scene which was loaded.
 * The name of the loaded scene is the same as the base name of the file
 * which was loaded (without extension and directory).
 * If this load event resulted from a loaded animation, there
 * will be an animator for the file. If there isn't an animator
 * with the same FileName as the scene, we ignore the load event
 * because it was not generated by an animator Load.
 */
bool Animator::OnEvent(Event* e)
{
	Scene*			scene = NULL;
	Engine*			root = NULL;
	const TCHAR*	p;
	intptr			n;

	switch (e->Code)
	{
		case Event::NAVIGATE:					// dispatch navigation event
		*GetMessenger() << *e;
		return true;

		default:
		return false;

		case Event::LOAD_SCENE:					// scene was loaded
		p = ((LoadSceneEvent*) e)->FileName;
		n = STRLEN(p) - STRLEN(m_FileName);		// compare file name lengths
		if (n < 0)								// animation file must match event file at the end 
			return false;
		if (m_FileName.CompareNoCase(p + n) != 0)
			return false;
		scene = (Scene*) (SharedObj*) ((LoadSceneEvent*) e)->Object;
		if (scene && scene->IsClass(VX_Scene))
		{
			const SharedObj* sender = e->Sender;
			root = scene->GetEngines();
			if ((sender != this) && (sender != Parent()))
				return false;					// load event not generated by us
			if (root == NULL)					// scene has no engines?
				return false;
			root = FindSrcRoot(root);
		}
		else
		{
			scene = NULL;
			root = (Engine*) (SharedObj*) ((LoadSceneEvent*) e)->Object;
			if (root == NULL)
				return false;
			if (!root->IsClass(VX_MeshAnimator) && !root->IsClass(VX_Skeleton))
				return false;
			root->Remove(Group::UNLINK_NOFREE);
		}
		m_StateFlags &= ~(ISLOADING | WAS_KILLED);
		if (GetRootEngine() == NULL)
			SetRootEngine(root);
		else if (root)
			AttachEngine(root);
		if ((m_Options & AUTO_PLAY) &&			// start automatically
			!(m_StateFlags & PLAYOPTS) &&
			(m_Leader == NULL))					// doesn't follow anything
			Init(true);
		else									// no, leave stopped
			Init(false);
		if (scene)
		{
			const TCHAR* name = scene->GetName();
			if (name != NULL)
				World3D::Get()->GetLoader()->SetSceneDict(name, NULL);
			//World3D::Get()->Unload(p);		// causes infinite loop in Save
		}
		VX_TRACE(Animator::Debug, ("Animator::Load %s finished loading\n", (const TCHAR*) m_FileName));
		SetChanged(true);
		break;
	}
	return true;
}

void Animator::OnBlend()
{
	int32	opts = GetControl() & (BLEND_TO | BLEND_BETWEEN);
	Engine* root = GetRootEngine();

	if (root == NULL)
		return;
	VX_TRACE(Animator::Debug, ("Animator::Blend %s for %f\n", GetName(), m_BlendTime));
	if (opts)
		root->Enable(opts);
	else
		root->Disable(BLEND_TO | BLEND_BETWEEN);
	if (opts & BLEND_TO)
		root->SetDuration(m_BlendTime);
}

bool Animator::OnStart()
{
	Engine*	follow = m_Leader;
	Enable(ACTIVE);
	if ((m_TimeStart > 0) && follow)
		follow->Enable(ACTIVE, CONTROL_CHILDREN);
	if (m_BlendTime != 0)
		OnBlend();
	return true;
}

/*!
 * @fn void Animator::Start(int who)
 *
 * Starts this animation and the engines in the hierarchy
 * that it is responsible for controlling. The starting time,
 * speed, duration, time offset and control options of
 * the animator are applied to the controlled hierarchy as well.
 *
 * @see Animator::Load Animator::Stop Engine::Start Animator::SetOptions
 */
void Animator::Start(int who)
{
	Engine*	eng = GetRootEngine();
	int		flags;

	Engine::Start(who);
	who = CONTROL_CHILDREN;
	if (eng == NULL)
		return;

	float		start = GetStartTime();
	VX_TRACE(Animator::Debug, ("Animator::Start %s at %f for %f sec\n", GetName(), start, GetDuration()));
//	if ((GetControl() & BLEND_TO) && (m_TimeStart > 0))
//		start += m_TimeStart;
	ObjectLock	lock(eng);
	Model*		mod = (Model*) eng->GetTarget();

	eng->SetStartTime(start);
	eng->SetSpeed(GetSpeed(), who);
	eng->SetTimeOfs(GetTimeOfs(), who);
	flags = GetControl() & (REVERSE | CYCLE);
	eng->Enable(ACTIVE | flags, who);
	flags = ~GetControl() & (REVERSE | CYCLE);
	eng->Disable(flags, who);
	VX_TRACE(Animator::Debug > 1, ("\tStart %s\n", eng->GetName()));
	/*
	 * If PLAY_RELATIVE is set and the target of this meshanimator is a model,
	 * mesh animations should be relative to the current position instead of the
	 * original mesh position. We can change the position of the mesh model
	 * to accomplish this.
	 */
	if (eng->IsClass(VX_Deformer) && mod && mod->IsClass(VX_Model))
	{
		Deformer*			deformer = (Deformer*) eng;
		const VertexArray*	restverts = deformer->GetRestLocs();
		const FloatArray*	active = deformer->GetActiveLocs();
		Box3				rest;
		Box3				now;
		Vec3				ofs(0, 0, 0);
		int					relative = GetControl() & PLAY_RELATIVE;
		Engine*				leader = GetLeader();

		if (leader && (leader->GetControl() & PLAY_RELATIVE))
			relative |= PLAY_RELATIVE;
		if (relative && restverts && active)
		{
			restverts->GetBound(&rest);
			deformer->GetBound(now);
			ofs = now.Center() - rest.Center();
			mod->SetTranslation(ofs);
			VX_TRACE(Animator::Debug, ("\tRelative %s (%0.3f, %0.3f, %0.3f\n", eng->GetName(), ofs.x, ofs.y, ofs.z));
		}
	}
	eng->Start(who);
}

void Animator::SetStartTime(float t, int who)
{
	if ((who & RELATIVE) == 0)
		m_TimeStart = t;
	else if (who & ONLYME)
		t += m_TimeStart;
	Engine::SetStartTime(t, 0);
}

/*!
 * @fn void Animator::Stop(int who)
 *
 * Stops this animation and the engines in the hierarchy
 * that it is responsible for controlling. Note that if
 * you have previously set the animator to cycle, it will restart
 * automatically again.
 *
 * @see Animator::Load Animator::Start Engine::Stop Animator::SetOptions
 */
void Animator::Stop(int who)
{
	if (!IsRunning())
		return;
	if ((Messenger*) m_Stream)		// stop streaming animation
	{
		m_Stream->Ignore(NULL, 0, NULL);
		m_Stream->Close();
		m_Stream = (Messenger*) NULL;
	}
	Engine::Stop(who);
//	SetStartTime(m_TimeStart, ONLYME);
	Engine*	eng = (Engine*) GetRootEngine();
	if (eng)
		eng->Stop();
}	

bool Animator::OnReset()
{
	if (m_Options & LOAD_STREAM)	// reload streaming animation
		Load(m_FileName, m_Options);
	if (!m_Leader.IsNull() ||
		(m_StateFlags & PLAYOPTS) ||
		(GetControl() & CYCLE))
		SetChanged(true);
	VX_TRACE(Animator::Debug, ("Animator::OnReset %s at %f\n", GetName(), GetStartTime()));
	return false;
}

/*
 * Called when the animator stops to handle the Animator::KILL_ON_STOP
 * option that will remove the animator from the hierarchy after
 * its animation has finished.
 */
bool Animator::OnStop()
{
	VX_TRACE(Animator::Debug, ("Animator::OnStop %s at %f\n", GetName(), GetStopTime()));

	if (m_Options & KILL_ON_STOP)
	{
		Ref<Animator> savethis = this;
		Kill();
		if (m_StateFlags & FROM_FILE)
			Remove(Group::UNLINK_FREE);
		return false;
	}
	if (!m_Leader.IsNull() ||
		(m_StateFlags & PLAYOPTS) ||
		(GetControl() & CYCLE))
		SetChanged(true);
	return true;
}

/*!
 * @fn void Animator::Blend(float time, int opts)
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
 * Keyframe animations and mesh deformations both support blending.
 *	
 * @see KeyBlender::Blend Engine::SetStartTime Engine::Start MeshAnimator
 */
void Animator::Blend(float time, int opts)
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_Animator, ANIM_Blend) << this << time << int32(opts);
	VX_STREAM_ASYNC_END( )

	Engine*	root = GetRootEngine();
	int		tmp = opts & (BLEND_TO | BLEND_BETWEEN);

	if (root == NULL)
		return;
	m_BlendTime = time;
	if (tmp)
	{
		Enable(tmp);
		if ((m_MaxDur > 0) && (m_TimeStart > 0) && (m_BlendTime > 0))
			m_Duration = m_MaxDur + m_TimeStart;
	}
	else
		Disable(BLEND_TO | BLEND_BETWEEN);
}

/*!
 * @fn TCHAR* Animator::GetPrefix(TCHAR* buf, const TCHAR* fullname)
 * @param buf		where to store the extracted prefix
 * @param fullname	string with complete object name
 *
 * Extracts the prefix before the dot in a 3D object name.
 * Names are of the form filename.objectname.
 */
TCHAR* Animator::GetPrefix(TCHAR* buf, const TCHAR* fullname)
{
	TCHAR* n = buf;
	if (fullname == NULL)
		return NULL;
	STRCPY(buf, fullname); 
	n = STRCHR(buf, TEXT('.'));	// find the dot
	if (n == NULL)
		return NULL;
	*n = 0;						// get only prefix
	n = STRCHR((TCHAR*) fullname, TEXT('.'));
	if (n) ++n;
	return n;
}

/*!
 * @fn TCHAR* Animator::GetTargetName(TCHAR* buf, const TCHAR* fullname)
 * @param buf		where to store the target name extracted
 * @param fullname	string containing entire name
 *
 * Extracts the suffix of the target from a 3D engine name.
 * Engine names are of the form  filename.targetname.enginetype.
 *  targetname is extracted into the input buffer.
 *
 * @return filename prefix
 */
TCHAR* Animator::GetTargetName(TCHAR* buf, const TCHAR* fullname)
{
	TCHAR* n = GetPrefix(buf, fullname);
	if (n == NULL)
		return NULL;
	return GetPrefix(buf, n);
}

bool	Animator::Copy(const SharedObj* animobj)
{
	const Animator* src = (const Animator*) animobj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Engine::Copy(animobj))
		return false;
	if (animobj->IsClass(VX_Animator))
	{
		m_StateFlags = src->m_StateFlags;
		m_Options = src->m_Options;
		m_AnimClass = src->m_AnimClass;
		m_EngineName = src->m_EngineName;
		m_FileName = src->m_FileName;
		m_DstRoot = src->m_DstRoot;
		m_Leader = src->m_Leader;
		m_TimeStart = src->m_TimeStart;
	}
	return true;

}

int Animator::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	Engine*	eng = m_Leader;
	if (h < 0)
		return h;
	s << OP(VX_Animator, ANIM_SetState) << h << m_StateFlags;
	s << OP(VX_Animator, ANIM_SetOptions) << h << m_Options;
	if (!m_EngineName.IsEmpty())
		s << OP(VX_Animator, ANIM_SetEngineName) << h << m_EngineName;
	if (eng && (eng->Save(s, opts) >= 0) && h)
		s << OP(VX_Animator, ANIM_SetLeader) << h << eng;
	eng = GetRootEngine();
	if (eng && (eng->Save(s, opts) >= 0) && h)
		s << OP(VX_Animator, ANIM_SetRootEngine) << h << eng;
	if (h <= 0)
		return h;
	return h;
}

/****
 *
 * class Animator override for SharedObj::Do
 *
 ****/
bool Animator::Do(Messenger& s, int op)
{
	TCHAR			name[VX_MaxPath];
	int32			n;
	SharedObj*		obj;

	switch (op)
	{
		case ANIM_SetFileName:
		s >> name;
		SetFileName(name);
		break;

		case ANIM_Load:
		s >> name;
		s >> n;
		Load(name, n);
		break;

		case ANIM_Kill:
		Kill();
		break;

		case ANIM_SetOptions:
		s >> n;
		SetOptions(n);
		break;

		case ANIM_SetEngineName:
		s >> name;
		SetEngineName(name);
		break;

		case ANIM_SetLeader:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Engine));
		SetLeader((Engine*) obj);
		break;

		case ANIM_SetRootEngine:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Engine));
		if (m_StateFlags & ISATTACHED)
			m_DstRoot = (Engine*) obj;
		else
			SetRootEngine((Engine*) obj);
		break;

		case ANIM_SetState:
		s >> n;
		m_StateFlags |= n;
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Animator::DoNames[op - ANIM_Load]
					   << " " << this);
#endif
	return true;
}

/*!
 * @fn void Animator::SetTarget(SharedObj* target)
 *
 * Establishes target hierarchy in the scene graph that this
 * animator should control. When the animator is directed
 * at hierarchy of engines, these engines are will be attached
 * to objects in this target hierarchy.
 *
 * If the animator target is NULL, a controlled engine tree
 * retains its original target associations.
 *
 * @see Animator::Load Animator::SetLeader Engine::SetTarget
 */
void Animator::SetTarget(SharedObj* target)
{
	Engine* root = GetRootEngine();
	if (root)
		root->SetTarget(target);
	Engine::SetTarget(target);
}

/*!
 * @fn bool Animator::SetFileName(const TCHAR* filename)
 *
 * Establishes the name of the animation file to load or record.
 * If the animator has not been given a name, the filename and
 * the engine name are used to compose a default name. The base
 * of the filename (without the extension) is used as a prefix
 * and the engine name (if given) follows. The ".anim" suffix
 * is appended to distinguish this animator from the engine
 * hierarchy it controls.
 *
 * If the caller loads engine "man.skeleton" from file "myman.vix"
 * the name of the animator becomes "myman.man.skeleton.anim"
 *
 * @return  true if input name describes a scene file, else  false
 *
 * @see Animator::Load Animator::SetLeader
 */
bool Animator::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Animator, ANIM_SetFileName) << this << filename;
	VX_STREAM_END( )

	m_FileName = filename;
	return true;	
}

const TCHAR* Animator::MakeName(const TCHAR* engname)
{
	const TCHAR*		name = GetName();
	TCHAR				animname[VX_MaxPath];
	const SharedObj*	targ = GetTarget();
	const TCHAR*		targname = NULL;

	if (name)
		return name;
	if (targ)
		targname = targ->GetName();
	*animname = 0;
	if (!m_FileName.IsEmpty())				// filename prefix supplied?
	{
		Core::Stream::ParseDirectory(m_FileName, animname);
		STRCAT(animname, TEXT("."));
		if (engname && *engname)			// engine name given?
			STRCAT(animname, engname);
		else if (targname && *targname)		// target name available?
		{
			const TCHAR* p;
			if (p = STRCHR(targname, TEXT('.')))
				STRCAT(animname, p + 1);
			else
				STRCAT(animname, name);
		}
		else
		{
			STRCAT(animname, TEXT(".anim"));
			return NULL;
		}
	}
	else if (engname)						// no filename, engine name has prefix included
		STRCAT(animname, engname);
	else if (targname)						// no filename, target name has prefix included
		STRCAT(animname, name);
	else
		return NULL;						// not enough info to make name
	STRCAT(animname, TEXT(".anim"));
	SetName(animname);
	return GetName();
}

}	// end Vixen