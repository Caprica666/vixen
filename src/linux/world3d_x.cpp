#include "vixen.h"
#include "winworld.h"

#if defined(VX_FREEIMAGE) && !defined(VX_NOTEXTURE)
#include "FreeImage.h"
#endif

namespace Vixen {

using namespace Core;
VX_IMPLEMENT_CLASS(DeviceInfo, Property);

#ifndef VX_NOTHREAD
/*
 * @class SharedStringPool
 * This subclass of string pool provides thread safety because
 * it locks around all string pool accesses. This is necessary
 * if you will have multiple threads making or deleting strings.
 * @internal
 * @ingroup vixen
 */
class SharedStringPool : public StringPool
{
public:
	SharedStringPool (GetAllocatorFunc func) : StringPool(func) { }

	void*	Alloc (size_t n)
	{ m_Lock.Enter(); void* p = StringPool::Alloc(n); m_Lock.Leave(); return p; }

	void	Free (void* ptr)
	{ m_Lock.Enter(); StringPool::Free(ptr); m_Lock.Leave(); }

	void	Free (void* ptr, size_t amount)
	{ m_Lock.Enter(); StringPool::Free(ptr, amount); m_Lock.Leave(); }

	void	FreeAll()
	{ m_Lock.Enter(); StringPool::FreeAll(); m_Lock.Leave(); }

protected:
	Core::CritSec		m_Lock;
};
#endif

void _cdecl World::Shutdown()
{
#ifndef VX_NOTHREAD
	CritSec::DoLock = false;	// disable locking
#endif
	CoreExit();
}

bool World::Startup()
{
	if (!CoreInit())
		return false;

	s_OnlyOne = NULL;
	BufMessenger::s_OnlyOne = NULL;

#ifdef VX_NOTHREAD
	Core::PoolAllocator* objalloc = (Core::PoolAllocator*) Core::PoolAllocator::Get();
	objalloc->SetOptions(ALLOC_ZeroMem);
	CLASS_(BaseObj)->SetAllocator(objalloc);

#else
	/*
	 * replace old string pool with locking version
	 */
	delete _vStringPool;
	CritSec::DoLock = true;
	_vStringPool = new SharedStringPool(GlobalAllocator::Get);
	_vStringPool->SetOptions(ALLOC_Lock);
	_vInitThreadData[0].pAlloc = _vStringPool;
	_vThreadData = (StringData*) &_vInitThreadData[0];
	_vEmptyStr = _vThreadData->data();
	/*
	 * replace object allocator with locking one
	 */
	PoolAllocator* objalloc = (PoolAllocator*) PoolAllocator::Get();
	BytePool*	pool = new (GlobalAllocator::Get()) BytePool(16, 8192);
#ifdef _DEBUG
	pool->SetOptions(ALLOC_ZeroMem | ALLOC_Lock);
	objalloc->SetOptions(ALLOC_ZeroMem | ALLOC_Lock);
#else
	pool->SetOptions(ALLOC_Lock);
	objalloc->SetOptions(ALLOC_Lock);
#endif
	objalloc->InitPools(pool, 6);
	CLASS_(BaseObj)->SetAllocator(objalloc);
#endif
	return true;
}

#if defined(VIXEN_OGL) || defined(VIXEN_GLES2)

/*!
 * @fn Scene* World3D::NewScene(Window win)
 * @param win	window to associate with the display scene
 *				if NULL, no window is bound to scene
 *
 * Class factory for the display scene. This function
 * is called once by the framework at startup to instantiate
 * a scene bound to a display (or capable of multi-threaded
 * operation). Usually, this scene is used throughout for
 * communicating with the display device. Changing the
 * current scene causes the existing display scene to
 * reference a new simulation tree and scene graph.
 *
 * Each port of the scene manager supplies a version of NewScene that
 * constructs a display scene bound to the appropriate device.
 * If you supply your own implementation for World3D::NewScene,
 * the library version will not be used. You may want to do this
 * to enable multi-threaded rendering.
 *
 * @return pointer to an empty display scene attached to the window
 *
 * @see World3D::SetScene Scene World3D::MakeScene
 */
Scene* World3D::NewScene(Window win)
{
#ifdef VX_NOTHREAD
	GLScene*	display_scene = (GLScene*) GetScene(win);
	GLScene*	scene;
	if (display_scene)
	{
		scene =  new GLScene(*display_scene);
		display_scene->Append(scene);
		return scene;
	}
	scene = new GLScene;
#else
	DualScene<GLScene>* display_scene = (DualScene<GLScene>*) GetScene(win);
	DualScene<GLScene>*	scene;
	if (display_scene)
	{
		scene =  new DualScene<GLScene>(*display_scene);
		display_scene->Append(scene);
		return scene;
	}
	scene = new DualScene<GLScene>;
	scene->SetNumThreads(0);
#endif
	//scene->EnableOptions(SCENE_ClearAll | SCENE_StateSort | SCENE_DoubleBuffer);
	scene->SetOptions(Scene::CLEARALL | Scene::STATESORT | Scene::DOUBLEBUFFER);
	if (win && !scene->SetWindow(win))
	{
		scene->Delete();
		return NULL;
	}
	return scene;
}
#endif

void World3D::OnExit()
{
	m_Scenes.Empty();
#pragma message ("OnExit Fix ME")
	// RAM_XX Core::NetStream::Shutdown();
	World::OnExit();
	Bitmap::Shutdown();
	if (Matrix::s_IdentityMatrix)			// clean up identity matrix
	{
		Matrix::s_IdentityMatrix->Delete();
		Matrix::s_IdentityMatrix = NULL;
	}
	if (Appearance::s_Default)			// clean up default appearance
	{
		Appearance::s_Default->Delete();
		Appearance::s_Default = NULL;
	}
#if defined(VX_FREEIMAGE) && !defined(VX_NOTEXTURE)
	FreeImage_DeInitialise();
#endif
}


WinWorld::WinWorld() : World3D()
{
	m_ThreadID = pthread_self();
}

void WinWorld::OnExit()
{
	World3D::OnExit();
}


bool WinWorld::OnInit()
{
	Window win = MakeWindow();
	if (win == 0)
		return false;
	if (!World3D::OnInit())
		return false;
	return Run((intptr) win);			// make the scene
}

int WinWorld::WinMain(int argc, char **argv)
{
	Scene*	scene = NULL;
	DeviceInfo* devinfo;

	ParseArgs(argc, argv);
	if (!OnInit())
		return 0;

	devinfo = (DeviceInfo*) GetProp(PROP_DevInfo);
	while (1)
	{
        /*  if at least one event queued, process it ...  */
		if (XPending(devinfo->DispHandle))
		{
			if (!ProcessEvent())
				exit(EXIT_SUCCESS);
		}
		else
		{
			if (scene == NULL)
				scene = GetScene(Window(devinfo->WinHandle));	// find scene for our window
			if (scene)
				scene->Repaint();
		}
	}
}


Window WinWorld::MakeWindow()
{
	Window			win;
	Display*		dpy;
	XVisualInfo*	vi;
	XSetWindowAttributes swa;
	DeviceInfo*		devinfo = (DeviceInfo*) GetProp(PROP_DevInfo);
	static int attributeList[] = {
		GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER, 1, GLX_DEPTH_SIZE, 1, None };

	if (devinfo == NULL)
	{
		devinfo = new DeviceInfo;
		AddProp(devinfo);
	}
	devinfo->DispHandle = dpy = XOpenDisplay(0);
	devinfo->Device = 0;
	devinfo->VisHandle = NULL;
	devinfo->WinHandle = 0;

	if (!dpy)
	{ VX_ERROR(("WinWorld: can't open display"), 0); }
	vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeList);
	if (!vi)
	{ VX_ERROR(("WinWorld: no suitable visual"), 0); }
	swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
	swa.border_pixel = 0;
	swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask |
		ButtonPressMask | ButtonReleaseMask |
		Button1MotionMask | Button2MotionMask | Button3MotionMask;
	/*  note: ButtonMotionMask is also valid but can be confusing during event parse */
	if (WinRect.Width() == 0)
		WinRect.Set(0, 0, 640, 480);
	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
			(int) WinRect.min.x, (int) WinRect.min.y,
			(int) WinRect.max.x, (int) WinRect.max.y,
			0, vi->depth, InputOutput, vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &swa);
	XStoreName(dpy, win, "vixenx");
	XMapWindow(dpy, win);
	devinfo->WinHandle = win;
	devinfo->DispHandle = dpy;
	devinfo->VisHandle = vi;
	return win;
}

bool WinWorld::ProcessEvent()
{
	KeyEvent*		ke;
	MouseEvent*		me;
	XEvent			theEvent;
	XEvent*			xevent;
	char			buf[31];
	KeySym			keysym;
	Scene*			scene;
	uint			mask_return;
	Window			tempWindow;
	int				tempInt;
	DeviceInfo*		devinfo = (DeviceInfo*) GetProp(PROP_DevInfo);

	do
	{
		/*  wait at XNextEvent() until new event is generated  */
		XNextEvent(devinfo->DispHandle, &theEvent);
		xevent = &theEvent;
		switch (xevent->type)
		{
			case ConfigureNotify:  /*  window moved or changed size  */
					scene = GetScene(Window(devinfo->WinHandle));
				if (scene)
				{
					scene->Suspend();
					scene->SetViewport(0, 0, xevent->xconfigure.width, xevent->xconfigure.height);
					scene->Resume();
				}
				break;

			case DestroyNotify:
			winkill:
				scene = GetScene(Window(devinfo->WinHandle));
				if (scene)			                      // kill scene for this window
					scene->SetWindow(NULL);
				return true;

			case KeyPress:
				XLookupString(&theEvent.xkey, buf, sizeof(buf), &keysym, NULL);
				if (keysym == XK_Escape)					/*  check for quit key  */
					goto winkill;
			/*  generate key event */
				ke = new KeyEvent;
				ke->Sender = this;
				ke->KeyCode = xevent->xkey.keycode;  /* copy keycode from X event to Vixen event  */
				ke->KeyFlags = 0;
				ke->Log();
				break;

			case ButtonPress:
			case MotionNotify:
			case ButtonRelease:
				me = new MouseEvent;

				me->MousePos.Set(float(xevent->xbutton.x), float(xevent->xbutton.y));
				me->MouseFlags = 0;

				XQueryPointer(devinfo->DispHandle, devinfo->WinHandle, &tempWindow, &tempWindow, &tempInt,
					&tempInt, &tempInt, &tempInt, &mask_return);

				if (mask_return & Button1Mask)
					me->MouseFlags |= MouseEvent::LEFT; // RAM_XX EVENT_MouseLeft;
				if (mask_return & Button2Mask)
					me->MouseFlags |= MouseEvent::MIDDLE; // RAM_XX EVENT_MouseMiddle;
				if (mask_return & Button3Mask)
					me->MouseFlags |= MouseEvent::RIGHT; // RAM_XX EVENT_MouseRight;
				me->Sender = this;
				me->Log();
				break;

			default:
				return true;  /*  Passes it on if unproccessed  */
		}
	}
	while (XPending(devinfo->DispHandle));
	return true;
}

}	// end Vixen
