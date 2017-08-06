/****
 *
 * inlines for Event subclasses used by the system internally
 *
 ****/
#pragma once

namespace Vixen {
	
inline LoadEvent::LoadEvent(const Event& src)
{
	*this = src;
}

inline Event& LoadEvent::operator=(const Event& src)
{
	Event::operator=(src);
	FileName = ((LoadEvent&) src).FileName;
	return *this;
}

inline LoadSceneEvent::LoadSceneEvent(const Event& src)
{
	*this = src;
}

inline Event& LoadSceneEvent::operator=(const Event& src)
{
	LoadEvent::operator=(src);
	if ((src.Code == Event::LOAD_SCENE) ||
		(src.Code == Event::LOAD_IMAGE))
		Object = ((LoadSceneEvent&) src).Object;
	return *this;
}

inline LoadTextEvent::LoadTextEvent(const Event& src)
{
	*this = src;
}

inline Event& LoadTextEvent::operator=(const Event& src)
{
	LoadEvent::operator=(src);
	if (src.Code == Event::LOAD_TEXT)
	{
		Text = ((LoadTextEvent&) src).Text;
		SequenceID = ((LoadTextEvent&) src).SequenceID;
	}
	return *this;
}

inline LoadDataEvent::LoadDataEvent(const Event& src)
{
	Data = intptr(0);
	Length = 0;
	*this = src;
}

inline Event& LoadDataEvent::operator=(const Event& src)
{
	LoadEvent::operator=(src);
	if (src.Code == Event::LOAD_DATA)
	{
		Length = ((LoadDataEvent&) src).Length;
		Data = ((LoadDataEvent&) src).Data;
	}
	return *this;
}

inline MouseEvent::MouseEvent(const Event& src)
{
	*this = src;
}

inline Event& MouseEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == Event::MOUSE)
	{
		MouseEvent& msrc = (MouseEvent&) src;
		MousePos = msrc.MousePos;
		MouseFlags = msrc.MouseFlags;
	}
	return *this;
}

inline KeyEvent::KeyEvent(const Event& src)
{
	*this = src;
}

inline Event& KeyEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == Event::KEY)
	{
		KeyEvent& ksrc = (KeyEvent&) src;
		KeyCode = ksrc.KeyCode;
		KeyFlags = ksrc.KeyFlags;
	}
	return *this;
}

inline SceneEvent::SceneEvent(const Event& src)
{
	*this = src;
}

inline Event& SceneEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if ((src.Code == ATTACH_SCENE) ||
		(src.Code == SCENE_CHANGE))
	{
		Target = ((SceneEvent&) src).Target;
		WindowHandle = ((SceneEvent&) src).WindowHandle;
	}
	return *this;
}

inline ErrorEvent::ErrorEvent(const Event& src)
{
	*this = src;
}

inline Event& ErrorEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == ERROR)
	{
		ErrCode = ((ErrorEvent&) src).ErrCode;
		ErrLevel = ((ErrorEvent&) src).ErrLevel;
		ErrString = ((ErrorEvent&) src).ErrString;
	}
	return *this;
}

inline SceneEvent::~SceneEvent()
{
	Target = (Scene*) NULL;
}

inline FrameEvent::FrameEvent(const Event& src)
{
	*this = src;
}

inline Event& FrameEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == SET_TIME)
		Frame = ((FrameEvent&) src).Frame;
	return *this;
}

inline bool MouseEngine::OnEvent(Event* e)
{
	MouseEvent* me = (MouseEvent*) e;
	if (me->Code == Event::MOUSE)	
		OnMouse(me->MousePos, me->MouseFlags);
	return true;
}

/*!
 * @fn void MouseEngine::OnMouse(const Vec2& pos, uint32 flags)
 * @param pos	Position of the mouse
 * @param flags	Mouse flags
 *
 * Handles mouse events for this engine. The base implementation
 * keeps track of the current position and flags so, if you want
 * this info, call MouseEngine::OnMouse in your override of this
 * function. It also passes the mouse event on to any child engines
 * that handle mouse events.
 *
 * @see SharedObj::OnEvent Messenger::Observe MouseEvent
 */
inline void MouseEngine::OnMouse(const Vec2& pos, uint32 flags)
{
	MousePos = pos;
	MouseFlags = flags;
}

inline NavInputEvent::NavInputEvent() : Event(Event::NAVINPUT, sizeof(Vec3) + 2 * sizeof(int32))
{
	DeviceID = 0;
}

inline NavInputEvent::NavInputEvent(const Event& src)
{
	*this = src;
}

inline Event& NavInputEvent::operator=(const Event& src)
{
	int	code = src.Code;

	if (code == Event::NAVINPUT)
	{
		NavInputEvent& jsrc = (NavInputEvent&) src;
		DeviceID = jsrc.DeviceID;
		Pos = jsrc.Pos;
		Buttons = jsrc.Buttons;
	}
	else if (code == Event::MOUSE)
	{
		MouseEvent& msrc = (MouseEvent&) src;
		Pos.x = msrc.MousePos.x;
		Pos.y = msrc.MousePos.y;
		Buttons = msrc.MouseFlags;
	}
	return *this;
}

inline NavigateEvent::NavigateEvent() :
	Event(Event::NAVIGATE, sizeof(Vec3) + 2 * sizeof(Quat) + sizeof(int32)),
	Pos(0,0,0),
	Rot(Model::ZAXIS, 0.0f),
	Look(Model::ZAXIS, 0.0f)
{
	Flags = 0;
}

inline NavigateEvent::NavigateEvent(const NavigateEvent& src)
{
	*this = src;
}

inline Event& NavigateEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (Code == Event::NAVIGATE)
	{
		NavigateEvent& e = (NavigateEvent&) src;
		Flags = e.Flags;
		Pos = e.Pos;
		Rot = e.Rot;
		Look = e.Look;
	}
	return *this;
}


inline TriggerEvent::TriggerEvent(int code)
  :	Event(code, 2 * sizeof(ObjRef) + 3 * sizeof(float)) { }

inline TriggerEvent::TriggerEvent(const TriggerEvent& src)
{
	*this = src;
}

inline Event& TriggerEvent::operator=(const Event& src)
{
	int	code = src.Code;

	Event::operator=(src);
	if ((code == ENTER) || (code == LEAVE))
	{
		TriggerEvent& tsrc = (TriggerEvent&) src;
		Target = tsrc.Target;
		Collider = tsrc.Collider;
		CollidePos = tsrc.CollidePos;
	}
	return *this;
}


inline ConnectEvent::ConnectEvent(int code)
  :	Event(code, sizeof(int32)) { }

inline ConnectEvent::ConnectEvent(const ConnectEvent& src)
{
	*this = src;
}

inline Event& ConnectEvent::operator=(const Event& src)
{
	int	code = src.Code;

	Event::operator=(src);
	if (code == CONNECT)
	{
		ConnectEvent& tsrc = (ConnectEvent&) src;
		ConnectID = tsrc.ConnectID;
	}
	return *this;
}

inline TriHitEvent::TriHitEvent(int code)
  :	Event(code, 2 * sizeof(int32) + sizeof(float) + sizeof(Vec3) + sizeof(ObjRef))
{
	Target = (Mesh*) NULL;
	Distance = FLT_MAX;
	TriIndex = -1;
}

inline TriHitEvent::TriHitEvent(const TriHitEvent& src)
{
	*this = src;
}

inline Event& TriHitEvent::operator=(const Event& src)
{
	int	code = src.Code;

	Event::operator=(src);
	if (code == TRI_HIT)
	{
		TriHitEvent& tsrc = (TriHitEvent&) src;
		Target = tsrc.Target;
		Distance = tsrc.Distance;
		Intersect = tsrc.Intersect;
		TriIndex = tsrc.TriIndex;
	}
	return *this;
}


inline PickEvent::PickEvent(int code, SharedObj* sender)
:	Event(code, sizeof(ObjRef), sender)
{ }

inline PickEvent::PickEvent(const Event& src)
{
	*this = src;
}

inline Event& PickEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if ((src.Code == Event::PICK) ||
		(src.Code == Event::NOPICK) ||
		(src.Code == Event::SELECT) ||
		(src.Code == Event::DESELECT))
	{
		Target = ((PickEvent&) src).Target;
	}
	return *this;
}

inline TrackEvent::TrackEvent(int code, const SharedObj* sender)
:	Event(code, 3 * sizeof(int32) + sizeof(Vec3) + sizeof(Quat) + sizeof(Core::String), sender),
	Type(UNKNOWN),
	UserID(0)
{
}

inline TrackEvent::TrackEvent(int type, int userid, const SharedObj* sender)
:	Event(TRACK, 3 * sizeof(int32) + sizeof(Vec3) + sizeof(Quat) + sizeof(Core::String), sender),
	Type(type),
	UserID(userid)
{
}


inline TrackEvent::TrackEvent(const Event& src)
{
	*this = src;
}

inline Event& TrackEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == Event::TRACK)
	{
		const TrackEvent& tev = (const TrackEvent&) src;
		Type = tev.Type;
		UserID = tev.UserID;
		PartName = tev.PartName;
		PartIndex = tev.PartIndex;
		Position = tev.Position;
		Rotation = tev.Rotation;
	}
	return *this;
}

} // end Vixen