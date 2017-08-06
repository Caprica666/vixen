#pragma once

namespace Vixen {
/*!
 * @class ArcBall
 * @brief a very simpler ArcBall manipulator.
 *
 * Uses the mouse to interactively rotate a model or camera
 * using the ArcBall user interface.
 *
 * @ingroup vixen
 */
class ArcBall : public MouseEngine
{
public:
	VX_DECLARE_CLASS(ArcBall);

	ArcBall();
	void		OnMouse(const Vec2&, uint32);
	void		SetButton(uint32 flags);
	uint32		GetButton() const;
	void		SetBall(const Sphere& ball);
	const Sphere& GetBall() const;

protected:
	void		Initialize(const Vec2&);
	Vec3		WhereOnSphere(const Vec2& pos) const;
	Quat		QuatFromPoints(const Vec3& from, const Vec3& to);
	void		LogEvent();

	uint32		m_Button;
	Sphere		m_Ball;
	Quat		m_Rotation;
};

inline void ArcBall::SetButton(uint32 button)
{
	m_Button = button;
}

inline uint32 ArcBall::GetButton() const
{
	return m_Button;
}

inline void ArcBall::SetBall(const Sphere& ball)
{
	m_Ball = ball;
}

inline const Sphere& ArcBall::GetBall() const
{
	return m_Ball;
}

} // end Vixen