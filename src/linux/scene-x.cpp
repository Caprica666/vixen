/****
 *
 * X-Windows specific Scene Class methods
 *
 ****/
#include "vixen.h"
#include "linux/vixenx.h"
#include "linux/winworld.h"

namespace Vixen {

void Scene::SetViewport()
{
	float w, h;

	if (m_pDevInfo->WinHandle)
	{
		XWindowAttributes attr;
		if (XGetWindowAttributes(m_pDevInfo->DispHandle, m_pDevInfo->WinHandle, &attr))
		{
			w = attr.width;
			h = attr.height;
			SetViewport(0, 0, w, h);
		}
	}
}


Window Scene::GetWindow()
{
	return ((Window) m_pDevInfo->WinHandle);
}

DeviceInfo* Scene::MakeDevInfo(const DeviceInfo* srcinfo)
{  
	DeviceInfo* dstinfo = new DeviceInfo; 

	if (srcinfo)
	{
		dstinfo->DispHandle = srcinfo->DispHandle;
		dstinfo->Device = srcinfo->Device;
		dstinfo->WinHandle = srcinfo->WinHandle;
		dstinfo->VisHandle = srcinfo->VisHandle;
	}
	else
	{	 
		dstinfo->DispHandle = (Display*) NULL;
		dstinfo->Device = NULL;
		dstinfo->WinHandle = 0;
		dstinfo->VisHandle = NULL;
	}
	return dstinfo;
}

bool Scene::SetWindow(Window win)
{
	World3D*	world = World3D::Get();

	if (win == NULL)
	{
		if ((m_pDevInfo == NULL) || (m_pDevInfo->WinHandle == 0))
			VX_TRACE(Scene::Debug, ("Scene::SetWindow NULL"));
		m_pDevInfo->WinHandle = 0;
/*
 * If running multiple threads, shut down the other scene threads
 */
#ifdef VX_NOTHREAD
		Exit();
		EndThread(NULL);
		DoExit = true;
#else
		TLS* g = GetTLS();
		if (!IsExit() && (g->ThreadType & (SCENE_DisplayThread | SCENE_RenderThread)) == 0)
			s_Threads->KillAll(true);
		else
			s_Threads->DoExit = true;
#endif
		return true;
	}
	if (win == (Window) m_pDevInfo->WinHandle)	// no net change
		return true;
	if (m_pDevInfo->WinHandle)			// already have a window?
		return false;
	else								// start scene for this window
	{
		m_pDevInfo->WinHandle = (Window) win;
		DeviceInfo* devinfo = (DeviceInfo*) world->GetProp(PROP_DevInfo);	// get OS-dependent info
		if (devinfo == NULL)						// should be some
			return false;
		m_pDevInfo->WinHandle = (Window) win;
		m_pDevInfo->DispHandle = devinfo->DispHandle;
		m_pDevInfo->VisHandle = devinfo->VisHandle;
		if (!Run())
			return false;
		VX_TRACE(Scene::Debug, ("Scene::SetWindow %x", win));
		world->AddScene(this, win);
	}
	m_pDevInfo->WinHandle = (Window) win;
	return true;
}

void Scene::Append(Scene* child)
{
	Suspend();
	VX_ASSERT(child->m_pDevInfo->Device == NULL);
	VX_ASSERT(child->m_Child == NULL);
	VX_ASSERT(child->m_Parent == NULL);
	VX_ASSERT(m_Parent == NULL);
	*(child->m_pDevInfo) = *m_pDevInfo;
	child->m_LightList = m_LightList;
	child->m_Engines = (Engine*) NULL;	// do not inherit engines from parent scene
	child->m_Child = m_Child;			// link into the render list
	m_Child = child;
	child->m_Parent = this;
	child->DisableOptions(DOUBLEBUFFER);
	Resume();
}

Scene::~Scene()
{
	delete m_pDevInfo;
	m_pDevInfo = NULL;
}

#ifdef VX_NOTHREAD

void Scene::Suspend()	{ }

void Scene::Resume()	{ }

void Scene::Repaint()		{ DoFrame(); }

void Scene::AllowUpdate()		{ }

#else			// VX_NOTHREAD

void Scene::Suspend()
{
	int renderthread = GetTLS()->ThreadType & (SCENE_RenderThread | SCENE_DisplayThread);
	if (!renderthread && m_pDevInfo->WinHandle && !IsExit() && (m_ChangePending++ < 1))
		m_UpdateOK.Wait();
}

void Scene::Resume()
{
	if (m_ChangePending <= 0)
		return;
	int renderthread = GetTLS()->ThreadType & (SCENE_RenderThread | SCENE_DisplayThread);
	if (!renderthread && m_pDevInfo->WinHandle && (--m_ChangePending <= 0))
		m_UpdateDone.Release();
};

/*!
 * @fn void Scene::Repaint()
 * @param clearopts	options flags to clear before resuming
 *
 * Forces scene graph display for at least one frame.
 * If in continuous refresh mode (the default), this routine does nothing.
 * If SCENE_Repaint is enabled, scene display processing suspends after each frame.
 * Calling Scene::Repaint resumes display again for another frame.
 * This permits you to switch between continuous refresh and explicit repaint mode.
 *
 * @see Scene::Suspend Scene::Resume Scene::SetOptions
 */
void Scene::Repaint()
{
	int opts = m_Options;
	if (opts & REPAINT)
		Resume();
}

void Scene::AllowUpdate()
{
	if (m_ChangePending)		// remote updater waiting?
	{
		m_UpdateOK.Release();	// signal remote update legal here
		m_UpdateDone.Wait();	// wait for remote update to finish
	}
//	Sleep(0);
}
#endif

}

