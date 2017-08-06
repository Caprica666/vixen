#include "vixen.h"
#include "vxutil.h"
#include "util/vxarcball.h"

namespace Vixen {
VX_IMPLEMENT_CLASSID(ArcBall, Engine, VX_ArcBall);

ArcBall::ArcBall() : MouseEngine()
{
	m_Ball.Set(Vec3(0,0,0), 0);
	m_Rotation.Set(Vec3(0,0,0), 1);
	m_Button = MouseEvent::LEFT;
	GetMessenger()->Observe(this, Event::MOUSE, NULL);
}

Vec3 ArcBall::WhereOnSphere(const Vec2& pos) const
{
	Vec3	ballMouse;
	float	mag;
	Model*	mod = (Model*) GetTarget();

    ballMouse.x = -(pos.x - m_Ball.Center.x) / m_Ball.Radius;
    ballMouse.y = (pos.y - m_Ball.Center.y) / m_Ball.Radius;
    mag = ballMouse.x * ballMouse.x + ballMouse.y * ballMouse.y;
	if (mag > 1)
	{
        float scale = float(1 / sqrt(mag));
        ballMouse.x = ballMouse.x * scale;
        ballMouse.y = ballMouse.y * scale;
        ballMouse.z = 0;
	}
    else
        ballMouse.z = sqrt(1.0f - mag);
    return ballMouse;
}

Quat ArcBall::QuatFromPoints(const Vec3& from, const Vec3& to)
{
	Quat qu;
    qu.x = from.y * to.z - from.z * to.y;
    qu.y = from.z * to.x - from.x * to.z;
    qu.z = from.x * to.y - from.y * to.x;
    qu.w = from.x * to.x + from.y * to.y + from.z * to.z;
	return qu;
}

void ArcBall::Initialize(const Vec2& pos)
{
	Model*	model = (Model*) GetTarget();
	Scene*	scene = GetMainScene();
	Sphere	sphere;

	if (model && model->IsClass(VX_Model))
	{
		if (model->IsClass(VX_Camera))
		{
			Vec3 p;
			scene->GetModels()->GetBound(&sphere, Model::WORLD);
			sphere.Radius = model->GetTranslation().Distance(sphere.Center);
		}
		else
			model->GetBound(&sphere, Model::WORLD);
		m_Ball.Center.Set(sphere.Center.x, sphere.Center.y, sphere.Center.z);
		m_Rotation.Set(model->GetRotation());
		m_Ball.Radius = sphere.Radius;
	}
	else
	{
		float w = scene->GetViewport().Width();
		float h = scene->GetViewport().Height();             
		m_Ball.Center.Set(w / 2, h / 2, 0);
		m_Rotation.Set(Vec3(0, 0, 0), 1);
		m_Ball.Radius = (w > h) ? m_Ball.Center.x : m_Ball.Center.y;
	}
}

void ArcBall::OnMouse(const Vec2& pos, uint32 flags)
{
	SharedObj*	target = GetTarget();
	int			mouseflags = MouseFlags;
	Vec2		oldpos = MousePos;

	MouseEngine::OnMouse(pos, flags);		// update position and flags
	if (flags != m_Button)					// not dragging now?
		return;
	if ((mouseflags & m_Button) == 0)		// starting a drag?
	{
		Initialize(pos);
		return;
	}
    Vec3 from = WhereOnSphere(oldpos);		// where we were on the sphere last event
    Vec3 to = WhereOnSphere(pos);			// where we are on the sphere now
    m_Rotation = QuatFromPoints(from, to);	// relative rotation quat
	if (target && target->IsClass(VX_Model))
	{
		Matrix	mtx;
		Model*	mod = (Model*) target;
		Vec3	t = mod->GetTranslation();

		if (mod->IsClass(VX_Camera))
		{
			mod->Translate(m_Ball.Center);
			mod->Rotate(m_Rotation);
			mod->Translate(-m_Ball.Center);
		}
		else
		{
			mtx.TranslationMatrix(-m_Ball.Center);
			mtx.Rotate(m_Rotation);
			mtx.SetTranslation(t);
			((Model*) target)->SetTransform(&mtx);
		}
	}
	if (IsSet(SharedObj::DOEVENTS))
		LogEvent();
}

/*
 * If we should be logging events, log the navigate event here
 */
void ArcBall::LogEvent()
{
	NavigateEvent*	event = new NavigateEvent;

	event->Sender = this;
	event->Time = 0;
	event->Flags = NavigateEvent::ROTATION | NavigateEvent::YROTATION | NavigateEvent::XROTATION | NavigateEvent::ZROTATION;
	event->Rot = m_Rotation;
	event->Log();
}

}	// end Vixen