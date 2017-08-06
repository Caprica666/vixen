#include "vixen.h"
#pragma optimize ("", off)	// VS2012 hangs during code generation if this file is optimized

namespace Vixen {
static const TCHAR* opnames[] = { TEXT("SetScene") };

const TCHAR** World3D::DoNames = opnames;

VX_IMPLEMENT_CLASSID(World3D, World, VX_World3D);

VX_IMPLEMENT_CLASSID(SceneLoader, FileLoader, VX_Loader);

/*****
 *
 *	Forces all of the built-in classes to be referenced.
 *	(If we don't do this, the entries in the classes table are not
 *	initialized unless the caller explicity references the classes.
 *	This doesn't work if you are reading in a binary file with classes
 *	in it that your program hasn't referenced)
 *
 ****/
World3D::World3D() : World()
{
	REGISTER_CLASS(Appearance);
	REGISTER_CLASS(Scene);
	REGISTER_CLASS(Shape);
	REGISTER_CLASS(TriMesh);
	REGISTER_CLASS(Texture);
	REGISTER_CLASS(Light);
	REGISTER_CLASS(Camera);
	REGISTER_CLASS(Material);
	REGISTER_CLASS(Sampler);
	REGISTER_CLASS(Deformer);
	REGISTER_CLASS(Skin);
	REGISTER_CLASS(Skeleton);
	REGISTER_CLASS(MeshAnimator);

	RawInput = false;
	WinRect.Set(0,0,0,0);
	Scene::Debug = 0;
}

bool World3D::OnInit()
{
//
// Initialize loader, messenger and observations
//
	Messenger*	disp = m_Messenger;
	FileLoader*	load = m_LoadQueue;

	if (load == NULL)
	{
		m_LoadQueue = load = new SceneLoader;
		SetFileName(m_FileName);
	}
	if (disp == NULL)
		m_Messenger = disp = new BufMessenger;
	if (!World::OnInit())
		return false;
	Matrix::GetIdentity();
	vixen_debug.precision(3);
//
// Initialize system event names
//
	Event::SetName(Event::LOAD_SCENE, TEXT("LoadScene"));
	Event::SetName(Event::MOUSE, TEXT("Mouse"));
	Event::SetName(Event::KEY, TEXT("Key"));
	Event::SetName(Event::START, TEXT("Start"));
	Event::SetName(Event::STOP, TEXT("Stop"));
	Event::SetName(Event::RESET, TEXT("Reset"));
	Event::SetName(Event::PICK, TEXT("Pick"));
	Event::SetName(Event::NOPICK, TEXT("NoPick"));
	Event::SetName(Event::SELECT, TEXT("Select"));
	Event::SetName(Event::DESELECT, TEXT("DeSelect"));
	Event::SetName(Event::SET_TIME, TEXT("SetTime"));
	Event::SetName(Event::NAVIGATE, TEXT("Navigate"));
	Event::SetName(Event::NAVINPUT, TEXT("NavInput"));
	Event::SetName(Event::ATTACH_SCENE, TEXT("AttachScene"));
	Event::SetName(Event::ENTER, TEXT("Enter"));
	Event::SetName(Event::LEAVE, TEXT("Leave"));
	Event::SetName(Event::MIDI, TEXT("Midi"));
	Event::SetName(Event::EXEC, TEXT("Exec"));
	Event::SetName(Event::SCENE_CHANGE, TEXT("SceneChange"));
	Event::SetName(Event::SAVE_SCREEN, TEXT("SaveScreen"));
	Event::SetName(Event::CONNECT, TEXT("Connect"));
	Event::SetName(Event::ERROR, TEXT("Error"));
	Event::SetName(Event::TRI_HIT, TEXT("TriHit"));
	Event::SetName(Event::TRACK, TEXT("Track"));
	Event::SetName(Event::SCENE_INIT, TEXT("SceneInit"));
//
// Initialize system event types
//
	Event::SetDataTypes(Event::LOAD_SCENE, TEXT("SO"));
	Event::SetDataTypes(Event::MOUSE, TEXT("FFI"));
	Event::SetDataTypes(Event::KEY, TEXT("II"));
	Event::SetDataTypes(Event::START, NULL);
	Event::SetDataTypes(Event::STOP, NULL);
	Event::SetDataTypes(Event::RESET, NULL);
	Event::SetDataTypes(Event::PICK, TEXT("O"));
	Event::SetDataTypes(Event::NOPICK, NULL);
	Event::SetDataTypes(Event::SELECT, TEXT("O"));
	Event::SetDataTypes(Event::DESELECT, TEXT("O"));
	Event::SetDataTypes(Event::SET_TIME, TEXT("I"));
	Event::SetDataTypes(Event::SCENE_CHANGE, TEXT("OI"));
	Event::SetDataTypes(Event::COLLIDE, TEXT("I"));
	Event::SetDataTypes(Event::NAVIGATE, TEXT("IFFFFFFFFFFF"));
	Event::SetDataTypes(Event::NAVINPUT, TEXT("FFFII"));
	Event::SetDataTypes(Event::ATTACH_SCENE, NULL);
	Event::SetDataTypes(Event::ENTER, TEXT("OOFFF"));
	Event::SetDataTypes(Event::LEAVE, TEXT("OOFFF"));
	Event::SetDataTypes(Event::MIDI, TEXT("II"));
	Event::SetDataTypes(Event::EXEC, TEXT("SSI"));
	Event::SetDataTypes(Event::SAVE_SCREEN, TEXT("SS"));
	Event::SetDataTypes(Event::CONNECT, TEXT("I"));
	Event::SetDataTypes(Event::ERROR, TEXT("IIS"));
	Event::SetDataTypes(Event::TRI_HIT, TEXT("OFFFFII"));
	Event::SetDataTypes(Event::TRACK, TEXT("IIIFFFFFFFS"));
//
// Initialize file types
//
	load->SetFileFunc(TEXT("vix"), &SceneLoader::ReadScene, Event::LOAD_SCENE);
//	load->SetFileFunc(TEXT("bvh"), &BVHLoader::ReadAnim, Event::LOAD_SCENE);
	load->SetFileFunc(TEXT("scp"), &FileLoader::ReadText, Event::LOAD_TEXT);
	load->SetFileFunc(TEXT("txt"), &FileLoader::ReadText, Event::LOAD_TEXT);
	load->SetFileFunc(TEXT("cso"), &FileLoader::ReadBinary, Event::LOAD_DATA);
	load->SetFileFunc(TEXT("bmp"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("txf"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("png"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("jpg"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("dds"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("tga"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("tif"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	load->SetFileFunc(TEXT("sgi"), &Bitmap::ReadFile, Event::LOAD_IMAGE);
	m_Messenger->t_NoLog = false;			// enable transaction logging
	Bitmap::Startup();
	disp->Observe(this, Event::KEY, NULL);
	disp->Observe(this, Event::MOUSE, NULL);
	disp->Observe(this, Event::SCENE_CHANGE, NULL);
	disp->Observe(this, Event::ATTACH_SCENE, NULL);
	disp->Observe(this, Event::LOAD_SCENE, NULL);
	disp->Observe(this, Event::ERROR, NULL);
	MakeLock();
	return true;
}


/*!
 * @fn Scene* World3D::NewScene(Window win, Scene* inscene)
 * @param win	window to associate with the display scene
 *				if NULL, no window is bound to scene
 *
 * Class factory for the display scene. This function
 * is called once by the framework at startup to instantiate
 * a scene bound to a display (or capable of multi-threaded
 * operation). Usually, this scene is used throughout for
 * communicating with the display device. Changing the
 * current scene causes the existing display scene to
 * reference a new simulation tree and scene graph.
 *
 * Each port of the scene manager supplies a version of NewScene that
 * constructs a display scene bound to the appropriate device.
 * If you supply your own implementation for World3D::NewScene,
 * the library version will not be used. You may want to do this
 * to enable multi-threaded rendering.
 *
 * @return pointer to an empty display scene attached to the window
 *
 * @see World3D::SetScene Scene World3D::MakeScene
 */
Scene* World3D::NewScene(Window win, Scene* inscene)
{
#ifdef VX_NOTHREAD
	Scene*		display_scene = (Scene*) GetScene(win);
	Scene*		scene = inscene;

	if (display_scene)
	{
		if (scene == NULL)
			scene =  new Scene(*display_scene);
		display_scene->Append(scene);
		return scene;
	}
	if (scene == NULL)
		inscene = scene = new Scene(Renderer::CreateRenderer());
#else
	DualScene* display_scene = (DualScene*) GetScene(win);
	DualScene*	scene;
	if (display_scene)
	{
		if (inscene)
		{
			display_scene->Append(inscene);
			return inscene;
		}
		else
		{
			scene =  new DualScene(*display_scene);
			display_scene->Append(scene);
			return scene;
		}
	}
	if (inscene == NULL)
	{
		scene = new DualScene(Renderer::CreateRenderer());
		scene->SetNumThreads(0);
		inscene = scene;
	}
#endif
	inscene->SetOptions(Scene::CLEARALL | Scene::STATESORT | Scene::DOUBLEBUFFER);
	return inscene;
}

void World3D::OnExit()
{
	m_Scenes.Empty();
	World::OnExit();
	Bitmap::Shutdown();
	if (Matrix::s_IdentityMatrix)			// clean up identity matrix
	{
		Matrix::s_IdentityMatrix->Delete();
		Matrix::s_IdentityMatrix = NULL;
	}
	Scene::Shutdown();
}

/*!
 * @fn Scene *World3D::GetScene(Window win) const
 * @param win	Window to get the scene for. if NULL, the first
 *				scene created is returned
 *
 * @return scene associated with the given window.
 *
 * @see Scene::Get World3D::SetScene
 */
Scene *World3D::GetScene(Window win) const
{
	int n = (int) m_Scenes.GetSize();
	for (int i = 0; i <  n; ++i)
	{
		Scene* scene = (Scene*) (SharedObj*) m_Scenes[i];
		if (scene && (scene->GetWindow() == win))
			return scene;
	}
	if ((win != Window(NULL)) || (n <= 0))
		return NULL;
// NOLA: m_Scenes[0] does not work the same as GetAt(0) with Intel Compiler
//	return (Scene*) (SharedObj*) m_Scenes[0];
	return (Scene*) (SharedObj*) m_Scenes.GetAt(0);
}

void World3D::AddScene(Scene *newscene, Window win)
{
	ObjectLock	lock(this);
	int n = (int) m_Scenes.GetSize();
	for (int i = 0; i < n; ++i)
	{
		Scene* scene = (Scene*) (SharedObj*) m_Scenes[i];
		if (scene == newscene)
			return;
		if (scene && (scene->GetWindow() == win))
		{
			m_Scenes.SetAt(i, newscene);
			return;
		}
	}
// NOLA: m_Scene.Append(newscene) does not work with Intel compiler
	m_Scenes.SetAt(n, newscene);
	VX_ASSERT(GetScene() == newscene);
}

bool World3D::KillScene(Scene *scene)
{
	ObjectLock	lock(this);
	for (intptr i = 0; i < m_Scenes.GetSize(); ++i)
		if (m_Scenes.GetAt(i) == scene)
		{
			m_Scenes.RemoveAt(i);
			--i;
		}
	if (m_Scenes.GetSize() > 0)
		return false;
	return true;
}

/*!
 * @fn bool World3D::Run(Window arg)
 * @param arg	window handle to use for primary display scene
 *
 * Adds a new window to the world and makes the primary display scene
 * for the window which starts up display thread(s).
 * If the application has been given a filename, the specified
 * content file is loaded.
 *
 * This routine is not automatically called by the system, it must be initiated
 * by application code.
 *
 * @return \b true if window and scene successfully made, else \b false
 *
 * @see Scene::SetWindow World::SetFileName World::Run World3D::MakeScene
 */
bool World3D::Run(Window arg)
{
	Window	win = arg;
	Scene*	scene = GetScene(win);	// get scene for this window
	int		nscenes = (int) m_Scenes.GetSize();
	ObjRef	inscene;
	Event*	ev;

	if (scene == NULL)					// no scene for input window?
		scene = GetScene(0);			// was any scene created?
	if (scene)							// make sure the scene has a window
	{
		if (scene->GetWindow() == 0)
			scene->SetWindow(win);		// start scene threads
	}
	else								// no scene
	{
		scene = NewScene(win);			// make a new scene
		if (scene == NULL)				// can't make a scene?
			return false;
		if (win && !scene->SetWindow(win))
		{
			scene->Delete();
			return false;
		}
	}
	ev = new Event(Event::SCENE_INIT, sizeof(ObjRef), scene);
	ev->Log();
	World::Run(arg);					// save the window handle
	if (DoAsyncLoad && m_FileName[0])	// load a content file?
	{
		SetFileName(m_FileName);
		return LoadAsync(m_FileName, scene);
	}
	inscene = MakeScene();				// blocking load
	if (!inscene.IsNull())
	{
		LoadSceneEvent* event = new LoadSceneEvent(Event::LOAD_SCENE);
		event->Object = inscene;
		event->Sender = scene;
		event->FileName = m_FileName;
		event->Log();
		return true;
	}
	m_Messenger->Flush();
	m_Messenger->Load();				// process any error events
	return false;
}

/*!
 * @fn void World3D::Stop()
 *
 * Initiates shutdown of all loading and display threads for all
 * scenes attached to this world. This routine does not return until
 * all loading, scene traversal and rendering threads have stopped.
 *
 * @see World3D::Run World::Stop
 */
void World3D::Stop()
{
	/*
	 * Lock around streaming only. We don't want to hold a lock
	 * when shutting the system down.
	 */
	{
		VX_STREAM_BEGIN( s )
			*s << OP(VX_World3D, WORLD_Stop) << this;
		VX_STREAM_END( );
	}

	Scene*		scene;
	int			nscenes = (int) m_Scenes.GetSize();

	for (int i = 0; i < nscenes; ++i)
	{
		scene = (Scene*) (SharedObj*) m_Scenes[i];
		if (scene)								// have another scene?
			scene->SetWindow(NULL);				// stop its display
	}
	OnExit();
}


/*!
 *@fn void World3D::SetScene(Scene *scene, Window win)
 * @param scene	New scene to display, NULL turns off display
 * @param win	Window whose scene to update. If NULL, the first
 *				scene created is updated
 *
 * The display scene associated with the input window is set
 * to reference the input scene (which is not used further
 * and is not garbage collected).
 * If the application has no display scene, nothing is done.
 * If the input scene is NULL, the display scene is removed
 * from the application and freed.
 *
 * This routine is called from the display thread to change the current scene.
 * If it is called directly from another thread, the command is logged by
 * the messenger and deferred until the start of the next frame.
 *
 * @see World3D::MakeScene World3D::NewScene
 */
void World3D::SetScene(Scene *scene, Window win)
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_World3D, WORLD3D_SetScene) << this << scene << int64(win);
	VX_STREAM_ASYNC_END();

	Scene*	display_scene;

	if (win == Window(NULL))
		display_scene = (Scene*) (SharedObj*) m_Scenes[0];
	else
	{
		for (intptr i = 0; i < m_Scenes.GetSize(); ++i)
		{
			display_scene = (Scene*) (SharedObj*) m_Scenes[i];
			if (display_scene->GetWindow() == win)
				break;
		}
	}
	if (display_scene == NULL)
		return;
	MergeLoaderDict(scene);
	display_scene->Copy(scene);
	SceneEvent* e = new SceneEvent(Event::SCENE_CHANGE, scene, display_scene);
	e->WindowHandle = (intptr) win;
	e->Log();
}


void World3D::AttachScene(Scene *scene)
{
	Scene*	display_scene;

	ObjectLock	lock(this);
	if (m_Scenes.GetSize() <= 0)
		return;
	display_scene = (Scene*) (SharedObj*) m_Scenes.GetAt(0);
	if ((display_scene == NULL) || (scene == display_scene))
		return;
	display_scene->Copy(scene);
}

/*!
 * @fn Scene* World3D::MakeScene(const TCHAR* filename)
 * @param filename	Name of file to load. if non-null, this file
 *					will be loaded. If not, the application filename
 *					defined by World::SetFileName is tried.
 *
 * Called by the framework at startup to establish
 * the initial scene to display. The input argument
 * has the name of the file or directory specified on the
 * command line. The default implementation tries to load
 * the input as a .Vixen binary file and returns a
 * 	scene from the file if it can find one.
 *
 * MakeScene will issue a blocking read that waits until all
 * of the geometry and associated textures have been loaded.
 * If you want to load an entire scene from a file and
 * replace the current scene, the LoadAsync function will do
 * this without blocking, but it does not return the scene
 * so you cannot do further updates after loading it.
 *
 * @return scene read from input file or NULL if scene cannot be loaded
 *
 * @see World3D::SetScene Scene World3D::NewScene World::LoadAsync
 */
Scene* World3D::MakeScene(const TCHAR* filename)
{
	if (filename == NULL)
	{
		filename = m_FileName;
		if (filename == NULL)
			return NULL;
	}
	if (*filename)
		m_LoadQueue->ReadFile(filename, NULL);
	return NULL;
}

/*!
 *@fn Event* World3D::MakeEvent(int code)
 * @param code	Event code to make event for, the default MakeEvent understands
 *				Event::MOUSE, Event::KEY and EVENT_Load. For other events, the
 *				default behavior is to create an event and initialize the
 *				code only.
 *
 * Class factory for all the events handled by this world.
 * When an event is produced by an object, its code and data
 * are logged to the current messenger and then dispatched to all
 * the observers of that event code. This function creates an
 * empty event class for the input event code. If you define
 * your own event class and override any functions, you must also
 * override MakeEvent to ensure that the system can construct the right
 * event class to handle your new event code.
 *
 * @return pointer to event constructed or NULL on error
 *
 * @see Messenger::Observe SharedObj::OnEvent Event
 */
Event* World3D::MakeEvent(int code)
{
	Event*	e;

	switch (code)
	{
		case Event::ERROR:			e = new ErrorEvent; break;
		case Event::MOUSE:			e = new MouseEvent; break;
		case Event::KEY:			e = new KeyEvent; break;
		case Event::NAVINPUT:		e = new NavInputEvent; break;
		case Event::NAVIGATE:		e = new NavigateEvent; break;
		case Event::SET_TIME:		e = new FrameEvent; break;
		case Event::SCENE_CHANGE:	e = new SceneEvent(code); break;
		case Event::EXEC:
		case Event::LOAD_IMAGE:
		case Event::LOAD_SCENE:		e = new LoadSceneEvent(code); break;
		case Event::LOAD_TEXT:		e = new LoadTextEvent; break;
		case Event::LOAD_DATA:		e = new LoadDataEvent; break;
		case Event::ENTER:
		case Event::LEAVE:			e = new TriggerEvent(code); break;
		case Event::TRACK:			e = new TrackEvent(code); break;
		case Event::PICK:
		case Event::NOPICK:
		case Event::SELECT:
		case Event::DESELECT:		e = new PickEvent(code); break;
		default:					e = new Event(code); break;
	}
	return e;
}

/*!
 * @fn bool World3D::OnEvent(Event* event)
 * @param event	event to process
 *
 * Default application event handler which responds to mouse, keyboard and
 * load events (codes Event::MOUSE, Event::KEY, EVENT_Load). Override this
 * function if you want the application to respond to other events.
 * Note that if you add your own event, you may have to override
 * World3D::MakeEvent if your new event type needs special initialization.
 *
 * @return \b true if event successfully handled, \b false if event not understood
 *
 * @see Event SharedObj::OnEvent World3D:MakeEvent Messenger::Observe
 */
bool World3D::OnEvent(Event* e)
{
	SceneEvent*	se;
	LoadSceneEvent*	le;
	Scene*		scene;

	switch (e->Code)
	{
		case Event::LOAD_SCENE:
		le = (LoadSceneEvent*) e;
		scene = (Scene*) (SharedObj*) le->Object;

		if (scene && scene->IsClass(VX_Scene) && le->Sender->IsClass(VX_Scene))
			SetScene(scene, ((Scene*) (SharedObj*) le->Sender)->GetWindow());
		else
			MergeLoaderDict(scene);
		break;

		case Event::ATTACH_SCENE:
		se = (SceneEvent*) e;
		scene = (Scene*) (SharedObj*) se->Sender;
		if (scene)
		{
			VX_ASSERT(scene->IsClass(VX_Scene));
			AttachScene(scene);
		}
		break;

		default:
		return false;
	}
	return true;
}

/*
 * Merge dictionary for loaded scene with the display scene's dictionary
 */
bool World3D::MergeLoaderDict(Scene* scene)
{
	Messenger*		stream = m_Messenger;
	NameTable*		curdict = stream->GetNameDict();
	NameTable*		newdict;
	SceneLoader*	load = GetLoader();
	const TCHAR*	name = scene->GetName();

	if (name)
	{
		newdict = load->GetSceneDict(name);
		if (newdict)
			curdict->Merge(*newdict);
		load->SetSceneDict(name, NULL);
		return true;
	}
	return false;
}

bool World3D::Do(Messenger& s, int op)
{
	int32	win;
	SharedObj*	scene;

	switch (op)
	{
		case WORLD3D_SetScene:
		s >> scene >> win;
		VX_ASSERT(scene && scene->IsClass(VX_Scene));
		SetScene((Scene*) scene, Window(win));
		break;

		default:
		return World::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << World3D::DoNames[op - WORLD3D_SetScene]
					   << " " << this);
#endif
	return true;
}

/*!
 * @fn bool SceneLoader::ReadScene(const TCHAR* filename, Core::Stream* instream, LoadEvent* event)
 * @param filename	name of scene file to load.
 * @param instream	stream to use for reading.
 * @param event		event to initialize if load is successful.
 *
 * Called by the loader to load a scene file.
 * ReadScene will issue a blocking read that waits until all
 * of the geometry and associated textures have been loaded.
 * If you want to load an entire scene from a file and
 * replace the current scene, the SceneLoader::Load function will do
 * this without blocking.
 *
 * @return \b true if load was successful, else \b false
 *
 * @see World::LoadAsync FileLoader::Load
 */
bool SceneLoader::ReadScene(const TCHAR* filename, Core::Stream* instream, LoadEvent* ev)
{
	LoadSceneEvent* event = (LoadSceneEvent*) ev;
	TCHAR			filebase[VX_MaxPath];
	TCHAR			dir[VX_MaxPath];
	Scene*			inscene = (Scene*) (SharedObj*) event->Object;
	size_t			len;
	FileMessenger	scenemaker;
	NameTable*		scenedict;
	const Model*	sceneroot;
	const Engine*	simroot;
	Messenger*		disp = GetMessenger();

	event->Code = Event::LOAD_SCENE;
	if (event->Sender.IsNull())
		event->Sender = GetMainScene();
	if (inscene)							// scene already loaded?
		return true;
	instream->ParseDirectory(filename, filebase, dir);
	instream->SetDirectory(dir);			// establish base directory
	len = STRLEN(filebase);
	scenemaker.SetInStream(instream);
	if (!scenemaker.Load())					// cannot load file?
		return false;						// no input scene
	instream->SetDirectory(NULL);
/*
 * Save the file scene's dictionary.
 * Closing the scene loader will delete this dictionary but we want to keep
 * it to merge with the display scene's dictionary later.
 */
	scenedict = scenemaker.GetNameDict();
	World3D::Get()->GetLoader()->SetSceneDict(filebase, scenedict);
/*
 * Define names in the current display scene's global dictionary for the
 * scene in the file, the simulation tree root and the scene graph
 */
	STRCAT(filebase, TEXT(".scene"));				// find the scene
	inscene = (Scene*) (SharedObj*) scenemaker.Find(filebase);
	if (inscene == NULL)							// found a scene?
	{
		STRCPY(filebase + len, TEXT(".simroot"));
		simroot = (const Engine*) scenemaker.Find(filebase);
		STRCPY(filebase + len, TEXT(".root"));
		sceneroot = (const Model*) scenemaker.Find(filebase);
		if (sceneroot != NULL)						// found object called filebase.root?
		{
			if (sceneroot->IsClass(VX_Model))		// we have a model
			{
				STRCPY(filebase + len, TEXT(".scene"));
				inscene = new Scene();				// make a new scene
				inscene->SetName(filebase);
				inscene->SetModels(sceneroot);		// attach the scene root
				inscene->SetEngines(simroot);		// attach the simulation root
			}
			else									// not a model?
			{
				event->Object = sceneroot;
				VX_TRACE(FileLoader::Debug, ("SceneLoader::ReadScene %s\n", (const char *) filebase));
				scenemaker.Close();
				return true;
			}
		}
		else if (simroot != NULL)
		{
			STRCPY(filebase + len, TEXT(".scene"));
			inscene = new Scene();				// make a new scene
			inscene->SetName(filebase);
			inscene->SetEngines(simroot);		// attach the simulation root
		}
		else
		{
			VX_ERROR(("SceneLoader::ReadScene %s ERROR: cannot find root\n", (const TCHAR*) filebase), false);
		}
		disp->Define(filebase, inscene);
	}
	else
	{
		sceneroot = ((Scene*) inscene)->GetModels();
		simroot = ((Scene*) inscene)->GetEngines();
	}
	VX_TRACE(FileLoader::Debug, ("SceneLoader::ReadScene %s\n", (const char *) filebase));
/*
 * Define simulation tree and scene tree root names in the global dictionary
 */
	const TCHAR* name;

	if (sceneroot)
	{
		name = sceneroot->GetName();
		if (name)
			disp->Define(name, sceneroot);
	}
	if (simroot)
	{
		name = simroot->GetName();
		if (name)
			disp->Define(name, simroot);
	}
	event->Object = inscene;
	scenemaker.Close();
	return true;
}

/*!
 * @fn NameTable* SceneLoader::GetSceneDict(const TCHAR* scenename) const
 * @param scenename	string name of scene, if NULL the last loaded scene dictionary is used
 *
 * Returns the dictionary for the specified scene. These dictionaries
 * are typically temporary and remain only until the scene load
 * event has been handled by an observer. The loader maintains these dictionaries
 * so that application code may look up objects by name while processing
 * a load event.
 *
 * @return dictionary if found or NULL if it does not exist
 *
 * @see SceneLoader::SetSceneDict
 */
NameTable* SceneLoader::GetSceneDict(const TCHAR* scenename) const
{
	TCHAR			namebuf[VX_MaxPath];
	ObjRef*		dictptr;
	TCHAR*			p;
	NameProp		np;

	STRCPY(namebuf, scenename);
	p = STRRCHR(namebuf, TEXT('.'));
	if (p)
		STRCPY(p, TEXT(".dict"));
	else
		STRCAT(namebuf, TEXT(".dict"));
	np = namebuf;
	dictptr = m_FileDict.Find(np);
	if (dictptr == NULL)
		return NULL;
	return (NameTable*) (SharedObj*) *dictptr;
}

/*!
 * @fn void SceneLoader::SetSceneDict(const TCHAR* scenename, NameTable* dict)
 * @param scenename	string name of scene
 * @param dict		dictionary to associate with the scene.
 *
 * Saves the dictionary under the scene's name. These dictionaries
 * are typically temporary and remain only until the scene load
 * event has been handled. The loader keeps them so that the
 * application may look up objects by name while processing
 * a load event.
 *
 * Setting the scene dictionary to NULL is the best way to get rid
 * of a scene file when it is no longer being referenced.
 *
 * @see SceneLoader::GetSceneDict
 */
void SceneLoader::SetSceneDict(const TCHAR* scenename, NameTable* dict)
{
	TCHAR		namebuf[VX_MaxPath];
	TCHAR*		p;
	NameProp	np;
	ObjectLock	lock(this);

	STRCPY(namebuf, scenename);
	p = STRRCHR(namebuf, TEXT('.'));
	if (p)
		STRCPY(p, TEXT(".dict"));
	else
		STRCAT(namebuf, TEXT(".dict"));
	np = namebuf;
	if (dict)
	{
		m_FileDict.Set(np, dict);
		m_LastDict = dict;
		dict->MakeLock();
	}
	else
	{
		if (m_LastDict)
			m_LastDict->KillLock();
		m_LastDict = NULL;
		m_FileDict.Remove(np);
	}
}


/*!
 * @fn SharedObj* SceneLoader::Find(const TCHAR* objname, const TCHAR* scenename)
 * @param objname	string name of object to retrieve, cannot be NULL
 * @param scenename	string name of scene, if NULL last scene loaded is used
 *
 * Finds the object with the given name in the dictionary for the
 * specified scene. Only named objects from scene files loaded but not
 * yet merged with the display scene are accessible.
 * If you are retrieving an object and it is within a hierarchy in the loaded file,
 * you must remove it from that hierarchy before you can put it into another.
 *
 * @return pointer to object found or NULL if object not there
 *
 * @see Messenger::Find
 */
SharedObj* SceneLoader::Find(const TCHAR* objname, const TCHAR* scenename)
{
	NameTable*	dict = m_LastDict;
	ObjRef*		ref;
	ObjectLock	lock(this);
	size_t		n;

	if (scenename)
		dict = GetSceneDict(scenename);
	if (dict == NULL)
		return NULL;
	if (objname == NULL)
		return NULL;
	n = STRLEN(objname);
	if ((objname[0] != '*') &&			// not a wildcard search?
		(objname[n - 1] != '*'))
		ref = dict->Find(objname);
	else
		ref = dict->FindWild(objname);
	if (ref)
		return *ref;
	return NULL;
}

/*!
 * @fn ObjArray* SceneLoader::FindAll(const TCHAR* objname, const TCHAR* scenename)
 * @param objname	string name of objects to retrieve. Only the objects whose names
 *					match this name (using "*" as a wildcard) are returned.
 * @param scenename	string name of scene, if NULL last scene loaded is used
 *
 * Asterisk is used as a special character which will match
 * anything. For example, "*ABC" will match any name that
 * ends in "ABC" and "ABC*" matches any name beginning with "ABC".
 * To match any name containing ABC, including at the beginning
 * or end, use "*ABC*".
 *
 * Only named objects from scene files loaded but not
 * yet merged with the display scene are accessible.
 *
 * @return array of objects found or NULL if no matches
 *
 * @see Messenger::FindAll SharedObj::SetName
 */
ObjArray* SceneLoader::FindAll(const TCHAR* objname, const TCHAR* scenename)
{
	NameTable*	dict = m_LastDict;

	if (scenename)
		dict = GetSceneDict(scenename);
	if (dict == NULL)
		return NULL;
	return dict->FindAll(objname);
}

/*!
 * @fn void SceneLoader::Unload(const TCHAR* filename)
 * @param filename	name of file to unload.
 *
 * When the loader loads a file, it caches the file objects in
 * memory so they can be used in simulation (even if they
 * are not displayed). This function frees the cached objects
 * if the file which are not being used in the scene.
 *
 * @see FileLoader::Load FileLoader::Unload
 */
void SceneLoader::Unload(const TCHAR* filename)
{
	TCHAR			basename[VX_MaxPath];
	Core::Stream::ParseDirectory(filename, basename);

	Messenger*		stream = GetMessenger();
	Core::String	lookfor(basename);
	ObjectLock		lock(this);
	ObjRef*			oref = m_FileDict.Find(filename);

	lookfor += "*";
	if (stream && oref)
	{
		SharedObj*			obj = (SharedObj*) *oref;
		TCHAR			namebuf[VX_MaxPath];
		const TCHAR*	name;

		if (obj)
		{
			name = obj->GetName();
			if (name)						// remove it's dictionary
			{
				STRCPY(namebuf, name);
				STRCPY(STRRCHR(namebuf, TEXT('.')), TEXT(".dict"));
				m_FileDict.Remove(NameProp(namebuf));
			}
			stream->Detach(obj);
			stream->DetachAll(lookfor, obj);
		}
	}
	FileLoader::Unload(filename);
}

void SceneLoader::Kill()
{
	m_LastDict = NULL;
	FileLoader::Kill();
}
}	// end Vixen