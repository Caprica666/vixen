#pragma once

/*!
 * @file vxposer.h
 * @brief animates a human body model.
 *
 * @author Nola Donato
 * @ingroup vxutil
 */

//#include "ScapeModel.h"

namespace Vixen {
class BodyPoser;

/*!
 * @class SCAPESkin
 *
 * @brief Engine that animates a human body model.
 *
 * The body is composed of 15 body parts. Animation is controlled by
 * setting the joint angles for each of these parts to pose the body.
 *
 * @see FaceAnimator Skin
 */
class SCAPESkin: public Skin
{
public:
	VX_DECLARE_CLASS(SCAPESkin);

	//!< Construct a body animator for the given body model
	SCAPESkin(const TCHAR* filename = NULL);

	//!< Returns the name of the body model
	const TCHAR*	GetBodyName() const		{ return m_BodyName; }
	
	//!< Sets the name of the body model
	void			SetBodyName(const TCHAR* bodyname);

	//!< Returns the joint angle for the given body part
	Quat			GetJointAngle(int i) const	{ return m_JointAngles[i]; }

	//! Load the given body model
	TriMesh*		Load(const TCHAR* bodyname = NULL);

	//! Update the internal body model based on the input joint angles
	void			Animate(const Quat* angles);

	//! Update joint angles from body tracker engine
	void			UpdateFromTracker(BodyPoser* bodytrack);

	//! Initialize body tracker joint angles from Poser
	void			InitTracker(BodyPoser* bodytrack);

	virtual	int		Save(Messenger&, int) const;
	virtual	bool	Do(Messenger& s, int op);
	virtual	bool	Copy(const SharedObj*);
	virtual	bool	Eval(float t);
	virtual	bool	OnStop();
	virtual	bool	OnStart();
	virtual	DebugOut&	Print(DebugOut& dbg = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief body parts whose joints may be manipulated in a pose.
	 */
	enum BodyParts
	{
		TORSO = 0,
		PELVIS = 1,
		LEFT_THIGH = 2,
		LEFT_CALF = 3,
		LEFT_FOOT = 4,
		RIGHT_THIGH = 5,
		RIGHT_CALF = 6,
		RIGHT_FOOT = 7,
		LEFT_UPPER_ARM = 8,
		LEFT_LOWER_ARM = 9,
		LEFT_HAND = 10,
		RIGHT_UPPER_ARM = 11,
		RIGHT_LOWER_ARM = 12,
		RIGHT_HAND = 13,
		HEAD = 14,
		NUM_BODY_PARTS = 15
	};

	enum Opcode
	{
		POSER_SetBodyName = SKIN_Next,
		POSER_Load,
		POSER_Animate,
		POSER_Next = SKIN_Next + 10
	};


protected:
	bool	UpdateActiveVerts();
	
	bool							m_IsLoaded;
//	SCAPE::ScapeModel::VectorList	m_ScapeAngles;
//	SCAPE::ScapeModel::SMatrixfp	m_BetaMatrix;
//	SCAPE::ScapeModel				m_ScapeModel;
	Core::String					m_BodyName;
	Quat							m_JointAngles[NUM_BODY_PARTS];
	static const TCHAR*				s_BodyPartNames[NUM_BODY_PARTS];
};

}	// end Vixen

