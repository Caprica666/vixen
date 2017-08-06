

/*!
 * @file vxdistscene.h
 * @brief Distributed 3D scene display.
 *
 * Distributes geometry and display processing across multiple threads.
 *
 * @author Nola Donato
 * @ingroup vixen
 * @internal
 *
 * @see vxscene.h vxdualscene.h
 */
#pragma once

namespace Vixen {

/*!
 * @class DistScene
 * @brief Distributes the geometry processing for a scene graph over multiple threads.
 *
 * This class is a device-independent base
 * class that performs thread management and synchronization.
 * It is designed to be subclassed for use with different
 * geometry and rendering engines.
 *
 * @ingroup vixen
 * @see DualScene
 */
class DistScene : public Scene
{
public:
	VX_DECLARE_CLASS(DistScene);
	DistScene();
	~DistScene();
	DistScene(Scene&);

	virtual void	DoFrame();
	virtual bool	Run(Window w);
	virtual void	SetNumThreads(int);
	int		PreVisit(const Model* mod);
	void	PostVisit(const Model* mod, int flags);

	static	bool	IsTask(const Model* mod);
	static	Core::ThreadFunc	SimFunc;		// simulation thread function
	static	Core::ThreadFunc	RenderFunc;		// rendering thread function
	static	Core::ThreadFunc	DisplayFunc;	// display thread function

	static	int32				NodeCounter;		// scene graph node counter
	static Core::TLS<int32>		t_NodeCounter;		// counts nodes in scene during traversal
	static Core::TLS<bool>		t_WithinTask;		// indicates that thread is currently a unique traverser
#ifndef VX_NOTHREAD
	Core::CritSec				m_LockCounter;		// thread safety lock
#endif

protected:
	int32	m_NumThreads;
};

} // end Vixen
