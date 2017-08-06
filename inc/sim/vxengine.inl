/****
 *
 * Inlines for Engine class
 *
 ****/

namespace Vixen {

/*!
 * @fn int	Engine::GetState() const
 *
 * The engine state is one of:
 * @code
 *	IDLE	engine is idle, waiting to be started
 *	DONE	engine has been stopped
 *	RUNNING	engine is running, calling Eval
 * @endcode
 *
 * You can pause an engine by calling Engine::SetActive with  false.
 * This does not affect the engine's state, it just prevents
 * the engine from being examined. Time still passes for inactive engines.
 *
 * @see Engine::SetActive Engine::SetControl Engine::GetElapsed Engine::IsRunning
 */
inline int	Engine::GetState() const		{ return m_State; }

/*!
 * @fn float 	Engine::GetElapsed() const
 *
 * The elapsed time is in seconds since the engine was started
 * and does not include the time offset.
 * If an engine has been reset and has started a new cycle,
 * GetElapsed is relative to the start of the last cycle.
 *
 * @see	Engine::GetStartTime Engine::ComputeTime Engine::SetDuration Engine::Eval
 */
inline float	Engine::GetElapsed() const	{ return m_Elapsed; }

inline int		Engine::GetControl() const	{ return m_Control; }
inline float	Engine::GetSpeed() const		{ return m_Speed; }
inline float	Engine::GetDuration() const	{ return m_Duration; }
inline float	Engine::GetStartTime() const	{ return m_StartTime; }
inline bool		Engine::IsRunning() const		{ return (m_State & RUNNING) != 0; }
inline float	Engine::GetStopTime() const	{ return m_StartTime - m_TimeOfs + m_Duration/m_Speed; }

inline SharedObj*	Engine::GetTarget() const
	{ return m_Target; }

inline float  Engine::GetTimeOfs() const
	{ return m_TimeOfs; }


} // end Vixen
