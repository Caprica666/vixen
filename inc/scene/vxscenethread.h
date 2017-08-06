#pragma once

namespace Vixen {

class Scene;
#define	SCENE_MaxThreads		8

#define	SCENE_RenderThread	1	// set for threads that render
#define	SCENE_SimThread		2	// set for threads doing simulation
#define	SCENE_DisplayThread	4	// set for threads doing traversal and geometry
#define	SCENE_UIThread		8	// windows UI thread
#define	SCENE_LoadThread	32	// content loading thread
#define	SCENE_MainThread	(SCENE_RenderThread|SCENE_SimThread|SCENE_DisplayThread)
#define	SCENE_TimeOut		VX_TimeOut

#ifndef VX_NOTHREAD
/*!
 * @class SceneThread
 *
 * @brief Encapsulates a working thread associated with a distributed scene graph.
 *
 * A scene thread may do simulation, scene graph traversal (display),
 * rendering or any combination. The implementation of this class is
 * device-dependent and it is only used when Vixen is running multi-threaded.
 * The thread does not start executing until Run is called.
 *
 * @see ThreadPool DistScene Scene::Run
 * @ingroup vixenint
 * @internal
 */
class SceneThread : public Core::Thread
{
public:
/*!
 * Constructs a single working thread for a Vixen scene.
 * A scene may have multiple threads. A thread
 * may do rendering, simulation and/or scene graph traversal. 
 *
 * @param scene	Scene thread belongs to.
 * @param opts	Type of work the thread will do:
 *	- SCENE_SimThread		thread does simulation
 *	- SCENE_DisplayThread	thread does display
 *	- SCENE_RenderThread	thread does rendering
 *	- SCENE_MainThread		thread does everything (default)
 *
 * @see Thread::Run
 */
	SceneThread(Scene* scene = NULL, uint32 opts = SCENE_MainThread)
	: Thread(opts), m_Scene(scene) { }
	
	Scene*	GetScene() const	{ return m_Scene; }

protected:
	Scene*		m_Scene;		// owning scene
};

#endif // VX_NOTHREAD

} // end Vixen