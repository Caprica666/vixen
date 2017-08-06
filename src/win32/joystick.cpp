#include "vixen.h"
#include "win32/vxjoystick.h"

namespace Vixen {

JoyStick::JoyStick(int eventcode, int joyid)
{
	if (joyid <= 0)
		m_Event.DeviceID = JOYSTICKID1; 
	else
		m_Event.DeviceID = joyid;
	m_Scale = 0.2f;
	m_Event.Code = eventcode;
	if (eventcode == Event::MOUSE)
		m_Event.Size = sizeof(Vec2) + sizeof(int32);
	m_Event.Pos.Set(0,0,0);
	m_Event.Buttons = 0;
	m_Event.Time = 0;
	m_LastPos.Set(0,0,0);
	m_MouseEvent = m_NavEvent = m_XYEvent = 0;
	m_LastCode = eventcode;
}

void	JoyStick::SetEventCode(int code, int buttons)
{
	switch (code)
	{
		case Event::MOUSE:	m_MouseEvent = buttons; break;
		case Event::NAVIGATE:	m_NavEvent = buttons; break;
		case Event::NAVINPUT: m_XYEvent = buttons; break;
	}
}

void	JoyStick::SetScale(float s)
{
	m_Scale = s;
}

bool JoyStick::Eval(float t)
{
	JOYINFOEX	jix;

	if (!GetJoyInfo(m_Event.DeviceID, &jix))
		return true;
	if (!IsSet(SharedObj::DOEVENTS))
		return true;

	Scene*	scene = GetMainScene();
	float		dt = (t - m_Event.Time) * m_Scale;	
	float		fx = 2.0f * float(jix.dwXpos) / 65535.0f - 1.0f;
	float		fy = 1.0f - 2.0f * float(jix.dwYpos) / 65535.0f;	
	float		fz = float(jix.dwZpos) / 65535.0f - 1.0f;
	int32		oldflags = m_Event.Buttons;
	int32		newflags = 0;
	int32		allflags;
	bool		moved = (m_LastPos != Vec3(fx, fy, fz));

	m_Event.Buttons = jix.dwButtons;
	m_Event.Sender = this;
	m_Event.Time = t;
	if (jix.dwButtons & JOY_BUTTON1)
		newflags |= MouseEvent::LEFT;
	if (jix.dwButtons & JOY_BUTTON2)
		newflags |= MouseEvent::RIGHT;
	if (jix.dwButtons & JOY_BUTTON3)
		newflags |= MouseEvent::MIDDLE;
	allflags = newflags | oldflags;
/*
 * Make generic XY input event from joystick info
 */
	if ((m_Event.Code == Event::NAVINPUT) || (m_XYEvent & allflags))
	{
		if (newflags && !moved)
			return true;
		m_LastPos.Set(fx, fy, fz);
		NavInputEvent* e = new NavInputEvent(m_Event);
		e->Code =  Event::NAVINPUT;
		e->Sender = this;
		e->Pos = m_LastPos;
		e->Buttons = newflags;
		e->Log();
		m_LastCode =  Event::NAVINPUT;
		return true;
	}
/*
 * Make mouse event from joystick info
 */
	if ((m_Event.Code == Event::MOUSE) || (m_MouseEvent & allflags))
	{
		Box2	vport = scene->GetViewport();
		Vec2	scale(vport.Width() / 65535.0f, vport.Height() / 65535.0f);
		Vec2	ctr = vport.Center();
		MouseEvent*	e = new MouseEvent;

		if (!moved)
			return true;
		m_LastPos.Set(fx, fy, fz);
		e->Sender = this;
		e->Code = Event::MOUSE;
		e->MousePos.x = float(jix.dwXpos) * scale.x + vport.min.x;
		e->MousePos.y = float(jix.dwYpos) * scale.y + vport.min.y;	
		e->MouseFlags = newflags;
		e->Log();
		m_LastCode = Event::MOUSE;
		return true;
	}
/****
 *
 * Make navigation event from joystick info
 * JOY_BUTTON2	X turns around Y, Y turns around X, Z turns around Z
 * JOY_BUTTON1	X moves left/right, Y moves fwd/back
 * Z moves up/down
 *
 ****/
	if ((m_Event.Code == Event::NAVIGATE) || (m_NavEvent & allflags))
	{
		NavigateEvent*	ne = new NavigateEvent;
		float		fr = 2.0f * float(jix.dwRpos) / 65535.0f - 1.0f;
		Quat		yturn(Model::XAXIS, -fr * dt * PI);

		ne->Sender = this;
		ne->Flags = 0;
		ne->Code = Event::NAVIGATE;
		m_LastPos.Set(fx, fy, fz);
		if (((jix.dwXpos >> 2) == 0x2000) &&	// at 0,0,0?
			((jix.dwYpos >> 2) == 0x2000) &&
			((jix.dwRpos & 0x3FFF) < 300))
		{
			m_Event.Pos.x = 0.0f;
			m_Event.Pos.y = 0.0f;
			m_Event.Pos.z = 0.0f;
			m_LastCode = Event::NAVIGATE;
			return true;
		}
		if (m_LastCode != Event::NAVIGATE)
			return true;
		m_Event.Pos.x += fx * dt;
		m_Event.Pos.y += fy * dt;
		m_Event.Pos.z += fz * dt;
		switch (jix.dwButtons)
		{
			case 0:								// no buttons, look and turn
			ne->Flags = NavigateEvent::LOCAL_POS | NavigateEvent::ROTATION | NavigateEvent::YROTATION;
			ne->Pos.z = -m_Event.Pos.y;
			ne->Pos.x = m_Event.Pos.x;
			ne->Rot = yturn;
			break;

			case JOY_BUTTON2:					// button 2, X and Y move
			ne->Flags = NavigateEvent::LOCAL_POS | NavigateEvent::LOOK | NavigateEvent::ABSOLUTE;
			if (fabs(fx) > fabs(fy))
				ne->Pos.x = -m_Event.Pos.x;
			else
				ne->Pos.y = -m_Event.Pos.y;
			ne->Look = Quat(0,0,0,1);
			break;
		}
		if (ne->Flags)
			ne->Log();
		else
			delete ne;
	}
	return true;		
}


bool JoyStick::GetJoyInfo(int nJoyID, JOYINFOEX* jinfo)
{
	 JOYINFOEX jix;
	 char szErr[256];

	if (nJoyID <= 0)
		nJoyID = JOYSTICKID1;
	memset (&jix, 0, sizeof (jix));
	jix.dwSize = sizeof(JOYINFOEX);
	jix.dwFlags = JOY_RETURNALL;
	strcpy(szErr,"");
//
// joyGetPoxEx will fill in the joyinfoex struct with all the
// joystick information. 
// 
	 switch(joyGetPosEx(nJoyID, &jix))
	{
	   case JOYERR_NOERROR:
		 // No problems!
		if (jinfo)
			*jinfo = jix;
		return TRUE;
	   case MMSYSERR_NODRIVER:
		 strcpy(szErr,"The joystick driver is not present.");
		 return FALSE;
	   case MMSYSERR_INVALPARAM:
		 strcpy(szErr,"An invalid parameter was passed.");
		 return FALSE;
	   case MMSYSERR_BADDEVICEID:        
		 strcpy(szErr,"The specified joystick identifier is invalid.");
		 return FALSE;
	   case JOYERR_UNPLUGGED:        
		 strcpy(szErr,"Your joystick is unplugged.");
		 return FALSE;  
	   default:
		 strcpy(szErr,"Unknown joystick error.");
		 return FALSE;
	}
	VX_ERROR(("JoyStick::GetJoyInfo %s", szErr), false);
}

}	// end Vixen 