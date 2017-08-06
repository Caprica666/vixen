/****
 *
 * Scene Class
 *
 ****/
#include "vixen.h"

namespace Vixen {

#ifdef VX_NOTHREAD
bool Scene::InitThread(SceneThread* thread)
{
	VX_TRACE(Scene::Debug, ("Scene::InitThread\n"));
	return true;
}

void	Scene::EndThread(SceneThread* thread)
{
	VX_TRACE(Scene::Debug, ("Scene::EndThread\n"));
	World3D::Get()->OnThreadExit(SCENE_MainThread);
}

bool Scene::Run(Window w)
{
	InitThreadGlobals();
	InitRender(w);
	m_Changed |= SCENE_WindowChanged;
	return InitThread(NULL);
}

#if defined(VX_PTHREAD)
void* Scene::FrameFunc(void* arg)
{
	Scene* scene = (Scene*) arg;
	VX_ASSERT(scene != NULL);

	if (!scene->InitThread(NULL))
		return NULL;
	while (!scene->IsExit())
		scene->DoFrame();
	return NULL;
}
#else
void Scene::FrameFunc(void* arg)
{
	Scene* scene = (Scene*) arg;
	VX_ASSERT(scene != NULL);

	if (!scene->InitThread(NULL))
		return;
	while (!scene->IsExit())
		scene->DoFrame();
}

#endif

#else	// VX_NOTHREAD

Core::ThreadPool*	Scene::s_Threads = NULL;

/*!
 * @fn bool Scene::InitThread(SceneThread* thread)
 *
 * Initializes the thread-specific state for spawning a new scene thread.
 *
 * @return  true if thread initializes properly, else  false
 * @see Scene::EndThread Scene::Run
 */
bool Scene::InitThread(SceneThread* thread)
{
	VX_TRACE(Scene::Debug, ("Scene::InitThread(options='0x%x' handle='0x%x'\n", thread->GetOptions(), thread->GetThreadHandle()));
	InitRender(GetWindow());
	return true;
}

void	Scene::EndThread(SceneThread* thread)
{
	VX_TRACE(Scene::Debug, ("Scene::EndThread(options='0x%x' handle='0x%x'\n", thread->GetOptions(), thread->GetThreadHandle()));

	World3D::Get()->OnThreadExit(GetTLS()->ThreadType);
	if (thread)
	{
		Core::InterlockDec(&(s_Threads->NumThreads));
		thread->Stop();
	}
}


/*!
 * @fn bool Scene::Run(Window w)
 * Starts scene graph traversal threads which run concurrently
 * to simulate and display. If you do not want Vixen to display
 * the scene graph autonomously, call Scene::DoFrame explicitly to
 * process a frame. The MFC subclass of World3D calls Run to
 * spawn the scene manager in a separate thread so that display does not
 * lock out user interface operations.
 *
 * @see Scene::DoFrame Scene::Suspend Scene::Resume
 */
bool Scene::Run(Window w)
{
	SceneThread*	main = new SceneThread(this, SCENE_MainThread);

	VX_ASSERT(s_Threads == NULL);
	s_Threads = new Core::ThreadPool();
	s_Threads->Add(main);
	main->SetThreadFunc(FrameFunc);
	InitThreadGlobals();
	GetTLS()->ThreadType = 0;
	m_Changed |= SCENE_WindowChanged;
	s_Threads->DoExit = false;
	s_Threads->Synchronized = true;
	s_Threads->RunAll(FrameFunc);
	return true;
}

/*!
 * @fn void Scene::FrameFunc(void* arg)
 * Thread function to display the scene graph continuously.
 * If another thread wants to change the scene graph, it suspends the scene
 * and waits until a safe time (between frames). When it is done
 * updating the scene graph, it resumes the scene to allow display
 * processing to continue.
 *
 * @see DistScene::FrameFunc Scene::Suspend Scene::Resume
 */
#if defined(_WIN32) && !defined(VX_PTHREAD)
void Scene::FrameFunc(void* arg)
{
	_mm_setcsr(_mm_getcsr() | /*FTZ:*/ (1<<15) | /*DAZ:*/ (1<<6));
#else
void* Scene::FrameFunc(void* arg)
{
#endif

	SceneThread*	thread = (SceneThread*) arg;
	Scene*			scene;

	VX_ASSERT(thread != NULL);
	scene = thread->GetScene();
	VX_ASSERT(scene != NULL);
	scene->InitThreadGlobals();
	GetTLS()->ThreadType = thread->GetOptions();
	if (scene->InitThread(thread))
	{
		while (!scene->IsExit())
			scene->DoFrame();
		scene->Exit();
		scene->EndThread(thread);
	}
#if !defined(_WIN32) || defined(VX_PTHREAD)
	return NULL;
#endif
}

#endif		// VX_NOTHREAD

}	// end Vixen