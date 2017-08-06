#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(Engine, Group, VX_Engine);
VX_IMPLEMENT_CLASSID(MouseEngine, Engine, VX_MouseEngine);

static const TCHAR* opnames[] =
{	TEXT("SetSpeed"), TEXT("SetStartTime"), TEXT("SetDuration"),
	TEXT("SetTarget"), TEXT("RemoveTarget"), TEXT("Compute"),
	TEXT("Start"), TEXT("Stop"), TEXT("Reset"), TEXT("SetControl"),
	TEXT("Enable"), TEXT("Disable"), TEXT("SetTimeOfs")
};

const TCHAR** Engine::DoNames = opnames;

#ifndef VX_NOTHREAD
ComputeThreadPool* Engine::s_ComputeThreads = NULL;
#endif

/*!
 * @fn Engine::Engine()
 *
 * The default engine settings are:
 * @code
 * StartTime	0
 * Speed		1
 * Control		ACTIVE
 * Duration		0
 * TimeOfs		0
 * Target		NULL
 * @endcode
 *
 * With these settings, the engine will start as soon as it is
 * put in the simulation tree of a displaying scene. It will run
 * forever and will have no target object.
 *
 * @see Engine::SetControl Engine::Start
 */
Engine::Engine() : Group()
{
	m_Speed = 1.0f;
	m_Duration = 0.0f;
	m_StartTime = 0.0f;
	m_Elapsed = 0.0f;
	m_State = IDLE;
	m_TimeOfs = 0.0f;
	m_EvalTime = 0.0f;
	m_Control = ACTIVE;
}

Engine::Engine(const Engine& src) : Group(src)
{
	m_Speed = src.m_Speed;
	m_Duration = src.m_Duration;
	m_StartTime = src.m_StartTime;
	m_Elapsed = src.m_Elapsed;
	m_State = src.m_State;
	m_Control = src.m_Control;
}

Engine::~Engine()
{

}

/*!
 * @fn void Engine::SetTarget(SharedObj* target)
 * @param target Target object that engine can update
 *
 * In a distributed environment, the target object may be the only
 * object guaranteed accessible to this engine.
 * This should be the primary object the engine updates.
 * The target of a distributed engine is automatically distributed
 * with the engine.
 *
 * Circular references are normally not permitted because they cause
 * memory to be leaked. Because of this, an Engine should not select
 * any of it's ancestors in the hierarchy as the target object.
 * There is one circular reference which is useful and is allowed.
 * An engine may set it's PARENT as a target object.
 * Some engines default to using the parent as a target if none is
 * specified. 
 *
 * @see Engine::FindTarget Engine::RemoveTarget
 */
void Engine::SetTarget(SharedObj* target)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetTarget) << this << target;
	VX_STREAM_END( )

	m_Target = target;
	if (target && IsGlobal())
		GetMessenger()->Distribute(target, 0);
}


/*!
 * @fn const Engine* Engine::FindTarget(const SharedObj* target) const
 * @param target	object to check for engine target, may not be NULL
 *
 * Finds the first child engine that refers to the given target.
 * The children of this engine are examined in a breadth-first
 * manner to check for the desired target. The first child
 * which has the input object as its target is returned.
 * If no children reference the target but this engine does,
 * the engine itself is returned.
 *
 * @return engine found or NULL if no engine has object as target
 *
 * @see Engine::SetTarget Engine::GetTarget
 */
const Engine* Engine::FindTarget(const SharedObj* target) const
{
	GroupIter<Engine>	iter(this, Group::CHILDREN);
	const Engine*			eng;

	if (target == NULL)
		return NULL;
	while (eng = iter.Next())
		if (eng = eng->FindTarget(target))
			return eng;
	if (GetTarget() == target)
		return this;
	return NULL;
}

/*!
 * @fn void Engine::SetActive(bool f)
 * @param f  true to enable,  false to disable
 *
 * Determines whether the engine and possibly its children
 * will be visited during simulation. This routine only
 * changes the active state of  this engine, none of
 * its children are changed. (You can use Engine::SetControl
 * with ACTIVE to activate or deactivate an engine hierarchy.)
 * The state of the engine is also untouched
 * (inactive engines can return  true from IsRunning).
 *
 * Marking an engine as \b inactive (by passing  false to this routine)
 * will cause it to be skipped during simulation (it's  Eval routine
 * will not be called.) If the engine controls its children (CONTROL_CHILDREN
 * control flag is set), they will not be evaluated either, even if they
 * are active.
 *
 * Inactive engines can still receive events and time still passes for them.
 * This means that reactivating an engine may result in a time jump
 * when it begins evaluating again.
 *
 * @see Group::SetActive Engine::SetControl Engine::GetState
 */
void Engine::SetActive(bool f)
{
	Group::SetActive(f);
	if (f)
		Core::InterlockOr(&m_Control, ACTIVE);
	else
		Core::InterlockAnd(&m_Control, ~ACTIVE);
}

/*!
 * @fn void Engine::SetSpeed(float speed, int who)
 * @param speed	engine speed
 * @param who	which engine(s) to update:
 * @code
 *		Engine::ONLYME	only update this engine
 *		Engine::CONTROL_CHILDREN update child engines
 *		else use SetControl to determine
 * @endcode
 *
 * A speed of 1.0 is considered normal speed.
 * Values less than one indicate slower than normal
 * and greater than one are faster than normal.
 * Speed actually controls the engine's concept of time passing.
 * The input time to the Engine::Eval function is multiplied
 * by the speed. Negative speeds are not prohibited but are not
 * guaranteed to work for most engines. 
 *
 * If the engine has the  Engine::CONTROL_CHILDREN flag enabled, setting the
 * speed of a parent engine will set the speed of alls of
 * its children. Otherwise, the  who argument controls who is affected.
 *
 * @see Engine::SetControl Engine::Compute Engine::SetControl
 */
void Engine::SetSpeed(float speed, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetSpeed) << this << s << int32(who);
	VX_STREAM_END( )

	m_Speed = speed;
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;

		while (e = (Engine*) iter.Next())
			e->SetSpeed(speed, who);
	}
}

/*!
 * @fn void Engine::SetStartTime(float t, int who)
 * @param t		start time (in seconds).
 * @param who	which engine(s) to update an how to interpret time.
 * @code
 *		ONLYME			 only update this engine
 *		CONTROL_CHILDREN update child engines
 *		RELATIVE		 input duration relative to current duration
 * @endcode
 *
 * If the engine has the  CONTROL_CHILDREN flag enabled,
 * its children are not visited unless it is has been started -
 * even if they have start times that are earlier.
 * The start time can be  relative to the current time
 * (RELATIVE) or to the start time of the application.
 * 
 * @see Engine::SetControl Engine::GetElapsed Engine::Eval Engine::Compute World::StartTime Engine::SetControl
 */
void Engine::SetStartTime(float t, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetStartTime) << this << t << int32(who);
	VX_STREAM_END( )

	if (who & RELATIVE)
		m_StartTime += t;
	else m_StartTime = t;
	VX_TRACE2(Engine::Debug, ("Engine::SetStartTime(%0.3f) %s\n", t, GetName()));
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;
		while (e = (Engine*) iter.Next())
			e->SetStartTime(t, who);
	}
}

/*!
 * @fn void Engine::SetDuration(float d, int who)
 * @param d		length of time the engine should run (seconds)
 * @param who	which engine(s) to update an how to interpret time.
 * @code
 *		ONLYME			 only update this engine
 *		CONTROL_CHILDREN update child engines
 *		RELATIVE		 input duration relative to current duration
 * @endcode
 *
 * When the duration has been exceeded, the engine is
 * automatically reset or stopped. A duration of zero seconds
 * indicates the engine should run forever.
 * If you set  CONTROL_CHILDREN in this call or it is
 * already set, the duration is also set
 * for the children of this engine.
 * The input duration can be  relative to the current duration
 * (RELATIVE) or an absolute duration (the default).
 *
 * @see Engine::SetControl Engine::SetStartTime Engine::SetSpeed  Engine::SetControl
 */
void Engine::SetDuration(float d, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetDuration) << this << d << int32(who);
	VX_STREAM_END( )

	if (who & RELATIVE)
		m_Duration += d;	
	else m_Duration = d;
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;
		while (e = (Engine*) iter.Next())
			e->SetDuration(d, who);
	}
}


/*!
 * @fn void   Engine::SetTimeOfs(float t, int who)
 * @param t		new time offset in seconds.
 * @param who	which engine(s) to update an how to interpret time.
 * @code
 *		ONLYME			 only update this engine
 *		CONTROL_CHILDREN update child engines
 *		RELATIVE		 start time relative to current time
 * @endcode
 *
 * This time is added to the real time to start playing an animation
 * in the middle of its sequence rather than only at the beginning.
 * This permits the same input data to be used for multiple,
 * simultaneous animations but produce different behaviors.
 * You should not use a negative value as the absolute offset.
 *
 * If you set CONTROL_CHILDREN in this call or it is
 * already set, the time offset is also set for children.
 *
 * @see Engine::SetControl Engine::SetStartTime Engine::SetSpeed Engine::SetDuration  Engine::SetControl
 */
void   Engine::SetTimeOfs(float t, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetTimeOfs) << this << t << int32(who);
	VX_STREAM_END()

	if (who & RELATIVE)
		m_TimeOfs += t;
	else m_TimeOfs = t;
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	   {
		GroupIter<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;
		while (e = (Engine*) iter.Next())
			e->SetTimeOfs(t, who);
	   }
}

/*!
 * @fn void Engine::Enable(int flags, int who)
 * @param flags	engine control flags to enable.
 * @param who	determines which engines to set control flags for
 *				(either CONTROL_CHILDREN or ONLYME)
 *
 * Enables one or more of the following control flags for this engine
 * and possibly its children.
 * @code
 *	REVERSE		causes the engine to operate in the reverse direction.
 *	CYCLE		causes the engine to repeat when its duration
 *				has been exhausted.
 *	PING_PONG
 *				causes the engine to operate alternately in forward
 *				and reverse directions when its duration is exhausted.
 *	ACTIVE		causes engine to be evaluated (the default)
 *	CONTROL_CHILDREN
 *				causes the operation of the parent engine to
 *				control its children. If set, children are
 *				not visited unless the parent is active and
 *				its Start, Stop, Reset and Eval
 *				functions return  true when called.
 *	CHILDREN_FIRST
 *				causes the engine's children to be evaluated first.
 * @endcode
 * If you set CONTROL_CHILDREN in either argument
 * or it is already set for the root engine,
 * the control  flags passed are also set in the children.
 * Specifying ONLYME will leave children unaffected.
 *
 * @see Engine::SetControl Engine::SetActive Engine::Disable Engine::GetState
 */
void Engine::Enable(int flags, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_Enable) << this << int32(flags) << int32(who);
	VX_STREAM_END( )

	Core::InterlockOr(&m_Control, flags);
	if (flags & ACTIVE)
		SetActive(true);
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine> iter(this, Group::CHILDREN);
		Engine*			 e;
		while (e = (Engine*) iter.Next())
			e->Enable(flags, who);
	}
}

/*!
 * @fn void Engine::Disable(int flags, int who)
 * @param flags	engine control flags to disable.
 * @param who	determines which engines to set control flags for
 *				(either ENG_ControlChildren or ENG_OnlyMe)
 *
 * Disables one or more of the following control flags for this engine
 * and possibly its children.
 * @code
 *	REVERSE		causes the engine to operate in the reverse direction.
 *	CYCLE		causes the engine to repeat when its duration
 *				has been exhausted.
 *	PING_PONG
 *				causes the engine to operate alternately in forward
 *				and reverse directions when its duration is exhausted.
 *	ACTIVE		causes engine to be evaluated (the default)
 *	CONTROL_CHILDREN
 *				causes the operation of the parent engine to
 *				control its children. If set, children are
 *				not visited unless the parent is active and
 *				its Start, Stop, Reset and Eval
 *				functions return  true when called.
 *	CHILDREN_FIRST
 *				causes the engine's children to be evaluated first.
 * @endcode
 * If you set CONTROL_CHILDREN in either argument
 * or it is already set for the root engine,
 * the control  flags passed are also set in the children.
 * Specifying ONLYME will leave children unaffected.
 *
 * @see Engine::SetControl Engine::SetActive Engine::Enable Engine::GetState
 */
void Engine::Disable(int flags, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_Disable) << this << int32(flags) << int32(who);
	VX_STREAM_END( )

	Core::InterlockAnd(&m_Control, ~flags);
	if (flags & ACTIVE)
		SetActive(false);
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine> iter(this, Group::CHILDREN);
		Engine*			 e;
		while (e = (Engine*) iter.Next())
			e->Disable(flags, who);
	}
}

/*!
 * @fn void Engine::SetControl(int flags, int who)
 * @param flags	engine control flags to set.
 * @param who	determines which engines to set control flags for
 *				(either CONTROL_CHILDREN or ONLYME)
 *
 * Controls the operation of the engine using one or more of
 * these control flags:
 * @code
 *	REVERSE		causes the engine to operate in the reverse direction.
 *	CYCLE		causes the engine to repeat when its duration
 *				has been exhausted.
 *	PING_PONG
 *				causes the engine to operate alternately in forward
 *				and reverse directions when its duration is exhausted.
 *	ACTIVE		causes engine to be evaluated (the default)
 *	CONTROL_CHILDREN
 *				causes the operation of the parent engine to
 *				control its children. If set, children are
 *				not visited unless the parent is active and
 *				its Start, Stop, Reset and Eval
 *				functions return  true when called.
 *	CHILDREN_FIRST
 *				causes the engine's children to be evaluated first.
 * @endcode
 * If you set CONTROL_CHILDREN in either argument
 * or it is already set for the root engine,
 * the control  flags passed are also set in the children.
 * Specifying ONLYME will leave children unaffected.
 *
 * @see Engine::Disable Engine::SetActive Engine::Enable Engine::GetState
 */
void Engine::SetControl(int flags, int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_SetControl) << this << int32(flags) << int32(who);
	VX_STREAM_END( )

	Core::InterlockSet(&m_Control, flags);
	if (flags & ACTIVE)
		SetActive(true);
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine> iter(this, Group::CHILDREN);
		Engine*			 e;
		while (e = (Engine*) iter.Next())
			e->SetControl(flags, who);
	}
}

/*!
 * @fn void Engine::Start(int who)
 * @param who	control flags. If you set CONTROL_CHILDREN in this
 *				call or it is already set, child engines are started, too.
 *				Using ONLYME will start this engine but not its children.
 *
 * Marks an engine as  idle by setting its state to  IDLE.
 * During the simulation pass, engines marked as idle are started
 * if the start time of the engine is later than the current time.
 *
 * @see Engine::OnStart Engine::Eval Engine::Stop Engine::OnStop Engine::SetControl Engine::ComputeTime Engine::GetState
 */
void Engine::Start(int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_Start) << this << int32(who);
	VX_STREAM_END( )

	Core::InterlockSet(&m_State, IDLE);
	if (who & ONLYME)
		return;
	if ((who | m_Control) & CONTROL_CHILDREN)
	{
		GroupIter<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;

		while (e = (Engine*) iter.Next())
			e->Start(who);
	}
}

/*!
 * @fn void Engine::DoStart(float starttime)
 * @param starttime	start time of engine 
 *
 * Called from the simulation thread to start an idle engine running.
 * The Engine::OnStart method is called to initialize the engine.
 * If the return value of OnStart is  true, the engine is
 * started by changing the state from  IDLE to  RUNNING.
 * This will cause Engine::Eval to be called for this engine.
 * A  false return value indicates a start failure. The engine
 * state is not changed, children are not visited and Eval
 * will not be called for this engine.
 *
 * Note: this function does no locking - it is only supposed to be
 * called during simulation tree traversal.
 *
 * @see Engine::Start Engine::OnStart Engine::Eval Engine::OnStop Engine::ComputeTime Engine::GetState
 */
void Engine::DoStart(float starttime)
{
	VX_TRACE(Engine::Debug, ("Engine::Start: %s\n", GetName()));
	if (!OnStart())
		return;
	Core::InterlockSet(&m_State, RUNNING | (m_Control & REVERSE));
	m_Elapsed = 0;
	m_EvalTime = 0;
	m_StartTime = starttime;
/*
 * Log the Start event
 */
	if (IsSet(SharedObj::DOEVENTS))
	{
		Event* e = new Event(Event::START, 0, this);
		e->Time = m_StartTime;
		e->Log();
	}
/*
 * If the OnStart was successful and we control our children,
 * start them, too
 */
	if (m_Control & CONTROL_CHILDREN)
	{
		GroupIterNotSafe<Engine>	iter(this, Group::CHILDREN);
		Engine*				e;

		m_State = RUNNING | (m_Control & REVERSE);
		while (e = (Engine*) iter.Next())
			if (e->m_State & IDLE)
				e->DoStart(starttime);
	}
}

/*!
 * @fn void Engine::Stop(int who)
 * @param who	control flags. If you set CONTROL_CHILDREN in this
 *				call or it is already set, child engines are stopped, too.
 *				Using ONLYME will stop this engine but not its children.
 *
 * Attempts to stop and engine and possibly its children.
 * The Engine::OnStop method is called to stop the engine.
 * If it returns  false, the engine is not stopped but will continue running
 * and calling Engine::Eval.
 * If  true is returned, the engine state is set to  DONE and
 * Eval will no longer be called for this engine. If the engine controls
 * its children, Stop is called for each child.
 *
 * @see Engine::OnStop Engine::Start Engine::OnStart Engine::ComputeTime Engine::OnReset Engine::GetState
 */
void Engine::Stop(int who)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Engine, ENG_Stop) << this << int32(who);
	VX_STREAM_END( )

	if (m_State & DONE)				// already done, don't stop again
		return;
/*
 * Log the Stop event
 */
	if (OnStop())
	{
		Core::InterlockSet(&m_State, DONE);
		VX_TRACE(Engine::Debug, ("Engine::Stop: %s\n", GetName()));
		if (IsSet(SharedObj::DOEVENTS))
		{
			Scene* scene = GetMainScene();	
			float time = scene ? scene->GetTime() : World3D::Get()->GetTime();
			Event* e = new Event(Event::STOP, 0, this);
			e->Time = time;
			e->Log();
		}
	}
	else
		who |= ONLYME;
/*
 * If the OnStop was successful and we control our children,
 * stop them, too
 */
	if (who & ONLYME)
		return;
	if ((m_State & DONE) && ((who | m_Control) & CONTROL_CHILDREN))
	{
		GroupIter<Engine> iter(this, Group::CHILDREN);
		Engine*			 e;
		while (e = (Engine*) iter.Next())
			e->Stop(who);
	}
}


/*!
 * @fn bool Engine::DoReset(float t)
 *
 * Called from the simulation thread to reset an engine whose
 * duration has been exhausted. If the engine is not cycling
 * (CYCLE not set in control flags), it cannot be reset and
 * this routine returns  false.
 *
 * The Engine::OnReset method is called to reset the engine.
 * If the return value of OnReset is  true, the engine is
 * considered to be successfully reset and  true is returned.
 *
 * If the engine controls its children (CONTROL_CHILDREN set
 * in control flags), the top level children are reset also.
 *
 * @returns  true if engine reset successfully, else  false.
 *
 * @see Engine::SetDuration Engine::OnReset Engine::SetControl Engine::GetState
 */
bool Engine::DoReset(float t)
{
	if ((m_Control & CYCLE) == 0)		// engine not cycling?
		return false;					// cannot reset
	if (m_Control & PING_PONG)
		m_State = m_State ^ REVERSE;	// TODO: some interlock thing here?
/*
 * Log the Reset event
 */
	VX_TRACE(Engine::Debug, ("Engine::DoReset: %s\n", GetName()));
	if (IsSet(SharedObj::DOEVENTS))
	{
		Event* e = new Event(Event::RESET, 0, this);
		e->Time = t;
		e->Log();
	}
/*
 * Restart the engine
 */
	float dt = t - m_StartTime;
	dt -= m_Duration / m_Speed;
	m_StartTime = t;
	m_Elapsed = 0;
	m_EvalTime = 0;
	bool rc = OnReset();
	if (rc)
		Core::InterlockSet(&m_State, RUNNING | (m_Control & REVERSE));
	else
		return false;
	if (dt > 0)
	{
		m_Elapsed = dt;
		m_EvalTime = dt * m_Speed;
	}
	if ((m_Control & CONTROL_CHILDREN) == 0)
		return rc;
/*
 * Restart its children
 */
	GroupIterNotSafe<Engine> iter(this, Group::CHILDREN);
	Engine*			 e;
	while (e = (Engine*) iter.Next())
		e->DoReset(t);
	return true;
}

/*!
 * @fn bool Engine::OnStart()
 *
 * This function is called by  Engine::DoStart when an idle engine
 * is started. The default implementation just returns  true.
 *
 * When  true is returned, the engine is marked as running by
 * setting the engine state to RUNNING. This will cause
 * the Engine::Eval function to be called during simulation
 *
 * If  false is returned, the engine is not started and
 * Eval will not be called for this engine.
 *
 * @return  true indicates engine started,  false don't start engine
 *
 * @see Engine::ComputeTime Engine::Eval Engine::SetControl Engine::Start Engine::DoStart Engine::GetState
 */
bool Engine::OnStart()
{
	return true;
}

/*!
 * @fn bool Engine::OnStop()
 *
 * This function is called by Engine::Stop when the engine
 * is stopped.  Subclasses of Engine can override this function
 * to customize their stop behavior. The default implementation
 * just returns  true. 
 *
 * When  true is returned, the engine is marked as stopped by
 * setting the state to DONE.
 *
 * If OnStop returns  false, neither the engine nor its children
 * will not be stopped and Eval will continue
 * to be called for this engine.
 *
 * @return  true indicates engine stopped,  false continue running
 *
 * @see Engine::ComputeTime Engine::SetControl Engine::OnStart Engine::Stop Engine::Start Engine::GetState
 */
bool Engine::OnStop()
{
	return true;
}

/*!
 * @fn bool Engine::OnReset()
 *
 * This function is called by Engine::ComputeTime when the engine
 * is restarted. This only happens if the CYCLE flag is
 * set to repeat the engine cycle. Subclasses of Engine can override
 * this function to customize their recycling behavior.
 * The default implementation just returns  true.
 *
 * If  false is returned, it signals failure to reset the engine
 * to the caller. In this case, the OnReset functions of the
 * child engines are not called, even if CONTROL_CHILDREN is set,
 * and the caller will try to stop this engine.
 *
 * @return  true means successful reset,  false means engine should be stopped
 *
 * @see Engine::Compute Engine::SetControl Engine::DoReset
 */
bool Engine::OnReset()
{
	return true;
}

/*!
 * @fn bool Engine::Eval(float t)

 * @param t	elapsed time since engine started in seconds
 *
 * This function is called by Engine::Compute internally every frame to
 * recompute the engine state for the current time.
 * Subclasses of Engine should override this function to
 * define the behavior of the engine. The default implementation
 * just returns  true.
 *
 * If the CONTROL_CHILDREN flag is set, the children
 * of this engine are not visited if Eval does not return  true.
 *
 * @return  true indicates children should be visited,  false to not visit them
 *
 * @see Engine::Compute Engine::ComputeTime Engine::SetControl Engine::GetElapsed
 */
bool Engine::Eval(float t)
{
	return true;
}

DebugOut& Engine::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Group::Print(dbg, opts & ~PRINT_Trailer);
	const SharedObj* target = m_Target;
	void* vobj = (void*) target;
	endl(dbg << "\t<attr name='Control'>" << m_Control << "</attr>");
	if (m_Speed != 1.0f)
		endl(dbg << "\t<attr name='Speed'>" << m_Speed << "</attr>");
	if (m_StartTime != 0.0f)
		endl(dbg << "\t<attr name='StartTime'>" << m_StartTime << "</attr>");
	if (m_Duration != 0.0f)
		endl(dbg << "\t<attr name='Duration'>" << m_Duration << "</attr>");
	if (m_TimeOfs != 0.0f)
		endl(dbg << "\t<attr name='TimeOfs'>" <<  m_TimeOfs << "</attr>");
	if (m_Elapsed != 0.0f)
		endl(dbg << "\t<attr name='Elapsed'>" << m_Elapsed << "</attr>");
	if (target && (target != Parent()))
	{
		endl(dbg << "\t<attr name='Target'>");
		target->Print(dbg, PRINT_Summary);
		endl(dbg << "</attr>");
	}
	return Group::Print(dbg, opts & PRINT_Trailer);
}

bool Engine::Copy(const SharedObj* src_obj)
{
	const Engine* src = (const Engine*) src_obj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Group::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_Engine))
	{
		m_Control = src->m_Control;
		m_Speed = src->m_Speed;
		m_Duration = src->m_Duration;
		m_StartTime = src->m_StartTime;
		m_Elapsed = src->m_Elapsed;
		m_TimeOfs = src->m_TimeOfs;
		m_Target = src->m_Target;
	}
	return true;
}

/*!
 * @fn void Engine::Compute(float time)
 * @param time	time in seconds since application started	
 *
 * Processes this engine hierarchy for a given scene
 * by calling the Engine::Eval functions for this engine and
 * its children.
 *
 * An engine and its children are only processed if the
 * engine is active. An engine can be started if:
 *	1. it has been explicitly activated by Engine::SetActive
 *	2. the current time is greater than its start time
 * An engine is stopped if:
 *	1. it is explicitly stopped by Engine::SetActive or Engine::Stop
 *	2. the duration of the engine has expired
 *
 * Before Compute automatically turns an engine on,
 * it calls Engine::OnStart. When an engine is automatically
 * shut off, Engine::OnStop is called. If the duration of
 * an engine is zero, it will never be automatically stopped
 * or restarted.
 *
 * An engine can be caused to repeat by setting the
 *  CYCLE control flag. Each time the engine duration is
 * exhausted, Engine::OnReset is called, the elapsed time is
 * reset and the engine continues.
 *
 * If the  CONTROL_CHILDREN control flag is set, child engines
 * will be started, stopped and reset along with the parent.
 *
 * The order of engine evaluation is normally to call the parent [Eval]
 * function first and then each of the child [Eval] functions. If the
 *  CHILDREN_FIRST control flag is set, the children are evaluated
 * before the parent.
 *
 * The engine's  speed and  time  offset are used to control
 * the actual time passed to Engine::Eval for the engine.
 * Unless the class overrides Engine::ComputeTime, the elapsed
 * time since the engine started is multiplied by the speed and the
 * time offset is added to the result to get the evaluation time.
 *
 * @see Engine::SetControl Engine::SetActive Engine::Eval Engine::Stop
 * @see Engine::Start Engine::OnReset Engine::ComputeTime
 */
void Engine::Compute(float t)
{
	Lock();
	bool		do_my_kids_no_matter_what = !(m_Control & CONTROL_CHILDREN);
	bool		kids_first = (m_Control & CHILDREN_FIRST) != 0;
	bool		eval_says_do_my_kids = true;
	float		eval_t = ComputeTime(t);
	bool		do_my_evals = (eval_t >= 0);
	bool		isactive = IsActive();
	Unlock();

	if (!IsParent())
	{
		if (do_my_evals)
			DoEval(eval_t);
		return;
	}
	// evaluate engine before children
	if (do_my_evals && !kids_first)
		eval_says_do_my_kids = DoEval(eval_t);
	// evaluate children
	if (eval_says_do_my_kids && (isactive || do_my_kids_no_matter_what))
		ComputeChildren(t);
	// evaluate engine (after children)
	if (do_my_evals && kids_first)
		DoEval(eval_t);
}


bool Engine::DoEval(float t)
{
	VX_TRACE2(Engine::Debug, ("Engine::Eval: %s\n", GetName()));
	return Eval(t);
}

/*!
 * @fn float Engine::ComputeTime(float t)
 * @param t	elapsed time since engine started in seconds.
 *
 * This framework function is called internally to compute the
 * time to use to evaluate this engine. The default behavior is
 * to start the engine if it is idle, and reset or stop it when
 * the elapsed time exceeds the engine duration. Implementing
 * engine reversal is also handled here.
 *
 * The engine's  speed and  time  offset are used to control
 * the actual time passed to Engine::Eval for the engine.
 * Unless the class overrides this function, the elapsed
 * time since the engine started is multiplied by the speed and the
 * time offset is added to the result to get the evaluation time.
 *
 * Subclasses can override this function to control how the current
 * time is mapped to the engine evaluation time. For example,
 * the time interpolator> engine overrides ComputeTime to control
 * the evaluation of its children using an array of input times.
 *
 * <B>Implementation</B>
 *
 * If the engine is not active, has already finished a -1 time value
 * is returned to indicate it should not be evaluated. Otherwise,
 * the time offset is added to the input time and this becomes the
 *  elapsed time - see Engine::GetElapsed.
 *
 * If the engine is idle, Engine::DoStart is called to start it. This
 * will call the Engine::OnStart method in this engine and possibly
 * its children. If Engine::OnStart returns  true the engine is
 * started. Otherwise -1 is returned.
 *
 * If the engine is not active or has already finished a -1 time value
 * is returned to indicate it should not be evaluated.
 * 
 * The  duration of an engine controls how long the engines
 * will run until it is restarted or stopped. A duration of 0
 * indicates the engine should run forever and the system will
 * not automatically stop or reset these engines. If the elapsed
 * time exceeds the duration, and the engine is cycling (CYCLE
 * is set) Engine::OnReset is called to restart the engine and possibly
 * its children. If it returns  true the engine is considered to
 * have restarted and its elapsed time becomes zero again.
 *
 * If the engine is not cycling or could not be restarted, Engine::Stop
 * is called. This will call Engine::OnStop for the engine and possibly
 * its children. If it returns  true the engine is stopped and
 * a -1 time is returned to suppress evaluation.
 *
 * If the engine is operating in  reverse mode (REVERSE is set),
 * the elapsed time is subtracted from the duration and this time is
 * returned, causing the engine to be evaluated in reverse time.
 * Otherwise, the elapsed time is returned asis.
 *
 * @return evaluation time for this engine or -1 to suppress evaluation
 *
 * Note: this function does no locking. It should only be called
 * during simulation tree traversal.
 *
 * @see Engine::Start Engine::DoReset Engine::Stop Engine::SetTimeOfs Engine::SetSpeed Engine::Compute TimeInterp
 */
float Engine::ComputeTime(float t)
{
	if (!IsActive())
		return -1;
	VX_TRACE2(Engine::Debug, ("Engine::ComputeTime(%0.3f) %s State=0x%X\n", t, GetName(), m_State));
	if ((m_State & (IDLE | DONE | RUNNING)) != RUNNING)
	{
		if ((m_State & IDLE) &&				// waiting for start?
			(m_StartTime >= 0.0f) &&		// start time not later?
			(t >= m_StartTime))
			DoStart(t);						// start it now
		else
			return -1;
	}

	float dt = t - m_StartTime - m_Elapsed;
	float evalt;
//	VX_ASSERT(dt >= 0);
	m_Elapsed += dt;
	m_EvalTime += dt * m_Speed;
	evalt = m_EvalTime + m_TimeOfs;
	if (m_Duration > 0)				// duration of 0 means run forever
	{
		if (evalt >= m_Duration)
		{
			if (!DoReset(t))		// engine was not reset?
				Stop(0);			// engine stopped?
			if (m_State & DONE)		// really did stop?
				return -1;			// tell the caller
			evalt = m_EvalTime + m_TimeOfs;
		}
		if (m_State & REVERSE)	// running in reverse?
			return m_Duration - evalt;
	}
	return evalt;
}

/*!
 *
 * @fn void Engine::ComputeChildren(float t, int filter)
 * @param t			elapsed time since engine started
 * @param filter	if non-zero, filter is the class ID of those children
 *					we wish to execute (if it is positive) or skip (if negative)
 *
 * Called during simulation to evaluate the children
 * of this engine. The default implementation can either use CILK
 * for tasking or a simpler pthreads implementation. Engines which
 * are designated as TASK_PARALLEL will be spawned as parallel tasks.
 * This function waits for all tasks to finish before continuing.
 * Serial tasks are executed in the main thread.
 *
 *
 * Note: This function does no locking. It is designed to be called
 * during simulation tree traversal.
 *
 * @see Engine::Compute Engine::ComputeTime Engine::Eval Engine::SetControl
 */
void Engine::ComputeChildren(float t, int filter)
{
	GroupIterNotSafe<Engine> iter(this, Group::CHILDREN);
	Engine*	g;
	vint32	parallel_tasks = 0;
	vint32	serial_tasks = 0;

	while (g = iter.Next())		// run these in parallel
	{
		if ((g->GetControl() & TASK_PARALLEL) == 0)
		{
			++serial_tasks;
			continue;
		}
		if ((filter > 0) && !g->IsClass(filter))
			continue;
		if ((filter < 0) && g->IsClass(-filter))
			continue;
		cilk_spawn g->Compute(t);
		Core::InterlockInc(&parallel_tasks);
	}
	if (serial_tasks)
	{
		iter.Reset(Group::CHILDREN);
		while (g = iter.Next())		// run these serially
		{
			if (g->GetControl() & TASK_PARALLEL)
				continue;
			if ((filter > 0) && !g->IsClass(filter))
				continue;
			if ((filter < 0) && g->IsClass(-filter))
				continue;
			g->Compute(t);
		}
	}
}


/****
 *
 * class Engine override for SharedObj::Do
 *		ENG_SetSpeed		<float>
 *		ENG_SetStartTime	<float>
 *		ENG_SetDuration		<float>
 *		ENG_Compute			<float>
 *		ENG_SetTarget		<SharedObj*>
 *		ENG_RemoveTarget	<SharedObj*>
 *
 ****/
bool Engine::Do(Messenger& s, int op)
{
	SharedObj*		obj;
	float		f;
	int32		who, flags;

	switch (op)
	{
		case ENG_SetControl:
		s >> flags >> who;
		SetControl(flags, who);
		break;

		case ENG_Enable:
		s >> flags >> who;
		Enable(flags, who);
		break;

		case ENG_Disable:
		s >> flags >> who;
		Disable(flags, who);
		break;

		case ENG_SetSpeed:
		s >> f >> who;
		SetSpeed(f, who);
		break;

		case ENG_SetStartTime:
		s >> f >> who;
		SetStartTime(f, who);
		break;

		case ENG_SetDuration:
		s >> f >> who;
		SetDuration(f, who);
		break;

		case ENG_SetTimeOfs:
		s >> f >> who;
		SetTimeOfs(f, who);
		break;

		case ENG_Start:
		s >> who;
		Start(who);
		break;

		case ENG_Stop:
		s >> who;
		Stop(who);
		break;

		case ENG_Compute:
		s >> f;
		Compute(f);
		break;

		case ENG_SetTarget:
		s >> obj;
		SetTarget(obj);
		break;

		default:
		return Group::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Engine::DoNames[op - ENG_SetSpeed]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class Engine override for SharedObj::Save
 *
 ****/
int Engine::Save(Messenger& s, int opts) const
{
	int32 h = Group::Save(s, opts);
	const SharedObj* target = GetTarget();
	const Group*	par = this;

	if (h < 0)
		return h;
	if (target)
	{
		while ((par = par->Parent()))	// avoid circular references
		{
			if (target == par)			// don't save target if it is a parent
			{
				if (h)
					s << OP(VX_Engine, ENG_SetTarget) << h << target;
				target = NULL;
				break;
			}
		}
		if (target && (target->Save(s, opts) >= 0) && h)
			s << OP(VX_Engine, ENG_SetTarget) << h << target;
	}
	if (h <= 0)
		return h;
	if (GetSpeed() != 1.0f)
		s << OP(VX_Engine, ENG_SetSpeed) << h << GetSpeed() << int32(ONLYME);
	if (GetStartTime() != 0.0f)
		s << OP(VX_Engine, ENG_SetStartTime) << h << GetStartTime() << int32(ONLYME);
	if (GetDuration() != 0.0f)
		s << OP(VX_Engine, ENG_SetDuration) << h << GetDuration() << int32(ONLYME);
	if (GetTimeOfs() != 0.0f)
		s << OP(VX_Engine, ENG_SetTimeOfs) << h << GetTimeOfs() << int32(ONLYME);
	if (GetControl() != 0)
		s << OP(VX_Engine, ENG_SetControl) << h << (int32) GetControl() << int32(ONLYME);
	return h;
}


}	// end Vixen