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
	KinectTracker(const KinectTracker& src);
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
		TORSO = 2,
		WAIST = 3,
		RIGHT_COLLAR = 4,
		RIGHT_SHOULDER = 5,
		RIGHT_ELBOW = 6,
		RIGHT_WRIST = 7,
		RIGHT_HAND = 8,
		RIGHT_FINGER = 9,
		LEFT_COLLAR = 10,
		LEFT_SHOULDER = 11,
		LEFT_ELBOW = 12,
		LEFT_WRIST = 13,
		LEFT_HAND = 14,
		LEFT_FINGER = 15,
		RIGHT_HIP = 16,
		RIGHT_KNEE = 17,
		RIGHT_ANKLE = 18,
		RIGHT_FOOT = 19,
		LEFT_HIP = 20,
		LEFT_KNEE = 21,
		LEFT_ANKLE = 22,
		LEFT_FOOT = 23,
		NUM_BONES = 24,
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