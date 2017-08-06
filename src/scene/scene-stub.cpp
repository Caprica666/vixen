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
// TODO: figure out what to put here
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

	if (win == 0)
	{
		if ((m_pDevInfo == NULL) || (m_pDevInfo->WinHandle == 0))
			return true;
		VX_TRACE(Scene::Debug, ("Scene::SetWindow(NULL)\n"));
		m_pDevInfo->WinHandle = NULL;
		Exit();
		EndThread(NULL);
		DoExit = true;
		return true;
	}
	if (win == (intptr) m_pDevInfo->WinHandle)	// no net change
		return true;
	if (m_pDevInfo->WinHandle)			// already have a window?
		return false;
	else							// start scene for this window
	{
		static bool rc;
		m_pDevInfo->WinHandle = (Window) win;
		rc = Run(win);
		if (!rc)
			return false;
		VX_TRACE(Scene::Debug, ("Scene::SetWindow(%x)\n", win));
		app->AddScene(this, win);
	}
	m_pDevInfo->WinHandle = (Window) win;
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
		dstinfo->DC = 0;
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
	VX_ASSERT(child->m_Child == NULL);
	VX_ASSERT(child->m_Parent == NULL);
	VX_ASSERT(m_Parent == NULL);
	*(child->m_pDevInfo) = *m_pDevInfo;
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

void Scene::Suspend()	{ }

void Scene::Resume()	{ }

void Scene::AllowUpdate()	{ }

void Scene::Repaint()		{ DoFrame(); }



}	// end Vixen