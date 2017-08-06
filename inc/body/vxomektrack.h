#pragma once
#pragma managed(push, off)

/*!
 * @file vxkinecttrack.h
 * @brief Kinect-based body tracker.
 *
 * Creates an animation hierarchy for controlling a human skeleton
 * with the Kinect.
 *
 * @author Nola Donato
 * @ingroup vixen
 */
namespace Vixen {


class OmekInfo;

/*!
 * @class OmekTracker
 *
 * @brief Engine that animates a human skeleton using the Kinect.
 * The Microsoft Kinect device uses a depth camera to track the motion
 * of one or more users. This class gathers the depth camera image and
 * the joint positions / rotations of from the Kinect to animate the
 * skeletons of one or more players.
 *
 * Typically the target of the OmekTracker is a Skeleton which animates
 * a human skeleton given a stream of joint rotations. The combination of
 * a OmekTracker and a Skeleton can animate a skinned character based on
 * a user's motions.
 *
 * The OmekTracker can also emit a series of events describing the state
 * of the Kinect which observers can listen for.
 *
 * @see Skeleton Transformer Skin TrackEvent
 * @ingroup vixen
 */
class OmekTracker : public BodyTracker
{
public:
	VX_DECLARE_CLASS(OmekTracker);

	OmekTracker(const TCHAR* configfile = NULL);
	OmekTracker(const OmekTracker& src);
	~OmekTracker();

	virtual bool	Configure(const TCHAR* configfile);

	//! Return dimensions of Kinect camera output bitmap.
	virtual Vec2	GetImageSize() const;

	//! Return Kinect camera view volume.
	virtual bool	GetViewVolume(Box3& vvol);

	/*
	 * Omek joint IDs
	 */
	enum
	{ 
		NONE,
		HEAD,
		TORSO,
		NECK,
		RIGHT_HAND,
		LEFT_HAND,
		RIGHT_WRIST,
		LEFT_WRIST,
		RIGHT_SHOULDER,
		LEFT_SHOULDER,
		RIGHT_ELBOW,
		LEFT_ELBOW,
		RIGHT_COLLAR,
		LEFT_COLLAR,
		PELVIS,
		RIGHT_KNEE,
		LEFT_KNEE,
		RIGHT_ANKLE,
		LEFT_ANKLE,
		RIGHT_HIP,
		LEFT_HIP,
		SPINE1,
		SPINE2,
		SPINE3,
		SPINE4,
		WAIST,
		NUM_BONES
};

protected:
	virtual void	Init();
	virtual	bool	OnStart();
	virtual bool	Eval(float t);
	virtual bool	ComputeBoneAngles(int userid);

	OmekInfo*		m_Omek;
	Quat			m_OmekRest[Skeleton::NUM_BONES];		// the "T" pose as world rotations
	static int		m_OmekSensorBoneMap[OmekTracker::NUM_BONES]; // map Omek bones to Vixen default skeleton
};


}	// end Vixen

#pragma managed(pop)