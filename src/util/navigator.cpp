#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Navigator, MouseEngine, VX_Navigator);

VX_IMPLEMENT_CLASSID(NavRecorder, Engine, VX_NavRecorder);

Navigator::Navigator() : MouseEngine()
{
	m_kForward = m_kBack = m_kLeft = m_kRight = m_kCtrl = m_kUp = m_kDown = false;
	m_TurnSpeed = 0.05f;
	m_LastEvent = 0;
	m_Buttons = MouseEvent::LEFT;
}

#define KTD_Forward   0x26  // forward arrow
#define KTD_Back      0x28  // back arrow
#define KTD_Left      0x25  // left arrow
#define KTD_Right     0x27  // right arrow
#define KTD_Up        0x41  // A
#define KTD_Down      0x5a  // Z
#define KTD_Ctrl      0x11  // control key


void Navigator::SetButtons(int mask)
{
	m_Buttons = mask;
}

void	Navigator::SetTurnSpeed(float s)
{
	m_TurnSpeed = s;
}

bool Navigator::OnEvent(Event* event)
{
	KeyEvent* ke;
	NavInputEvent* ne;

	switch (event->Code)
	{
		case Event::KEY:
		ke = (KeyEvent*) event;
		m_LastEvent = 0;
		return OnKey(ke->KeyCode, ke->KeyFlags, 1);

		case Event::NAVINPUT:
		ne = (NavInputEvent*) event;
		MouseFlags = ne->Buttons;
		m_LastEvent = Event::NAVINPUT;
		MousePos.x = ne->Pos.x;
		MousePos.y = ne->Pos.y;
		return true;
	}
	return MouseEngine::OnEvent(event);
}

void Navigator::OnMouse(const Vec2& pos, uint32 flags)
{
	Scene* scene = GetMainScene();
	if (!scene)
		return;

	Box2	sViewBox = scene->GetViewport();
	float	fWidth2 = sViewBox.Width() / 2.0f;
	float	fHeight2 = sViewBox.Height() / 2.0f;

	m_LastEvent = flags ? Event::NAVINPUT : 0;
	MouseFlags = flags;
	MousePos.x = (pos.x - fWidth2) / fWidth2;
	MousePos.y = -(pos.y - fHeight2) / fHeight2;
}

bool Navigator::OnKey(int32 key, int32 flags, int32 repeat)
{
	flags &= 0xc000;
	key &= ~(0x100);
	switch (flags)
	{
		//--- key down ---
		case 0x0000:
		switch (key)
		{
			case KTD_Forward: m_kForward = true; break;
			case KTD_Back:    m_kBack = true;    break;
			case KTD_Left:    m_kLeft = true;    break;
			case KTD_Right:   m_kRight = true;   break;
			case KTD_Ctrl:    m_kCtrl = true;    break;
			case KTD_Up:      m_kUp = true;      break;
			case KTD_Down:    m_kDown = true;    break;
		}
		break;

		case 0xc000:
		case 0x8000:
		//--- key up ---
		switch (key)
		{
			case KTD_Forward: m_kForward = false; break;
			case KTD_Back:    m_kBack = false;    break;
			case KTD_Left:    m_kLeft = false;    break;
			case KTD_Right:   m_kRight = false;   break;
			case KTD_Ctrl:    m_kCtrl = false;    break;
			case KTD_Up:      m_kUp = false;      break;
			case KTD_Down:    m_kDown = false;    break;
		}
		break;
	}
	m_LastEvent = Event::KEY;
	return true;
}

void Navigator::OnNavInput(NavInputEvent* e)
{
	if (e->Buttons == 0)
		return;
	NavigateEvent*	event = new NavigateEvent;
	event->Time = e->Time;
	event->Flags = 0;
	event->Sender = this;

	switch (e->Buttons & m_Buttons)
	{
		case MouseEvent::LEFT:	//	Left button: forward-backward, turn around Y
		event->Pos.z = -e->Pos.y * m_Speed;
		event->Rot.Set(Model::YAXIS, -e->Pos.x * m_TurnSpeed);
		event->Flags |= NavigateEvent::LOCAL_POS | NavigateEvent::ROTATION | NavigateEvent::YROTATION;
		break;

		case MouseEvent::RIGHT:	//  Right button: rot left-right, look up-down
		if (fabs(e->Pos.x) > fabs(e->Pos.y))
		{
			event->Rot.Set(Model::YAXIS, -e->Pos.x * m_TurnSpeed);
			event->Flags |= NavigateEvent::ROTATION;
		}	
		else
		{
			event->Look.Set(Model::XAXIS, e->Pos.y * m_TurnSpeed);
			event->Flags |= NavigateEvent::LOOK;
		}
		break;

		case MouseEvent::LEFT | MouseEvent::RIGHT: //  Both buttons: up-down only
		event->Pos.y = e->Pos.y * m_Speed;
		event->Flags |= NavigateEvent::LOCAL_POS;
		break;
	}
	MouseFlags = e->Buttons;
	LogEvent(event);
}

/*
 * If we have a valid event and should be logging events,
 * log the navigate event here
 */
void Navigator::LogEvent(NavigateEvent *event)
{
	event->Sender = this;
	if (IsSet(SharedObj::DOEVENTS))
		event->Log();
}




#define MAX_dt 1.0f

bool Navigator::Eval(float t)
{
	static float last_t = 0.0;
	float dt;

	//--- find dt - clamp it if it gets too big - catch some other conditions
	if (t == 0.0f || last_t > t)
	{
		last_t = t;
		return false;
	}

	dt = t - last_t;
	last_t = t;
	if (dt > MAX_dt) dt = MAX_dt;

	if (m_LastEvent == Event::NAVINPUT)
	{
		NavInputEvent* e = new NavInputEvent;
		dt *= 128.0f;
		e->Pos.x = MousePos.x * dt;
		e->Pos.y = MousePos.y * dt;
		e->Buttons = MouseFlags;
		OnNavInput(e);
		return true;
	}
	if (m_LastEvent != Event::KEY)
		return false;
	NavigateEvent *ne = new NavigateEvent;
	if (m_kCtrl)
	{
		dt *= 64.0f;
		//--- turning and looking ---
		if (m_kForward || m_kBack)
		{
			if (! m_kBack)
			{
				//--- look down ---
				Quat look(Model::XAXIS, dt * m_TurnSpeed);
				ne->Look = look;
				ne->Flags |= NavigateEvent::LOOK;
			}
			else if (! m_kForward)
			{
				//--- look up ---
				Quat look(Model::XAXIS, -dt * m_TurnSpeed);
				ne->Look = look;
				ne->Flags |= NavigateEvent::LOOK;
			}
		}
		if (m_kLeft || m_kRight)
		{
			if (! m_kRight)
			{
				//--- turn left ---
				Quat turn(Model::XAXIS, dt * m_TurnSpeed);
				ne->Rot = turn;
				ne->Flags |= NavigateEvent::ROTATION;
			}
			else if (! m_kLeft)
			{
				//--- turn right ---
				Quat turn(Model::XAXIS, -dt * m_TurnSpeed);
				ne->Rot = turn;
				ne->Flags |= NavigateEvent::ROTATION;
			}
		}
	}
	else
	{
		dt *= 128.0f;
		//--- moving ---
		if (m_kForward || m_kBack)
		{
			if (! m_kBack)
			{
				//--- move forward ---
				ne->Pos.z = -dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
			else if (! m_kForward)
			{
				//--- move back ---
				ne->Pos.z = dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
		}
		if (m_kLeft || m_kRight)
		{
			if (! m_kRight)
			{
				//--- slide left ---
				ne->Pos.x = -dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
			else if (! m_kLeft)
			{
				//--- slide right ---
				ne->Pos.x = dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
		}
		if (m_kUp || m_kDown)
		{
			if (! m_kDown)
			{
				//--- move up ---
				ne->Pos.y = dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
			else if (! m_kUp)
			{
				//--- move down ---
				ne->Pos.y = -dt * m_Speed;
				ne->Flags |= NavigateEvent::LOCAL_POS;
			}
		}
	}
	LogEvent(ne);
	return true;
}

/*!
 * @fn bool NavRecorder::OnEvent(Event* e)
 *
 * Records navigation events (type Event::NAVIGATE) to the output stream.
 * All other events are ignored. This object is the sender of the
 * events and their time is relative to the first event recorded.
 * This engine will not record unless it is running.
 *
 * @see SharedObj::OnEvent
 */
bool NavRecorder::OnEvent(Event* e)
{
	NavigateEvent		RecordEvent;
	Messenger*	mess = m_Stream;
	NavigateEvent*		event;

	if (e->Code != Event::NAVIGATE)
		return false;
	event = (NavigateEvent*) e;
	if (!IsRunning() ||	(mess == NULL))		// engine not running?
		return true;
	if (m_RecordTime == 0.0f)				// first time thru?
	{									// describe initial conditions
		Model* target = (Model*)GetTarget();
		if (target)							// initial position, orientation
		{
			RecordEvent.Pos = target->GetCenter();
			RecordEvent.Rot = target->GetRotation();
		}
		else								// no target
		{
			RecordEvent.Pos.Set(0,0,0);
			RecordEvent.Rot.Set(Vec3(0,0,0),1);
		}
		RecordEvent.Time = 0;				// time since previous event
		RecordEvent.Sender = this;
		RecordEvent.Flags = NavigateEvent::ABSOLUTE | NavigateEvent::POSITION | NavigateEvent::ROTATION;
		*mess << RecordEvent;				// log to event stream
		*mess << int32(Messenger::VIXEN_End);
		return true;
	}
	RecordEvent = *event;					// event to record
	RecordEvent.Sender = this;
	RecordEvent.Time = event->Time - m_RecordTime;
	m_RecordTime = event->Time;				// save new event time
	*mess << RecordEvent;					// log to event stream
	*mess << int32(Messenger::VIXEN_End);
	return true;
}

bool NavRecorder::OnStart()
{
	m_RecordTime = 0;
	GetMessenger()->Observe(this, Event::NAVIGATE, NULL);
	return true;
}

bool NavRecorder::OnStop()
{
	GetMessenger()->Ignore(this, Event::NAVIGATE, NULL);
	return true;
}

}	// end Vixen