

/*!
 * @file vxdualscene.h
 * @brief Dual processor 3D scene display.
 *
 * Distributes display traversal and rendering across two threads
 * to leverage dual processor machines.
 *
 * @author Nola Donato
 * @ingroup vixen
 * @internal
 *
 * @see vxscene.h
 */
#pragma once

namespace Vixen {

#ifndef VX_NOTHREAD

/*!
 * @class DualScene
 * @brief Splits the real time processing into two threads suitable
 * for use on dual processor, shared memory machines.
 *
 * The main thread has the Windows message pump, simulation, culling and
 * scene graph traversal. The rendering thread does all transformation,
 * lighting, clipping and rendering (it is the only thread which accesses
 * the graphics display device).
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
 * is always enabled. Each Shape in the DualScene is tagged with
 * a property containing information cached during display list
 * traversal. There are two sets of caches - one for the scene
 * graph traversal thread and the other for the geometry rendering
 * thread. The cached information is the same data saved to prepare
 * for render state sorting except that a copy is kept for each thread.
 * While frame N-1 is being rendered by one thread, frame N can be
 * calculated by the other. This scheme does not duplicate geometry,
 * it just keeps two sets of state sorting buckets that reference
 * the same geometric data.
 *
 * @image html dualscene.jpg
 *
 * @see Scene
 * @ingroup vixen
 */
class DualScene : public Scene
{
public:
	DualScene(Renderer* r = NULL) : Scene(r) { };
	DualScene(Scene& src) : Scene(src) { };

	void				SetNumThreads(int);		//!< Establish number of threads.
	int					GetNumThreads() const;
	virtual bool		Run(Window);			//!< Spawns display and rendering threads.
	virtual void		DoFrame();
	virtual void		Begin();
	virtual void		Exit();
	virtual void		InitCamera();
	virtual bool		InitRender(Vixen::Window window);
	virtual	Renderer*	GetRenderer() const;
	int					GetFrameIndex() const;
	// RAM_XX
	static	void		RenderFunc(void *arg);		//!< Rendering thread function
	//static	Core::ThreadFunc	RenderFunc;		//!< Rendering thread function

protected:
	THREAD_LOCAL Renderer*	t_Renderer;
	Ref<Renderer>			m_Renderers[2];
};

#endif	// VX_NOTHREAD


/*!
 * @class StereoScene
 * @brief Renders a scene as stereo pairs.
 *
 * The simulation and display traversal is done once but
 * the rendering is done twice - for the left and right eyes.
 * The camera used for each eye is slightly different.
 * This class sets the cull volume of the camera to include
 * everything visible with both eyes.
 *
 * @see Scene DualScene Camera::SetEyeSep Camera::SetCullVol
 * @ingroup vixen
 */
class StereoScene : public Scene
{
public:
	StereoScene(Renderer* r = NULL) : Scene(r) { };
	StereoScene(Scene& src) : Scene(src) { };

	virtual void	DoFrame();
	virtual void	Begin();
	virtual void	InitCamera();

	Box3			m_MonoViewVol;
};

} // end Vixen
