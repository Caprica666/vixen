#include "vixen.h"
#include "media/vxmediabase.h"
#include "body/vxbodytrack.h"


namespace Vixen {

static const TCHAR* opnames[] = {
		TEXT("SetOptions"),
		TEXT("Configure"),
		TEXT("SetPoseMapper"),
};

const TCHAR** BodyTracker::DoNames = opnames;

VX_IMPLEMENT_CLASS(BodyTracker, Engine);


/*!
 * @fn BodyTracker::BodyTracker()
 *
 * Constructs a human body animation engine which tracks the body.
 */

BodyTracker::BodyTracker()
:	Engine(),
	m_Skeleton(Skeleton::NUM_BONES),
	m_BindPoseRelative(true)
{
	m_Skeleton.SetName(TEXT("bodytracker.skeleton"));
	Init();
	m_PoseMapper = new PoseMapper();
	m_PoseMapper->SetName(TEXT("bodytracker.default.posemapper"));
	m_PoseMapper->SetSource(&m_Skeleton);
	Debug = 1;
}


void BodyTracker::Init()
{
	m_UserIndex = 0;
	m_LastTime = 0;
	for (int u = 0; u < MAX_USERS; ++u)
	{
		m_Users[u].UserID = 0;
		m_Users[u].Visible = 0;
		for (int i = 0; i < Skeleton::NUM_BONES; ++i)
			m_Users[u].Rotations[i].Set(0, 0, 0, 1);
	}
}

/*
 * Allow the data area for the bitmaps for color and depth to be freed
 */
BodyTracker::~BodyTracker()
{
	Texture* tex = m_ColorTexture;
	Bitmap* bmap;
	if (tex)
	{
		bmap = tex->GetBitmap();
		if (bmap)
			bmap->Format &= ~Bitmap::NOFREE_DATA;
	}
	tex = m_DepthTexture;
	if (tex)
	{
		bmap = tex->GetBitmap();
		if (bmap)
			bmap->Format &= ~Bitmap::NOFREE_DATA;
	}
}

/*!
 * @fn void BodyTracker::SetPoseMapper(PoseMapper* mapper)
 * @param mapper	new pose mapper to use
 *
 * Replaces the current pose mapper used to map between the body tracker
 * skeleton and a rendered avatar with a new one.
 *
 * @see PoseMapper SkeletonMapper
 */
void BodyTracker::SetPoseMapper(PoseMapper* mapper)
{
	VX_ASSERT(mapper);

	VX_STREAM_BEGIN(s)
		*s << OP(VX_BodyTracker, BODYTRACK_SetPoseMapper) << this << mapper;
	VX_STREAM_END(  )

	m_PoseMapper = mapper;
	if (mapper->GetName() == NULL)
		mapper->SetName(TEXT("bodytracker.default.posemapper"));
	mapper->SetSource(&m_Skeleton);
}

/*!
 * @fn bool BodyTracker::Configure(const TCHAR* configfile)
 * @param configfile	Name of the configuration file.
 *
 * Configures the body tracker from a file. The base implementation
 * does nothing. Subclasses can override to control initialization behavior
 * but base implementation should be always be called. 
 *
 * @return true if configuration successful, else false
 */
bool BodyTracker::Configure(const TCHAR* configfile)
{
	m_ConfigFile = configfile;
	if ((configfile == NULL) || (*configfile == 0))
		return false;
	return true;
}

/*!
 * @fn Pose* BodyTracker::GetPose(int userid) const
 * @param userid	ID of user whose pose is desired
 *
 * @returns the current pose from the Kinect, NULL if none can be obtained
 */
Pose* BodyTracker::GetPose(int userid) const
{
	Pose*		pose;
	Skeleton*	skel = (Skeleton*) GetSkeleton();
	int			i = FindUserIndex(userid);

	if (i < 0)
		return NULL;
	pose = m_Users[i].CurrentPose;
	if (pose == NULL)
	{
		pose = new Pose(skel);
		m_Users[i].CurrentPose = pose;
	}
	pose->SetWorldPositions(m_Users[i].Positions);
	pose->SetWorldRotations(m_Users[i].Rotations);
	return pose;
}

/*!
 * @fn Vec3 BodyTracker::GetUserPosition(int userid) const
 * @param userid	0-based index of user whose position we want
 *
 * The current position of the TORSO from the Kinect
 * is returned, adjusted for the target coordinate space.
 */
Vec3 BodyTracker::GetUserPosition(int) const
{
	return m_UserPos;
}

/*!
 * @fn Vec3 BodyTracker::GetViewVolume(Box3& vvol)
 * @param vvol	box to get the view volume of the depth sensor
 *
 * This function should return the 3D bounding volume of the
 * coordinates returned by the sensor. The default implementation
 * returns false - subclasses are expected to override and implement
 * this function for a specific sensor type.
 *
 * @return true if view volume can be obtained, else false
 *
 * @see KinectTracker::GetViewVolume
 */
bool BodyTracker::GetViewVolume(Box3& vvol)
{
	return false;
}

/*!
 * @fn void BodyTracker::SetOptions(int opts)
 * @param opts	body tracking options
 *				TRACK_FRONT_ONLY	only track the user if facing the sensor
 *				TRACK_SKELETON		enable skeleton tracking
 *				TRACK_HANDS			enable hand tracking
 *				TRACK_TORSO			track torso position
 *
 * @see KinectTracker::GetOptions BodyTracker::GetUserPosition
 */
void BodyTracker::SetOptions(int opts)
{
	m_Options = opts;
}


/*
 * Updates the color and depth map textures from the Kinect cameras.
 * If a Skeleton is our target or a child, we update the joint angles
 * for the user associated with it.
 */
bool BodyTracker::Eval(float t)
{
	if (m_UserIndex != 0)
	{
		Sphere		s;
		Camera*		cam = World3D::Get()->GetScene()->GetCamera();
		int			index = m_UserIndex - 1;
		bool		changed =	ComputeBoneAngles(index);

		if (m_Options & TRACK_FRONT_ONLY)
		{
			User&	user(m_Users[index]);

			s.Center = m_Skeleton.GetPosition();
			s.Radius = 0.05f;
			if (!changed || !cam->IsVisible(s))
			{
				VX_TRACE(Debug, ("BodyTracker user %d exit", m_UserIndex));
				OnPauseJointTrack(user.UserID);
				return true;
			}
			if (!user.FacingSensor)
				return true;
		}
		m_Skeleton.Eval(t);
		m_PoseMapper->Eval(t);
		return true;
	}
	float	mindist = 100000.0f;
	int		userid = 0;
	for (int i = 0; i < MAX_USERS; ++i)
	{
		float dist;

		if (!m_Users[i].Visible)
			continue;
		if (!ComputeBoneAngles(i))
			continue;
		if (!m_Users[i].FacingSensor)
			continue;
		dist = m_Users[i].Positions[0].Length();
		if (dist < mindist)
		{
			mindist = dist;
			userid = i + 1;
		}
	}
	if (userid != 0)
		OnStartJointTrack(userid);
	return true;
}

void BodyTracker::ApplyCurrentPose(int userid, bool changed[Skeleton::NUM_BONES])
{
	Skeleton*	skeleton = (Skeleton*) GetSkeleton();
	Pose		pose(skeleton, Pose::BIND_POSE_RELATIVE);
	Vec3		zaxis(Model::ZAXIS);
	Vec3		zrotated;
	float		d;
	User&		user(GetUser(userid));
	Matrix		mtx(user.Rotations[Skeleton::TORSO]);

	zrotated.TransformVector(mtx, zaxis);	// Z axis rotated by torso rotation
	d = zaxis.Dot(zrotated);
	if (d < 0.5f)							// this user not facing the sensor
	{
		if (user.FacingSensor)				// but was facing sensor before?
			OnUserTurn(userid, zrotated);
		user.FacingSensor = false;
		if (m_Options & TRACK_FRONT_ONLY)	// only track if facing front?
			return;
	}
	else									// this user is facing the sensor
	{
		if (!user.FacingSensor)				// but was not facing it before?
			OnUserTurn(userid, zrotated);
		user.FacingSensor = true;
	}
	for (int i = 0; i < Skeleton::NUM_BONES; ++i)
	{
		Quat	q(user.Rotations[i]);

		if (changed[i] && !q.IsEmpty())
			pose.SetWorldRotation(i, q);
	}
	m_Skeleton.ApplyPose(&pose);
}


/*!
 * @fn bool BodyTracker::ComputeBoneAngles(int userindex)
 * @param userindex		index of the user to drive the skeleton
 *
 * Gathers joint rotations from the sensor and updates the
 * internal skeleton. The default implementation returns false
 * and does nothing. Subclasses are expected to override this
 * function and provide an implementation which queries the sensor.
 *
 * @return true if bone angles for this user changed, else false
 *
 * @see KinectTracker::ComputeBoneAngles
 */
bool BodyTracker::ComputeBoneAngles(int userindex)
{
	return false;
}


/*!
 * @fn Vec2 BodyTracker::GetImageSize() const
 * Get the size of the camera frames
 * for depth and color capture (default is 640 x 480).
 *
 * Subclasses are expected to override this function to provide
 * a sensor-specific implementation. The default returns 640x480.
 *
 * @return width & height of color sensor frame
 */
Vec2	BodyTracker::GetImageSize() const
{
	return Vec2(640, 480);
}

/*!
 * @fn void BodyTracker::OnInitDone()
 *
 * Callback invoked when the sensor initialization is complete.
 * If DOEVENTS is enabled, a TrackEvent of type INIT_DONE is logged.
 *
 * @see BodyTracker::OnStartJointTrack BodyTracker::OnStopBodyTrack
 */
void BodyTracker::OnInitDone()
{
	VX_TRACE(Debug, ("BodyTracker initialization done"));
	if (IsSet(SharedObj::DOEVENTS) && IsActive())
	{
		TrackEvent* ev = new TrackEvent(TrackEvent::INIT_DONE, 0, this);
		ev->Log();
	}
}

/*!
 * @fn void BodyTracker::OnNewUser(int kinectID)
 * @param kinectID	Kinect ID of new user
 *
 * Callback invoked when the sensor detects a new user.
 * If DOEVENTS is enabled, a TrackEvent of type NEW_USER is logged.
 * If the maximum number of users is exceeded, more new users
 * will not be recognized.
 *
 * @see BodyTracker::OnStartJointTrack BodyTracker::OnStopBodyTrack
 */
void BodyTracker::OnNewUser(int kinectID)
{
	VX_TRACE(Debug, ("BodyTracker new user %d", kinectID));
	int i = FindUserIndex(kinectID);

	if (i >= 0)
		return;
	for (i = 0; i < MAX_USERS; ++i)
		if (!m_Users[i].Visible)
		{
			m_Users[i].UserID = kinectID;
			m_Users[i].Visible = true;
			m_Users[i].Tracking = false;
			m_Users[i].FacingSensor = true;
			if (IsSet(SharedObj::DOEVENTS) && IsActive())
			{
				TrackEvent* ev = new TrackEvent(TrackEvent::NEW_USER, i + 1, this);
				ev->Log();
			}
			return;
		}
}

/*!
 * @fn void BodyTracker::OnLostUser(int kinectID)
 * @param kinectID	Kinect ID of lost user
 *
 * Callback invoked when the sensor loses a  user.
 * If DOEVENTS is enabled, a TrackEvent of type LOST_USER is logged.
 *
 * @see BodyTracker::OnStartJointTrack BodyTracker::OnStopBodyTrack
 */
void BodyTracker::OnLostUser(int kinectID)
{
	VX_TRACE(Debug, ("BodyTracker new user %d", kinectID));

	int i = FindUserIndex(kinectID);
	if (i < 0)
		return;
	m_Users[i].UserID = 0;
	m_Users[i].Visible = false;
	m_Users[i].Tracking = false;
	if (IsSet(SharedObj::DOEVENTS) && IsActive())
	{
		TrackEvent* ev = new TrackEvent(TrackEvent::LOST_USER, i + 1, this);
		ev->Log();
	}
}

/*!
 * @fn void BodyTracker::OnUserTurn(int userid, const Vec3& torsoz)
 * @param userid	User ID of user
 * @param torsoz	The torso forward axis after rotation
 *
 * Callback invoked when the user turns away or faces the sensor.
 * If DOEVENTS is enabled, a TrackEvent of type USER_TURN is logged.
 *
 * @see BodyTracker::OnStartJointTrack BodyTracker::OnStopJointTrack
 */
void BodyTracker::OnUserTurn(int userid, const Vec3& torsoz)
{
	VX_TRACE(Debug, ("BodyTracker user turned %d (%0.3f, %0.3f, %0.3f)", userid, torsoz.x, torsoz.y, torsoz.z));
	if (IsSet(SharedObj::DOEVENTS) && IsActive())
	{
		TrackEvent* ev = new TrackEvent(TrackEvent::USER_TURN, userid, this);
		ev->Position = torsoz;
		ev->Log();
	}
}


/*!
 * @fn void BodyTracker::OnStartJointTrack(int kinectID)
 * @param kinectID	Kinect ID of user
 *
 * Callback invoked when the sensor starts tracking a user.
 * If DOEVENTS is enabled, a TrackEvent of type START_BODY_TRACK is logged.
 *
 * @see BodyTracker::OnNewUser BodyTracker::OnPauseJointTrack BodyTracker::OnStopJointTrack
 */
void BodyTracker::OnStartJointTrack(int kinectID)
{
	int i = FindUserIndex(kinectID);
	if (i < 0)
		return;
	m_Users[i].Tracking = true;
	VX_TRACE(Debug, ("BodyTracker started tracking user %d", kinectID));

	if (m_UserIndex == 0)
	{
		m_UserIndex = i + 1;
		if (IsSet(SharedObj::DOEVENTS))
		{
			TrackEvent* ev = new TrackEvent(TrackEvent::START_BODY_TRACK, m_UserIndex, this);
			ev->Log();
		}
	}
}

/*!
 * @fn void BodyTracker::OnStopJointTrack(int userid)
 * @param kinectID	Kinect ID of user
 *
 * Callback invoked when the sensor drops a user.
 * If DOEVENTS is enabled, a TrackEvent of type STOP_BODY_TRACK is logged.
 *
 * @see BodyTracker::OnNewUser BodyTracker::OnPauseJointTrack BodyTracker::OnStopJointTrack
 */
void BodyTracker::OnStopJointTrack(int kinectID)
{
	int i = FindUserIndex(kinectID);
	if (i < 0)
		return;
	m_Users[i].FacingSensor = false;
	m_Users[i].Tracking = false;
	if (m_UserIndex == (i + 1))
	{
		if (IsSet(SharedObj::DOEVENTS))
		{
			TrackEvent* ev = new TrackEvent(TrackEvent::STOP_BODY_TRACK, m_UserIndex, this);
			ev->Log();
		}
		m_UserIndex = 0;
	}
	VX_TRACE(Debug, ("BodyTracker stopped tracking user %d", kinectID));
}

/*!
 * @fn void BodyTracker::OnPauseJointTrack(int kinectID)
 * @param kinectID	Kinect ID of user
 *
 * Callback invoked when the sensor temporarily stops tracking a user
 * (they leave the frame or cannot be tracked).
 * If DOEVENTS is enabled, a TrackEvent of type STOP_BODY_TRACK is logged.
 *
 * @see BodyTracker::OnNewUser BodyTracker::OnPauseJointTrack BodyTracker::OnStopJointTrack
 */
void BodyTracker::OnPauseJointTrack(int kinectID)
{
	int i = FindUserIndex(kinectID);
	if (i < 0)
		return;
	if (m_UserIndex == (i + 1))
	{
		if (IsSet(SharedObj::DOEVENTS))
		{
			TrackEvent* ev = new TrackEvent(TrackEvent::PAUSE_BODY_TRACK, m_UserIndex, this);
			ev->Log();
		}
		m_UserIndex = 0;
	}
	VX_TRACE(Debug, ("BodyTracker paused tracking user %d", kinectID));
}

/*!
 * @fn void BodyTracker::OnUserMove(int userid, int partindex, const Vec3& p, const Quat& r)
 * @param userid	User ID of user which moved
 * @param partindex	index of joint that moved
 * @param p			current joint position
 * @param r			current joint rotation
 *
 * Callback invoked when a joint position or rotation changes.
 * If DOEVENTS is enabled and the body tracker is active, a TrackEvent of type USER_MOVE
 * is logged. Observers may subscribe to this event to track skeleton changes.
 *
 * @see BodyTracker::OnStartJointTrack BodyTracker::OnPauseJointTrack
 */
void BodyTracker::OnUserMove(int userid, int partindex, const Vec3& p, const Quat& r)
{
	if (IsSet(SharedObj::DOEVENTS) && IsActive())
	{
		TrackEvent* ev = new TrackEvent(TrackEvent::USER_MOVE, userid, this);
		ev->Position = p;
		ev->Rotation = r;
		ev->PartIndex = partindex;
		ev->Log();
	}
}

bool BodyTracker::Copy(const SharedObj* src_obj)
{
	const BodyTracker* src = (const BodyTracker*) src_obj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Engine::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_BodyTracker))
	{
		m_Options = src->m_Options;
		m_PoseMapper = src->m_PoseMapper;
		return Configure(m_ConfigFile);
	}
	return true;
}

int BodyTracker::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h <= 0)
		return h;
 	s << OP(VX_BodyTracker, BODYTRACK_SetOptions) << h << m_Options;
 	s << OP(VX_BodyTracker, BODYTRACK_SetPoseMapper) << h << m_PoseMapper;
	s << OP(VX_BodyTracker, BODYTRACK_Configure) << h << m_ConfigFile;
	return h;
}

bool BodyTracker::Do(Messenger& s, int op)
{
	int32		n;
	SharedObj*	obj;
	TCHAR		path[VX_MaxPath];

	switch (op)
	{
		case BODYTRACK_SetOptions:
		s >> n;
		SetOptions(n);
		break;

		case BODYTRACK_SetPoseMapper:
		s >> obj;
		VX_ASSERT(obj && obj->IsClass(VX_PoseMapper));
		SetPoseMapper((PoseMapper*) obj);
		break;

		case BODYTRACK_Configure:
		s >> path;
		Configure(path);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << BodyTracker::DoNames[op - BODYTRACK_SetOptions]
					   << " " << this);
#endif
	return true;
}
}	// end Vixen