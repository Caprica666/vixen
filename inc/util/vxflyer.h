#pragma once

namespace  Vixen {

/*!
 * @class Flyer
 * @brief Mouse engine that flys around a 3D scene.
 * 
 * The @htmlonly <A><HREF="oview/viewer_help.htm"> Vixen Viewer</A> @endhtmlonly
 * uses the the flyer for its navigating. Two dimensional mouse
 * motion is interpreted as movement in the 3D scene
 * depending on which mouse button is pressed:
 * @code
 *	No buttons:		ignored
 *	Left button:	vertical forward-backward, horizontal yaw
 *	Right button:	vertical up-down, horizontal left-right
 *	Both buttons:	vertical pitch, horizontal roll
 * @endcode
 *
 * @see SharedObj::OnEvent MouseEvent Navigator Transformer
 *
 * @author Ben Discoe
 */
class Flyer : public Navigator
{
public:
	VX_DECLARE_CLASS(Flyer);

	Flyer(float scale = 1.0f);

	float		GetScale() const;
	int			GetDOF() const;
	void		SetScale(float);	//!< Set amount to scale mouse position by for translation.
	void		SetDOF(int iDOF);	//!< set degrees of freedom.

	virtual bool		Copy(const SharedObj*);
	virtual bool		Do(Messenger& s, int32 op);
	virtual int			Save(Messenger& s, int op) const;
	virtual bool		Eval(float t);
	virtual bool		OnEvent(Event *event);
	virtual void		OnMouse(const Vec2&, uint32);
	virtual void		OnNavigate(NavigateEvent *event);
	virtual DebugOut&	Print(DebugOut& dbg = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Degrees Of Freedom options
	 * @see Flyer::SetDOF
	 */
	enum
	{
		X_TRANS = 1,		//!< allow horizontal motion
		Y_TRANS = 2,		//!< allow vertical motion
		Z_TRANS = 4,		//!< allow zoom
		X_ROT = 8,			//!< allow rotation about X axis
		Y_ROT = 16,			//!< allow rotation about Y axis
		Z_ROT = 32,			//!< allow rotation about Z axis
		ALL = 63			//!< allow unconstrained motion
	};

	/*
	 * Flyer::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		FLYER_SetScale = Engine::ENG_NextOp,
		FLYER_SetDOF,
		FLYER_LastOp = Engine::ENG_NextOp + 10,
	};
protected:
	float	m_Scale;		// distance scale
	int32	m_iDOF;			// degrees of freedom
	float	m_TimeInc;
};


inline int Flyer::GetDOF() const
{
	return m_iDOF;
}


inline float Flyer::GetScale() const
{
	return m_Scale;
}

} // Vixen