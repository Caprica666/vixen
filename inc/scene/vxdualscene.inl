/****
 *
 * class DualScene
 * Splits the scene manager application into two threads suitable
 * for use on dual processor, shared memory machines. The main
 * thread has the Windows message pump, simulation, culling and
 * scene graph traversal. The rendering thread does all transformation,
 * lighting, clipping and rendering (it communicates with the graphics device).
 *
 * DualScene is a good choice if your application is evenly balanced
 * with respect to geometry processing and simulation. Usually,
 * applications which do serious simulation each frame like
 * collision detection, physics or skinning are best able
 * to take advantage of this fixed type of load balancing on
 * the processors. Applications which involved a lot of fixed
 * geometry but not much simulation will not be likely to benefit.
 *
 * DualScene works in conjunction with state sorting so this option
 * is always enabled. During display list traversal, information about
 * each mesh that is not culled is cached in the state sorter.
 * There are two sets of caches - one for the scene
 * graph traversal thread and the other for the geometry rendering
 * thread. The cached information is the same data saved to prepare
 * for render state sorting except that a copy is kept for each thread.
 * While frame N-1 is being rendered by one thread, frame N can be
 * calculated by the other. This scheme does not duplicate geometry,
 * it just keeps two sets of state sorting buckets that reference
 * the same geometric data.
 *
 ****/

#ifndef VX_NOTHREAD

template <class S> Core::TLS<Renderer*> DualScene<S>::t_Renderer;

template <class S> int	DualScene<S>::GetNumThreads() const
{
	if (S::m_IsMultiframe)
		return 2;
	return 0;
}

template <class S> int DualScene<S>::GetFrameIndex() const
{
	int frame = S::GetTLS()->Frame;
	frame &= int(S::m_IsMultiframe);
	return frame;
}

/*!
 * @fn void DualScene::DoFrame()
 *
 * Called from the main scene thread to execute the
 * logic for a single frame. Simulation and scene graph traversal
 * is handled by the main thread. Geometry transformations and
 * rendering is handled by a separate render thread.
 *
 * Only the main and simulation threads actually change the scene properties like
 * camera view volume, light source positions/directions. Notification
 * to update render thread properties based on scene changes is done
 * via a set of flags in Scene::m_Changed. The following
 * flags may be set by the main thread:
 * @code
 *	SCENE_RootChanged		set if scene graph root changed
 *	SCENE_WindowChanged		set if display viewport changed
 *	SCENE_EnvChanged		set if environment changed (lighting)
 * @endcode
 *
 * @see Scene::DoFrame DistScene::DoFrame Scene::ThreadFunc
 */
template <class S> void DualScene<S>::DoFrame()
{
	Scene::TLS* threadg = S::GetTLS();

	if (!S::m_IsMultiframe)
	{
		t_Renderer = S::m_Renderer;
		S::DoFrame();
		return;
	}
//
// At start of frame we allow safe update by remote threads to change
// the hierarchy or scene list. Scenes posted for delete are killed now
//
	float save_start = S::OnFrame(threadg->Frame);
	Scene* scene = this;
	Scene* prev = NULL;
	if (S::IsExit())
		return;
	S::AllowUpdate();					// allow update by remote threads
 	S::m_Stats.TotalVerts = S::m_Stats.CulledVerts = 0;
	S::m_Stats.TotalModels = S::m_Stats.CulledModels = 0;
	while (scene)						// for each child scene
	{
		if ((scene->GetChanged() & SCENE_KillMe) && prev)
		{
			scene = prev->RemoveChild();	// delete this scene?
			continue;
		}
		prev = scene;
		scene = scene->GetChild();
	}
	S::s_Threads->ResumeAll(SCENE_RenderThread);
//
//	Scene graph traversal
//
	GetMessenger()->t_NoLog = true;			// no logging during traversal
	scene = this;
	if (S::GetChanged() & SCENE_RootChanged)// if hierarchy changed
		S::GetRenderer()->Empty();			// detach lights used by previous hierarchy
	while (scene)							// for each child scene
	{
		if (scene->IsActive())				// display this scene?
		{
			scene->InitCamera();			// initialize camera view properties
			scene->DoDisplay();				// do traversal, culling
		}
		scene = scene->GetChild();
	}
	GetMessenger()->t_NoLog = false;		// enable logging again
	S::DoSimulation();						// run simulation
//
//	Synchronize and swap frames
//
	S::s_Threads->WaitAll(SCENE_RenderThread);// wait for rendering to finish
	if (S::IsExit())
		return;
	GetMessenger()->Flush();				// distribute scene graph updates
	S::m_Changed = 0;
	S::m_Stats.StartTime = save_start;
	++(threadg->Frame);
	S::m_Stats.EndTime = World3D::Get()->GetTime();
}
			
/*!
 * @fn GeoSorter* DualScene::GetRenderer() const
 * Fetch the geometry sorter for this frame.
 * There are actually two instances of GeoSorter, one
 * for the display traversal thread and one for the
 * rendering thread. This routine will return a different
 * value depending on which frame you are in.
 *
 * @see Scene::InitRender
 */
template <class S> Renderer* DualScene<S>::GetRenderer() const
{
	return t_Renderer;
}

/*!
 * @fn void	DualScene::SetNumThreads(int nthreads)
 *
 * Establishes the number of threads to use for distributing
 * the scene graph. The system can use one or two threads.
 * When two threads are used, the main thread is dedicated to
 * simulation, culling and scene graph traversal. The other does
 * transformation, lighting, clipping and rendering. 
 *
 * This function should be called once in World3D::NewScene before
 * the initial window is created and display beings.
 * For DualScene, the valid inputs are 0 (for normal single-threaded
 * operation) and 2 (for dual-threaded operation)
 *
 * @see Scene::Run DistScene::Run
 */
template <class S> void	DualScene<S>::SetNumThreads(int nthreads)
{
	if (nthreads > 0)
	{
		S::EnableOptions(Scene::STATESORT);
		S::m_IsMultiframe = true;
	}
	t_Renderer = NULL;
}

/****
 *
 * Start scene display threads
 *
 ****/
template <class S> bool DualScene<S>::Run(Window win)
{
	InitRender(win);
	if (!S::m_IsMultiframe)
		return Scene::Run();
	SceneThread*	main = new SceneThread(this, SCENE_SimThread | SCENE_DisplayThread);
	main->SetThreadFunc(Scene::FrameFunc);
	S::s_Threads->Add(main);	// Make the main thread for simulation and synchronization
	SceneThread* render = new SceneThread(this, SCENE_RenderThread);

#if defined(_WIN32) && !defined(VX_PTHREAD)
  main->SetThreadFunc(DualScene<S>::RenderFunc);
#else
	main->SetThreadFunc((Core::ThreadFunc *) &DualScene<S>::RenderFunc);
#endif
	S::s_Threads->Add(render);	// Make the render thread
	S::s_Threads->RunAll();		// Start all the threads running
	return true;
}

template <class S> bool DualScene<S>::InitRender(Vixen::Window win)
{
	Renderer* r;

	if (!S::InitRender(win))
		return false;
	r = S::GetRenderer();
	VX_ASSERT(r);
	m_Renderers[0] = r;
	m_Renderers[1] = (Renderer*) r->Clone();
	m_Renderers[1]->Init(this, win, RenderOptions);
	return true;
}

template <class S> void DualScene<S>::Exit()
{
	m_Renderers[0]->Exit();
	m_Renderers[1]->Exit();
	t_Renderer = NULL;
	S::Exit();
	m_Renderers[0] = NULL;
	m_Renderers[1] = NULL;
}


template <class S> void DualScene<S>::Begin()
{
	int	frame = GetFrameIndex();
	t_Renderer = m_Renderers[frame];
	S::Begin();
}

template <class S> void DualScene<S>::InitCamera()
{
	int	frame = GetFrameIndex();
	t_Renderer = m_Renderers[frame];
	S::InitCamera();
}

/*!
 * @fn void DistScene::RenderFunc(void* arg)
 *
 * Thread function for rendering threads. Controls the swapping
 * of the display buffers.	There can be only one render thread
 * for a scene.
 *
 * @see DistScene::SimFunc DistScene::DisplayFunc Scene::ThreadFunc
 */
template <class S> void DualScene<S>::RenderFunc(void* arg)
{
	SceneThread*	thread = (SceneThread*) arg;
	Ref<Scene>	scene = thread->GetScene();
	Scene::TLS*	g = S::GetTLS();
	Renderer*	render;

	scene->InitThreadGlobals();
	g->ThreadType = thread->GetOptions();
	g->Frame++;
	scene->InitThread(thread);		// initialize thread storage
	render = scene->GetRenderer();
	thread->Suspend();
	while (!scene->IsExit())
	{
		Scene* child = scene;		// render procedural texturing scenes
		while (child = child->GetChild())
		{
			child->Notify(SCENE_WindowChanged);
			child->Begin();			// begin display frame
			child->DoRender();		// do rendering
			child->End();			// end the scene
		}
		if (scene->GetChild())		// force viewport change if multiple scenes
			scene->Notify(SCENE_WindowChanged);
		scene->Begin();				// do parent scene last
		scene->DoRender();
		scene->End();
		render->Flip();				// flip buffers
		thread->SignalDone();
		if (scene->IsExit())
			break;
		thread->Suspend();
		(g->Frame)++;
	}
	scene->EndThread(thread);
}
#endif
