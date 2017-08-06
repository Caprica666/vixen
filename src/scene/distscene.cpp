#include "vixen.h"


#define	SCENE_MaxVtxChunk	1024

namespace Vixen {

#ifndef VX_NOTHREAD

Renderer* DualScene::t_Renderer;

int	DualScene::GetNumThreads() const
{
	if (Scene::m_IsMultiframe)
		return 2;
	return 0;
}

int DualScene::GetFrameIndex() const
{
	int frame = Scene::GetTLS()->Frame;
	frame &= int(Scene::m_IsMultiframe);
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
void DualScene::DoFrame()
{
	Scene::TLS* threadg = Scene::GetTLS();

	if (!Scene::m_IsMultiframe)
	{
		t_Renderer = Scene::m_Renderer;
		Scene::DoFrame();
		return;
	}
//
// At start of frame we allow safe update by remote threads to change
// the hierarchy or scene list. Scenes posted for delete are killed now
//
	float save_start = Scene::OnFrame(threadg->Frame);
	Scene* scene = this;
	Scene* prev = NULL;
	if (Scene::IsExit())
		return;
	Scene::AllowUpdate();					// allow update by remote threads
 	Scene::m_Stats.TotalVerts = Scene::m_Stats.CulledVerts = 0;
	Scene::m_Stats.TotalModels = Scene::m_Stats.CulledModels = 0;
	while (scene)							// for each child scene
	{
		if ((scene->GetChanged() & SCENE_KillMe) && prev)
		{
			scene = prev->RemoveChild();	// delete this scene?
			continue;
		}
		prev = scene;
		scene = scene->GetChild();
	}
	Scene::s_Threads->ResumeAll(SCENE_RenderThread);
//
//	Scene graph traversal
//
	GetMessenger()->t_NoLog = true;			// no logging during traversal
	scene = this;
	if (Scene::GetChanged() & SCENE_RootChanged)// if hierarchy changed
		Scene::GetRenderer()->Empty();		// detach lights used by previous hierarchy
	DoDisplay();
	GetMessenger()->t_NoLog = false;		// enable logging again
	Scene::DoSimulation();					// run simulation
//
//	Synchronize and swap frames
//
	Scene::s_Threads->WaitAll(SCENE_RenderThread);// wait for rendering to finish
	if (Scene::IsExit())
		return;
	GetMessenger()->Flush();				// distribute scene graph updates
	Scene::m_Changed = 0;
	Scene::m_Stats.StartTime = save_start;
	++(threadg->Frame);
	Scene::m_Stats.EndTime = World3D::Get()->GetTime();
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
Renderer* DualScene::GetRenderer() const
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
void	DualScene::SetNumThreads(int nthreads)
{
	if (nthreads > 0)
	{
		Scene::EnableOptions(Scene::STATESORT);
		Scene::m_IsMultiframe = true;
	}
	t_Renderer = NULL;
}

/****
 *
 * Start scene display threads
 *
 ****/
bool DualScene::Run(Window win)
{
	if (!Scene::m_IsMultiframe)
		return Scene::Run(win);

	SceneThread*	main = new SceneThread(this, SCENE_SimThread | SCENE_DisplayThread);
	SceneThread*	render = new SceneThread(this, SCENE_RenderThread);
	
	Scene::s_Threads = new Core::ThreadPool();
	main->SetThreadFunc(Scene::FrameFunc);
#if defined(_WIN32) && !defined(VX_PTHREAD)
	render->SetThreadFunc(DualScene::RenderFunc);
#else
	render->SetThreadFunc((Core::ThreadFunc *) &DualScene::RenderFunc);
#endif
	Scene::s_Threads->Add(main);	// Make the main thread for simulation and synchronization
	Scene::s_Threads->Add(render);	// Make the render thread
	Scene::s_Threads->RunAll();		// Start all the threads running
	return true;
}

bool DualScene::InitRender(Vixen::Window win)
{
	int	frameindex = GetFrameIndex();

	if (Scene::InitRender(win))
	{
		m_Renderers[0] = m_Renderer;
		if (Scene::m_IsMultiframe)
		{
			m_Renderers[1] = (Renderer*) m_Renderer->Clone();
			m_Renderers[1]->Init(this, win, RenderOptions);
		}
		else
			m_Renderers[1] = m_Renderer;
		t_Renderer = m_Renderers[frameindex];
		return true;
	}
	t_Renderer = m_Renderers[frameindex];
	return false;
}

void DualScene::Exit()
{
	m_Renderers[0]->Exit(false);
	m_Renderers[1]->Exit(true);
	t_Renderer = NULL;
	Scene::Exit();
	m_Renderers[0] = NULL;
	m_Renderers[1] = NULL;
}


void DualScene::Begin()
{
	int	frame = GetFrameIndex();
	t_Renderer = m_Renderers[frame];
	Scene::Begin();
}

void DualScene::InitCamera()
{
	int	frame = GetFrameIndex();
	t_Renderer = m_Renderers[frame];
	Scene::InitCamera();
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
void DualScene::RenderFunc(void* arg)
{
	SceneThread*	thread = (SceneThread*) arg;
	Ref<Scene>		scene = thread->GetScene();
	Scene::TLS*		g = Scene::GetTLS();

	scene->InitThreadGlobals();
	g->ThreadType = thread->GetOptions();
	g->Frame++;
	scene->InitThread(thread);
	thread->Suspend();
	while (!scene->IsExit())
	{
		Renderer* render = scene->GetRenderer();
		scene->DoRender();
		render->Flip(g->Frame);
		thread->SignalDone();
		if (scene->IsExit())
			break;
		thread->Suspend();
		(g->Frame)++;
	}
	scene->EndThread(thread);
}


#endif	// NOTHREAD


/*!
 * @fn void StereoScene::DoFrame()
 *
 * Called from the main scene thread to execute the
 * logic for a single frame. Simulation and scene graph traversal
 * are performed once. Rendering is done twice - once for the left
 * eye and once for the right. Each eye has a slightly different camera.
 *
 * @see Scene::DoFrame Scene::ThreadFunc
 */
void StereoScene::DoFrame()
{
//
// compute start time of frame and process remote updates
//
	TLS*		g = GetTLS();
	Renderer*	r = GetRenderer();
	int			frame = g->Frame;
	float		save_start = OnFrame(frame);
	Scene*		scene = this;

	if (IsExit())						// got exit request?
		return;							// don't do next frame
//
//	REMOTE THREADS CAN UPDATE THE SCENE GRAPH HERE
//
	frame &= 1;
	if (frame == 0)
		r->Flip(frame);					// flip buffers
	AllowUpdate();						// allow update by other threads
	GetMessenger()->t_NoLog = true;		// no logging during traversal
 	Core::InterlockSet(&m_Stats.TotalVerts, 0);
	Core::InterlockSet(&m_Stats.CulledVerts, 0);
	Core::InterlockSet(&m_Stats.TotalModels, 0);
	Core::InterlockSet(&m_Stats.CulledModels, 0);
//
// do display traversal for this scene and all the child scenes
// which use its display context (procedural texture generation scenes)
//
	DoDisplay();
	if (GetChild())						// force viewport change if multiple scenes
		Notify(SCENE_CameraChanged);	// render this scene last
	DoRender();
	GetCamera()->SetViewVol(m_MonoViewVol);
//
// done rendering, enable logging and do simulation
//
	GetMessenger()->t_NoLog = false;	// enable logging again
	DoSimulation();						// run simulation (hierarchy locked here)
	GetMessenger()->Flush();			// distribute scene graph updates
	m_Stats.StartTime = save_start;
	++(g->Frame);
	m_Stats.EndTime = World3D::Get()->GetTime();
}

void StereoScene::Begin()
{
	Scene::TLS*	tls = Scene::GetTLS();
	Renderer*	r = GetRenderer();
	Box3		stereovol;
	Matrix		view;
	Vec3		offset(m_Camera->GetEyeSep(), 0, 0);
	const Box2&	vp = GetViewport();
	float		w = vp.Width() / 2.0f;
	int			frame = tls->Frame & 1;

	if (m_Changed & SCENE_WindowChanged)
	{
		SetViewport();						// update viewport based on window size
		if (m_AutoAdjust)					// preserve aspect ratio of view volume?
			m_Camera->SetAspect(w / m_Viewport.Height());
	}
	m_MonoViewVol = m_Camera->GetViewVol();
	m_Camera->GetStereoView(frame, stereovol);
	if (frame == 0)
		offset.x = -offset.x;
	view.TranslationMatrix(offset);
	m_Changed |= SCENE_CameraChanged;
	m_Camera->SetViewVol(stereovol);
	m_Camera->SetViewTrans(this, &view);
	if (frame == 0)
		r->SetViewport(vp.min.x, vp.min.y, w, vp.max.y);
	else
		r->SetViewport(vp.min.x + w, vp.min.y, vp.max.x, vp.max.y);
	r->Begin(m_Changed, frame);
}

void StereoScene::InitCamera()
{
/*
 * Calculate the camera view matrix. The scene manager allows the
 * camera to be positioned anywhere. The display device supports a
 * fixed camera positioned at the origin looking down the Z axis.
 * The view matrix transforms the scene objects into this camera space.
 */
	Camera*	cam = m_Camera;
	if (cam->HasChanged())					// camera changed?
	{
		Notify(SCENE_CameraChanged);		// alert other threads
		cam->CalcViewPlanes();				// recalculate camera clip planes
		cam->SetChanged(false);
	}
}

}	// end Vixen


