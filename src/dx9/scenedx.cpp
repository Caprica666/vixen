#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen {


static HRESULT hr;	// so you can see it in the debugger in release builds

Renderer*	DXScene::InitRender(Vixen::Window win)
{
	Renderer* gs = GetRenderer();
	if (gs == NULL)
	{
		m_Changed |= SCENE_WindowChanged;
		gs = new DXRenderer(this);
		gs->Init(win, RenderOptions);
		m_Renderer = gs;
	}
	return gs;
}


/*!
 * @fn void DXScene::EndThread(SceneThread* thread)
 * This routine is called whenever a thread used for traversing or
 * rendering a scene exits. Threads marked for rendering release
 * the DirectX resources attached to the scene.
 *
 * @see Scene::EndThread Scene::InitThread
 */
void DXScene::EndThread(SceneThread* thread)
{
	if (GetTLS()->ThreadType & SCENE_RenderThread)
	{
		Empty();
		m_pDevInfo->Device = NULL;		// same as d3dDevice
		if (m_pDevInfo->DC)
		{
			::ReleaseDC(m_pDevInfo->WinHandle, m_pDevInfo->DC);
			m_pDevInfo->DC = NULL;
		}
		VX_WARNING(("Scene::EndThread Releasing DirectX resources"));
	}
	Scene::EndThread(thread);
}


bool DXScene::OnEvent(Event* event)
{
	LoadEvent*	fe = (LoadEvent*) event;

	if (event->Code != Event::SAVE_SCREEN)
		return false;
	return true;
}

}	// end Vixen