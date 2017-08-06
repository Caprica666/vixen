#pragma once
#pragma managed(push, off)
/*!
 * @file vxscapeskin.h
 * @brief Generative human body model.
 *
 * The SCAPE body model provides a skinned character whose shape
 * can be described by a set of six simple parameters.
 * The body model can be driven by the Skeleton to provide
 * an animatable human avatar.
 *
 * @see Skeleton KinectTracker
 *
 * @author Nola Donato
 * @ingroup vixen
 */

namespace Vixen {
class Skeleton;
class ScapeModel;


/*!
 * @class SCAPESkin
 *
 * @brief Engine that animates a human body model.
 *
 * The SCAPE body model provides a skinned character whose shape
 * can be described by a set of six simple parameters.
 * The body model can be driven by the Skeleton to provide
 * an animatable human avatar.
 *
 * @see Skeleton Skin KinectTracker
 * @ingroup vixen
 */
class SCAPESkin: public Skin
{
public:
	friend class ScapeCore;
	enum
	{
		HEIGHT = 0,
		WEIGHT = 1,
		BREAST = 2,
		ABDOMEN = 3,
		HIP = 4,
		LEG = 5,
		NUM_SHAPE_PARAMS
	};

	VX_DECLARE_CLASS(SCAPESkin);

	//! Construct a body animator for the given body model
	SCAPESkin(const TCHAR* filename = NULL);
	~SCAPESkin();

	//! Returns the name of the body model
	const TCHAR*	GetBodyName() const		{ return m_BodyName; }
	
	//! Set the name of the body model
	void			SetBodyName(const TCHAR* bodyname);

	//! Get the current shape of the body model.
	void			GetBodyShape(float* floatArray)		{ VX_ASSERT(floatArray); memcpy(floatArray, m_ShapeParams, NUM_SHAPE_PARAMS); }

	//! Set the shape of the body model.
	void			SetBodyShape(const float* floatArray);

	//! Set the size of a body part
	void			SetSize(int i, float v);

	//! Get the size of a body part
	float			GetSize(int i)	{ VX_ASSERT((i >= 0) && (i < NUM_SHAPE_PARAMS)); return m_ShapeParams[i]; }

	//! Get the current bind pose of the SCAPE mesh.
	void			ComputeBindPose(Vec3 pos[Skeleton::NUM_BONES], Quat rot[Skeleton::NUM_BONES], float* BoneLengths);
	void			ComputeBindPoseOLD(Quat* RestRot, Vec3* RestPos, float* BoneLengths);

	//! Load the given body model
	Shape*			Load(const TCHAR* bodyname = NULL, Shape* mesh = NULL);

	//! Create body poser skeleton from SCAPE model
	Model*			AttachSkeleton(Skeleton* skeleton, Model* skelshapes = NULL);

	virtual	int		Save(Messenger&, int) const;
	virtual	bool	Do(Messenger& s, int op);
	virtual	bool	Copy(const SharedObj*);
	virtual	bool	Eval(float t);
	virtual void	Compute(float t);
	virtual bool	OnStart();
	virtual	bool	OnStop();
	virtual	bool	OnEvent(Event*);
	virtual	DebugOut&	Print(DebugOut& dbg = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	
protected:
	enum Joints
	{
		WAIST = 0,
		LEFT_HIP_1 = 1,
		LEFT_HIP_2 = 2,
		LEFT_HIP_3 = 3,
		LEFT_KNEE = 4,
		LEFT_HEEL = 5,
		RIGHT_HIP_1 = 6,
		RIGHT_HIP_2 = 7,
		RIGHT_HIP_3 = 8,
		RIGHT_KNEE = 9,
		RIGHT_HEEL = 10,
		TORSO_ROT = 11,
		TORSO_BEND = 12,
		NECK_1 = 13,
		NECK_2 = 14,
		NECK_3 = 15,
		LEFT_SHOULDER_1 = 16,
		LEFT_SHOULDER_2 = 17,
		LEFT_SHOULDER_3 = 18,
		LEFT_ELBOW = 19,
		LEFT_WRIST = 20,
		RIGHT_SHOULDER_1 = 21,
		RIGHT_SHOULDER_2 = 22,
		RIGHT_SHOULDER_3 = 23,
		RIGHT_ELBOW = 24,
		RIGHT_WRIST = 25,
		NUM_BODY_PARTS
	};

	enum Opcode
	{
		SCAPE_SetBodyName = SKIN_Next,
		SCAPE_Load,
		SCAPE_SetBodyShape,
		SCAPE_SetSize,
		SCAPE_UpdateBindPose,
		SCAPE_Next = SKIN_Next + 10
	};

	void			FindClosestJointVerts();
	int				SelectByBone(IntArray& selected, int bone1, int bone2, const Vec3& jpos, float maxdist);

	bool			m_IsLoaded;
	bool			m_IsReady;
	int				m_MaxBones;
	Core::String	m_BodyName;
	ScapeModel*		m_Scape;
	float			m_ShapeParams[NUM_SHAPE_PARAMS];
	Ref<Deformer>	m_BodyDeformer;
	Ref<Morph>		m_BodyShaper;
};

}	// end Vixen

#pragma managed(pop)