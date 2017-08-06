/*!
 * @file vxengine.h
 * @brief encapsulates a real-time code pieces.
 *
 * Sections of code which are designed to run real-time every frame
 * and modify the 3D world are described in terms of engines.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxscene.h vxmodel.h
 */
#pragma once
#undef RELATIVE

namespace Vixen {

class ComputeThreadPool;
class ComputeThread;

/*!
 * @class Engine
 * @brief Object that can animate other objects over time.
 *
 * Movement of models in the scene graph is controlled by engines.
 * An engine has a single  target object that it affects.
 * Each frame, the engine is called with the current time. The engine
 * uses its current attributes and the time to update its  target.
 * The target may be a model in the scene graph or another engine.
 *
 * Like models, engines are kept in a hierarchy. They are visited
 * in the order they occur in the hierarchy with the parent engines
 * being evaluated first. An engine may control its children, passing
 * events, messages or evaluation status downward or they may be unrelated.
 * If an engine controls its children, the system will try to keep them
 * together when load balancing in the distributed environment. Engines
 * are also kept with their target objects whenever possible.
 *
 * @image html head_tracker1.png
 *
 * The scene manager art tools output engines as well as models. The keyframe
 * animation controllers in Maya are expressed as KeyFramer
 * and Transformer engines. Texture animations use ImageSwitch.
 *
 * An engine becomes available by putting it in the  simulation  tree
 * of a  scene being displayed. An engine must also be active (see
 * Engine::SetActive and Engine::Start) to begin processing.
 *
 * Special engines exist to process input events such as keypresses
 * or mouse events. These engines are managed by the 3D world
 * object (see World3D). Application event engines must also be
 * associated with a scene to be evaluated regularly.
 *
 * The base implementation of Engine does no evaluation of its own.
 * It provides a hookable framework for controlling animation
 * behavior of derived classes which do the actual evaluation.
 *
 * If the DOEVENTS flag is set, the engine will generate the
 * following events:
 * @code
 *	Event::START	event logged when an engine is started
 *	Event::STOP		event logged when an engine is stopped
 *	Event::RESET	event logged when an engine is reset
 * @endcode
 *
 * @see Group World3D MouseEngine Scene::SetEngines
 */
class Engine : public Group
{
	friend class ComputeThreadPool;
public:
	/*!
	 * @brief Engine control values that modify engine behavior.
	 *
	 * This is a 32-bit field and values up to 2048 are reserved.
	 * Subclasses may use the rest of the upper bits for their own control flags.
	 *
	 * @see Engine::SetControl
	 */
	enum
	{
		CYCLE = 1,				//!< engine repeats
		CONTROL_CHILDREN = 2,	//!< engine controls its children
		CHILDREN_FIRST = 4,		//!< evaluate child engines first
		REVERSE = 8,			//!< operate in reverse
		DATA_PARALLEL = 16,		//!< schedule different parts of a task on different threads
		ACTIVE = 32,			//!< engine is currently active (maps to !OBJ_InActive)
		PING_PONG = 64,			//!< alternate direction when cycling
		TASK_PARALLEL = 128,	//!< schedule different tasks on different threads
		RECORD = 256,			//!< record results of this engine
		BLEND_TO = 512,			//!< blend from current result
		BLEND_BETWEEN = 1024,	//!< blend between two results
		USER_AVAILABLE = 8192,
		CYCLE_REVERSE =	CYCLE | REVERSE,		//!< cycle in reverse
		CYCLE_PING_PONG = CYCLE | PING_PONG,	//!< cycle forward then reverse
		CYCLE_REVERSE_PING_PONG = CYCLE | REVERSE | PING_PONG, //!< cycle reverse then forward
	};

	/*!
	 * @brief Engine execution states.
	 * @see GetState
	 */
	enum
	{
		DONE = 1,		//!< finished executing
		IDLE = 2,		//!< idle, waiting to be started
		RUNNING = 4,	//!< active and running forward
//		REVERSE = 8		//!< active and running in reverse
	};

	/*!
	 * @brief Engine update flags.
	 *
	 * These are the valid values of the  who argument for the functions which
	 * update the engine (and/or it's children).
	 *
	 * @see SetControl SetSpeed SetDuration SetTimeOfs SetStartTime Enable Disable Start Stop
	 */
	enum
	{
		RELATIVE = 8,	//!< relative to current time
		ONLYME = 4		//!<* update engine, not its children
	};
	VX_DECLARE_CLASS(Engine);
	VX_GROUP_DECLARE(Engine);

//! Construct empty engine.
	Engine();
//! Construct one engine from another.
	Engine(const Engine&);
	~Engine();

//! Get engine run state.
	int				GetState() const;
//! Return \b true if engine is currently running, else \b false.
	bool			IsRunning() const;
//! Get elapsed time since engine was last started.
	float			GetElapsed() const;
//! Get scheduled time engine will stop (if speed is not changed).
	float			GetStopTime() const;
//! Get engine control flags.
	int				GetControl() const;
//! Set control flags which determine engine behavior.
	virtual void	SetControl(int, int who = 0);
//! Get engine speed.
	float			GetSpeed() const;
//! Set execution speed.
	virtual void	SetSpeed(float, int who = 0);
//! Get time in seconds when engine was last started or reset.
	float			GetStartTime() const;
//! Get duration of engine (how long it should run) in seconds.
	float			GetDuration() const;
//! Set time in seconds engine should run for.
	virtual void	SetDuration(float, int who = 0);
//! Get time offset in seconds.
	float			GetTimeOfs() const;
//! Set time offset to be added to engine evaluation time.
	virtual void	SetTimeOfs(float, int who = 0);		
//! Get current target of engine.
	SharedObj*		GetTarget() const;
//! Set target object which is controlled by engine.
	virtual void	SetTarget(SharedObj*);
//! Set time engine should start.
	virtual void	SetStartTime(float, int who = 0);
//! Enable one or more engine control flags.
	virtual void	Enable(int, int who = 0);
//! Disable one or more engine control flags.
	virtual void	Disable(int, int who = 0);
//! Start this engine and possibly its children.
	virtual void	Start(int who = 0);
//! Stop this engine and possibly its children.
	virtual void	Stop(int who = 0);
//! Find the first engine in the hierarchy with the given target.
	virtual	const Engine*	FindTarget(const SharedObj*) const;

//! Called each frame to evaluate the engine state.
	virtual bool	Eval(float t);
//! Called when the engine is started.
	virtual	bool	OnStart();
//! Called to compute the state of this engine and its children serially.
	virtual	void	Compute(float time);
//! Called to evaluate the engine's children.
	virtual	void	ComputeChildren(float time, int filter = 0);
//! Called to compute the engine evaluation time.
	virtual	float	ComputeTime(float t);

// Overrides
	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Copy(const SharedObj*);
	virtual void	SetActive(bool);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Engine::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		ENG_SetSpeed = Group::GROUP_NextOp,
		ENG_SetStartTime,
		ENG_SetDuration,
		ENG_SetTarget,
		ENG_RemoveTarget,
		ENG_Compute,
		ENG_Start,
		ENG_Stop,
		ENG_Reset,
		ENG_SetControl,
		ENG_Enable,
		ENG_Disable,
		ENG_SetTimeOfs,
		ENG_NextOp = Group::GROUP_NextOp + 30
	};

protected:
//! Called when the engine is stopped.
	virtual	bool	OnStop();
//! Called when the engine is reset.
	virtual	bool	OnReset();
	void			DoStart(float t);
	bool			DoEval(float t);
	bool			DoReset(float t);

protected:
//	Data members
	vint32		m_Control;		// engine control flags
	vint32		m_State;		// engine run state
	float		m_TimeOfs;		// time offset
	float		m_StartTime;	// start time
	float		m_Speed;		// execution speed
	float		m_Duration;		// time to run for
	float		m_Elapsed;		// time elapsed since start
	float		m_EvalTime;		// last time passed to Eval
	WeakRef<SharedObj> m_Target;// target object affected by engine
#ifndef VX_NOTHREAD
	static ComputeThreadPool* s_ComputeThreads;
#endif
};

/*!
 * @class MouseEngine
 * @brief Engine that handles mouse events.
 *
 * When this engine gets a mouse event thru OnEvent, it dispatches the event
 * to the OnMouse call which can be overridden by subclasses.
 * The mouse engine also maintains the position and flags of the
 * previous mouse event.
 *
 * @see SharedObj::OnEvent Messenger::Observe
 */
class MouseEngine : public Engine
{
public:
	VX_DECLARE_CLASS(MouseEngine);
	MouseEngine() : Engine() { MouseFlags = 0; }

//! Callback to handle mouse events.
	virtual	void	OnMouse(const Vec2& pos, uint32 flags);
	virtual	bool	OnEvent(Event*);

protected:
	Vec2	MousePos;		//!< mouse position of previous mouse event
	uint32	MouseFlags;		//!< mouse flags of previous mouse event
};

} // end Vixen