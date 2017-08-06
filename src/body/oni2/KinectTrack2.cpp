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

	INuiSensor 			Sensor;
	Event				SkeletonEvent;
	NUI_SKELETON_FRAME	SkeletonFrame;
	HANDLE				ColorStream;
	HANDLE				DepthStream;
	HANDLE				LabelStream;
	int					LastUser;
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
	void		Update();
	bool		GetJointRotation(nite::UserId userid, nite::JointType jointindex, Quat& rotationk);
	bool		GetJointPosition(nite::UserId nId, nite::JointType jointindex, Vec3& position);
	void		NewUser(nite::UserId userid);
	void		LostUser(nite::UserId userid);
	void		ExitUser(nite::UserId userid);
	void		EnterUser(nite::UserId userid);
	void		CalibrationGood(nite::UserId userid);
	void		CalibrationBad(nite::UserId userid);
	void		MakeDepthMap(Texture* image);
	void		MakeUserMap(Texture* image);
	void		UpdateImageFromDepth(Texture* image, const openni::DepthPixel* depthmap);
	void		UpdateImageFromLabels(Texture* image, const char* colorpixels, const nite::UserId* userpixels);
};

VX_IMPLEMENT_CLASS(Kinect, RefObj);
VX_IMPLEMENT_CLASS(KinectTracker, BodyTracker);

/*
 * Maps from Kinect bone ordering to Skeleton bone ordering
 */
int	KinectTracker::m_SensorBoneMap[KinectTracker::NUM_BONES] = {
		Skeleton::HEAD,
		Skeleton::NECK,
		Skeleton::TORSO,
		Skeleton::WAIST,
		Skeleton::RIGHT_COLLAR,
		Skeleton::RIGHT_SHOULDER,
		Skeleton::RIGHT_ELBOW,
		Skeleton::RIGHT_WRIST,
		Skeleton::RIGHT_HAND,
		Skeleton::RIGHT_FINGER,
		Skeleton::LEFT_COLLAR,
		Skeleton::LEFT_SHOULDER,
		Skeleton::LEFT_ELBOW,
		Skeleton::LEFT_WRIST,
		Skeleton::LEFT_HAND,
		Skeleton::LEFT_FINGER,
		Skeleton::RIGHT_HIP,
		Skeleton::RIGHT_KNEE,
		Skeleton::RIGHT_ANKLE,
		Skeleton::RIGHT_FOOT,
		Skeleton::LEFT_HIP,
		Skeleton::LEFT_KNEE,
		Skeleton::LEFT_ANKLE,
		Skeleton::LEFT_FOOT,
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

	m_UserID = 0;
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
	m_KinectRest[RIGHT_FOOT].Set(0, 0, 0, 1);
	for (int u = 0; u < MAX_USERS; ++u)
	{
		for (int i = 0; i < Skeleton::NUM_BONES; ++i)
			if (m_BindPoseRelative)
				m_Users[u].Rotations[i].Set(0, 0, 0, 1);
			else
				m_Users[u].Rotations[i] = m_KinectRest[i];
	}
}

/*!
 * @fn bool KinectTracker::Configure(const TCHAR* configfile)
 * @param configfile	Name of the Kinect configuration file. It defaults to
 *						"KinectConfig.xml" from the current directory if NULL.
 *
 * Configures the Kinect from an XML file.
 *
 * @return true if configuration successful, else false
 */
bool KinectTracker::Configure(const TCHAR* configfile)
{
	Core::String	config(configfile);

	if (config.IsEmpty())
		config = TEXT("KinectConfig.xml");
	if (m_Kinect == NULL)
		m_Kinect = new Kinect(this);
	else if (m_Kinect->Owner != this)
		return false;
	if (m_Kinect->Configure(config))
		return true;
	if (m_Kinect->Configure(TEXT("KinectConfig.xml")))
		return BodyTracker::Configure(config);
	return false;
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

		hither = 2000.0f;		// Kinect cannot track closer than this
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
		m_Kinect->Update();
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

	/*
	 * Set rotations on the individual bones from the Kinect joint angles
	 */
	GetViewVolume(kvv);
	for (int i = 0; i < KinectTracker::NUM_BONES; ++i)
	{
		nite::JointType jointindex = (nite::JointType) (i + 1);
		int				boneindex = m_SensorBoneMap[i];

		rotchanged[boneindex] = false;
		poschanged[boneindex] = false;
		if (m_Kinect->GetJointRotation(userid, jointindex, krot))
		{
			VX_TRACE(BodyTracker::Debug > 1, ("KinectTracker: Joint %d rot(%.3f, %.3f, %.3f, %.3f)", boneindex, krot.x, krot.y, krot.z, krot.w));
			if (!m_BindPoseRelative)
				krot *= m_KinectRest[boneindex];
			/*
			 * Sometimes OpenNI flips the legs backwards.
			 * This code filters that out.
			 */
			if (m_Options & TRACK_FRONT_ONLY)
				switch (i)
				{
					case KinectTracker::RIGHT_HIP:
					case KinectTracker::RIGHT_KNEE:
					case KinectTracker::LEFT_HIP:
					case KinectTracker::LEFT_KNEE:
					if (ZForward(krot) < 0.1f)		// Z axis flipped back?
						continue;					// filter this out - bad data
				}
			rotations[boneindex] = krot;
			changed = true;
			rotchanged[boneindex] = true;
		}
		if (m_Kinect->GetJointPosition(userid, jointindex, kpos))
		{
//			VX_TRACE(BodyTracker::Debug > 1, ("KinectTracker: Joint %d pos(%.3f, %.3f, %.3f)", boneindex, kpos.x, kpos.y, kpos.z));
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
			VX_TRACE(BodyTracker::Debug > 1, ("KinectTracker: HAND kinect(%f, %f, %f) world(%f, %f, %f)", kpos.x, kpos.y, kpos.z, wpos.x, wpos.y, wpos.z));
			if (m_UserID == userid)
				OnUserMove(userid, i, wpos, rotations[i]);
			break;

			case Skeleton::TORSO:
			if (wpos.z < 0.0f)
				continue;
			wpos.z *= vvol.Depth();
			wpos.x *= vvol.Width() * wpos.z / vvol.min.z;
			wpos.y *= vvol.Height() * wpos.z / vvol.min.z;
			wpos.z = -wpos.z / 2.0f;
			wpos.y = m_WaistOffset.y;
			m_UserPos = wpos;
			VX_TRACE(BodyTracker::Debug > 1, ("KinectTracker: TORSO kinect(%f, %f, %f) world(%f, %f, %f)", kpos.x, kpos.y, kpos.z, wpos.x, wpos.y, wpos.z));
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
	Depth(0),
	FOV(0.0f),
	Sensor(NULL),
	IsStarted(false)
{
}

Kinect::~Kinect()
{
	NuiShutdown();
}

/*
 * Open Kinect configuration file.
 */
bool  Kinect::Configure(const TCHAR* file)
{
	HRESULT rc;
	
	if (IsStarted)
		return true;
	rc = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH);
	if (rc != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot initialize Kinect"), false);
	rc = NuiCreateSensorByIndex(0, &Sensor);
	if (rc != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot create sensor"), false);
	return true;
}

/*
 * Start Kinect processing for user skeleton tracking.
 */
bool Kinect::Start()
{
    HRESULT hr;
	
	if (Sensor != NULL)		// already started
		return true;
	SkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hr = Sensor->NuiSkeletonTrackingEnable(SkeletonEvent, 0);
	if (hr != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot enable skeleton tracking"), false);
	rc = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, NULL, ColorStream);
	if (hr != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot open color stream"), false);
	rc = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_320x240, 0, 2, NULL, DepthStream);
	if (hr != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot open depth stream"), false);
	rc = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, 0, 2, NULL, LabelStream);
	if (hr != S_OK)
		VX_ERROR(("KinectTracker: ERROR cannot open player index stream"), false);
	IsStarted = true;
	return true;
}

/*
 * Called every frame to update Kinect state.
 */
void Kinect::Update()
{
	if (!IsStarted)
		return;
	// Wait for 0ms, just quickly test if it is time to process a skeleton
	if (!WAIT_OBJECT_0 == WaitForSingleObject(SkeletonEvent, 0))
		return;
	// Get the skeleton frame that is ready
	if (Sensor->NuiSkeletonGetNextFrame(0, &SkeletonFrame) != S_OK)
		return;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        const NUI_SKELETON_DATA& skeleton = SkeletonFrame->SkeletonData[i];
        switch (skeleton.eTrackingState)
        {
			case NUI_SKELETON_TRACKED:
          
            break;
 
			case NUI_SKELETON_POSITION_ONLY:
            
            break;
        }
    }
}

/*
 * Callback for new user.
 */
void Kinect::NewUser(nite::UserId userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: New User %d", userid));
	UserTracker->startSkeletonTracking(userid);
	LastUser = userid;
	if (Owner)
	{
		Owner->OnNewUser(userid);
		Owner->OnStartJointTrack(userid);
	}
}

/*
 * Callback for lost user.
 */
void Kinect::LostUser(nite::UserId userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: Lost User %d", userid));

	if (Owner)
		Owner->OnStopJointTrack(userid);
}

/*
 * Callback for user exits scene.
 */
void Kinect::ExitUser(nite::UserId userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: stopped tracking user %d EXIT", userid));

	if (Owner)
		Owner->OnPauseJointTrack(userid);
}


/*
 * Callback for calibration success.
 */
void  Kinect::CalibrationGood(nite::UserId nId)
{
	// Calibration succeeded
	VX_TRACE(BodyTracker::Debug, ("Calibration complete, start tracking user %d", nId));
	LastUser = nId;
	if (Owner)
		Owner->OnStartJointTrack(nId);
}

/*
 * Callback for user re-entering the scene.
 */
void  Kinect::EnterUser(nite::UserId nId)
{
	// Calibration succeeded
	VX_TRACE(BodyTracker::Debug, ("start tracking user %d ENTER", nId));
	if (Owner)
		Owner->OnStartJointTrack(nId);
	LastUser = nId;
}

/*
 * Callback for calibration failure.
 */
void  Kinect::CalibrationBad(nite::UserId nId)
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
	if (!user->isVisible())
		return false;
	skel = user->getSkeleton();
	joint = skel.getJoint(jointindex);
	if (joint.getOrientationConfidence() < 0.5)
		return false;
	nquat = joint.getOrientation();
	rotation.Set(nquat.x, nquat.y, nquat.z, nquat.w);
	return true;
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
	if (!user->isVisible())
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
	NUI_IMAGE_FRAME*	depthframe;
	HRESULT				hr;
	
	if (!IsStarted || (DepthStream == NULL))
		return;
	hr = NuiImageStreamGetNextFrame(DepthStream, 0, &depthframe);
	if (hr != S_OK)
		return;
	switch (depthframe.eResolution)
	{
		case	NUI_IMAGE_RESOLUTION_80x60:		Width = 80; Height = 60; break;
		case	NUI_IMAGE_RESOLUTION_320x240:	Width = 320; Height = 240; break;
		case	NUI_IMAGE_RESOLUTION_640x480:	Width = 640; Height = 480; break;
		case	NUI_IMAGE_RESOLUTION_1280x960:	Width = 1280; Height = 960; break;
		default:								return;
	}
	UpdateImageFromDepth(image, depthframe.pFrameTexture);
}

/*
 * Grabs the Kinect color image and copies it into the input texture.
 */
void Kinect::MakeUserMap(Texture* image)
{
	NUI_IMAGE_FRAME*	colorframe;
	NUI_IMAGE_FRAME*	labelframe;
	HRESULT				hr;
	
	if (!IsStarted || (ColorStream == NULL) || (LabelStream == NULL))
		return;
	hr = NuiImageStreamGetNextFrame(ColorStream, 0, &colorframe);
	if (hr != S_OK)
		return;
	hr = NuiImageStreamGetNextFrame(LabelStream, 0, &labelframe);
	if (hr != S_OK)
		return;
	switch (colorframe.eResolution)
	{
		case	NUI_IMAGE_RESOLUTION_80x60:		Width = 80; Height = 60; break;
		case	NUI_IMAGE_RESOLUTION_320x240:	Width = 320; Height = 240; break;
		case	NUI_IMAGE_RESOLUTION_640x480:	Width = 640; Height = 480; break;
		case	NUI_IMAGE_RESOLUTION_1280x960:	Width = 1280; Height = 960; break;
		default:								return;
	}
	UpdateImageFromLabels(image, colorframe.pFrameTexture, labelframe.pFrameTexture);
}


/*!
 * @fn void Kinect::UpdateImageFromDepth(Texture* image, const XnDepthPixel* depthmap)
 * @param image		Texture image to update with depth pixels
 * @param depthtex	Kinect depth map
 *
 * Copies the Kinect depth map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromDepth(Texture* image, INuiFrameTexture* depthtex)
{
	ObjectLock	locki(image);
	Bitmap*		bitmap = image->GetBitmap();
	NUI_LOCKED_RECT	lockrect;
	
	if (bitmap == NULL)
		if (MediaBase::InitImage(image, Width, Height))
			bitmap = image->GetBitmap();
		else
			return;
	depthtex->LockRect(0, &lockrect, NULL, 0);
 	ObjectLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	int			texWidth = image->GetWidth();
	int			texHeight = image->GetHeight();
	uint32		nIndex = 0;
	const char*	depthmap = lockrect.pBits;

	VX_ASSERT(videobuffer);
	VX_ASSERT(Width > 0);
	VX_ASSERT(Height > 0);
	VX_ASSERT(texWidth >= Width);
	VX_ASSERT(texHeight >= Height);
	for (int y = 0; y < Height; y++)
	{
		const char* src = depthmap + y * lockrect.Pitch;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 3;
		for (int x = 0; x < Width; x++, nIndex++)
		{
			int v = *src++;
			*dst++ = (char) v;
			*dst++ = (char) v;
			*dst++ = (char) v;
		}
	}
	depthtex->UnlockRect(0);
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
}

/*!
 * @fn void Kinect::UpdateImageFromLabels(Texture* image, INuiFrameTexture* colormap, INuiFrameTexture* usermap)
 * @param image			Texture image to update with color pixels
 * @param colormap		color map from Kinect camera
 * @param usermap		user map from Kinect camera
 *
 * Copies the Kinect color map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromLabels(Texture* image, INuiFrameTexture* colormap, INuiFrameTexture* usermap)
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
		const char* src = depthpixels + y * Width * 2;
		const nite::UserId* mask = userpixels + y * Width;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 4;
		for (int x = 0; x < Width; x++, nIndex++)
		{
			dst[2] = 0;
			dst[1] = *src++;
			dst[0] = *src++;
			dst[3] = ((LastUser == 0) || *mask++) ? 0xff : 0;
			dst += 4;
		}
	}
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 32;
	bitmap->SetChanged(true);
}

}	// end Vixen