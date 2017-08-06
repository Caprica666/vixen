#include "vixen.h"
#include "media/vxmediabase.h"
#include "body/vxbodytrack.h"
#include "body/vxkinecttrack.h"
#include "NiTE.h"


namespace Vixen {

/*
 * Kinect helper class to isolate all the Kinect API calls.
 */
class Kinect : public Core::RefObj
{
public:
	VX_DECLARE_CLASS(Kinect);

	nite::UserTrackerFrameRef	UserFrame;
	nite::UserTracker	UserTracker;
	nite::UserId		LastUser;
	int					Width;
	int					Height;
	int					Depth;
	float				FOV;
	bool				IsStarted;
	KinectTracker*		Owner;

	Kinect(KinectTracker* owner = NULL);
	~Kinect();
	bool		OpenRecording(const TCHAR* file);
	bool		Configure(const TCHAR* file);
	bool		Start();
	void		Update(BodyTracker::User users[BodyTracker::MAX_USERS]);
	bool		GetJointRotation(nite::UserId userid, nite::JointType jointindex, Quat& rotationk);
	bool		GetJointPosition(nite::UserId nId, nite::JointType jointindex, Vec3& position);
	void		NewUser(int userid);
	void		LostUser(int userid);
	void		ExitUser(int userid);
	void		EnterUser(int userid);
	void		CalibrationBad(int userid);
	void		MakeDepthMap(Texture* image);
	void		MakeUserMap(Texture* image);
	void		UpdateImageFromDepth(Texture* image, const openni::DepthPixel* depthmap);
	void		UpdateImageFromLabels(Texture* image, const openni::DepthPixel* depthpixels, const nite::UserId* userpixels);
};

VX_IMPLEMENT_CLASS(Kinect, RefObj);
VX_IMPLEMENT_CLASS(KinectTracker, BodyTracker);

/*
 * Maps from Kinect bone ordering to Skeleton bone ordering
 */
int	KinectTracker::m_SensorBoneMap[KinectTracker::NUM_BONES] = {
// REVERSE RIGHT AND LEFT
		Skeleton::HEAD,
		Skeleton::NECK,
		Skeleton::RIGHT_SHOULDER,
		Skeleton::LEFT_SHOULDER,
		Skeleton::RIGHT_ELBOW,
		Skeleton::LEFT_ELBOW,
		Skeleton::RIGHT_HAND,
		Skeleton::LEFT_HAND,
		Skeleton::TORSO,
		Skeleton::RIGHT_HIP,
		Skeleton::LEFT_HIP,
		Skeleton::RIGHT_KNEE,
		Skeleton::LEFT_KNEE,
		Skeleton::RIGHT_ANKLE,
		Skeleton::LEFT_ANKLE,

};  

/*
 * Called when new skeleton data available
 */
class NiteListener : public nite::UserTracker::Listener
{
public:
	NiteListener(Kinect* kinect, BodyTracker::User* users)
	:	nite::UserTracker::Listener(),
		Owner(kinect),
		Users(users)
	{ }

	virtual void onNewFrame(nite::UserTracker& ut)
	{
		Owner->Update(Users);
	}

	Kinect*				Owner;
	BodyTracker::User*	Users;
};

/*!
 * @fn KinectTracker::KinectTracker(const TCHAR* configfile)
 * @param configfile	Name of the Kinect configuration file. It defaults to
 *						"KinectConfig.xml" from the current directory if NULL.
 *
 * Constructs a human body animation engine which tracks the body
 * using the Kinect controller. 
 */
KinectTracker::KinectTracker(const TCHAR* configfile)
:	m_Kinect(NULL),
	BodyTracker()
{
	m_Kinect = new Kinect(this);
	if (configfile)
		Configure(configfile);
}

/*!
 * @fn KinectTracker::KinectTracker(const KinectTracker& src)
 * @param src	Master Kinect tracker, this one actually talks to the Kinect
 *				and subsequent trackers made from this one just share data
 *
 * Constructs a human body animation engine which tracks the body
 * using the Kinect controller. 
 */
KinectTracker::KinectTracker(const KinectTracker& src)
:	m_Kinect(src.m_Kinect),
	BodyTracker(src)
{
}


/*
 * Allow the data area for the bitmaps for color and depth to be freed
 */
KinectTracker::~KinectTracker()
{
	if (m_Kinect)
	{
		m_Kinect->Delete();
		m_Kinect = NULL;
	}
}

void KinectTracker::Init()
{
	Vec3	vecs[3];

	m_UserID = -1;
	m_LastTime = 0;
	/*
	 * Rest pose for the Kinect is the "T" pose (legs down, arms out horizontally)
	 * It might be different from the rest pose of the target body so
	 * we use this info to transform one to the other.
	 */
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	m_KinectRest[Skeleton::HEAD].Set(vecs);
	m_KinectRest[Skeleton::NECK].Set(vecs);
	m_KinectRest[Skeleton::TORSO].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	m_KinectRest[Skeleton::WAIST].Set(vecs);
	m_KinectRest[Skeleton::LEFT_HIP].Set(vecs);
	m_KinectRest[Skeleton::LEFT_KNEE].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_HIP].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_KNEE].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	m_KinectRest[Skeleton::LEFT_COLLAR].Set(vecs);
	m_KinectRest[Skeleton::LEFT_SHOULDER].Set(vecs);
	m_KinectRest[Skeleton::LEFT_ELBOW].Set(vecs);
	m_KinectRest[Skeleton::LEFT_WRIST].Set(vecs);
	m_KinectRest[Skeleton::LEFT_HAND].Set(vecs);
	m_KinectRest[Skeleton::LEFT_FINGER].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	m_KinectRest[Skeleton::RIGHT_COLLAR].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_SHOULDER].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_ELBOW].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_WRIST].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_HAND].Set(vecs);
	m_KinectRest[Skeleton::RIGHT_FINGER].Set(vecs);
	m_KinectRest[Skeleton::LEFT_ANKLE].Set(0, 0, 0, 1);
	m_KinectRest[Skeleton::LEFT_FOOT].Set(0, 0, 0, 1);
	m_KinectRest[Skeleton::RIGHT_ANKLE].Set(0, 0, 0, 1);
	m_KinectRest[Skeleton::RIGHT_FOOT].Set(0, 0, 0, 1);
	for (int u = 0; u < MAX_USERS; ++u)
	{
		m_Users[u].UserID = 0;
		for (int i = 0; i < Skeleton::NUM_BONES; ++i)
			if (m_BindPoseRelative)
				m_Users[u].Rotations[i].Set(0, 0, 0, 1);
			else
				m_Users[u].Rotations[i] = m_KinectRest[i];
	}
}

/*!
 * @fn bool KinectTracker::Configure(const TCHAR* configfile)
 * @param configfile	Currently unused for OpenNI 2.0
 *
 * Configures the Kinect from an XML file.
 *
 * @return true if configuration successful, else false
 */
bool KinectTracker::Configure(const TCHAR* configfile)
{
	Core::String	config(configfile);

	if (m_Kinect == NULL)
		m_Kinect = new Kinect(this);
	else if (m_Kinect->Owner != this)
		return false;
	return m_Kinect->Configure(NULL);

}

/*!
 * @fn float KinectTracker::GetFOV()
 *
 * This function returns the field of view of the Kinect
 * camera (from the Kinect configuration file).
 *
 * @return field of view in radians.
 *
 * @see KinectTracker::GetViewVolume
 */
float KinectTracker::GetFOV()
{
	return m_Kinect->FOV;
}

/*!
 * @fn Vec3 KinectTracker::GetViewVolume(Box3& vvol)
 * @param vvol	box to get the view volume of the depth sensor
 *
 * This function returns the 3D bounding volume of the Kinect
 * camera (from the Kinect configuration file).
 *
 * @return true if view volume can be obtained, else false
 *
 * @see KinectTracker::GetFOV
 */
bool KinectTracker::GetViewVolume(Box3& vvol)
{
	if ((m_Kinect->Width > 0) && (m_Kinect->Height > 0) && (m_Kinect->Depth > 0) && (m_Kinect->FOV > 0.0f))
	{
		float w = m_Kinect->Width / 2.0f;
		float h = m_Kinect->Height / 2.0f;
		float yon = (float) m_Kinect->Depth;
		float hither = w / (2.0f * tanf(m_Kinect->FOV / 2.0f));

		hither = 1500.0f;		// Kinect cannot track closer than this
		vvol.Set(-w, -h, hither, w, h, yon);
		return true;
	}
	return false;
}

bool KinectTracker::OnStart()
{
	if (m_Kinect->IsStarted)
		return true;
	if (m_Kinect->Start())
	{
		OnInitDone();
		return BodyTracker::OnStart();
	}
	return false;
}

/*
 * Updates the color and depth map textures from the Kinect cameras.
 * If a Skeleton is our target or a child, we update the joint angles
 * for the user associated with it.
 */
bool KinectTracker::Eval(float t)
{
	if (m_Kinect == NULL)
		return false;
	if (m_Kinect->Owner == this)
	{
		m_Kinect->Update(m_Users);
		if (!m_DepthTexture.IsNull())
			m_Kinect->MakeDepthMap(m_DepthTexture);
		if (!m_ColorTexture.IsNull())
			m_Kinect->MakeUserMap(m_ColorTexture);
	}
	return BodyTracker::Eval(t);
}


/*!
 * @fn bool KinectTracker::ComputeBoneAngles(Skeleton* target)
 * @param target	body poser target to update
 *
 * Gathers joint rotations for the skeleton bones from the Kinect
 * and updates the pose of the target skeleton.
 */
bool KinectTracker::ComputeBoneAngles(int userid)
{
	Scene*		scene = GetMainScene();
	Camera*		cam = scene->GetCamera();
	Box3		vvol(cam->GetViewVol());
	Box3		kvv;
	Matrix		mtx;
	Quat		krot;
	Vec3		kpos;
	Vec3		campos = cam->GetTranslation();
	bool		rotchanged[Skeleton::NUM_BONES];
	bool		poschanged[Skeleton::NUM_BONES];
	bool		changed = false;
	Quat		(&rotations)[Skeleton::NUM_BONES] = m_Users[userid].Rotations;
	Vec3		(&positions)[Skeleton::NUM_BONES] = m_Users[userid].Positions;
	int			kinectID = m_Users[userid].UserID;

	/*
	 * Set rotations on the individual bones from the Kinect joint angles
	 */
	if (!GetViewVolume(kvv))
		return false;
	for (int i = 0; i < KinectTracker::NUM_BONES; ++i)
	{
		nite::JointType jointindex = (nite::JointType) i;
		int				boneindex = m_SensorBoneMap[i];

		rotchanged[boneindex] = false;
		poschanged[boneindex] = false;
		if (m_Kinect->GetJointRotation(kinectID, jointindex, krot))
		{
			VX_TRACE2(BodyTracker::Debug, ("KinectTracker: Joint %d rot(%.3f, %.3f, %.3f, %.3f)", boneindex, krot.x, krot.y, krot.z, krot.w));
			if (!m_BindPoseRelative)
				krot *= m_KinectRest[boneindex];
			rotations[boneindex] = krot;
			changed = true;
			rotchanged[boneindex] = true;
		}
		if (m_Kinect->GetJointPosition(kinectID, jointindex, kpos))
		{
			VX_TRACE2(BodyTracker::Debug, ("KinectTracker: Joint %d pos(%.3f, %.3f, %.3f)", boneindex, kpos.x, kpos.y, kpos.z));
			positions[boneindex] = kpos;
			poschanged[boneindex] = true;
		}
	}
	if (!changed)
		return false;
	for (int i = 0; i < Skeleton::NUM_BONES; ++i)
	{
		Vec3	wpos;

		if (!poschanged[i])
			continue;
		kpos = positions[i];
		wpos.x = kpos.x / kvv.Width();
		wpos.y = kpos.y / kvv.Height();
		wpos.z = (kpos.z - kvv.min.z) / kvv.Depth();
		switch (i)
		{
			case Skeleton::RIGHT_WRIST:
			case Skeleton::RIGHT_HAND:
			case Skeleton::LEFT_WRIST:
			case Skeleton::LEFT_HAND:
			if ((m_Options & TRACK_HANDS) == 0)
				continue;
			VX_TRACE2(BodyTracker::Debug, ("KinectTracker: HAND kinect(%f, %f, %f) world(%f, %f, %f)", kpos.x, kpos.y, kpos.z, wpos.x, wpos.y, wpos.z));
			if (m_UserID == userid)
				OnUserMove(userid, i, wpos, rotations[i]);
			break;

			case Skeleton::TORSO:
			if (wpos.z < 0.5f)
				continue;
			if (true)
			{
				wpos.z -= 0.5f;
				wpos.z *= vvol.Depth() / 4.0f;
				wpos.x *= vvol.Width() * wpos.z / vvol.min.z;
				wpos.y *= vvol.Height() * wpos.z / vvol.min.z;
				wpos.z = -wpos.z;
				wpos.y = m_WaistOffset.y;
			}
			else
			{
				wpos.z = (0.5f - wpos.z) * vvol.Depth() / 4.0f;
				wpos.x *= vvol.Width();
				wpos.y *= vvol.Height();
				wpos.y += m_WaistOffset.y;
			}
			m_UserPos = wpos;
			VX_TRACE2(BodyTracker::Debug, ("KinectTracker: TORSO kinect(%f, %f, %f) world(%f, %f, %f)", kpos.x, kpos.y, kpos.z, wpos.x, wpos.y, wpos.z));
			m_Skeleton.SetPosition(wpos);
			if ((m_Options & TRACK_TORSO) &&
				(m_UserID == userid))
				OnUserMove(userid, i, wpos, rotations[i]);
			break;
		}
	}
	ApplyCurrentPose(userid, rotchanged);
	return true;
}


/*!
 * @fn Vec2 KinectTracker::GetImageSize() const
 * Get the size of the Kinect camera frames
 * for depth and color capture (typically 640 x 480)
 *
 * @return width & height of Kinect frame
 */
Vec2	KinectTracker::GetImageSize() const
{
	float w = (float) m_Kinect->Width;
	float h = (float) m_Kinect->Height;

	if (w == 0)
		return BodyTracker::GetImageSize();
	return Vec2(w, h);
}


Kinect::Kinect(KinectTracker* owner)
:	Owner(owner),
	LastUser(0),
	Width(0),
	Height(0),
	Depth(3000),
	FOV(1.0f),
	UserTracker(),
	IsStarted(false)
{
}

Kinect::~Kinect()
{
	UserTracker.destroy();
	nite::NiTE::shutdown();
	openni::OpenNI::shutdown();
}

/*
 * Open Kinect configuration file.
 */
bool  Kinect::Configure(const TCHAR* file)
{
	openni::Status rc1 = openni::OpenNI::initialize();
	if (rc1 != openni::STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot initialize Kinect %s", openni::OpenNI::getExtendedError()), false);

	nite::Status rc = nite::NiTE::initialize();
	if (rc == nite::STATUS_OK)
		rc = UserTracker.create();
	if (rc != nite::STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot create user tracker %s", openni::OpenNI::getExtendedError()), false);
	IsStarted = true;
	return true;
}

/*
 * Start Kinect processing for user skeleton tracking.
 */
bool Kinect::Start()
{
	return IsStarted;
}

/*
 * Called every frame to update Kinect state.
 */
void Kinect::Update(BodyTracker::User vxusers[BodyTracker::MAX_USERS])
{
	nite::Status rc;

	if (!IsStarted)
		return;
	rc = UserTracker.readFrame(&UserFrame);
	if (rc != nite::STATUS_OK)
		return;
	const nite::Array<nite::UserData>& users = UserFrame.getUsers();
	for (int i = 0; i < users.getSize(); ++i)
	{
		const nite::UserData&	user = users[i];
		nite::UserId			id = user.getId();
		int						index = Owner->FindUserIndex(id);

		if (user.isNew())
		{
			if (index >= 0)
				return;
			for (int j = 0; j < BodyTracker::MAX_USERS; ++j)
				if (vxusers[j].UserID == 0)
				{
					vxusers[j].UserID = id;
					LastUser = id;
					UserTracker.startSkeletonTracking(id);
					NewUser(j);
					return;
				}
		}
		if (user.isLost())
		{
			if ((index >= 0) && vxusers[index].Tracking)
				LostUser(index);
			return;
		}
		switch (user.getSkeleton().getState())
		{
			case nite::SKELETON_NONE:
			if ((index >= 0) && vxusers[index].Tracking)
				ExitUser(index);
			return;

			case nite::SKELETON_TRACKED:
			if ((index >= 0) && vxusers[index].Tracking)
				EnterUser(index);
			return;

			case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
			case nite::SKELETON_CALIBRATION_ERROR_HANDS:
			case nite::SKELETON_CALIBRATION_ERROR_LEGS:
			case nite::SKELETON_CALIBRATION_ERROR_HEAD:
			case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			CalibrationBad(index);
			return;
		}
	}
}

/*
 * Callback for new user.
 */
void Kinect::NewUser(int userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: New User %d", userid));
	if (Owner)
		Owner->OnNewUser(userid);
}

/*
 * Callback for lost user.
 */
void Kinect::LostUser(int userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: Lost User %d", userid));

	if (Owner)
		Owner->OnStopJointTrack(userid);
}

/*
 * Callback for user exits scene.
 */
void Kinect::ExitUser(int userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: stopped tracking user %d EXIT", userid));

	if (Owner)
		Owner->OnPauseJointTrack(userid);
}


/*
 * Callback for user re-entering the scene.
 */
void  Kinect::EnterUser(int id)
{
	VX_TRACE(BodyTracker::Debug, ("start tracking user %d ENTER", id));
	if (Owner)
		Owner->OnStartJointTrack(id);
	LastUser = id;
}

/*
 * Callback for calibration failure.
 */
void  Kinect::CalibrationBad(int nId)
{
	// Calibration failed
	VX_TRACE(BodyTracker::Debug, ("Calibration failed for user %d", nId));
}

/*
 * Get the rotation of the given joint from the Kinect.
 */
bool Kinect::GetJointRotation(nite::UserId id, nite::JointType jointindex, Quat& rotation)
{
	const nite::UserData*	user = UserFrame.getUserById(id);
	nite::SkeletonJoint		joint;
	nite::Quaternion		nquat;
	nite::Skeleton			skel;

	if (user == NULL)
		return false;
	skel = user->getSkeleton();
	joint = skel.getJoint(jointindex);
	if (joint.getOrientationConfidence() < 0.5)
		return false;
	nquat = joint.getOrientation();
	rotation.Set(-nquat.x, -nquat.y, nquat.z, nquat.w); 
}

/*
 * Get the position of the given joint from the Kinect.
 */
bool Kinect::GetJointPosition(nite::UserId id, nite::JointType jointindex, Vec3& position)
{
	const nite::UserData*	user = UserFrame.getUserById(id);
	nite::SkeletonJoint		joint;
	nite::Point3f			pos;
	nite::Skeleton			skel;

	if (user == NULL)
		return false;
	skel = user->getSkeleton();
	joint = skel.getJoint(jointindex);
	if (joint.getPositionConfidence() < 0.5)
		return false;
	pos = joint.getPosition();
	position.Set(pos.x, pos.y, pos.z);
	return true;
}

/*
 * Grabs the Kinect depth map and copies it into the input texture.
 */
void Kinect::MakeDepthMap(Texture* image)
{
	openni::VideoFrameRef		depthframe;
	const openni::DepthPixel*	depthpixels;

	if (!IsStarted)
		return;
	depthframe = UserFrame.getDepthFrame();
	if (!depthframe.isValid())
		return;
	depthpixels = (const openni::DepthPixel*) depthframe.getData();
	Width = depthframe.getWidth();
	Height = depthframe.getHeight();
	UpdateImageFromDepth(image, depthpixels);
}

/*
 * Grabs the Kinect color image and copies it into the input texture.
 */
void Kinect::MakeUserMap(Texture* image)
{
	if (IsStarted)
	{
		const nite::UserMap&	usermap = UserFrame.getUserMap();
		const nite::UserId*		labels;
		openni::VideoFrameRef	depthframe;
		const openni::DepthPixel*	depthpixels;
		int		w, h;

		depthframe = UserFrame.getDepthFrame();
		if (!depthframe.isValid())
			return;
		depthpixels = (const openni::DepthPixel*) depthframe.getData();
		w = depthframe.getWidth();
		h = depthframe.getHeight();
		labels = usermap.getPixels();
		Width = usermap.getWidth();
		Height = usermap.getHeight();
		VX_ASSERT(w == Width);
		VX_ASSERT(h == Height);
		UpdateImageFromLabels(image, depthpixels, labels);
	}
}


/*!
 * @fn void Kinect::UpdateImageFromDepth(Texture* image, const XnDepthPixel* depthmap)
 * @param image		Texture image to update with depth pixels
 * @param depthmap	pixels from Kinect depth map
 *
 * Copies the Kinect depth map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromDepth(Texture* image, const openni::DepthPixel* depthmap)
{
	ObjectLock	locki(image);
	Bitmap*		bitmap = image->GetBitmap();

	if (bitmap == NULL)
		if (MediaBase::InitImage(image, Width, Height))
			bitmap = image->GetBitmap();
		else
			return;

	ObjectLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	int			texWidth = image->GetWidth();
	int			texHeight = image->GetHeight();
	uint32		nIndex = 0;

	VX_ASSERT(videobuffer);
	VX_ASSERT(Width > 0);
	VX_ASSERT(Height > 0);
	VX_ASSERT(texWidth >= Width);
	VX_ASSERT(texHeight >= Height);
	for (int y = 0; y < Height; y++)
	{
		const openni::DepthPixel* src = depthmap + y * Width;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 3;
		for (int x = 0; x < Width; x++, nIndex++)
		{
			int v = (*src++) >> 6;
			*dst++ = (char) v;
			*dst++ = (char) v;
			*dst++ = (char) v;
		}
	}
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
}

/*!
 * @fn void Kinect::UpdateImageFromLabels(Texture* image, const openni::DepthPixel* depthpixels, const nite::UserId* userpixels)
 * @param image			Texture image to update with color pixels
 * @param depthpixels	depth map pixels
 * @param userpixels	user mask pixels from Kinect camera
 *
 * Copies the Kinect color map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromLabels(Texture* image, const openni::DepthPixel* depthpixels, const nite::UserId* userpixels)
{
	ObjectLock	locki(image);
	Bitmap*		bitmap = image->GetBitmap();

	if (bitmap == NULL)
		if (MediaBase::InitImage(image, Width, Height, 32))
			bitmap = image->GetBitmap();
		else
			return;

	ObjectLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	int			texWidth = image->GetWidth();
	int			texHeight = image->GetHeight();
	uint32		nIndex = 0;

	VX_ASSERT(videobuffer);
	VX_ASSERT(Width > 0);
	VX_ASSERT(Height > 0);
	VX_ASSERT(texWidth >= Width);
	VX_ASSERT(texHeight >= Height);
	for (int y = 0; y < Height; y++)
	{
		const openni::DepthPixel* src = depthpixels + y * Width;
		const nite::UserId* mask = userpixels + y * Width;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 4;
		for (int x = 0; x < Width; x++)
		{
			uint16 v = *src++;
			dst[0] = v & 0xFFFF;
			dst[1] = v >> 8;
			dst[2] = 0;
			dst[3] = ((LastUser == 0) || *mask++) ? 0xff : 0;
			dst += 4;
		}
	}
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 32;
	bitmap->SetChanged(true);
}

}	// end Vixen