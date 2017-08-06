#include "vixen.h"

namespace Vixen {

/*!
 * @fn void Scene::SetViewport()
 *
 * Sets the screen viewport to which the 3D viewport maps.
 * You need to call this function whenever the window
 * origin or size changes to notify the scene manager to change the
 * 3D screen viewport the current scene is using.
 * This version of the function sets the viewport to cover
 * the entire window.
 *
 * @see Scene::Begin Camera::SetViewVol
 */
void Scene::SetViewport()
{
	float	w, h;
	HWND	hwnd = m_pDevInfo->WinHandle;
	
	VX_ASSERT(m_pDevInfo);
	hwnd = m_pDevInfo->WinHandle;
	if (hwnd && (hwnd != HWND(-1)))
	{
		RECT	r;

		::GetClientRect(hwnd, &r);
		w = (float) fabs(float(r.right - r.left));
		h = (float) fabs(float(r.bottom - r.top));
		VX_TRACE(Scene::Debug, ("<scene_setviewport width='%f' height='%f' />", w, h));
		SetViewport(0, 0, w, h);
	}
}

/*!
 * @fn bool Scene::SetWindow(Window win)
 *
 * Sets the window the scene should use to display its graphics.
 * This usually performs some device-specific initialization.
 *
 * @see World3D::Run
 */
bool Scene::SetWindow(Window win)
{
	World3D*	app = World3D::Get();

	if (win == NULL)
	{
		if ((m_pDevInfo == NULL) || (m_pDevInfo->WinHandle == NULL))
			return true;
		VX_TRACE(Scene::Debug, ("<scene_setwindow handle='0' />"));
/*
 * If running multiple threads, shut down the other scene threads
 */
#ifdef VX_NOTHREAD
		m_pDevInfo->WinHandle = NULL;
		Exit();
		EndThread(NULL);
		DoExit = true;
#else
		TLS* g = GetTLS();
		if (!IsExit())
		{
			Core::InterlockSet((voidptr*) &m_pDevInfo->WinHandle, NULL);
			s_Threads->KillAll(true);
			if (m_pDevInfo)
			{
				delete m_pDevInfo;
				m_pDevInfo = NULL;
			}
			World3D::Get()->KillScene(this);
		}
#endif
		return true;
	}
	if (win == (intptr) m_pDevInfo->WinHandle)	// no net change
		return true;
	if (m_pDevInfo->WinHandle)					// already have a window?
		return false;
	else										// start scene for this window
	{
		static bool rc;
		m_pDevInfo->WinHandle = (HWND) win;
		rc = Run(win);
		if (!rc)
			return false;
		VX_TRACE(Scene::Debug, ("<scene_setwindow handle='%x' />", win));
		app->AddScene(this, win);
	}
	m_pDevInfo->WinHandle = (HWND) win;
	return true;
}

Window Scene::GetWindow()
{
	return (Window) m_pDevInfo->WinHandle;
}


DeviceInfo* Scene::MakeDevInfo(const DeviceInfo* srcinfo)
{
	DeviceInfo* dstinfo = new DeviceInfo;

	if (srcinfo)
	{
		dstinfo->DC = srcinfo->DC;
		dstinfo->Device = srcinfo->Device;
		dstinfo->WinHandle = srcinfo->WinHandle;
	}
	else
	{
		dstinfo->DC = NULL;
		dstinfo->Device = NULL;
		dstinfo->WinHandle = NULL;
	}
	 return dstinfo;
}

/*!
 * @fn void Scene::Append(Scene* child)
 * @param child scene to attach to this display scene
 *
 * The input scene is appended to this display scene's list
 * of child scenes. All of these scenes share the same window,
 * graphics device context, light list and graphics resources.
 * Usually, they share all or part of the same scene graph, too.
 * Child scenes cannot have engines and do not do simulation.
 * They are traversed and rendered in the same threads as the
 * display scene to which they are attached.
 *
 * This mechanism is primarily intended for procedural texturing
 * effects such as environment mapping and shadow mapping where the
 * same scene is rendered multiple times with a different cameras and lighting
 * to produce textures used in the main scene. Although the scenes the same
 * light sources, different lights may be enabled and disabled for each scene.
 *
 * @see LightList::LightsOff Scene::SetColorBuffer
 */
void Scene::Append(Scene* child)
{
	Suspend();
	VX_ASSERT(child->m_pDevInfo->Device == NULL);
	VX_ASSERT(child->m_Child == NULL);
	VX_ASSERT(child->m_Parent == NULL);
	VX_ASSERT(m_Parent == NULL);
	*(child->m_pDevInfo) = *m_pDevInfo;
//	child->m_LightList = m_LightList;
	child->m_Engines = (Engine*) NULL;	// do not inherit engines from parent scene
	child->m_Child = m_Child;			// link into the render list
	m_Child = child;
	child->m_Parent = this;
	child->DisableOptions(DOUBLEBUFFER);
	child->InitRender(GetWindow());		// make state sorting buckets
	Resume();
}

/****
 *
 * Scene Destructor
 * Remove the scene from the scene list and dereference
 * all of its attributes (display groups, ...)
 *
 ****/
Scene::~Scene()
{
	Exit();
}

#ifdef VX_NOTHREAD

void Scene::Suspend()	{ }

void Scene::Resume()	{ }

void Scene::AllowUpdate()	{ }

void Scene::Repaint()		{ DoFrame(); }

#else			// VX_NOTHREAD

/*!
 * @fn void Scene::Suspend()
 *
 * Suspends current thread until it is safe to update
 * the scene graph or simulation tree. The current thread is suspended
 * until the end of the frame. Between the Suspend and Resume calls,
 * it is safe for the application to modify the hierarchy.
 * Suspend calls may be nested but only within a single thread.
 *
 * @see Scene::Run Scene::Resume
 */
void Scene::Suspend()
{
	int renderthread = GetTLS()->ThreadType & (SCENE_RenderThread | SCENE_DisplayThread);
	if (!renderthread && m_pDevInfo->WinHandle && !IsExit())
		if ((m_Options & REPAINT) && m_ChangePending)
			return;							// don't suspend again if waiting between frames
		else if (Core::InterlockInc(&m_ChangePending) == 1)
			m_UpdateOK.Wait();
}

/*!
 * @fn void Scene::Resume()
 *
 * Resumes scene graph display after it has been suspended.
 * Applications should minimize the amount of time a scene is suspended
 * or display processing will appear jerky.
 *
 * @see Scene::Suspend Scene::Run
 */
void Scene::Resume()
{
	if (m_ChangePending <= 0)
		return;
	int renderthread = GetTLS()->ThreadType & (SCENE_RenderThread | SCENE_DisplayThread);
	if (!renderthread && m_pDevInfo->WinHandle && (Core::InterlockDec(&m_ChangePending) <= 0))
		m_UpdateDone.Release();
};

/*!
 * @fn void Scene::Repaint(int clearopts)
 * Forces scene graph display for at least one frame.
 * If in continuous refresh mode (the default), this routine does nothing.
 * If Scene::REPAINT is enabled, scene display processing suspends after each frame.
 * Calling Scene::Repaint resumes display again for another frame.
 * This permits you to switch between continuous refresh and explicit repaint mode.
 *
 * @see Scene::Suspend Scene::Resume Scene::SetOptions
 */
void Scene::Repaint()
{
	if (m_Options & REPAINT)
		Resume();
}

void Scene::AllowUpdate()
{
	if (m_ChangePending)						// remote updater waiting?
	{
		m_UpdateOK.Release();					// signal remote update legal here
		m_UpdateDone.Wait();					// wait for remote update to finish
	}
	else if (m_Options & REPAINT)				// in repaint mode?
	{
		Core::InterlockInc(&m_ChangePending);	// suspend rendering until repaint event
		m_UpdateDone.Wait();					// wait for Resume() call
	}
}

#endif

}	// end Vixen