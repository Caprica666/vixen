#include "vixen.h"
#include "media/vxmediabase.h"
#include "body/vxbodytrack.h"
#include "body/vxkinecttrack.h"
#include "XnOpenNI.h"
#include "XnCodecIDs.h"
#include "XnCppWrapper.h"

#if ((XN_MAJOR_VERSION == 1) && (XN_MINOR_VERSION <= 3))
#define	USE_XN_OLD_API
#endif

//#define USE_XN_OLD_API

namespace Vixen {

/*
 * Kinect helper class to isolate all the Kinect API calls.
 */
class Kinect : public Core::RefObj
{
public:
	VX_DECLARE_CLASS(Kinect);

	xn::Context			Context;
	xn::DepthGenerator	DepthGenerator;
	xn::UserGenerator	UserGenerator;
	xn::ImageGenerator	ImageGenerator;
	xn::HandsGenerator	HandsGenerator;
	xn::ScriptNode		ScriptNode;
	Core::String		PoseName;
	XnUserID			LastUser;
	int					Width;
	int					Height;
	int					Depth;
	float				FOV;
	bool				NeedPose;
	bool				IsStarted;
	KinectTracker*		Owner;

	Kinect(KinectTracker* owner = NULL);
	~Kinect();
	bool		OpenRecording(const TCHAR* file);
	bool		Configure(const TCHAR* file);
	bool		Start();
	void		Update();
	bool		GetJointRotation(XnUserID userid, XnSkeletonJoint jointindex, Quat& rotationk);
	bool		GetJointPosition(XnUserID nId, XnSkeletonJoint jointindex, Vec3& position);
	void		NewUser(XnUserID userid);
	void		LostUser(XnUserID userid);
	void		PoseDetected(xn::PoseDetectionCapability& capability, XnUserID userid, const XnChar* strPose);
	void		ExitUser(XnUserID userid);
	void		EnterUser(XnUserID userid);
	void		CalibrationGood(xn::SkeletonCapability& capability, XnUserID userid);
	void		CalibrationBad(xn::SkeletonCapability& capability, XnUserID userid);
	void		HandCreate(XnUserID user, const XnPoint3D *position, XnFloat time);
	void		HandUpdate(XnUserID user, const XnPoint3D *position, XnFloat time);
	void		HandDestroy(XnUserID user, XnFloat time);
	void		MakeDepthMap(Texture* image);
	void		MakeColorMap(Texture* image);
	void		MakeDepthHistogram(const xn::DepthMetaData& dmd, float* histogram, int maxdepth);
	void		UpdateImageFromDepth(Texture* image, const XnDepthPixel* depthmap);
	void		UpdateImageFromHistogram(Texture* image, float* DepthHist, const xn::DepthMetaData& dmd, const XnLabel* Labels);
	void		UpdateImageFromColor(Texture* image, const char* colorpixels, const XnLabel* userpixels);
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
			m_Kinect->MakeColorMap(m_ColorTexture);
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
		XnSkeletonJoint jointindex = (XnSkeletonJoint) (i + 1);
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



// Callback: New user was detected
void XN_CALLBACK_TYPE Kinect_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	kinect->NewUser(nId);
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE Kinect_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	kinect->LostUser(nId);
}


// Callback: An existing user exited
void XN_CALLBACK_TYPE Kinect_ExitUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	kinect->ExitUser(nId);
}

// Callback: An existing user re-entered
void XN_CALLBACK_TYPE Kinect_EnterUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	kinect->EnterUser(nId);
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE Kinect_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	kinect->PoseDetected(capability, nId, strPose);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE Kinect_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: Calibration started for user %d", nId));
}

// Callback: Finished calibration
#ifdef USE_XN_OLD_API
void XN_CALLBACK_TYPE Kinect_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	if (bSuccess)
		kinect->CalibrationGood(capability, nId);
	else
		kinect->CalibrationBad(capability, nId);
}
#else
void XN_CALLBACK_TYPE Kinect_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus bSuccess, void* pCookie)
{
	Kinect* kinect = (Kinect*) pCookie;

	VX_ASSERT(kinect);
	if (bSuccess == XN_CALIBRATION_STATUS_OK)
		kinect->CalibrationGood(capability, nId);
	else
		kinect->CalibrationBad(capability, nId);
}
#endif

Kinect::Kinect(KinectTracker* owner)
:	Owner(owner),
	NeedPose(false),
	LastUser(0),
	Width(0),
	Height(0),
	Depth(0),
	FOV(0.0f),
	IsStarted(false)
{
}

Kinect::~Kinect()
{
	Context.Release();
}

/*
 * Open Kinect recording file.
 */
bool Kinect::OpenRecording(const TCHAR* file)
{
	xn::ProductionNode player;

	int rc = Context.Init();
	if (rc != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot initialize OpenNI context %s", xnGetStatusString(rc)), false);
	rc = Context.OpenFileRecording(file, player);
	if (rc != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot open %s %s", file, xnGetStatusString(rc)), false);
	return true;
}

/*
 * Open Kinect configuration file.
 */
bool  Kinect::Configure(const TCHAR* file)
{
#ifdef USE_XN_OLD_API
	int rc = Context.InitFromXmlFile(file);
#else
	int rc = Context.InitFromXmlFile(file, ScriptNode, NULL);
#endif
	if (rc != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot open XML file %s %s", file, xnGetStatusString(rc)), false);
	return true;
}

/*
 * Start Kinect processing for user skeleton tracking.
 */
bool Kinect::Start()
{
	int		rc;
	XnChar	strPose[20];
	XnFieldOfView fov;
	XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;
	
	rc = Context.FindExistingNode(XN_NODE_TYPE_DEPTH, DepthGenerator);
	if (rc != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot find depth generator %s", xnGetStatusString(rc)), false);
	if (DepthGenerator.GetFieldOfView(fov) != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot get field of view from depth generator %s", xnGetStatusString(rc)), false);
	FOV = (float) fov.fVFOV;
	Depth = DepthGenerator.GetDeviceMaxDepth();
	rc = Context.FindExistingNode(XN_NODE_TYPE_IMAGE, ImageGenerator);
	if (rc == XN_STATUS_OK)
	{
		xn::ImageMetaData imd;
		ImageGenerator.GetMetaData(imd);
		Width = imd.XRes();
		Height = imd.YRes();
	}
	else
		VX_ERROR(("KinectTracker: ERROR cannot find image generator %s", xnGetStatusString(rc)), false);
	xn::AlternativeViewPointCapability altview = DepthGenerator.GetAlternativeViewPointCap();
	if (altview.SetViewPoint(ImageGenerator) != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot set color image to depth viewpoint %s", xnGetStatusString(rc)), false);
	rc = Context.FindExistingNode(XN_NODE_TYPE_USER, UserGenerator);
	if (rc != XN_STATUS_OK)
	{
		rc = UserGenerator.Create(Context);
		if (rc != XN_STATUS_OK)
			VX_ERROR(("KinectTracker: ERROR cannot find user generator %s", xnGetStatusString(rc)), false);
	}
	if (!UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
		VX_ERROR(("KinectTracker: ERROR Supplied user generator doesn't support skeleton"), false);
	UserGenerator.RegisterUserCallbacks(Kinect_NewUser, Kinect_LostUser, this, hUserCallbacks);
#ifdef USE_XN_OLD_API
	UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(Kinect_CalibrationStart, Kinect_CalibrationEnd, this, hCalibrationCallbacks);
#else
	UserGenerator.RegisterToUserExit(Kinect_ExitUser, this, hCalibrationCallbacks);
	UserGenerator.RegisterToUserReEnter(Kinect_EnterUser, this, hCalibrationCallbacks);
	UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(Kinect_CalibrationStart, this, hCalibrationCallbacks);
	UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(Kinect_CalibrationEnd, this, hCalibrationCallbacks);
#endif
	if (UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		NeedPose = TRUE;
		if (!UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
			VX_ERROR(("KinectTracker: ERROR Pose detection required, but not supported"), false);
		UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(Kinect_PoseDetected, this, hPoseCallbacks);
		UserGenerator.GetPoseDetectionCap().RegisterToOutOfPose(Kinect_PoseDetected, this, hPoseCallbacks);
		UserGenerator.GetSkeletonCap().GetCalibrationPose(strPose);
		UserGenerator.GetSkeletonCap().SetSmoothing(0.5f);
		PoseName = strPose;
	}
	UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
	if (Context.StartGeneratingAll() != XN_STATUS_OK)
		VX_ERROR(("KinectTracker: ERROR cannot start tracking %s", xnGetStatusString(rc)), false);
	IsStarted = true;
	return true;
}

/*
 * Called every frame to update Kinect state.
 */
void Kinect::Update()
{
	if (IsStarted)
		Context.WaitAndUpdateAll();
}

/*
 * Callback for new user.
 */
void Kinect::NewUser(XnUserID userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: New User %d", userid));
	if (NeedPose)
		UserGenerator.GetPoseDetectionCap().StartPoseDetection(PoseName, userid);
	else
		UserGenerator.GetSkeletonCap().RequestCalibration(userid, TRUE);
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
void Kinect::LostUser(XnUserID userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: Lost User %d", userid));

	if (Owner)
		Owner->OnStopJointTrack(userid);
}

/*
 * Callback for user exits scene.
 */
void Kinect::ExitUser(XnUserID userid)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: stopped tracking user %d EXIT", userid));

	if (Owner)
		Owner->OnPauseJointTrack(userid);
}

/*
 * Callback for pose detection.
 */
void Kinect::PoseDetected(xn::PoseDetectionCapability& capability, XnUserID nId, const XnChar* strPose)
{
	VX_TRACE(BodyTracker::Debug, ("KinectTracker: Pose %s detected for user %d\n", strPose, nId));

	PoseName = strPose;
	UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

/*
 * Callback for calibration success.
 */
void  Kinect::CalibrationGood(xn::SkeletonCapability& capability, XnUserID nId)
{
	// Calibration succeeded
	VX_TRACE(BodyTracker::Debug, ("Calibration complete, start tracking user %d", nId));
	UserGenerator.GetSkeletonCap().StartTracking(nId);
	LastUser = nId;
	if (Owner)
		Owner->OnStartJointTrack(nId);
}

/*
 * Callback for user re-entering the scene.
 */
void  Kinect::EnterUser(XnUserID nId)
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
void  Kinect::CalibrationBad(xn::SkeletonCapability& capability, XnUserID nId)
{
	// Calibration failed
	VX_TRACE(BodyTracker::Debug, ("Calibration failed for user %d", nId));
	if (NeedPose)
		UserGenerator.GetPoseDetectionCap().StartPoseDetection(PoseName, nId);
	else
		UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

/*
 * Get the rotation of the given joint from the Kinect.
 */
bool Kinect::GetJointRotation(XnUserID nId, XnSkeletonJoint jointindex, Quat& rotation)
{
	XnSkeletonJointOrientation joint;
	Vec3	vecs[3];

	if (nId <= 0)
		return false;
	if (!UserGenerator.GetSkeletonCap().IsTracking(nId))
		return false;
	UserGenerator.GetSkeletonCap().GetSkeletonJointOrientation(nId, jointindex, joint);
	if (joint.fConfidence < 0.5)
		return false;
	vecs[0].Set(joint.orientation.elements[0], joint.orientation.elements[1], joint.orientation.elements[2]);
	vecs[1].Set(joint.orientation.elements[3], joint.orientation.elements[4], joint.orientation.elements[5]);
	vecs[2].Set(joint.orientation.elements[6], joint.orientation.elements[7], joint.orientation.elements[8]);
	vecs[0].Normalize();
	vecs[1].Normalize();
	vecs[2].Normalize();
	Quat q(vecs);
	q.z = -q.z;
	rotation = q;
	return true;
}

/*
 * Get the position of the given joint from the Kinect.
 */
bool Kinect::GetJointPosition(XnUserID nId, XnSkeletonJoint jointindex, Vec3& position)
{
	XnSkeletonJointPosition jpos;
	Vec3	vecs[3];
	XnPoint3D pt[1];

	if (nId <= 0)
		return false;
	if (!UserGenerator.GetSkeletonCap().IsTracking(nId))
		return false;
	UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(nId, jointindex, jpos);
	if (jpos.fConfidence < 0.5)
		return false;
	pt[0] = jpos.position;
	position.Set(pt[0].X, pt[0].Y, pt[0].Z);
	return true;
}

#define MAX_DEPTH 10000

Color Colors[11] =
{
	Color(0, 255, 255),
	Color(0, 0, 255),
	Color(0, 255, 0),
	Color(255, 255, 0),
	Color(255, 0, 0),
	Color(255, 128, 0),
	Color(128, 255, 0),
	Color(0, 128, 255),
	Color(128, 0, 255),
	Color(255, 255, 128),
	Color(255, 255, 255)
};

/*
 * Grabs the Kinect depth map and copies it into the input texture.
 */
void Kinect::MakeDepthMap(Texture* image)
{
//	float	DepthHist[MAX_DEPTH];
	xn::DepthMetaData	dmd;
	xn::SceneMetaData	smd;
	const XnDepthPixel* pDepth;

	if (!IsStarted)
		return;
	DepthGenerator.GetMetaData(dmd);
//	UserGenerator.GetUserPixels(0, smd);
	pDepth = dmd.Data();
	Width = dmd.XRes();
	Height = dmd.YRes();

	UpdateImageFromDepth(image, dmd.Data());
}

/*
 * Grabs the Kinect color image and copies it into the input texture.
 */
void Kinect::MakeColorMap(Texture* image)
{
	xn::ImageMetaData	imd;
	xn::SceneMetaData	umd;
	const XnRGB24Pixel* data;

	if (!IsStarted)
		return;
	ImageGenerator.GetMetaData(imd);
	UserGenerator.GetUserPixels(0, umd);
	data = ImageGenerator.GetRGB24ImageMap();
	Width = imd.XRes();
	Height = imd.YRes();

	UpdateImageFromColor(image, (const char*) data, umd.Data());
}

void Kinect::MakeDepthHistogram(const xn::DepthMetaData& dmd, float* DepthHist, int maxdepth)
{
	uint32	nIndex = 0;
	uint32	nValue = 0;
	uint32	nHistValue = 0;
	uint32	nNumberOfPoints = 0;
	int		width = dmd.XRes();
	int		height = dmd.YRes();
	const XnDepthPixel* pDepth = dmd.Data();

	/*
	 * Calculate the accumulative histogram
	 */
	memset(DepthHist, 0, maxdepth * sizeof(float));
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			nValue = *pDepth;
			if (nValue != 0)
			{
				DepthHist[nValue]++;
				nNumberOfPoints++;
			}
			pDepth++;
		}
	}
	for (nIndex = 1; nIndex < MAX_DEPTH; nIndex++)
		DepthHist[nIndex] += DepthHist[nIndex - 1];
	if (nNumberOfPoints)
	{
		for (nIndex = 1; nIndex < MAX_DEPTH; nIndex++)
			DepthHist[nIndex] = (256.0f * (1.0f - (DepthHist[nIndex] / nNumberOfPoints)));
	}
}

void Kinect::UpdateImageFromHistogram(Texture* image, float* DepthHist, const xn::DepthMetaData& dmd, const XnLabel* labels)
{
	const XnDepthPixel*	pDepth = dmd.Data();
	int					nColors = sizeof(Colors) / sizeof(Color);
	ObjectLock			locki(image);
	Bitmap*				bitmap = image->GetBitmap();

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
		const XnDepthPixel* src = pDepth + y * Width * 3;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 3;
		for (int x = 0; x < Width; x++, nIndex++)
		{
			dst[0] = 0;
			dst[1] = 0;
			dst[2] = 0;
			if (*labels != 0)
			{
				uint32 nValue = *pDepth;
				XnLabel label = *labels;
				int nColorID = label % nColors;
				if (label == 0)
					nColorID = nColors;
				if (nValue != 0)
				{
					float nHistValue = DepthHist[nValue];
					dst[0] = (char) nHistValue * Colors[nColorID].r; 
					dst[1] = (char) nHistValue * Colors[nColorID].g;
					dst[2] = (char) nHistValue * Colors[nColorID].b;
				}
			}
			pDepth++;
			labels++;
			dst += 3;
		}
	}
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
}

/*!
 * @fn void Kinect::UpdateImageFromDepth(Texture* image, const XnDepthPixel* depthmap)
 * @param image		Texture image to update with depth pixels
 * @param depthmap	pixels from Kinect depth map
 *
 * Copies the Kinect depth map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromDepth(Texture* image, const XnDepthPixel* depthmap)
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
		const XnDepthPixel* src = depthmap + y * Width;
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
 * @fn void Kinect::UpdateImageFromColor(Texture* image, const char* imagepixels, const XnLabel* userpixels)
 * @param image			Texture image to update with color pixels
 * @param imagepixels	color pixels from Kinect camera
 * @param userpixels	user mask pixels from Kinect camera
 *
 * Copies the Kinect color map into the given texture as 24 bit RGB color data.
 */
void Kinect::UpdateImageFromColor(Texture* image, const char* imagepixels, const XnLabel* userpixels)
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
		const char* src = imagepixels + y * Width * 3;
		const XnLabel* mask = userpixels + y * Width;
		char *dst =	videobuffer + (Height - y - 1) * texWidth * 4;
		for (int x = 0; x < Width; x++, nIndex++)
		{
			dst[2] = *src++;
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