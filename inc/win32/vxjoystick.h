#pragma once
#include <mmsystem.h>

namespace Vixen {
/*!
 * @class JoyStick
 *
 * @brief Joystick navigation engine.
 *
 * Polls the joystick every frame and generates one of three navigation events:
 * @code
 *	Event::MOUSE	MouseEvent mouse event (default)
 *	Event::NAVINPUT	NavInput joystick event 
 *	Event::NAVIGATE	NavEvent navigation event
 * @endcode
 *
 * The joystick engine also responds to the navigation events
 * generated when Event::NAVIGATE is the event code specified.
 *
 * @see Navigator NavigateEvent MouseEvent
 */
class JoyStick : public Engine
{
public:
	JoyStick(int eventcode = Event::MOUSE, int joyid = 0);
	virtual bool	Eval(float t);
	void			SetScale(float scale);
	float			GetScale() const	{ return m_Scale; }
	void			SetEventCode(int code, int buttons = 0);
	static bool		GetJoyInfo(int joyid = 0, JOYINFOEX* jinfo = NULL);

protected:
	NavInputEvent	m_Event;
	Vec3		m_LastPos;
	float		m_Scale;
	int32		m_MouseEvent;
	int32		m_NavEvent;
	int32		m_XYEvent;
	int32		m_LastCode;
};

} // end Vixen