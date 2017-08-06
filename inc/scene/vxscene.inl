/****
 *
 * Inlines for class Scene
 *
 ****/

namespace Vixen {

#define	SCENE_DisplayMe			1	// flag to display model geometry
#define	SCENE_DisplayChildren	2	// flag to display model's children
#define	SCENE_WasCulled			4	// flag to indicate model was culled
#define SCENE_IsTask			8	// indicates that the subtree rooted here
									// is a single task (for multithreading)

/*!
 * @fn const DeviceInfo* Scene::GetDevInfo() const
 *
 * Returns information about the underlying display device.
 * The format of the DeviceInfo structure is different for
 * different ports of the scene manager. The OpenGL version looks like:
 * @code
 * struct DeviceInfo
 *   {
 *  	HWND	Window;	// handle to display window
 *  	HDC		DC;		// handle to GDI DC
 *  	HGLRC	Device;	// handle to OpenGL rendering context
 *   };
 * @endcode
 *
 * @return pointer to device information descriptor
 */
inline const DeviceInfo* Scene::GetDevInfo() const
	{ return m_pDevInfo; }

inline const Box2& Scene::GetViewport() const
	{ return m_Viewport; }

inline void Scene::GetViewport(Box2* box) const
	{ *box = m_Viewport; }

inline const Model* Scene::GetModels() const
	{ return m_Models; }

inline Model* Scene::GetModels()
	{ return m_Models; }

inline const Engine* Scene::GetEngines() const
	{ return m_Engines; }

inline Engine* Scene::GetEngines()
	{ return m_Engines; }

inline const Camera* Scene::GetCamera() const
	{ return m_Camera; }

inline Camera* Scene::GetCamera()
	{ return m_Camera; }

inline const Col4& Scene::GetBackColor() const
	{ return m_BackColor; }

inline Bitmap* Scene::GetColorBuffer() const
	{ return m_ColorBuffer; }

inline Bitmap* Scene::GetDepthBuffer() const
	{ return m_DepthBuffer; }

inline float Scene::GetTimeInc() const
	{ return m_TimeInc; }

inline Renderer*	Scene::GetRenderer() const
	{ return m_Renderer; }

inline Scene::TLS* Scene::GetTLS()
//	{ return t_State.Get(); }
	{ return &t_State; }

inline Matrix* Scene::GetWorldMatrix() const
	{ return (Matrix*) &m_WorldMatrix; }

inline Appearance* Scene::GetPostProcess() const
	{ return m_PostProcess; }

#ifdef VX_NOTHREAD
inline bool	Scene::IsExit() const
	{ return DoExit; }

#else
inline bool	Scene::IsExit() const
	{ return s_Threads->DoExit != 0; }
#endif

//inline LightList* Scene::GetLights()
//	{ return m_LightList; }

inline int Scene::GetFrameIndex() const
	{ return 0; }

inline float Scene::GetTime() const
	{ return m_Time; }

inline bool Scene::IsAutoAdjust() const
	{ return m_AutoAdjust; }

inline int Scene::GetOptions() const
	{ return m_Options; }

inline long	Scene::GetChanged() const
	{ return m_Changed; }

inline void	Scene::Notify(long flags)
	{ Core::InterlockOr(&m_Changed, flags); }

inline Scene*	Scene::GetChild() const
{
	return m_Child;
}

inline Scene*	Scene::RemoveChild()
{
	m_Child->m_Parent = NULL;
	m_Child = m_Child->m_Child;
	return m_Child;
}

inline bool Scene::IsChild() const
{
	return m_Parent != NULL;
}

inline void Scene::SetPriority(int priority)
{
#ifndef VX_NOTHREAD
	s_Threads->SetPriority(priority);
#endif
}

inline bool Scene::IsSuspended() const
{
	return m_ChangePending > 0;
}


/*!
 * @fn Scene* GetMainScene()
 *
 * Returns a pointer to the current scene associated with
 * the application. If there is no Scene, NULL is returned.
 *	
 * @see World3D::GetScene Scene World3D
 */
inline Scene* GetMainScene(void)
{
	World3D* world = World3D::Get();
	if (world != NULL)
		return world->GetScene();
	return NULL;
}

} // end Vixen