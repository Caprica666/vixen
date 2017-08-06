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


class Kinect;

/*!
 * @class KinectTracker
 *
 * @brief Engine that animates a human skeleton using the Kinect.
 * The Microsoft Kinect device uses a depth camera to track the motion
 * of one or more users. This class gathers the depth camera image and
 * the joint positions / rotations of from the Kinect to animate the
 * skeletons of one or more players.
 *
 * Typically the target of the KinectTracker is a Skeleton which animates
 * a human skeleton given a stream of joint rotations. The combination of
 * a KinectTracker and a Skeleton can animate a skinned character based on
 * a user's motions.
 *
 * The KinectTracker can also emit a series of events describing the state
 * of the Kinect which observers can listen for.
 *
 * @see Skeleton Transformer Skin TrackEvent
 * @ingroup vixen
 */
class KinectTracker : public BodyTracker
{
public:
	VX_DECLARE_CLASS(KinectTracker);

	KinectTracker(const TCHAR* configfile = NULL);
	~KinectTracker();

	virtual bool	Configure(const TCHAR* configfile);

	//! Return dimensions of Kinect camera output bitmap.
	virtual Vec2	GetImageSize() const;

	//! Return Kinect camera view volume.
	virtual bool	GetViewVolume(Box3& vvol);

	//! Return depth camera field of view in radians.
	virtual	float	GetFOV();

	/*
	 * Sensor Joint IDs - these are NOT the same as the bone ordering
	 * in the Skeleton class. We use an internal mapping table to
	 * convert between Skeleton joint IDs and Sensor joint IDsl.
	 */
	enum
	{
		HEAD = 0,
		NECK = 1,
		LEFT_SHOULDER = 2,
		RIGHT_SHOULDER = 3,
		LEFT_ELBOW = 4,
		RIGHT_ELBOW = 5,
		LEFT_HAND = 6,
		RIGHT_HAND = 7,
		TORSO = 8,
		LEFT_HIP = 9,
		RIGHT_HIP = 10,
		LEFT_KNEE = 11,
		RIGHT_KNEE = 12,
		LEFT_ANKLE = 13,
		RIGHT_ANKLE = 14,
		NUM_BONES = 15,
	};

protected:
	virtual void	Init();
	virtual bool	Eval(float t);
	virtual bool	OnStart();
	virtual bool	ComputeBoneAngles(int userid);

	Kinect*			m_Kinect;
	Quat			m_KinectRest[NUM_BONES];	// the Kinect "T" pose as world rotations
	static int		m_SensorBoneMap[NUM_BONES];
};


}	// end Vixen

#pragma managed(pop)