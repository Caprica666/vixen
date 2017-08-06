#pragma once

namespace Vixen {
/*!
 * @class Navigator
 *
 * @brief Navigates within a 3D scene using the mouse and/or keyboard.
 *
 * Observes mouse and keyboard events and converts them into
 * navigation events. They events can be used by Transformer or Flyer
 * to control the movement of an object. NavRecorder can save them
 * to an output stream, allowing the output of any type of navigator
 * to be saved.
 *
 * What mouse motion does depends on which button you press. Just moving
 * but not dragging does nothing. You can also mask individual buttons
 * to eliminate features.
 * @code
 *	left button		vertical mouse motion moves forward and backward.
 *					horizontal motion turns around Y axis.
 *	right button	vertical motion looks up and down (turn about X).
 *					horizontal motion turns around Y.
 *	both buttons	vertical motion moves up and down.
 *					horizontal moves is ignored. 
 * @endcode
 *
 * @author Dennis Crowley
 *
 * @see NavigateEvent Event NavRecorder Transformer Flyer
 */
class Navigator : public MouseEngine
{
public:
	VX_DECLARE_CLASS(Navigator);
	Navigator();
	void			SetTurnSpeed(float s);	//!< Establish angular velocity
	float			GetTurnSpeed() const	{ return m_TurnSpeed; }
	int				GetButtons() const		{ return m_Buttons; }
	void			SetButtons(int mask);	//!< Determine which mouse buttons to act upon.

// Overrides
	virtual bool	Eval(float t);
	void			OnMouse(const Vec2& p, uint32 flags);
	virtual bool	OnKey(int32 key, int32 flags, int32 repeat);
	virtual void	LogEvent(NavigateEvent *event);

protected:
	virtual void	OnNavInput(NavInputEvent*);
	bool			OnEvent(Event*);

	//--- key states ---
	bool	m_kForward, m_kBack, m_kLeft, m_kRight, m_kCtrl, m_kUp, m_kDown;
	float	m_TurnSpeed;	//--- turning speed (radians per second?) ---
	int32	m_LastEvent;
	int32	m_Buttons;		// mask of buttons we look at
};


/*!
 * @class NavRecorder
 * @brief Records navigation events on the supplied output stream.
 *
 * If the engine's target is a model, it is assumed the navigation events
 * apply to its movement.
 *
 * This engine traps and records all events of type Event::NAVIGATE
 * which are sent by its target. If it has no target, all navigation
 * events are logged. The engine will not record if it is not running
 * and has not been given an open output stream.
 *
 * @par Example
 * Here is an example of how to add a navigation recorder to a 3D
 * application based on the scene manager viewer. It will record
 * viewer mouse movement in a file called navlog.dsm.
 * @code
 *	NavRecorder* navrec = new NavRecorder;
 *	Viewer* viewer = (Viewer*) World3D::Get();
 *	FileMessenger* mess = new FileMessenger;
 *	navrec->SetMessenger(mess);
 *	mess->SetOutStream(new Core::FileStream);
 *	mess->Open("navlog.vix", STREAM_Write);
 *	viewer->GetNavigator()->Append(navrec);
 * @endcode
 *
 * @see Navigator Flyer NavigateEvent Messenger
 *
 * @author Nola Donato
 */
class NavRecorder : public Engine
{
public:
	VX_DECLARE_CLASS(NavRecorder);

	NavRecorder() : Engine()				{ m_RecordTime = 0; }
	Messenger*	GetMessenger() const		{ return m_Stream; }

	//! Establish messenger used to serialize and log events.
	void		SetMessenger(Messenger* s);

	bool		OnEvent(Event* event);
	bool		OnStart();
	bool		OnStop();

protected:
	Ref<Messenger>	m_Stream;		// stream to record on
	float			m_RecordTime;	// time of most recently logged event
};

/*!
 * @fn void NavRecorder::SetMessenger(Messenger* s)
 * @param s	messenger to record with. If NULL, previous messenger (if any)
 *			is dereferenced and closed	
 *
 * Each time a navigation event is observed, it is serialized by the messenger.
 * This engine does not open the messenger, that is the responsibility of the caller.
 * It will close the messenger when it is no longer using it.
 *
 * @see Messenger Event::Log
 */
inline void NavRecorder::SetMessenger(Messenger* s)
{ m_Stream = s; }

} // end Vixen
