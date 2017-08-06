#include "vixen.h"
#include "vxutil.h"


namespace Vixen {

VX_IMPLEMENT_CLASSID(TrackBall, MouseEngine, VX_TrackBall);

TrackBall::TrackBall() : MouseEngine()
{
	m_TrackButton = MouseEvent::LEFT;
	m_ZoomButton = MouseEvent::LEFT | MouseEvent::CONTROL;
	m_Zoom = 1.0f;
	m_XRot.Set(Vec3(0, 0, 0), 1);
	m_YRot.Set(Vec3(0, 0, 0), 1);
	m_Init.Set(Vec3(0, 0, 0), 1);
	GetMessenger()->Observe(this, Event::MOUSE, NULL);
}

void TrackBall::SetTarget(SharedObj* obj)
{
	SharedObj* oldtarget = GetTarget();
	Engine::SetTarget(obj);
	if (oldtarget)
		if (oldtarget->IsClass(VX_Model))
			((Model*) oldtarget)->SetRotation(m_Init);
		else if (oldtarget->IsClass(VX_Transformer))
			((Transformer*) oldtarget)->SetRotation(m_Init);
	m_XRot.Set(Vec3(0, 0, 0), 1);
	m_YRot.Set(Vec3(0, 0, 0), 1);
	m_Init.Set(Vec3(0, 0, 0), 1);
	if (obj == NULL)
		return;
	if (obj->IsClass(VX_Model))
		m_Init = ((Model*) obj)->GetRotation();
	else if (obj->IsClass(VX_Transformer))
		m_Init = ((Transformer*) obj)->GetRotation();
}

void TrackBall::OnMouse(const Vec2& pos, uint32 flags)
{
	SharedObj*	target = GetTarget();
	float	dx = pos.x - MousePos.x;
	float	dy = pos.y - MousePos.y;
	int		buttons = m_TrackButton | m_ZoomButton;

	MouseEngine::OnMouse(pos, flags);
	if (!IsActive() || ((flags & buttons) == 0))
		return;

	Box2	vport = GetMainScene()->GetViewport();
	float	yrot = -PI * dx / vport.Width();
	float	xrot = PI * dy / vport.Height();

	Quat	yorient(Model::YAXIS, yrot);
	Quat	xorient(Model::XAXIS, xrot);
	m_YRot.Mul(m_YRot, yorient);
	m_XRot.Mul(m_XRot, xorient);
	Quat orient(m_Init);
	orient.Mul(m_XRot, m_YRot);	
	m_Zoom = (2.0f * dy) / vport.Height();
	if (target == NULL)
		return;
	if (flags == m_ZoomButton)		// control key zooms camera
	{
		Camera* cam = GetMainScene()->GetCamera();
		Vec3 wpos;

		if (fabs(m_Zoom) > VX_SMALL_NUMBER)
		{
			wpos = cam->GetTranslation();
			wpos *= m_Zoom;		
			cam->Translate(wpos);
		}
	}
	else if (flags == m_TrackButton)
		if (target->IsClass(VX_Model))
			((Model*) target)->SetRotation(orient);
		else if (target->IsClass(VX_Transformer))
			((Transformer*) target)->SetRotation(orient);
}

}	// end Vixen