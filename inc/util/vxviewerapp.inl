
#ifdef VX_JOYFLYER
#include "win32/vxjoystick.h"
#endif

template <class WORLD> const TCHAR* Viewer<WORLD>::OpcodeNames[] =
{
	TEXT("SetMediaDir"),
	TEXT("SetScriptFile"),
	TEXT("SetEngines"),
	TEXT("SetModels"),
	TEXT("AddEngine"),
	TEXT("AddModel")
};


template <class WORLD> Viewer<WORLD>::Viewer() : WORLD()
{
/*
 *	Initializes this instance of the application. Because we are using
 *	plugins, we need to force them to get linked in by referencing them
 */
	REGISTER_CLASS(Skeleton);
	REGISTER_CLASS(Skin);
	REGISTER_CLASS(Morph);
	REGISTER_CLASS(MeshAnimator);
	REGISTER_CLASS(Deformer);
	REGISTER_CLASS(TextGeometry);
	REGISTER_CLASS(Transformer);
	REGISTER_CLASS(KeyFramer);
	REGISTER_CLASS(KeyBlender);
	REGISTER_CLASS(Picker);
	REGISTER_CLASS(RayPicker);
	REGISTER_CLASS(ExtModel);
	REGISTER_CLASS(Trigger);
	m_StereoEyeSep = 0.0f;
	m_IsFullScreen = false;
}

template <class WORLD> bool Viewer<WORLD>::OnInit()
{
	m_SimRoot = new Engine;
	m_SceneRoot = new Model;
	m_SceneRoot->SetName(TEXT("scene.root"));
	return WORLD::OnInit();
}

template <class WORLD> void Viewer<WORLD>::OnExit()
{
	Engine*	nav = m_Navigator;
	if (nav)
	{
		nav->SetTarget(NULL);
		nav->Remove();
	}
	m_Navigator = (Engine*) NULL;
	if (m_Scriptor != NULL)
	{
		m_Scriptor->SetTarget(NULL);
		m_Scriptor->Remove();
	}
	m_Scriptor = (Scriptor*) NULL;
	m_FrameStats = (Sprite*) NULL;
	m_StatLog = (FrameStats*) NULL;
	m_SimRoot = (Engine*) NULL;
	m_SceneRoot = (Model*) NULL;
	WORLD::OnExit();
}

template <class WORLD> bool Viewer<WORLD>::ParseArgs(int argc, TCHAR **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		const TCHAR* p = argv[i];

		if ((p == NULL) || (*p == 0))
			continue;
		else if (STRCASECMP(p, TEXT("-fullscreen")) == 0)
			m_IsFullScreen = true;
		else if (STRCASECMP(p, TEXT("-render")) == 0)
		{
			if (i >= argc)
				break;
			p = argv[++i];
			if ((*p != 0) && (*p != TEXT('-')))
				Scene::RenderOptions = p;
		}
		else if (STRCASECMP(p, TEXT("-statlog")) == 0)
		{
			if (i >= argc)
				break;
			p = argv[++i];
			if ((*p != 0) && (*p != TEXT('-')))
				m_StatLogFile = p;
		}
		else if (STRCASECMP(p, TEXT("-stereo")) == 0)
		{
			if (i >= argc)
				break;
			p = argv[++i];
			if ((*p != 0) && (*p != TEXT('-')))
				m_StereoEyeSep = (float) STRTOD(p, NULL);
		}
	}
	return WORLD::ParseArgs(argc, argv);
}

/*!
 * @fn void Viewer<WORLD>::SetNavigator(Engine* e)
 *
 * Returns the engine used for camera navigation. Usually, this will
 * be Flyer or Navigator. This can be any engine which
 * accepts a model as a target that can process mouse events.
 *
 * @see Viewer::SetEngines Viewer::GetScriptor Flyer
 */
template <class WORLD> void Viewer<WORLD>::SetNavigator(Engine* e)
{
	Engine*	oldnav = m_Navigator;
	Messenger*	stream = GetMessenger();

	if (oldnav == e)
		return;
/*
 * Disconnect old navigator and children
 */
	if (oldnav)
	{
		GroupIter<Engine> iter(oldnav, Group::DEPTH_FIRST);
		Engine*	child;

		while (child = iter.Next())			// scan children of old navigator
		{
			if (child->IsClass(VX_Navigator))
			{
				stream->Ignore(child, Event::NAVINPUT, NULL);
				stream->Ignore(child, Event::KEY, NULL);
				stream->Ignore(child, Event::MOUSE, NULL);
			}
			else if (child->IsClass(VX_Transformer))
				stream->Ignore(child, Event::NAVIGATE, NULL);

		}
		oldnav->Remove();
	}
	m_Navigator = e;
	if (e == NULL)
		return;
/*
 * Connect new navigator and children
 */
	if (e->IsClass(VX_Transformer))			// new navigator is transformer
	{
		Engine* childnav = e->First();
		stream->Observe(e, Event::NAVIGATE, NULL);
		if (childnav && childnav->IsClass(VX_Navigator))
		{										// child handles mouse, key events
			stream->Observe(childnav, Event::NAVINPUT, NULL);
			stream->Observe(childnav, Event::MOUSE, NULL);
			stream->Observe(childnav, Event::KEY, NULL);
			childnav->SetFlags(SharedObj::DOEVENTS);	// child generates navigation events
		}
	}
	else			
	{
		stream->Observe(e, Event::MOUSE, NULL);
		stream->Observe(e, Event::KEY, NULL);
		stream->Observe(e, Event::NAVINPUT, NULL);
	}
	if (!e->IsChild() && !m_StatLog.IsNull())
		m_StatLog->Append(e);
}




/*++++
 *
 * @fn void Viewer<WORLD>::SetScriptFile(const TCHAR* filename)
 * @param filename	name of script file
 * 
 * Establishes the name of the file to load the camera script from.
 * If the name is relative, it is assumed to be in the directory
 * containing the most recently loaded file.
 *
 * @see Viewer::GetScriptor Scriptor Animator FileLoader::GetDirectory
 */
template <class WORLD> void Viewer<WORLD>::SetScriptFile(const TCHAR* name)
{
	if (name)
		m_ScriptFile = name;
	else
		m_ScriptFile.Empty();
}

template <class WORLD> const TCHAR* Viewer<WORLD>::GetScriptFile() const
{
	if (m_ScriptFile.IsEmpty())
		return NULL;
	return m_ScriptFile;
}

template <class WORLD> void Viewer<WORLD>::SetFileName(const TCHAR* name)
{
	TCHAR		namebuf[VX_MaxPath];
	size_t		n;

	WORLD::SetFileName(name);
	STRCPY(namebuf, name);
	n = STRLEN(namebuf);
	if (m_ScriptFile.IsEmpty() && (STRCASECMP(&namebuf[n - 4], TEXT(".vix")) == 0))
	{
		STRCPY(&namebuf[n - 4], TEXT(".scp"));
//		SetScriptFile(namebuf);
	}
}

/*!
 * @fn void Viewer<WORLD>::SetEngines(Engine* simroot)
 *
 * Establishes a simulation tree that is application-specific
 * to attach to an input scene after it is loaded. This tree
 * would contain engines to navigate the camera, script animations,
 * track application statistics, etc. The default engines
 * created look like:
 * @code
 *	VXStatusLog
 *		Scriptor
 *			Animator -> camera
 *		Transformer  -> camera
 *			Flyer
 *				JoyStick
 * @endcode
 * If an initial simulation tree has already been constructed
 * by Viewer::MakeEngines, this new tree will replace it.
 *
 * @see Viewer::MakeEngines Viewer::AddEngine Scene::SetEngines
 */
template <class WORLD> void Viewer<WORLD>::SetEngines(Engine* simroot)
{
	Engine* oldroot = m_SimRoot->First();
	if (simroot == NULL)
		return;
	if (oldroot)
	{
		oldroot->Replace(simroot);
		oldroot->Delete();
	}
	else
		m_SimRoot->Append(simroot);
}

template <class WORLD> Engine* Viewer<WORLD>::GetEngines() const
{
	if (m_SimRoot.IsNull())
		return NULL;
	Engine* eng = (Engine*) m_SimRoot->First();
	if (eng)
		return eng;
	return GetStatLog();
}

/*!
 * @fn Engine* Viewer<WORLD>::MakeEngines()
 *
 * Called during initialization to create the application-specific
 * simulation tree if one has not already been established by
 * Viewer::SetEngines. The default implementation makes this tree:
 * @code
 *	VXStatusLog viewer.statlog
 *		Scriptor viewer.camera.scriptor
 *			Animator -> camera
 *		Transformer viewer.camera.xform -> camera
 *			Flyer viewer.camera.fly
 *				JoyStick
 * @endcode
 *
 * @see Viewer::SetEngines Viewer::AddEngine Scene::SetEngines
 *
 */
template <class WORLD> Engine* Viewer<WORLD>::MakeEngines()
{
	Engine*	oldnav = GetNavigator();
	Messenger* mess = GetMessenger();

/*
 * Create a navigator and status log if not already there.
 * If there is a joystick available, use it.
 * The status log will be the root of the default simulation tree,
 * with the navigator and scriptor as children
 */
	if (m_SimRoot.IsNull())
		m_SimRoot = new Engine;
	m_SimRoot->SetName(TEXT("scene.root"));
	if (GetStatLog() == NULL)
	{
		FrameStats* tmp = new FrameStats();
		m_StatLog = tmp;
		if (!m_StatLogFile.IsEmpty())
		{
			tmp->OpenLog(m_StatLogFile, TEXT("frame"));
			InitStats();
		}
		mess->Define(TEXT("viewer.statlog"), (const SharedObj*) tmp);
	}
	if (m_Scriptor.IsNull())
	{
		Scriptor* scriptor = new Scriptor;
		m_Scriptor = scriptor;
		m_StatLog->Append(scriptor);
		mess->Define(TEXT("viewer.camera.scriptor"), (const SharedObj*) scriptor);
	}
	if (oldnav)
	{
		if (oldnav->Parent() == NULL)
			m_StatLog->Append(oldnav);
	}
	else
	{
		Transformer* nav = new Transformer;
		m_Flyer = new Flyer();
		m_Flyer->SetSpeed(1.0f);
		nav->Append(m_Flyer);
		SetNavigator(nav);
		mess->Define(TEXT("viewer.camera.xform"), nav);
		mess->Define(TEXT("viewer.camera.flyer"), m_Flyer);
	}
#ifdef VX_JOYFLYER
	if (JoyStick::GetJoyInfo())
	{
		JoyStick* joy = new JoyStick(Event::NAVIGATE);
		joy->SetFlags(SharedObj::DOEVENTS);
		joy->SetEventCode(Event::NAVINPUT, MouseEvent::LEFT | MouseEvent::RIGHT);
		mess->Define("viewer.camera.joystick", joy);
		m_Navigator->Append(joy);
	}
#endif
	return m_StatLog;
}

/*!
 * @fn void Viewer<WORLD>::AddEngine(Engine* eng)
 *	
 * Add another engine to the application-specific simulation tree.
 * The engine is added as the first child of the engine returned
 * by GetEngines(). If there are no engines in the simulation tree,
 * Viewer::MakeEngines is called to first construct the default
 * tree and then the input engine is appended.
 *
 * @see Viewer::SetEngines Viewer::MakeEngines
 */
template <class WORLD> void Viewer<WORLD>::AddEngine(Engine* eng)
{
	Engine* oldroot = GetEngines();
	if (oldroot == NULL)
	{
		SetEngines(oldroot = MakeEngines());
		if (oldroot == NULL)
		{
			SetEngines(eng);
			return;
		}
	}
	oldroot->Append(eng);
}

/*!
 * @fn void Viewer<WORLD>::SetModels(Model* sceneroot)
 *
 * Establishes a scene graph that is application-specific
 * to attach to an input scene after it is loaded. This hierarchy
 * could contain default lights, sprites for displaying statistics, 
 * 3D cursors, etc. The default model tree contains a single directional light.
 * If an initial scene graph has already been constructed
 * by Viewer::MakeModels, this new one will replace it.
 *
 * @see Viewer::SetEngines Scene::SetModels
 */
template <class WORLD> void Viewer<WORLD>::SetModels(Model* sceneroot)
{
	Model* oldroot = m_SceneRoot->First();

	if (sceneroot == NULL)
		return;
	if (oldroot == sceneroot)
		return;
	if (oldroot)
		oldroot->Replace(sceneroot);
	else
		m_SceneRoot->Append(sceneroot);
}

template <class WORLD> Model* Viewer<WORLD>::GetModels() const
{
	if (m_SceneRoot.IsNull())
		return NULL;
	return (Model*) m_SceneRoot->First();
}

template <class WORLD> Model* Viewer<WORLD>::GetUserModels() const
{
	Model* root;

	if (m_SceneRoot.IsNull())
		return NULL;
	root = (Model*) m_SceneRoot->First();
	if (root)
		return root->Next();
	return NULL;
}

template <class WORLD> Engine* Viewer<WORLD>::GetUserEngines() const
{
	Engine* root;

	if (m_SimRoot.IsNull())
		return NULL;
	root = (Engine*) m_SimRoot->First();
	if (root)
		return root->Next();
	return NULL;
}

/*!
 * @fn Model* Viewer<WORLD>::MakeModels()
 *
 * alled during initialization to create the default scene graph
 * if one has not already been established by
 * Viewer::SetModels. The default implementation makes a tree
 * with a directional light and a sprite for displaying status:
 * @code
 *	Model viewer.root
 *		Light viewer.sunlight
 *		Texture viewer.statimage
 * @endcode
 *
 * @see Viewer::SetModels Viewer::AddModel Scene::SetModels
 */
template <class WORLD> Model* Viewer<WORLD>::MakeModels()
{
	Model*	root = new Model;
	Light*	light = new DirectLight;
	Sprite*	sprite;
	Messenger* mess = GetMessenger();

	if (m_SceneRoot.IsNull())
		m_SceneRoot = new Model;
	m_SceneRoot->SetName(TEXT("scene.root"));
	root->SetName(TEXT("viewer.root"));
	m_SceneRoot->Append(root);
	root->SetCulling(false);
	light->Translate(1000.0f, 1000.0f, 1000.0f);
	light->Turn(Model::YAXIS, -PI/4.0f);
	light->Turn(Model::XAXIS, -PI/4.0f);
	mess->Define(TEXT("viewer.sunlight"), light);
//	light->SetActive(false);
	root->Append(light);

	sprite = new Sprite;
	m_FrameStats = sprite;
	mess->Define(TEXT("viewer.statimage"), (const SharedObj*) sprite);
	sprite->SetActive(false);
	sprite->SetUpperLeft(0, 0);
	sprite->SetLowerRight(1.0f, 1.0f);
//	sprite->SetUpperLeft(0.7f, 0);
//	sprite->SetLowerRight(1.0f, 0.1f);
	if (WORLD::GetMediaDir())
	{
		Core::String font(WORLD::GetMediaDir());
		font = font + TEXT("/default.txf");
		sprite->SetTextFont(font);
	}
	sprite->SetText(TEXT("Vixen"));
	sprite->SetForeColor(Col4(0.4f, 0.4f, 0.8f));
	root->Append(sprite);
	return root;
}

/*!
 * @fn void Viewer<WORLD>::AddModel(Model* mod)
 *	
 * Add another model to the default scene graph. The model is added
 * as the first child of the root returned by GetModels();
 * If there are no models in the simulation tree,
 * Viewer::MakeModels is called to first construct the default
 * tree and then the input model is appended.
 *
 * @see Viewer::SetModels Viewer::MakeModels
 */
template <class WORLD> void Viewer<WORLD>::AddModel(Model* mod)
{
	Model* oldroot = m_SceneRoot->First();
	if (oldroot == NULL)
	{
		SetModels(oldroot = MakeModels());
		if (oldroot == NULL)
		{
			SetModels(mod);
			return;
		}
	}
	oldroot->Append(mod);
}

/****
 *
 * InitStats
 * Make stat counter and attach to scene
 *
 ****/
template <class WORLD> bool Viewer<WORLD>::InitStats()
{
	FrameStats*	statlog = (FrameStats*) GetStatLog();

	if ((statlog == NULL) || !statlog->IsClass(VX_FrameStats))
		return false;
	statlog->AddProperty(TEXT("fps"), STAT_FrameRate);
	statlog->EnableProp(STAT_FrameRate);
	statlog->SetDuration(1);
	statlog->SetActive(true);
	statlog->SetTarget((Sprite*) m_FrameStats);
	m_FrameStats->SetActive(true);
	return true;
}

template <class WORLD> Scene* Viewer<WORLD>::MakeScene(const TCHAR* filename)
{
	if (filename == NULL)
	{
		if (GetModels() == NULL)
			SetModels(MakeModels());
		if (GetEngines() == NULL)
			SetEngines(MakeEngines());
	}
	return WORLD::MakeScene(filename);
}


template <class WORLD> void Viewer<WORLD>::SetScene(Scene* scene, Window win)
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_World3D, World3D::WORLD3D_SetScene) << this << scene << int64(win);
	VX_STREAM_ASYNC_END();

	Model*			oldmodels;				// models from previous scene
	Engine*			oldengines;				// engines from previous scene
	Engine*			simroot = scene->GetEngines();
	Model*			sceneroot = scene->GetModels();
	Sphere			bsp;

/*
 * Clean up old scene graph from previous scene.
 * It will be the second child of m_SceneRoot.
 * If m_SceneRoot has no children, we make the default
 * models and add them to m_SceneRoot.
 */
	scene->GetBound(&bsp);				// bounding volume of scene w/o extra models
	oldmodels = m_SceneRoot->GetAt(1);
	if (oldmodels)						// delete previous scene's models
		oldmodels->Remove();
	else if (m_SceneRoot->First() == NULL)
		SetModels(MakeModels());		// make default models
/*
 * Remove the scene graph from the input scene and append it to m_SceneRoot,
 * making it the second child. m_SceneRoot becomes the real root of
 * the scene graph for the new scene 
 */
	Ref<Scene> newroot(sceneroot);
	if (sceneroot && (sceneroot != m_SceneRoot))
	{
		sceneroot->Remove(Group::UNLINK_NOFREE);
		m_SceneRoot->Append(sceneroot);
	}
	scene->SetModels(m_SceneRoot);
/*
 * If the camera if not part of scene, add two default lights
 * since it is unlikely lighting has been added
 */
	Light*	light = new DirectLight;
	Camera*	cam = scene->GetCamera();

	light->SetName(TEXT("viewer.headlight"));
	cam->Append(light);

	if (!cam->GetName() && sceneroot)
	{
		light = (Light*) m_SceneRoot->Find(TEXT("viewer.sunlight"), Group::FIND_DESCEND | Group::FIND_EXACT);
		if (light)
			light->SetActive(true);
		sceneroot->Append(cam);
		cam->SetFOV(1.2f);
		cam->SetName(TEXT("viewer.camera"));
		scene->ShowAll();
	}
	else
		light->SetActive(false);
/*
 * Clean up old simulation tree from previous scene.
 * It will be the second child of m_SimRoot.
 * If m_SimRoot has no children, we make the default
 * engines and add them to m_SimRoot.
 */
	oldengines = m_SimRoot->GetAt(1);
	if (oldengines)						// delete previous scene's engines
		oldengines->Remove();
	else if (m_SimRoot->First() == NULL)
		SetEngines(MakeEngines());		// make default engines
/*
 * If the input scene has a simulation tree, append it to m_SimRoot,
 * making it the second child. m_SimRoot becomes the real root of
 * the simulation tree for the new scene 
 */
	if (simroot && (simroot != m_SimRoot))
	{
		simroot->Remove(Group::UNLINK_NOFREE);
		m_SimRoot->Append(simroot);
	}
	scene->SetEngines(m_SimRoot);
	Engine*	nav = m_Navigator;
	if (nav && sceneroot)
	{	
		if (!m_Flyer.IsNull())
		{
			Sphere bsp;
			sceneroot->GetBound(&bsp);
			m_Flyer->SetScale(bsp.Radius);
		}
		AttachNav(cam);
	}
#if 1
	if (!m_FrameStats.IsNull() && (m_FrameStats->Parent() != cam))
	{
		m_FrameStats->Remove(Group::UNLINK_NOFREE);
		cam->Append(m_FrameStats);
	}
#endif
	Scriptor*	scp = m_Scriptor;

	if (scp && GetScriptFile())
	{
		scp->LoadScript(GetScriptFile());
		SetScriptFile(NULL);
	}
	WORLD::SetScene(scene, win);
}

template <class WORLD> Scene* Viewer<WORLD>::NewScene(Window win, Scene* inscene)
{
	Scene*	scene;

	if ((inscene == NULL) &&
		(m_StereoEyeSep > 0))
		inscene = new StereoScene(Renderer::CreateRenderer());
	scene = WORLD::NewScene(win, inscene);
	if (m_IsFullScreen)
		scene->SetOptions(scene->GetOptions() & Scene::FULLSCREEN);
	if (m_StereoEyeSep > 0)
		scene->GetCamera()->SetEyeSep(m_StereoEyeSep);
	return scene;
}

template <class WORLD> void Viewer<WORLD>::AttachScene(Scene* scene)
{
	Messenger*	mess = GetMessenger();
	Camera*	cam = scene->GetCamera();
	Engine*	nav = NULL;
	Engine*	eng;

	m_SimRoot = scene->GetEngines();
	m_SceneRoot = scene->GetModels();
	if (m_SimRoot.IsNull())			// no engines?
	{
		m_SimRoot = new Engine;
		SetEngines(MakeEngines());	// make what we need
		if (!m_Navigator.IsNull())
			AttachNav(cam);
		scene->SetEngines(m_SimRoot);
		return;
	}
	eng = m_SimRoot->First();
	if (eng == NULL)
		return;

	GroupIter<Engine> iter((Engine*) m_SimRoot, Group::DEPTH_FIRST);
	while (eng = iter.Next())
	{
		SharedObj* target = eng->GetTarget();
		if (eng->IsClass(VX_Scriptor))
		{
			m_Scriptor = (Scriptor*) eng;
			continue;
		}
		if (eng->IsClass(VX_FrameStats))
		{
			m_StatLog = (FrameStats*) eng;
			m_FrameStats = (Sprite*) target;
			continue;
		}
		if (eng->IsClass(VX_Navigator))
		{
			mess->Observe(eng, Event::NAVINPUT, NULL);
			mess->Observe(eng, Event::MOUSE, NULL);
			mess->Observe(eng, Event::KEY, NULL);
			nav = eng;
			continue;
		}
		if (target != cam)
			continue;
		if (eng->IsClass(VX_Transformer))
		{
			if (nav == NULL)
			{
				mess->Observe(eng, Event::NAVINPUT, NULL);
				mess->Observe(eng, Event::MOUSE, NULL);
				mess->Observe(eng, Event::KEY, NULL);
			}
			else
				nav->SetFlags(SharedObj::DOEVENTS);
			nav = eng;
		}
	}
	m_Navigator = nav;
	mess->Observe(nav, Event::NAVIGATE, NULL);
	WORLD::AttachScene(scene);
}

/*
 * Attach the navigator to the camera and dispatch mouse events to it
 * Attach the scriptor to the camera, too. Make a bone animator for
 * the camera so that multiple camera paths may be blended.
 */
template <class WORLD> void Viewer<WORLD>::AttachNav(Camera* cam)
{
	Messenger*		mess = GetMessenger();
	Core::String	camname(cam->GetName());
	Engine*			nav = m_Navigator;
	Core::String	navname = nav->GetName();

	nav->SetTarget(cam);
	if (nav->IsClass(VX_Transformer))
	{
		FindTerrain((Transformer*) nav, navname);
		if (navname.Find(TEXT(".xform")) > 0)
		{
			Scriptor* scriptor = m_Scriptor;
			nav->SetName(camname + TEXT(".xform"));
			nav->SetFlags(SharedObj::DOEVENTS);
			mess->Define(camname + TEXT(".scriptor"), (const SharedObj*) scriptor);
			if (!m_Scriptor.IsNull())
			{
				Animator* anim = new BoneAnimator;
				m_Scriptor->SetTarget(cam);
				m_Scriptor->Append(anim);
				mess->Define(camname + TEXT(".xform.anim"), anim);
				anim->SetTarget(cam);
				anim->SetRootEngine(nav);
			}
		}
	}
	mess->Attach(cam);
	mess->Attach(nav);
	mess->Observe(nav, Event::NAVINPUT, NULL);
	mess->Observe(nav, Event::NAVIGATE, NULL);
}

template <class WORLD> void Viewer<WORLD>::FindTerrain(Transformer* navroot, const TCHAR* targname) 
{
	const Shape*		mod = (const Shape*) Find(TEXT("@terr*"));
	TerrainCollider*	tcoll = (TerrainCollider*) navroot->Find(TEXT(".terraincollider"), Group::FIND_CHILD | Group::FIND_END);

	if (tcoll != NULL)
		return;
	VX_ASSERT(navroot->IsClass(VX_Transformer));
	if (mod && mod->IsClass(VX_Shape))
	{
		Core::String name(targname);
		tcoll = new TerrainCollider;
		tcoll->BuildTerrain(mod, 5, 5);
		tcoll->SetHeight(5.0f);
		name += TEXT(".terraincollider");
		GetMessenger()->Define(name, tcoll);
		navroot->Append(tcoll);
	}
}

template <class WORLD> void Viewer<WORLD>::SetCamera(Camera* cam)
{
	Scriptor*	scp = GetScriptor();
	Camera*		oldcam = (Camera*) (SharedObj*) m_Scriptor->GetTarget();
	GroupIter<Engine> iter((Engine*) m_StatLog);
	Engine* e;

	if ((oldcam == NULL) || !oldcam->IsClass(VX_Camera))
		oldcam = GetMainScene()->GetCamera();
	GetMainScene()->SetCamera(cam);
	scp->Lock();
	while (e = iter.Next())	
		if (e->GetTarget() == oldcam)
		{
			e->SetTarget(cam);
			if (e->IsClass(VX_Transformer))
				((Transformer*) e)->SetTransform(cam->GetTransform());
		}
	scp->Unlock();
}

template <class WORLD> SharedObj* Viewer<WORLD>::Find(const TCHAR* objname, const Scene* scene) const
{
	TCHAR			namebuf[VX_MaxPath];
	TCHAR*			p;
	const TCHAR*	start;
	Messenger*		mess = GetMessenger();
	SharedObj*		obj;
	const TCHAR*	scenename = NULL;

	if (scene)
		scenename = scene->GetName();
	if (mess == NULL)
		return NULL;
	if (WORLD::GetFileName() == NULL)
		return NULL;
	if (STRRCHR(objname, TEXT('.')) == 0)			// no dot in model name?
	{
		start = WORLD::GetFileName();
		p = (TCHAR*) start + STRLEN(start);	// -> at the dot
		while (--p > start)					// find start of file name
			if ((*p == TEXT('\\')) ||
				(*p == TEXT('/')) ||
				(*p == TEXT(':')))
			{
				++p;
				break;
			}
		STRCPY(namebuf, p);					// copy only filename
		if (p = STRRCHR(namebuf, TEXT('.')))// dot in filename?
			*(p + 1) = 0;
		else
			STRCAT(namebuf, TEXT("."));
		STRCAT(namebuf, objname);			// add object name at end
	}
	else
		STRCPY(namebuf, objname);
	obj = mess->Find(namebuf);				// look for object
	if (obj)								// found on output stream?
		return obj;
	obj = WORLD::GetLoader()->Find(namebuf, scenename);	// look on app input stream
	return obj;
}


template <class WORLD> ObjArray* Viewer<WORLD>::FindAll(const TCHAR* objname, const Scene* scene) const
{
	TCHAR			namebuf[VX_MaxPath];
	TCHAR*			p;
	const TCHAR*		start;
	Messenger*	mess = GetMessenger();
	ObjArray*		sobjs;
	ObjArray*		fobjs;

	if (mess == NULL)
		return NULL;
	if (WORLD::GetFileName() == NULL)
		return NULL;
	if (STRRCHR(objname, TEXT('.')) == 0)	// no dot in model name?
	{
		start = WORLD::GetFileName();
		p = (TCHAR*) start + strlen(start);	// -> at the dot
		while (--p > start)					// find start of file name
			if ((*p == TEXT('\\')) ||
				(*p == TEXT('/')) ||
				(*p == TEXT(':')))
			{
				++p;
				break;
			}
		STRCPY(namebuf, p);					// copy only filename
		if (p = STRRCHR(namebuf, TEXT('.')))// dot in filename?
			*(p + 1) = 0;
		else
			STRCAT(namebuf, TEXT("."));
		STRCAT(namebuf, objname);			// add object name at end
	}
	else
		STRCPY(namebuf, objname);
	sobjs = mess->FindAll(namebuf);			// look in scene output stream
	fobjs = WORLD::GetLoader()->FindAll(namebuf);	// look on app input stream
	if (sobjs)
	{
		if (fobjs)
			sobjs->Merge(*fobjs);
	}
	else sobjs = fobjs;
	return sobjs;
}

template <class WORLD> void Viewer<WORLD>::Apply(Scene* scene, int32 attr, int32 val)
{
	Model* root = scene->GetModels();
	if (root)
		Appearance::Apply(root, attr, val);
}


template <class WORLD> bool Viewer<WORLD>::Do(Messenger& s, int op)
{
	TCHAR	filename[VX_MaxPath];
	SharedObj*	obj;
	Core::String tmp;

	switch (op)
	{
#ifdef VX_USE_SCRIPTOR
		case VIEWER_SetScriptFile:
		s >> filename;
		tmp = filename;
		SetScriptFile(tmp);
		break;
#endif

		case VIEWER_SetEngines:
		s >> obj;
		SetEngines((Engine*) obj);
		break;

		case VIEWER_SetModels:
		s >> obj;
		SetModels((Model*) obj);
		break;

		case VIEWER_AddEngine:
		s >> obj;
		AddEngine((Engine*) obj);
		break;

		case VIEWER_AddModel:
		s >> obj;
		AddModel((Model*) obj);
		break;

		default:
		return WORLD::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << SharedObj::ClassName() << "::"
					   << Viewer<WORLD>::OpcodeNames[op - VIEWER_SetScriptFile]
					   << " " << this);
#endif
	return true;
}
