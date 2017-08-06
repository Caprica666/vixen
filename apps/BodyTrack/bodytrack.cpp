#include "vxmfc.h"
#include "body/vxbodytrack.h"
#include <fstream>

//#define	USE_PHYSICS 1
#define USE_KINECT

#ifdef USE_KINECT
#include "body/vxkinecttrack.h"
#endif
#ifdef USE_PHYSICS
#include "physics/havok/vxphysics.h"
#endif

using namespace Vixen;


/*
 * @class BodyTrackDemo
 * Body tracking application with a pose-able avatar movable with
 * the Kinect controller. The default behavior with no arguments
 * is to bring up the generic Skeleton engine with the test body.
 * Limbs of the test body are pickable by holding down the SHIFT key.
 * Limbs may also be selected via keyboard commands.
 *
 * The application has several other options:
 *	-kinect		use the Microsoft Kinect controller to move the body
 *	-scape		use the SCAPE generative body model instead of the test body
 *	if a filename is provided after the options, the application will load
 *	that model instead of the test body. This option will be useful when
 *	Maya export of skinned characters is a reality...
 */
class BodyTrackDemo : public MFC::Demo
{
public:
	BodyTrackDemo();
	BodyTrackDemo(const BodyTrackDemo& src);
	~ BodyTrackDemo();

protected:
	// Make the body tracker engine to control the avatar.
	Skeleton*			MakeTracker(Engine* parent = NULL);

	// Attach the body tracker to the avatar.
	Skeleton*			AttachTracker(Model* root, Camera* cam);

	// Make SCAPE skin generative body engine.
	void				SetSkin(Skin* skin);

	// Print keyboard commands.
	void				Usage();

	// Make shape for Kinect camera output.
	Shape*				MakeCameraOutputShape(Camera* cam, float bodydist = 0.0f);

	// Select the bone attached to the given shape.
	void				Select(Shape* shape);

	// Update angle of selected joing
	void				SetJointAngle(float angle,  bool replace = false);

	// Update size of selected body part
	void				UpdateShape(int shapeindex, float inc);

	// Apply a pose to skeleton
	void				ApplyPose(Pose* pose);

	// Make poses for testing
	void				MakeTestPoses();
	void				MakeRelativeTestPoses();

	//! Connect skinned avatar
	Model*				ConnectAvatar(Scene* scene, Skeleton* skel);

	virtual Scene*		MakeScene(const TCHAR* filename = NULL);
	virtual	void		SetScene(Scene* scene, Window win = NULL);
	virtual	Engine*		MakeEngines();
	virtual	bool		ParseArgs(int argc, TCHAR** argv);
	virtual	bool		OnEvent(Event* e);
	virtual void		OnKey(int keycode);
	virtual bool		OnInit();
	virtual void		OnExit();
	virtual void		OnThreadExit(int threadtype);

	bool				UseKinect;			// true to use Kinect body tracker
	bool				UsePhysics;			// true to use rigid body physics instead of tracking
	int					m_BoneID;			// ID of selected bone (0 - NUM_BONES)
	int					m_Axis;				// 'X', 'Y' or 'Z' axis
	int					m_ShapeIndex;		// index of body shape parameter to change
	Ref<RayPicker>		m_Picker;			// picking engine
	Ref<BodyTracker>	m_Tracker;			// body tracker
	Ref<Skeleton>		m_Poser;			// body tracking engine
	Ref<Shape>			m_SelectedShape;	// shape selected by the user
	Ref<Appearance>		m_SelectAppear;		// appearance to use for selected shapes
	Ref<Appearance>		m_ShapeAppear;		// saved appearance of selected shape
	Ref<Transformer>	m_SelectedBone;		// -> engine that animates selected bone
	Ref<Shape>			m_CameraOutput;		// Kinect camera output
	Ref<Skin>			m_Skin;				// skinned body
	Ref<Pose>			m_JumpPose;
	Ref<Pose>			m_TrackerPose;
	Ref<Pose>			m_EgyptPose;
	Ref<Pose>			m_HelloPose;
	Ref<Pose>			m_RunPose;
	Ref<PoseMapper>		m_PoseMapper;		// map from body tracker to arbitrary skeleton
	Core::String		m_SaveFile;			// name of file to save scene in
	Core::String		m_SkelName;			// name of skeleton to use from scene
	Core::String		m_BodyName;			// name of body model to use from scene
};

BodyTrackDemo::BodyTrackDemo() : MFC::Demo()
{
	WinRect.min.x = 100.0f;
	WinRect.min.y = 100.0f;
	WinRect.max.x = 580.0f;
	WinRect.max.y = 740.0f;

	SetOptions(true);		// create the default directional light
	UseKinect = false;
	UsePhysics = false;
	m_SelectAppear = new Appearance(Col4(1.0f, 0.0f, 0.0f));
	m_SelectAppear->Set(Appearance::SHADING, Appearance::WIRE);
	Debug = 0;
	m_BoneID = Skeleton::RIGHT_KNEE;
}


BodyTrackDemo::~BodyTrackDemo()
{
	m_Poser = NULL;
}

/*
 * Checks for the following command line arguments:
 *	-kinect		use the Microsoft Kinect controller to move the body
 *	-scape		use the SCAPE generative body model instead of the test body
 */
bool BodyTrackDemo::ParseArgs(int argc, TCHAR** argv)
{
	for (int i = 0; i < argc; ++i)
	{
		TCHAR* p = argv[i];

		if (*p != TEXT('-'))
			continue;
		if (STRCMP(p, "-kinect") == 0)
			UseKinect = true;
		else if (STRCMP(p, "-physics") == 0)
		{
			UsePhysics = true;
		}
		else if (STRCMP(p, "-save") == 0)
		{
			if ((i + 1) < argc)
			{
				m_SaveFile = argv[++i];
				argv[i] = 0;
			}
		}
		else if (STRCMP(p, "-skeleton") == 0)
		{
			if ((i + 1) < argc)
			{
				m_SkelName = argv[++i];
				argv[i] = 0;
			}
		}
		else if (STRCMP(p, "-body") == 0)
		{
			if ((i + 1) < argc)
			{
				m_BodyName = argv[++i];
				argv[i] = 0;
			}
		}
	}
	return MFC::Demo::ParseArgs(argc, argv);
}

bool BodyTrackDemo::OnInit()
{
	Debug = World::Debug;	// this is set by ParseArgs when -debug is encountered
	BodyTracker::Debug = Debug;
	Skeleton::Debug = Debug;
	Pose::Debug = Debug;
#ifdef USE_PHYSICS
	Physics::Debug = Debug;
#endif
	if (MFC::Demo::OnInit())
	{
		FileLoader* loader = GetLoader();
		
		loader->SetFileFunc(TEXT("bvh"), &BVHLoader::ReadAnim, Event::LOAD_SCENE);
#ifdef USE_PHYSICS
		Physics::Startup();
#endif
		return true;
	}
	return false;
}

void BodyTrackDemo::OnThreadExit(int threadtype)
{
#ifdef USE_PHYSICS
	if (Scene::GetTLS()->ThreadType & SCENE_SimThread)
		Physics::Shutdown(SCENE_SimThread);
#endif
	MFC::Demo::OnThreadExit(threadtype);
}

void BodyTrackDemo::OnExit()
{
#ifdef USE_PHYSICS
	Physics::Shutdown(SCENE_UIThread);
#endif
	m_Poser = NULL;
	m_Tracker = NULL;
	MFC::Demo::OnExit();
}


Model* BodyTrackDemo::ConnectAvatar(Scene* scene, Skeleton* skeleton)
{
	Engine::Iter	iter(skeleton, Group::DEPTH_FIRST);
	Skin*			skin;
	Model*			body = (Model*) skeleton->GetTarget();
	Camera*			cam = scene->GetCamera();

	VX_TRACE(Debug, ("Found skeleton engine %s", skeleton->GetName()));
	MakeTracker(skeleton);
	while (skin = (Skin*) iter.Next())
		if (skin->IsClass(VX_Skin))
		{
			if ((skin->Parent() == skeleton) ||
				(skin->GetSkeleton() == skeleton))
			m_Skin = skin;	// find the skin driven by this skeleton
			break;
		}
	if (skin == NULL)
		{ VX_WARNING(("WARNING: cannot find skin for skeleton %s", skeleton->GetName())); }
	if (!m_BodyName.IsEmpty())
		body = (Model*) scene->GetModels()->Find(m_BodyName, Group::FIND_DESCEND | Group::FIND_END);
	if (body && body->IsClass(VX_Model))
	{
		Vec3	campos, bodypos;
		float	bodydist;
		Box3	bounds;

		body->GetBound(&bounds);
		bodypos = bounds.Center();
//		cam->SetFOV(1.4f);
//		cam->Translate(0.0f, 0.0f, -50.0f);
		scene->ZoomToModel(body);
		campos = cam->GetTranslation();
		bodydist = campos.z - bodypos.z;
		AttachTracker(body, cam);
		if (UseKinect)
		{
			MakeCameraOutputShape(cam, bodydist);
			if (!m_CameraOutput.IsNull())
				cam->Append(m_CameraOutput);
		}
	}
	else
		VX_WARNING(("ERROR: Cannot find body model"));
#ifdef USE_PHYSICS
	RagDoll* ragdoll = (RagDoll*) skeleton;
	if (ragdoll->IsClass(VX_RagDoll))
	{
		int waistid = ragdoll->GetBoneIndex("Proxy_Waist");

		Scriptor::SetAnimationScale(0.01f);	// centimeters -> meters
		if (waistid >= 0)
			ragdoll->SetBoneOptions(waistid, Skeleton::BONE_ANIMATE);
		if (UsePhysics)
		{
			ragdoll->SetControl(ragdoll->GetControl() | RagDoll::DYNAMIC);
			VX_TRACE(Debug, ("Physics enabled for %s", ragdoll->GetName()));
		}
	}
#endif
	return body;
}

Scene* BodyTrackDemo::MakeScene(const TCHAR* filename)
{
	Scene*			scene = new Scene();
	Camera*			cam = scene->GetCamera();
	Light*			light = new Light();
	Model*			root = new Model();
	Model*			body = NULL;
	Engine*			simroot = new Engine();
//	Core::String	name("dina_morph_col");
	Core::String	name("scape_morph_tex2");
	float			fov = 1.4f;
	Box3			bodysize;
	Vec3			campos;

	scene->SetName(name + ".scene");
	scene->SetBackColor(Col4(0.8f, 0.9f, 1.0f));
	cam->SetName(name + ".camera");
	cam->Append(light);
	simroot->SetName(name + ".simroot");
	m_Poser = MakeTracker(simroot);
	m_Poser->SetName(name + ".skeleton");
	simroot->Append(m_Poser);
	root->SetName(name + ".root");
	scene->SetModels(root);
	scene->SetEngines(simroot);

	body = m_Poser->MakeSkeleton(name);
	body->Scale(100.0f, 100.0f, 100.0f);
	root->Append(body);
	body->GetBound(&bodysize);		// get dimensions of skeleton
	cam->SetHeight(bodysize.Height());
	cam->SetFOV(fov);
	scene->ShowAll();
	/*
	 * Make the camera view volume the same as the Kinect camera
	 */
	AttachTracker(body, cam);
	if (!m_PoseMapper.IsNull())
		MakeRelativeTestPoses();
	if (!m_SaveFile.IsEmpty())
	{
		FileMessenger	savefile;
		Core::Stream*	stream = World3D::Get()->GetLoader()->OpenStream(m_SaveFile, Messenger::OPEN_WRITE);
		savefile.SetOutStream(stream);
		scene->Save(savefile, Messenger::OPEN_WRITE);
		savefile.Close();
		Stop();
	}
	return scene;
}

void BodyTrackDemo::SetSkin(Skin* body_skin)
{
	DoAsyncLoad = true;
	GetMessenger()->Observe(body_skin, Event::LOAD_IMAGE, NULL);
	GetMessenger()->Observe(body_skin, Event::LOAD_SCENE, NULL);
}

Skeleton* BodyTrackDemo::AttachTracker(Model* root, Camera* cam)
{
	Skeleton*	skel;
	Box3		vvol;
	Box3		bodysize;
	Vec3		campos;
	float		dist;
	Core::String name;

	if (root == NULL)
		return NULL;
	root->GetBound(&bodysize);			// get dimensions of skeleton
	campos = cam->GetTranslation();		// calculate distance of body from camera
	dist = campos.Distance(bodysize.Center());
	dist += 4.0f * bodysize.Height() + bodysize.Depth();
	cam->SetYon(dist);
	skel = MakeTracker();				// get or make the body tracker
	if (skel == NULL)					// failed to make tracker
		return NULL;
	if (!m_Tracker.IsNull())
		m_Tracker->SetWaistOffset(Vec3(0.0f, campos.y, 0.0f));
	name = root->GetName();
	if (skel->GetName() == NULL)
		skel->SetName(name += ".skeleton");
	if (skel->GetTarget() == NULL)
		skel->SetTarget(root);
	return skel;
}

Skeleton* BodyTrackDemo::MakeTracker(Engine* parent)
{
	if (m_Poser.IsNull())
	{
		m_Poser = new Skeleton(Skeleton::NUM_BONES);
		m_Poser->SetName("bodytrack.skeleton");
		if (parent)
			parent->Append(m_Poser);
	}
	if (!m_Tracker.IsNull())
		return m_Poser;

	PoseMapper*		mapper = m_PoseMapper;
	Core::String	name;
	Skeleton*		skel = NULL;

	if (mapper != NULL)
		return m_Poser;
#ifdef USE_KINECT
	if (UseKinect)
	{
		TCHAR configfile[VX_MaxPath];

		GetMediaPath("KinectConfig.xml", configfile, VX_MaxPath);
		m_Tracker = new KinectTracker(configfile);
		m_Tracker->SetOptions(KinectTracker::TRACK_SKELETON);
		m_Tracker->SetControl(Engine::CONTROL_CHILDREN);
		mapper = m_Tracker->GetPoseMapper();
		KinectTracker::Debug = BodyTracker::Debug;
		VX_TRACE(Debug, ("Attaching Kinect body tracking engine"));
	}
#endif
#ifdef USE_OMEK
	if (UseOmek)
	{
		TCHAR configfile[VX_MaxPath];

		GetMediaPath("OmekConfig.xml", configfile, VX_MaxPath);
		m_Tracker = new OmekTracker(configfile);
		m_Tracker->SetOptions(BodyTracker::TRACK_SKELETON);
		m_Tracker->SetControl(Engine::CONTROL_CHILDREN);
		m_Tracker->Append(mapper);
		OmekTracker::Debug = BodyTracker::Debug;
		VX_TRACE(Debug, ("Attaching Omek body tracking engine"));
	}
#endif
	if (!m_Tracker.IsNull())
	{
		skel = m_Tracker->GetSkeleton();
		m_Tracker->PutBefore(m_Poser);
	}
	else
	{
		mapper = new PoseMapper();
		skel = new Skeleton(Skeleton::NUM_BONES);
		skel->SetName("default.skeleton");
		mapper->SetSource(skel);
		mapper->SetActive(false);
	}
	m_PoseMapper = mapper;
	VX_TRACE(Debug, ("Attaching pose mapper %s", name));
	mapper->SetTarget(m_Poser);
	if (Core::String(m_Poser->GetBoneName(0)) != TEXT("Proxy_Waist"))
	{
		if (Core::String(m_Poser->GetBoneName(0)) == TEXT("amyroot"))
			name = "amy";
		else
			name = "genesis";
	}
	if (mapper->SelectBoneMap(name))
	{	VX_TRACE(Debug, ("Selected %s bone map", name)); }
	else
		VX_ERROR(("ERROR: Cannot select bone map %s - incompatible skeletons", name), m_Poser);
	mapper->MapWorldToSource();
	return m_Poser;
}

Engine* BodyTrackDemo::MakeEngines()
{
	Engine*		simroot = MFC::Demo::MakeEngines();

	if (simroot == NULL)
		return NULL;
	Appearance*	app = new Appearance();
	Scriptor*	scriptor = GetScriptor();

	if (scriptor)
		scriptor->SetTarget(m_Poser);
	if (m_Picker.IsNull())
		m_Picker = new RayPicker;
	m_Picker->SetName("RayPicker");
	app->Set(Appearance::SHADING, Appearance::WIRE);
	m_Picker->SetHilite(app);
	m_Picker->SetOptions(Picker::MESH | Picker::HILITE_SELECTED);
	m_Picker->SetButtons(MouseEvent::SHIFT);
	AddEngine(m_Picker);
	return simroot;
}

void BodyTrackDemo::SetScene(Scene* scene, Window win)
{
	Model*		root;
	Model*		skelmod = NULL;
	Engine*		simroot = NULL;
	Skeleton*	skeleton = NULL;
	Box3		bounds;
	Camera*		cam;
	Scriptor*	scriptor = GetScriptor();

	if (scene == NULL)
		return;
	ObjectLock	lock(scene);
	root = scene->GetModels();
	if (root == NULL)
		return;
//	scene->SetTimeInc(1.0f / 30.0f);	// 30 fps
	simroot = scene->GetEngines();
	root->GetBound(&bounds);
	cam = scene->GetCamera();
	GetMessenger()->Observe(this, Event::KEY, NULL);
	/*
	 * Find the first skeleton in the scene graph and allow the picker to
	 * select bones from it
	 */
	skelmod = (Model*) root->Find(m_SkelName.IsEmpty() ? TEXT(".skeleton") : m_SkelName, Group::FIND_DESCEND | Group::FIND_END);
	if (skelmod == NULL)
		skelmod = (Model*) root->Find(m_SkelName + TEXT("_skeleton"), Group::FIND_DESCEND | Group::FIND_END);
	if (skelmod)
	{
		VX_TRACE(Debug, ("Found bone models for skeleton %s", skelmod->GetName()));
		if (m_Picker.IsNull())
			m_Picker = new RayPicker;
		m_Picker->SetFlags(SharedObj::DOEVENTS);
		GetMessenger()->Observe(this, Event::PICK, m_Picker);
		GetMessenger()->Observe(this, Event::NOPICK, m_Picker);
		m_Picker->SetTarget(skelmod);
		skelmod->GetBound(&bounds);
	}
	/*
	 * Now find the skeleton in the simulation tree.
	 */
	if (simroot == NULL)
	{
		VX_WARNING(("WARNING: cannot find skin or skeleton"));
		MFC::Demo::SetScene(scene, win);
		return;
	}
	skeleton = (Skeleton*) simroot->Find(m_SkelName + TEXT(".skeleton"), Group::FIND_DESCEND | Group::FIND_END);
	if (m_Poser.IsNull() && skeleton && skeleton->IsClass(VX_Skeleton))
	{
		m_Poser = skeleton;
		SetSkeleton(skeleton);
	}
	/*
	 * Not using SCAPE or could not find SCAPE in the content.
	 * It may be a skinned avatar or just rigid bodies.
	 * Look for a skeleton and attach the body tracker.
	 */
	if (skeleton != NULL)
	{
		Model* body = ConnectAvatar(scene, skeleton);

		if (body && !m_PoseMapper.IsNull())
			MakeRelativeTestPoses();
	}
	else
		VX_WARNING(("ERROR: Cannot find skeleton %s", m_SkelName));
	// Output scene as XML
	//std::ofstream textout(TEXT("BodyTracker.xml"), std::ios_base::out);
	//scene->Print(textout, SharedObj::PRINT_All);
	if (!m_Poser.IsNull())
		m_Poser->Start();
	MFC::Demo::SetScene(scene, win);
	Scriptor* scp = GetScriptor();
	if (scp)
		scp->SetTarget(m_Poser);
#ifdef RAM_TEST
	GetMessenger()->Observe(this, Event::SET_TIME, NULL);
	GetScene()->SetFlags(SharedObj::DOEVENTS);
	m_FrameStats->SetActive(false);
#endif
}

void BodyTrackDemo::MakeTestPoses()
{
	static Quat	KINECT[Skeleton::NUM_BONES];
	static Quat	EGYPT[Skeleton::NUM_BONES];
	static Quat	RUN[Skeleton::NUM_BONES];
	static Quat	JUMP[Skeleton::NUM_BONES];

	Vec3	vecs[3];
	Vec3	positions[Skeleton::NUM_BONES];

	m_Poser->GetBindPose()->GetWorldPositions(positions);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	JUMP[Skeleton::HEAD].Set(vecs);
	JUMP[Skeleton::NECK].Set(vecs);
	JUMP[Skeleton::TORSO].Set(vecs);
	JUMP[Skeleton::LEFT_SHOULDER].Set(vecs);
	JUMP[Skeleton::LEFT_ELBOW].Set(vecs);
	JUMP[Skeleton::LEFT_WRIST].Set(vecs);
	JUMP[Skeleton::LEFT_HAND].Set(vecs);
	JUMP[Skeleton::RIGHT_SHOULDER].Set(vecs);
	JUMP[Skeleton::RIGHT_ELBOW].Set(vecs);
	JUMP[Skeleton::RIGHT_WRIST].Set(vecs);
	JUMP[Skeleton::RIGHT_HAND].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	JUMP[Skeleton::WAIST].Set(vecs);
	JUMP[Skeleton::LEFT_HIP].Set(vecs);
	JUMP[Skeleton::LEFT_ANKLE].Set(vecs);
	JUMP[Skeleton::LEFT_FOOT].Set(vecs);
	JUMP[Skeleton::RIGHT_HIP].Set(vecs);
	JUMP[Skeleton::RIGHT_ANKLE].Set(vecs);
	JUMP[Skeleton::RIGHT_FOOT].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	JUMP[Skeleton::LEFT_COLLAR].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	JUMP[Skeleton::RIGHT_COLLAR].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, -1, 0), vecs[2].Set(0, 0, -1);
	JUMP[Skeleton::LEFT_KNEE].Set(vecs);
	JUMP[Skeleton::RIGHT_KNEE].Set(vecs);
	m_JumpPose = new Pose(m_Poser, Pose::WORLD);
	m_JumpPose->SetName("JUMP.pose");
	m_JumpPose->SetWorldRotations(JUMP);
	m_JumpPose->SetWorldPositions(positions);

	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	KINECT[Skeleton::HEAD].Set(vecs);
	KINECT[Skeleton::NECK].Set(vecs);
	KINECT[Skeleton::TORSO].Set(vecs);
	KINECT[Skeleton::LEFT_ELBOW].Set(vecs);
	KINECT[Skeleton::LEFT_WRIST].Set(vecs);
	KINECT[Skeleton::LEFT_HAND].Set(vecs);
	KINECT[Skeleton::RIGHT_ELBOW].Set(vecs);
	KINECT[Skeleton::RIGHT_WRIST].Set(vecs);
	KINECT[Skeleton::RIGHT_HAND].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	KINECT[Skeleton::WAIST].Set(vecs);
	KINECT[Skeleton::LEFT_HIP].Set(vecs);
	KINECT[Skeleton::LEFT_KNEE].Set(vecs);
	KINECT[Skeleton::RIGHT_HIP].Set(vecs);
	KINECT[Skeleton::RIGHT_KNEE].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	KINECT[Skeleton::LEFT_COLLAR].Set(vecs);
	vecs[0].Set(0, -1, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(1, 0, 0);
	KINECT[Skeleton::LEFT_SHOULDER].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	KINECT[Skeleton::RIGHT_COLLAR].Set(vecs);
	vecs[0].Set(0, 1, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(-1, 0, 0);
	KINECT[Skeleton::RIGHT_SHOULDER].Set(vecs);
	KINECT[Skeleton::LEFT_ANKLE].Set(0, 0, 0, 1);
	KINECT[Skeleton::LEFT_FOOT].Set(0, 0, 0, 1);
	KINECT[Skeleton::RIGHT_ANKLE].Set(0, 0, 0, 1);
	KINECT[Skeleton::RIGHT_FOOT].Set(0, 0, 0, 1);
	m_TrackerPose = new Pose(m_Poser, Pose::WORLD);
	m_TrackerPose->SetWorldRotations(KINECT);
	m_TrackerPose->SetWorldPositions(positions);
	m_TrackerPose->SetName("KINECT.pose");

	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	EGYPT[Skeleton::HEAD].Set(vecs);
	EGYPT[Skeleton::NECK].Set(vecs);
	EGYPT[Skeleton::TORSO].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	EGYPT[Skeleton::WAIST].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	EGYPT[Skeleton::LEFT_COLLAR].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	EGYPT[Skeleton::RIGHT_COLLAR].Set(vecs);

	vecs[0].Set(0, 1, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(-1, 0, 0);
	EGYPT[Skeleton::RIGHT_SHOULDER].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(-1, 0, 0), vecs[2].Set(0, 1, 0);
	EGYPT[Skeleton::RIGHT_ELBOW].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, -1, 0), vecs[2].Set(-1, 0, 0);
	EGYPT[Skeleton::RIGHT_WRIST].Set(vecs);
	EGYPT[Skeleton::RIGHT_HAND].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	EGYPT[Skeleton::RIGHT_HIP].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(-1, 0, 0), vecs[2].Set(0, -1, 0);
	EGYPT[Skeleton::RIGHT_KNEE].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	EGYPT[Skeleton::RIGHT_ANKLE].Set(vecs);
	EGYPT[Skeleton::RIGHT_FOOT].Set(vecs);

	vecs[0].Set(0, -1, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(1, 0, 0);
	EGYPT[Skeleton::LEFT_SHOULDER].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(1, 0, 0), vecs[2].Set(0, 1, 0);
	EGYPT[Skeleton::LEFT_ELBOW].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, -1, 0), vecs[2].Set(1, 0, 0);
	EGYPT[Skeleton::LEFT_WRIST].Set(vecs);
	EGYPT[Skeleton::LEFT_HAND].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	EGYPT[Skeleton::LEFT_HIP].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(1, 0, 0), vecs[2].Set(0, -1, 0);
	EGYPT[Skeleton::LEFT_KNEE].Set(vecs);
	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	EGYPT[Skeleton::LEFT_ANKLE].Set(vecs);
	EGYPT[Skeleton::LEFT_FOOT].Set(vecs);
	m_EgyptPose = new Pose(m_Poser, Pose::WORLD);
	m_EgyptPose->SetWorldRotations(EGYPT);
	m_EgyptPose->SetWorldPositions(positions);
	m_EgyptPose->SetName("EGYPT");

	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, -1), vecs[2].Set(0, 1, 0);
	RUN[Skeleton::HEAD].Set(vecs);
	RUN[Skeleton::NECK].Set(vecs);
	RUN[Skeleton::TORSO].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, 0, 1), vecs[2].Set(0, -1, 0);
	RUN[Skeleton::WAIST].Set(vecs);
	RUN[Skeleton::RIGHT_HIP].Set(vecs);
	RUN[Skeleton::RIGHT_ANKLE].Set(vecs);
	RUN[Skeleton::RIGHT_FOOT].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[1].Set(0, -1, 0); vecs[2].Set(0, 0, -1);
	RUN[Skeleton::RIGHT_KNEE].Set(vecs);

	vecs[0].Set(0, 0, -1); vecs[1].Set(0, 1, 0), vecs[2].Set(1, 0, 0);
	RUN[Skeleton::LEFT_COLLAR].Set(vecs);
	vecs[0].Set(0, 0, 1); vecs[1].Set(0, 1, 0), vecs[2].Set(-1, 0, 0);
	RUN[Skeleton::RIGHT_COLLAR].Set(vecs);
	vecs[0].Set(0, -1, -1); vecs[0].Normalize(); vecs[2].Set(0, -1, 1); vecs[2].Normalize(); vecs[1] = vecs[2].Cross(vecs[0]); 
	RUN[Skeleton::LEFT_SHOULDER].Set(vecs);
	vecs[0].Set(0, -1, 1); vecs[0].Normalize(); vecs[2].Set(0, 1, 1); vecs[2].Normalize(); vecs[1].Set(1, 0, 0); 
	RUN[Skeleton::LEFT_ELBOW].Set(vecs);
	RUN[Skeleton::LEFT_WRIST].Set(vecs);
	RUN[Skeleton::LEFT_HAND].Set(vecs);
	vecs[0].Set(0, -1, 1); vecs[0].Normalize(); vecs[2].Set(0, -1, -1); vecs[2].Normalize(); vecs[1].Set(-1, 0, 0); 
	RUN[Skeleton::RIGHT_SHOULDER].Set(vecs);
	vecs[0].Set(0, 1, 1); vecs[0].Normalize(); vecs[2].Set(0, -1, 1); vecs[2].Normalize(); vecs[1].Set(-1, 0, 0); 
	RUN[Skeleton::RIGHT_ELBOW].Set(vecs);
	RUN[Skeleton::RIGHT_WRIST].Set(vecs);
	RUN[Skeleton::RIGHT_HAND].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[2].Set(0, -1, 1); vecs[2].Normalize(); vecs[1].Set(0, 1, 1); vecs[1].Normalize();
	RUN[Skeleton::LEFT_HIP].Set(vecs);
	RUN[Skeleton::LEFT_ANKLE].Set(vecs);
	RUN[Skeleton::LEFT_FOOT].Set(vecs);
	vecs[0].Set(1, 0, 0); vecs[2].Set(0, -1, 0); vecs[1].Set(0, 0, 1); 
	RUN[Skeleton::LEFT_KNEE].Set(vecs);
	m_RunPose = new Pose(m_Poser, Pose::WORLD);
	m_RunPose->SetWorldRotations(RUN);
	m_RunPose->SetWorldPositions(positions);
	m_RunPose->SetName("RUN");
}

void BodyTrackDemo::MakeRelativeTestPoses()
{
	Quat	NONE(0,0,0,1);
	Quat	Y90(Model::YAXIS, PI / 2);
	Quat	YNeg90(Model::YAXIS, -PI / 2);
	Quat	X90(Model::XAXIS, PI / 2);
	Quat	XNeg90(Model::XAXIS, -PI / 2);
	Quat	Z90(Model::ZAXIS, PI / 2);
	Quat	ZNeg90(Model::ZAXIS, -PI / 2);

	Quat	LEFT_ARM;
	Quat	RIGHT_ARM;
	Quat	LEFT_LEG;
	Quat	RIGHT_LEG;
	Quat	LEFT_HIP;
	Quat	RIGHT_HIP;
	Quat	KNEE(Model::XAXIS, PI / 2);
	Quat	JUMP[Skeleton::NUM_BONES];
	Quat	KINECT[Skeleton::NUM_BONES];
	Quat	EGYPT[Skeleton::NUM_BONES];
	Skeleton*	srcskel = m_PoseMapper->GetSource();

	for (int i = 0; i < Skeleton::NUM_BONES; ++i)
	{
		JUMP[i].Set(NONE);
		KINECT[i].Set(NONE);
		EGYPT[i].Set(NONE);
	}
	LEFT_ARM = Z90;
	RIGHT_ARM = ZNeg90;
	LEFT_HIP = Z90;
	RIGHT_HIP = ZNeg90;
	LEFT_LEG = Y90;
	RIGHT_LEG = YNeg90;
	m_HelloPose = new Pose(srcskel, Pose::SKELETON);
	m_HelloPose->SetName("HELLO");
	m_HelloPose->SetLocalRotation(Skeleton::LEFT_KNEE, Quat(Model::ZAXIS, PI / 4));
	m_HelloPose->Sync();

	JUMP[Skeleton::LEFT_ELBOW].Set(LEFT_ARM);
	JUMP[Skeleton::LEFT_SHOULDER].Set(LEFT_ARM);
	JUMP[Skeleton::LEFT_WRIST].Set(LEFT_ARM);
	JUMP[Skeleton::LEFT_HAND].Set(LEFT_ARM);
	JUMP[Skeleton::RIGHT_SHOULDER].Set(RIGHT_ARM);
	JUMP[Skeleton::RIGHT_ELBOW].Set(RIGHT_ARM);
	JUMP[Skeleton::RIGHT_WRIST].Set(RIGHT_ARM);
	JUMP[Skeleton::RIGHT_HAND].Set(RIGHT_ARM);
	JUMP[Skeleton::LEFT_ANKLE].Set(KNEE);
	JUMP[Skeleton::LEFT_FOOT].Set(KNEE);
	JUMP[Skeleton::RIGHT_ANKLE].Set(KNEE);
	JUMP[Skeleton::RIGHT_FOOT].Set(KNEE);
	JUMP[Skeleton::LEFT_KNEE].Set(KNEE);
	JUMP[Skeleton::RIGHT_KNEE].Set(KNEE);
	m_JumpPose = new Pose(srcskel, Pose::BIND_POSE_RELATIVE);
	m_JumpPose->SetName("JUMP");
	m_JumpPose->SetWorldRotations(JUMP);
	m_JumpPose->Sync();

	KINECT[Skeleton::LEFT_ELBOW].Set(LEFT_ARM);
	KINECT[Skeleton::LEFT_WRIST].Set(LEFT_ARM);
	KINECT[Skeleton::LEFT_HAND].Set(LEFT_ARM);
	KINECT[Skeleton::RIGHT_ELBOW].Set(RIGHT_ARM);
	KINECT[Skeleton::RIGHT_WRIST].Set(RIGHT_ARM);
	KINECT[Skeleton::RIGHT_HAND].Set(RIGHT_ARM);
	m_TrackerPose = new Pose(srcskel, Pose::BIND_POSE_RELATIVE);
	m_TrackerPose->SetName("KINECT");
	m_TrackerPose->SetWorldRotations(KINECT);
	m_TrackerPose->Sync();

	EGYPT[Skeleton::LEFT_ELBOW].Set(LEFT_ARM);
	EGYPT[Skeleton::LEFT_WRIST].Set(LEFT_ARM);
	EGYPT[Skeleton::LEFT_HAND].Set(LEFT_ARM);
	EGYPT[Skeleton::RIGHT_ELBOW].Set(RIGHT_ARM);
	EGYPT[Skeleton::RIGHT_WRIST].Set(RIGHT_ARM);
	EGYPT[Skeleton::RIGHT_HAND].Set(RIGHT_ARM);
	EGYPT[Skeleton::LEFT_HIP].Set(LEFT_HIP);
	EGYPT[Skeleton::LEFT_KNEE].Set(LEFT_LEG);
	EGYPT[Skeleton::RIGHT_HIP].Set(RIGHT_HIP);
	EGYPT[Skeleton::RIGHT_KNEE].Set(RIGHT_LEG);
	m_EgyptPose = new Pose(srcskel, Pose::BIND_POSE_RELATIVE);
	m_EgyptPose->SetName("EGYPT");
	m_EgyptPose->SetWorldRotations(EGYPT);
	m_EgyptPose->Sync();
}

bool BodyTrackDemo::OnEvent(Event* ev)
{
	const ObjArray* selected;
	KeyEvent* ke;
	MouseEvent* me;
	int	tmp;
	
	switch (ev->Code)
	{
		case Event::MOUSE:
		me = (MouseEvent*) ev;
		tmp = (MouseEvent::CONTROL | MouseEvent::RIGHT);
		if ((me->MouseFlags & tmp) == tmp)
		{
			Vec3		tmp1(me->MousePos);
			Vec3		tmp2(m_Poser->GetPosition());
			Scene*		scene = GetScene();
			Camera*		cam = scene->GetCamera();
			const Box2&	vport = scene->GetViewport();
			const Box3&	vvol = cam->GetViewVol();
			Vec3		camtrans(cam->GetTranslation());
			float		bodydist = camtrans.Length();

			tmp1.x *= vvol.Width() / vport.Width();
			tmp1.y *= vvol.Height() / vport.Height();
			tmp1.x += vvol.min.x;
			tmp1.y = vvol.max.y - tmp1.y;
			tmp1.z = bodydist / vvol.min.z;
			tmp1.x *= tmp1.z;
			tmp1.y *= tmp1.z;
			tmp2.x = tmp1.x;
			m_Poser->SetPosition(tmp2);
		}
		break;

		case Event::KEY:
		ke = (KeyEvent*) ev;
		OnKey(ke->KeyCode);
		break;

		case Event::PICK:
		if (ev->Sender != (const SharedObj*) m_Picker)
			break;
		selected = m_Picker->GetSelected();
		if (selected && selected->GetSize())
			Select((Shape*) selected->GetAt(0));
		break;

		case Event::NOPICK:
		if (ev->Sender != (const SharedObj*) m_Picker)
			break;
		Select(NULL);
		break;

#ifdef RAM_TEST
		case Event::SET_TIME:
		UpdateShape(0, 0.01f);
#endif
	}
	return MFC::Demo::OnEvent(ev);
}

void BodyTrackDemo::OnKey(int keycode)
{
	int	boneid = -1;

	switch (keycode)
	{
		case KeyEvent::K_F2: endl(vixen_debug << "JUMP POSE"); ApplyPose(m_JumpPose); break;
		case KeyEvent::K_F3: endl(vixen_debug << "KINECT POSE"); ApplyPose(m_TrackerPose); break;
		case KeyEvent::K_F4: endl(vixen_debug << "EGYPT POSE"); ApplyPose(m_EgyptPose); break;
		case KeyEvent::K_F5: endl(vixen_debug << "HELLO POSE"); ApplyPose(m_HelloPose); break;

		case KeyEvent::K_F1:
		endl(vixen_debug << "REST POSE");
#if REMOVED
		if (UseSCAPE)
		{
			memset(m_ShapeParams, 0, sizeof(m_ShapeParams));
			m_Skin->SetBodyShape(NULL);
		}
		else
#endif
			m_Poser->RestoreBindPose();
		break;

		case KeyEvent::K_RIGHT:
		m_Axis = 'Z';
		endl(vixen_debug << "Selected Z axis"); 
		endl(vixen_debug << "increment angle");
		SetJointAngle(PI / 20);
		return;

		case KeyEvent::K_LEFT:
		m_Axis = 'Z';
		endl(vixen_debug << "Selected Z axis");
		endl(vixen_debug << "decrement angle");
		SetJointAngle(-PI / 20);
		return;
 
		case KeyEvent::K_UP:
		m_Axis = 'X';
		endl(vixen_debug << "Selected X axis"); 
		endl(vixen_debug << "increment angle");
		SetJointAngle(PI / 20);
		return;

		case KeyEvent::K_DOWN:
		m_Axis = 'X';
		endl(vixen_debug << "Selected X axis");
		endl(vixen_debug << "decrement angle");
		SetJointAngle(-PI / 20);
		return;
 
		case KeyEvent::K_HOME:
		m_Axis = 'Y';
		endl(vixen_debug << "Selected Y axis"); 
		endl(vixen_debug << "increment angle");
		SetJointAngle(PI / 20);
		return;

		case KeyEvent::K_END:
		m_Axis = 'Y';
		endl(vixen_debug << "Selected Y axis");
		endl(vixen_debug << "decrement angle");
		SetJointAngle(-PI / 20);
		return;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		m_ShapeIndex = keycode - '0';
		endl(vixen_debug << "Shape index " << m_ShapeIndex);
		return;

		case 'l': UpdateShape(m_ShapeIndex, 0.01f); return;	 
		case 's': UpdateShape(m_ShapeIndex, -0.01f); return;
		case '+': endl(vixen_debug << "increment angle"); SetJointAngle(PI / 20); return;
		case '-': endl(vixen_debug << "decrement angle"); SetJointAngle(-PI / 20); return;
		case 'X': case 'x': m_Axis = 'X'; endl(vixen_debug << "Selected X axis"); return;
		case 'Y': case 'y': m_Axis = 'Y'; endl(vixen_debug << "Selected Y axis"); return;
		case 'Z': case 'z': m_Axis = 'Z'; endl(vixen_debug << "Selected Z axis"); return;

		default: Usage(); return;
	}
}

void BodyTrackDemo::Usage()
{
	endl(vixen_debug << "To rotate joints using the keyboard:");
	endl(vixen_debug << "\tX rotate about X axis");
	endl(vixen_debug << "\tY rotate about Y axis");
	endl(vixen_debug << "\tZ rotate about Z axis");
	endl(vixen_debug << "\t+ increase angle");
	endl(vixen_debug << "\t- decrease angle");
	endl(vixen_debug << "\tarrows rotate about Z and X");
	endl(vixen_debug << "\tHOME / END rotates about Y");
	endl(vixen_debug << "To change body shape using the keyboard:");
	endl(vixen_debug << "\t0-5 select body part");
	endl(vixen_debug << "\tl make body part larger");
	endl(vixen_debug << "\ts make body part smaller");
	endl(vixen_debug << "Function keys F1-F4 select poses");
}

void  BodyTrackDemo::ApplyPose(Pose* pose)
{
	if (pose == NULL)
		return;
	if (m_PoseMapper != NULL)
	{
		Skeleton* srcskel = m_PoseMapper->GetSource();
		srcskel->ApplyPose(pose);
		m_PoseMapper->MapLocalToTarget();
	}
	else
		m_Poser->ApplyPose(pose);
}

void BodyTrackDemo::Select(Shape* shape)
{
	Shape* oldsel = (Shape*) m_SelectedShape;

	m_Picker->UnHiliteAll();
	if (oldsel)
		oldsel->SetAppearance(m_ShapeAppear);
	if (shape && shape->IsClass(VX_Shape))
	{
		Core::String name(shape->GetName());
		m_ShapeAppear = shape->GetAppearance();
		shape->SetAppearance(m_SelectAppear);
		endl(vixen_debug << "Selected shape " << shape->GetName());
		m_BoneID = -1;
		m_SelectedBone = (Transformer*) m_Poser->FindTarget(shape);
		if (m_SelectedBone == NULL)
			m_SelectedBone = (Transformer*) m_Poser->FindTarget(shape->Parent());
		if (!m_SelectedBone.IsNull() && m_SelectedBone->IsClass(VX_Transformer))
		{
			endl(vixen_debug << "found bone engine " <<  m_SelectedBone->GetName());		
			m_BoneID = m_Poser->GetBoneIndex(m_SelectedBone);
		}
		if (m_BoneID < 0)
		{
			endl(vixen_debug << "Cannot find bone for " << shape->GetName());
			m_SelectedBone = (Transformer*) NULL;
		}
		else
			endl(vixen_debug << "Selected bone " << m_BoneID  << " " <<  m_Poser->GetBoneName(m_BoneID));		
	}
	else
		m_ShapeAppear = (Appearance*) NULL;
	m_SelectedShape = shape;
}

void BodyTrackDemo::UpdateShape(int shapeindex, float inc)
{
	if (!m_Skin.IsNull())
	{
		Engine::Iter iter((Engine*) m_Skin, Group::CHILDREN);
		Morph* def;

		while (def = (Morph*) iter.Next())
			if (def->IsClass(VX_Morph))
				def->SetWeight(shapeindex + 1, def->GetWeight(shapeindex + 1) + 10 * inc);
	}
}


void BodyTrackDemo::SetJointAngle(float angle, bool replace)
{
	Vec3	axis;
	Quat	q1;
	Quat	q2;
	const char* saxis = " Z axis";
	Pose*	pose = m_Poser->GetPose();
	int		index;

	if (m_BoneID < 0)
	{
		if (!m_SelectedBone.IsNull())
			m_BoneID = m_Poser->GetBoneIndex(m_SelectedBone);
		if (m_BoneID < 0)
			return;
	}
	index = m_BoneID;
	if (!m_PoseMapper.IsNull())
	{
		index = m_PoseMapper->GetBoneMap().GetAt(m_BoneID);
		if (index < 0)
			return;
	}
	switch (m_Axis)
	{
		case 'X':	axis.Set(1, 0, 0); saxis = " X axis"; break;
		case 'Y':	axis.Set(0, 1, 0); saxis = " Y axis"; break;
		default:	axis.Set(0, 0, 1); break;
	}
	q1.Set(axis, angle);
	if (replace)
		endl(vixen_debug << "replace " << m_Poser->GetBoneName(index) << " " << angle << " radians about " << saxis);
	else
	{
		endl(vixen_debug << "add " << m_Poser->GetBoneName(index) << " " << angle << " radians about " << saxis);
		q2 = pose->GetLocalRotation(index);
		q1.Mul(q2, q1);
	}
	pose->SetLocalRotation(index, q1);
}

Shape* BodyTrackDemo::MakeCameraOutputShape(Camera* cam, float bodydist)
{
	if (!m_CameraOutput.IsNull())
		return m_CameraOutput;

	Shape*		shape = new Shape();
	Texture*	tex = new Texture();
	Appearance*	app = new Appearance();
	Sampler*	sampler = new Sampler();
	TriMesh*	mesh = new TriMesh(VertexPool::NORMALS | VertexPool::TEXCOORDS);
	Vec2		videosize(640, 480);
	Vec2		texsize(1024, 512);
	Vec3		camtrans(cam->GetTranslation());
	Box3		vvol;
	float		w, h, backdist;
	bool		iscolor = true;

	cam->GetViewVol(&vvol);						// get camera view volume
	if (bodydist == 0.0f)
		bodydist = camtrans.Length();
	backdist = cam->GetYon() - bodydist - 0.1f;
	w = vvol.max.z * vvol.Width() / vvol.min.z;
	h = vvol.max.z * vvol.Height() / vvol.min.z;

	m_CameraOutput = shape;
	shape->SetAppearance(app);
	GeoUtil::Rect(mesh, h * videosize.x / videosize.y, h, videosize.x / texsize.x, videosize.y / texsize.y);
	shape->SetGeometry(mesh);
	shape->SetName(TEXT("kinect.camerashape"));
	shape->Translate(0, 0, -backdist);
	app->Set(Appearance::CULLING, false);
	app->SetSampler(0, sampler);
	sampler->SetTexture(tex);
	sampler->Set(Sampler::TEXCOORD, 0);
	if (iscolor)
	{
		tex->SetName(TEXT("kinect.colorimage"));
		sampler->Set(Sampler::TEXTUREOP, Sampler::EMISSION);
		app->Set(Appearance::TRANSPARENCY, true);
		app->Set(Appearance::LIGHTING, false);
		if (!m_Tracker.IsNull())
			m_Tracker->SetColorTexture(tex);
	}
	else
	{
		tex->SetName(TEXT("kinect.depthimage"));
		sampler->Set(Sampler::TEXTUREOP, Sampler::EMISSION);
		app->Set(Appearance::LIGHTING, true);
		if (!m_Tracker.IsNull())
			m_Tracker->SetDepthTexture(tex);
	}
	return shape;
}

VIXEN_MAKEWORLD(BodyTrackDemo);

MFC::App	theMFCApp;
