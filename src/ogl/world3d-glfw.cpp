#include "vixen.h"

#ifdef VIXEN_GLFW
#include "ogl/vxglfwworld.h"

namespace Vixen {

double WinWorld::CurTime = 0.0;
double WinWorld::FrameTime = 1.0 / 60.0;

void WinWorld::OnExit()
{
	World3D::OnExit();
}


/*!
 * @fn bool WinWorld::OnInit()
 * Creates a new window for display of a 3D scene.
 * The dimensions of the window are specified by World3D::WinRect.
 *
 * @see WinWorld::WinMain WinWorld::WndProc
 */
bool WinWorld::OnInit()
{
	int			width = (int) WinRect.Width();
	int			height = (int) WinRect.Height();
	Window		nativewin = 0;
	int			rc;

	if (!glfwInit())			// Call first to initialize time
		VX_ERROR(("WinWorld::OnInit ERROR failed to initialize GLFW\n"), false);
	if (!World3D::OnInit())		// Initialize application
		return false;
	if (width == 0)
		width = 640;
	if (height == 0)
		height = 480;
	CurTime = StartTime;
	if (Scene::DeviceDepth == 32)
		rc = glfwOpenWindow(width, height, 8, 8, 8, 8, 24, 0,  GLFW_WINDOW);
	else if (Scene::DeviceDepth == 16)
		rc = glfwOpenWindow(width, height, 5, 6, 5, 8, 0, 0,  GLFW_WINDOW);
	else
		rc = glfwOpenWindow(width, height, 0, 0, 0, 0, 16, 0,  GLFW_WINDOW);
	if (!rc)
		VX_ERROR(("World3D::OnInit ERROR cannot create GLFW window"), false);
	glfwDisable(GLFW_AUTO_POLL_EVENTS);
	glfwSetWindowSizeCallback(OnResize);
	glfwSetWindowCloseCallback(OnClose);
	glfwSetWindowRefreshCallback(OnPaint);
	glfwSetMouseButtonCallback(OnMouseButton);
	glfwSetMousePosCallback(OnMouseMove);
	glfwSetKeyCallback(OnKey);
	if (!RawInput)
		glfwSetCharCallback(OnChar);
#ifdef VIXEN_GLES2
	nativewin = (Window) eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
	nativewin = (Window) ::GetForegroundWindow();
#endif
	return Run(nativewin);	// make the scene
}

int	 WinWorld::OnClose()
{
	World3D::Get()->Stop();
	return true;
}

void	 WinWorld::OnPaint()
{
	Scene*	scene = GetMainScene();

	if (scene == NULL)
		return;
	try
	{
		scene->Repaint();
	}
	catch (...)
	{
		VX_WARNING(("WinWorld::OnPaint ERROR: exception thrown"));
	}
	ForceRefresh(scene);
}

void	WinWorld::ForceRefresh(Scene* scene)
{
#if defined(VX_NOTHREAD) && defined(_WIN32)
	if ((scene->GetOptions() & Scene::REPAINT) == 0)
	{
		const DeviceInfo* devinfo = scene->GetDevInfo();

		::InvalidateRect(devinfo->WinHandle, NULL, false);
	}
#endif
}

void	 WinWorld::OnResize(int width, int height)
{
	Scene*	scene = GetMainScene();
	if (scene)
	{
		scene->Suspend();
		scene->SetViewport(0, 0, (float) width, (float) height);
		scene->Resume();
	}
}

void WinWorld::OnKey(int key, int action)
{
	KeyEvent*	k;
	WinWorld*	world = WinWorld::Get();

	switch (key)
	{
		case GLFW_KEY_LSHIFT:
		case GLFW_KEY_RSHIFT:
		if (action == GLFW_PRESS)
			world->MouseMods |= MouseEvent::SHIFT;
		else
			world->MouseMods &= ~MouseEvent::SHIFT;
		break;

		case GLFW_KEY_LCTRL:
		case GLFW_KEY_RCTRL:
		if (action == GLFW_PRESS)
			world->MouseMods |= MouseEvent::CONTROL;
		else
			world->MouseMods &= ~MouseEvent::CONTROL;
		break;
	}
	if (!world->RawInput)
		return;
	k = new KeyEvent;
	k->KeyCode = key;
	k->KeyFlags = (action == GLFW_RELEASE) ? KeyEvent::UP : 0;
	k->Sender = world;
	k->Log();
	world->OnPaint();
}

void WinWorld::OnChar(int key, int action)
{
	KeyEvent*	k;
	WinWorld*	world = WinWorld::Get();

	if (world->RawInput)
		return;
	k = new KeyEvent;
	k->KeyCode = key;
	k->KeyFlags = (action == GLFW_RELEASE) ? KeyEvent::UP : 0;
	k->Sender = world;
	k->Log();
	world->OnPaint();
}

void WinWorld::OnMouseMove(int x, int y)
{
	MouseEvent* m = new MouseEvent;
	WinWorld*	world = WinWorld::Get();

	world->MousePos.Set((float) x, (float) y);
	m->MousePos.Set((float) x, (float) y);
	m->MouseFlags = world->MouseButtons | world->MouseMods;
	m->Sender = WinWorld::Get();
	m->Log();
	world->OnPaint();
}

void WinWorld::OnMouseButton(int button, int action)
{
	MouseEvent* m = new MouseEvent;
	WinWorld*	world = WinWorld::Get();
	int			flags = 0;

	m->MousePos = world->MousePos;
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			flags |= MouseEvent::LEFT;
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			flags |= MouseEvent::RIGHT;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			flags |= MouseEvent::MIDDLE;
	}
	m->MouseFlags = flags | world->MouseMods;
	world->MouseButtons = flags;
	m->Sender = WinWorld::Get();
	m->Log();
	world->OnPaint();
}

#ifdef _WIN32
int WinWorld::WinMain(HINSTANCE instance, TCHAR* lpCmdLine)
{
	ParseArgs(lpCmdLine);
	World3D::Startup();
	if (!OnInit())
		return 0;
	GetMessenger()->Flush();
	while (glfwGetWindowParam(GLFW_OPENED))	// window still open?
		glfwPollEvents();
	return 1;
}
#endif

int WinWorld::Main(int argc, TCHAR** argv)
{
	WinWorld*		world = WinWorld::Get();

	world->IncUse();
	world->ParseArgs(argc, argv);
	world->Startup();
	if (!world->OnInit())
		return 0;
	world->GetMessenger()->Flush();
	while (glfwGetWindowParam(GLFW_OPENED))	// window still open?
		glfwPollEvents();
	return 1;
}

}	// end Vixen

#endif