#pragma once
#pragma managed(push, off)
/*!
 * @file vxbodytrack.h
 * @brief Body tracker base class.
 *
 * Creates an animation hierarchy for controlling a human skeleton
 * with a sensor.
 *
 * @author Nola Donato
 * @ingroup vixen
 */
namespace Vixen {


/*!
 * @class BodyTracker
 *
 * @brief Engine that animates a human skeleton using a sensor.
 * This class provides the infrastructure for implementing a body tracker
 * that can control an articulated human character.
 *
 * Typically the target of the BodyTracker is a Skeleton which animates
 * a human skeleton given a stream of joint rotations. The combination of
 * a BodyTracker and a Skeleton can animate a skinned character based on
 * a user's motions.
 *
 * The BodyTracker can also emit a series of events which observers can listen for.
 *
 * @see Skeleton Transformer Skin TrackEvent BodyTracker
 * @ingroup vixen
 */
class BodyTracker : public Engine
{
public:
	VX_DECLARE_CLASS(BodyTracker);

	BodyTracker();
	~BodyTracker();

	virtual bool	Configure(const TCHAR* configfile);

	//! Return the current tracking options.
	int				GetOptions() const				{ return m_Options; }

	//! Set the current tracking options.
	void			SetOptions(int options);

	//! Return the image being updated with the Kinect depth camera output.
	const Texture*	GetDepthTexture() const			{ return m_DepthTexture; }

	//! Designate an image to get Kinect depth camera output.
	void			SetDepthTexture(Texture* tex)	{ m_DepthTexture = tex; }

	//! Return the image being updated with the Kinect color camera output.
	const Texture*	GetColorTexture() const			{ return m_ColorTexture; }

	//! Designate an image to get Kinect color camera output.
	void			SetColorTexture(Texture* tex)	{ m_ColorTexture = tex; }

	//! Return dimensions of color camera output bitmap.
	virtual Vec2	GetImageSize() const;

	//! Return depth camera view volume.
	virtual bool	GetViewVolume(Box3& vvol);

	//! Return depth camera field of view in radians.
	virtual	float	GetFOV()						{ return 1.4f; }

	//! Return the initial user waist offset.
	Vec3			GetWaistOffset() const			{ return m_WaistOffset; }

	//! Set the initial user waist offset (default is 0,0,0).
	void			SetWaistOffset(const Vec3& v)	{ m_WaistOffset = v; }

	//! Get current pose from body tracker for the given user.
	Pose*			GetPose(int userid) const;

	//! Get internal skeleton for the current user.
	Skeleton*		GetSkeleton() const				{ return (Skeleton*) &m_Skeleton; }

	//! Get internal pose mapper.
	PoseMapper*		GetPoseMapper() const			{ return m_PoseMapper; }

	//! Replace internal pose mapper.
	void			SetPoseMapper(PoseMapper*);

	//! Get the position of the user.
	Vec3			GetUserPosition(int userid = 0) const;

	//! Get the user ID of the user being tracked.
	int				GetUserID() const				{ return m_UserID; }

	//! Set the Kinect user ID of the user to track.
	void			SetUserID(int userid)			{ m_UserID = userid; }

	virtual void	OnNewUser(int userid);
	virtual	void	OnStartJointTrack(int userid);
	virtual void	OnPauseJointTrack(int userid);
	virtual void	OnStopJointTrack(int userid);
	virtual void	OnUserMove(int userid, int partindex, const Vec3& pos, const Quat& dir);
	virtual void	OnInitDone();
	virtual void	OnUserTurn(int userid, const Vec3& torsoz);

	/*!
	 * Joints may be tracked individually based on their joint ID.
	 * The JointID is a zero based index used to identify the joint.
	 * Although different sensors use different bone mappings, the
	 * BodyTracker class uses a single bone mapping conventions for
	 * all sensors, converting internally when necessary.
	 */
	enum
	{
		TRACK_SKELETON = 1,		//! dynamically update the target skeleton
		TRACK_TORSO = 2,		//! update torso movement
		TRACK_HANDS = 4,		//! generate hand tracking events
		TRACK_FRONT_ONLY = 8,	//! only track when user facing front
		MAX_USERS = 8
	};

	enum Opcode
	{
		BODYTRACK_SetOptions = ENG_NextOp,
		BODYTRACK_Configure,
		BODYTRACK_SetPoseMapper,
		BODYTRACK_NextOp = ENG_NextOp + 10
	};

protected:
	virtual	int		Save(Messenger&, int) const;
	virtual	bool	Do(Messenger& s, int op);
	virtual	bool	Copy(const SharedObj*);
	virtual bool	Eval(float t);
	virtual bool	ComputeBoneAngles(int userid);
	virtual void	Init();

	//! Determine if Z axis forward after rotation.
	static float	ZForward(const Quat& rotation);

	//! Apply the bone angles which have changed to the internal skeleton.
	void			ApplyCurrentPose(int userid, bool changed[Skeleton::NUM_BONES]);

	/*
	 * Information for each user being tracked.
	 */
	class User
	{
	public:
		User() : UserID(0), Tracking(false) {  }

		int					UserID;							// ID of user being tracked
		bool				Tracking;						// true if user is being tracked now
		float				FacingSensor;					// true if user is facing sensor now
		mutable Ref<Pose>	CurrentPose;					// current user pose
		Quat				Rotations[Skeleton::NUM_BONES];	// the cuurent Kinect pose as world rotations
		Vec3				Positions[Skeleton::NUM_BONES];	// the current Kinect joint positions
	}				m_Users[MAX_USERS];

	Core::String	m_ConfigFile;						// name of configuration file
	int				m_UserID;							// Kinect user ID being tracked
	Ref<Texture>	m_DepthTexture;						// depth map texture
	Ref<Texture>	m_ColorTexture;						// color map texture
	Vec3			m_UserPos;							// position of user's waist
	bool			m_BindPoseRelative;					// produce joint angles relative to bind pose
	int				m_Options;							// tracking options
	float			m_LastTime;							// time of last track event
	Skeleton		m_Skeleton;							// internal skeleton
	Ref<PoseMapper>	m_PoseMapper;						// internal pose mapper
	Vec3			m_WaistOffset;						// initial waist offset for skeleton
};

/*
 * Computes the dot product of the Z Axis and the Z Axis rotated by the
 * input quaternion. A larger dot product indicates the quaternion
 * does not rotate far from the Z Axis and, if the object was
 * facing forward, it still is after rotation.
 */
inline float BodyTracker::ZForward(const Quat& rotation)
{
	Vec3	zaxis(Model::ZAXIS);
	Vec3	zrotated;
	Matrix	mtx(rotation);

	zrotated.TransformVector(mtx, zaxis);
	return zaxis.Dot(zrotated);
}

}	// end Vixen

#pragma managed(pop)