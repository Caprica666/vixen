#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Flyer, Navigator, VX_Flyer);

#define ANGLE_SCALE	(PI / 800.0f)

static const TCHAR* opnames[] =
{	TEXT("SetScale"), TEXT("SetDOF"),  };

const TCHAR** Flyer::DoNames = opnames;


Flyer::Flyer(float scale) : Navigator()
{
	m_iDOF = ALL;
	m_Scale = scale;
	m_TimeInc = 0.0f;
}

//
// set degrees of freedom
//
void Flyer::SetDOF(int iDOF)
{
	m_iDOF = iDOF;
}

void Flyer::SetScale(float scale)
{
	if (scale == 0.0f)
		return;
	m_Scale = scale;
}

bool Flyer::OnEvent(Event* event)
{
	MouseEvent* me = (MouseEvent*) event;

	switch (event->Code)
	{
		case Event::NAVIGATE:
		OnNavigate((NavigateEvent*) event);
		return true;

		case Event::MOUSE:
		MouseEngine::OnMouse(me->MousePos, me->MouseFlags);
		return true;
	}
	return MouseEngine::OnEvent(event);
}


//
//  This engine flies the viewpoint around using the mouse position.
//
//	Left button: forward-backward, yaw
//  Right button: up-down, left-right
//  Both buttons: pitch, roll
//
//  Position is considered relative to the center of the window.
//
void Flyer::OnMouse(const Vec2& pos, uint32 flags)
{
	Scene* scene = GetMainScene();
	if (!scene)
		return;

	Box2		sViewBox = scene->GetViewport();
	float		fWidth2 = sViewBox.Width() / 2.0f;
	float		fHeight2 = sViewBox.Height() / 2.0f;
	float		scale = m_Speed * m_Scale / 10.0f;
	NavigateEvent*	ev = new NavigateEvent;

	m_LastEvent = Event::MOUSE;
	ev->Time = 0;
	ev->Flags = 0;
	ev->Sender = this;
	ev->Rot = Quat(0,0,0,1);

	if (flags & (MouseEvent::CONTROL | MouseEvent::SHIFT))
		return;
	//	Left button: forward-backward, yaw
	if ((flags & MouseEvent::LEFT) &&
		!(flags & MouseEvent::RIGHT))
	{
		float zoom = scale * (pos.y - fHeight2) / fHeight2;
		if (m_iDOF & Z_TRANS)
		{
			ev->Pos.z = zoom;
			ev->Flags |= NavigateEvent::LOCAL_POS;
		}
		float rotate = ANGLE_SCALE * (fWidth2 - pos.x) / fWidth2;
		if ((m_iDOF & Y_ROT) && (rotate != 0))
		{
			ev->Rot.Mul(ev->Rot, Quat(Model::YAXIS, rotate));
			ev->Flags |= NavigateEvent::ROTATION | NavigateEvent::YROTATION;
		}
	}

	//  Right button: up-down, left-right
	if ((flags & MouseEvent::RIGHT) &&
		!(flags & MouseEvent::LEFT))
	{
		float updown = scale * (pos.y - fHeight2) / fHeight2;
		float leftright = scale * (pos.x - fWidth2) / fWidth2;
		if (m_iDOF & X_TRANS)
		{
			ev->Pos.x = leftright;
			ev->Flags |= NavigateEvent::LOCAL_POS;
		}
		if (m_iDOF & Y_TRANS)
		{
			ev->Pos.y = updown;
			ev->Flags |= NavigateEvent::LOCAL_POS;
		}
	}

	//  Both buttons: pitch, roll
	if ((flags & MouseEvent::LEFT) &&
		(flags & MouseEvent::RIGHT))
	{
		float updown = -ANGLE_SCALE * (pos.y - fHeight2) / fHeight2;
		float leftright = -ANGLE_SCALE * (pos.x - fWidth2) / fWidth2;
		if (fabs(updown) > fabs(leftright))
			if (m_iDOF & X_ROT)
			{
				ev->Rot.Mul(ev->Rot, Quat(Model::XAXIS, updown));
				ev->Flags |= NavigateEvent::XROTATION | NavigateEvent::ROTATION;
			}
		else
			if (m_iDOF & Z_ROT)
			{
				ev->Rot.Mul(ev->Rot, Quat(Model::ZAXIS, leftright));
				ev->Flags |= NavigateEvent::ROTATION | NavigateEvent::ZROTATION;
			}
	}
	MouseEngine::OnMouse(pos, flags);
	if (flags == 0)
		m_LastEvent = 0;
	else if (ev->Flags)
	{
		LogEvent(ev);
		return;
	}
	delete ev;
}

void Flyer::OnNavigate(NavigateEvent* e)
{
	Model*	target = (Model*) GetTarget();
	Vec3		v;

	if (!target || !target->IsClass(VX_Model))
		return;
	if (e->Flags & (NavigateEvent::LOCAL_POS | NavigateEvent::POSITION))
	{
		if (e->Flags & NavigateEvent::ABSOLUTE)
			v = target->GetTranslation();
		else
			v.Set(0,0,0);
		if (m_iDOF & Z_TRANS)
			v.z += e->Pos.z;
		if (m_iDOF & Y_TRANS)
			v.y += e->Pos.y;
		if (m_iDOF & X_TRANS)
			v.x += e->Pos.x;
		if (e->Flags & NavigateEvent::ABSOLUTE)
			target->SetTranslation(v);
		else
			target->Move(v);
	}
	if (m_iDOF & (X_ROT | Y_ROT | X_ROT) && (e->Flags & NavigateEvent::ROTATION))
		if (e->Flags & NavigateEvent::ABSOLUTE)
			target->SetRotation(e->Rot);
		else
			target->Turn(e->Rot);
}

bool Flyer::Eval(float t)
{
	m_TimeInc += 0.05f;
	OnMouse(MousePos, MouseFlags);
	if (MouseFlags == 0)
		m_TimeInc = 0;
	return true;
}

/****
 *
 * class Flyer override for SharedObj::Copy
 *
 ****/
bool Flyer::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!MouseEngine::Copy(srcobj))
		return false;
	const Flyer* src = (const Flyer*) srcobj;
	if (src->IsClass(VX_Flyer))
	{
		Lock();
		m_Scale = src->m_Scale;
		m_iDOF = src->m_iDOF;
		Unlock();
	}
	return true;
}

/****
 *
 * class Flyer override for SharedObj::Save
 *
 ****/
int Flyer::Save(Messenger& s, int v) const
{
	int32 h = MouseEngine::Save(s, v);
	if (h <= 0)
		return h;
 	s << OP(VX_Flyer, FLYER_SetScale) << h << GetScale();
 	s << OP(VX_Flyer, FLYER_SetDOF) << h << int32(GetDOF());
	return h;
}

DebugOut& Flyer::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Scale'>" << GetScale() << "<attr>");
	endl(dbg << "\t<attr name='DegreesOfFreedom'>" << GetDOF() << "</attr>");
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Flyer override for SharedObj::Do
 *	FLYER_SetScale		<float>
 *	FLYER_SetDOF		<int32>
 *
 ****/
bool Flyer::Do(Messenger& s, int32 op)
{
	float	v;
	int32	n;

	switch (op)
	{
		case FLYER_SetDOF:
		s >> n;
		SetDOF(n);
		break;

		case FLYER_SetScale:
		s >> v;
		SetScale(v);
		break;

		default:
		return MouseEngine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Flyer::DoNames[op - FLYER_SetScale]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen