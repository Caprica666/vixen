#pragma once

namespace Vixen {

class TrackBall : public MouseEngine
{
public:
	VX_DECLARE_CLASS(TrackBall);

	TrackBall();
	void		OnMouse(const Vec2&, uint32);
	void		SetButtons(int track, int zoom);
	void		SetTarget(SharedObj*);

protected:
	uint32		m_TrackButton;
	uint32		m_ZoomButton;
	Quat		m_Init;
	Quat		m_XRot;
	Quat		m_YRot;
	float		m_Zoom;
};

inline void TrackBall::SetButtons(int track, int zoom)
{
	m_TrackButton = track;
	m_ZoomButton = zoom;
}

} // end Vixen