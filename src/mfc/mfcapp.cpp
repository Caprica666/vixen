#include "vxmfc.h"
#include "mfc/cameradlg.h"
#include "mfc/framectr.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

namespace Vixen {
namespace MFC {

/*!
 * @note This module cannot be compiled with "Optimize Speed" options
 * under Windows. "Default" or "Debug" options work fine. Maximum speed
 * optimization breaks scene switching causing a new scene to never be displayed
 */

BEGIN_MESSAGE_MAP(App, CWinApp)
	//{{AFX_MSG_MAP(Demo)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_REGISTERED_THREAD_MESSAGE(s_SceneChange, OnSetScene)
END_MESSAGE_MAP()

const UINT App::s_SceneChange = RegisterWindowMessage(TEXT("SceneChange"));


/*!
 * @fn void App::OnSetScene(WPARAM hwnd, LPARAM val)
 * @param hwnd		handle to window whose scene we are replacing
 *
 * The \b SceneChanged message is sent when World3D::SetScene is
 * called to request a scene change from the user interface thread.
 * Scene changes are done here because sometimes they involve
 * calling MFC functions which can only be done from this thread.
 * You should NOT call MFC functions from within an override of
 * World3D::SetScene because this function is not called
 * from the user interface thread.
 *
 * @see World3D::SetScene	
 */
void App::OnSetScene(WPARAM hwnd, LPARAM val)
{
	CFrameWnd* view = (CFrameWnd*) AfxGetApp()->GetMainWnd();
	if (view)
		view->GetActiveDocument()->UpdateAllViews(NULL, NULL, NULL);
}

void App::OnAppAbout() { }

/*!
 * @fn BOOL App::InitInstance()
 *
 * Scene manager override of CWinApp::InitInstance. The base
 * implementation sets up a document template that uses DemoView,
 * Frame and Doc. If your application subclasses any of these,
 * you will need to override  InitInstance to set up your document template
 * correctly. In this case, you should not call the App implementation
 * but instead copy the code after the document template setup into
 * your own InitInstance.
 *
 * Only MFC specific code should be in InitInstance.
 * Scene manager world initialization is handled separately by
 * World3D::OnInit, which you should explicitly call from InitInstance
 * if you override it.
 *
 * @see World3D::OnInit World3D::Run
 */
BOOL App::InitInstance()
{
	// Allow the AppFrame application object to do some
	//  scene manager initialization
	Demo* w = Demo::Get();
	if (w == NULL)
	{
		Vixen::MakeWorld();
		w = Demo::Get();
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate *pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(Doc),
		RUNTIME_CLASS(Frame),       // main SDI frame window
		RUNTIME_CLASS(DemoView));
	AddDocTemplate(pDocTemplate);

	// Do not parse command line for standard shell commands, DDE, file open
	// Scene manager does its own parsing with ParseArgs
	// ProcessShellCommand is needed to set up MFC stuff
	CCommandLineInfo cmdInfo;
//	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
#ifdef _DEBUG
	/*
     * Set the debug-heap flag to keep freed blocks in the
     * heap's linked list - This will allow us to catch any
     * inadvertent use of freed memory
	 */
	if (Core::Allocator::Debug > 3)
		Core::EnableMemoryDebug();
#endif
	w->ParseArgs(m_lpCmdLine);
	if (w->OnInit())
	{
	// The one and only window has been initialized, so show and update it.
		if (w->m_IsFullScreen)
			((Frame*) m_pMainWnd)->SetFullScreen(true);
		m_pMainWnd->ShowWindow(SW_SHOW);
		m_pMainWnd->UpdateWindow();
	}
	return CWinApp::InitInstance();
}

int App::ExitInstance()
{
	Demo* w = Demo::Get();
	if (w)
	{
		w->Stop();
		w->Delete();
		World::Shutdown();
	}
	return CWinApp::ExitInstance();
}

Demo::Demo() : Viewer<World3D>()
{
	m_MenuNames = NULL;
	m_iInitialViewpoint = -1;
	m_ThreadID = ::GetCurrentThreadId();
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	// do not enable default light
	m_bCreateLight = false;
}

void Demo::OnExit()
{
	DWORD threadid = (DWORD) m_ThreadID;
	::PostThreadMessage(threadid, WM_QUIT, 0, 0);
	Viewer<World3D>::OnExit();
}

void Demo::Stop()
{
	Viewer<World3D>::Stop();
}

bool Demo::OnEvent(Event* e)
{
	ErrorEvent* errevent;
	SceneEvent* se;
	DWORD		threadid;

	switch (e->Code)
	{
		case Event::ERROR:				// notify user of serious errors
		errevent = (ErrorEvent*) e;
		if (errevent->ErrLevel == 1)
		{
			AfxMessageBox(errevent->ErrString);
			Stop();
		}
		break;

		case Event::SCENE_CHANGE:			// notify MFC of scene changes
		se = (SceneEvent*) e;
		threadid = (DWORD) m_ThreadID;
		::PostThreadMessage(threadid, App::s_SceneChange, (UINT) se->WindowHandle, 0);
		break;
	}
	return Viewer<World3D>::OnEvent(e);
}


/*!
 * @fn void Demo::LoadSceneMenus(const TCHAR* menunames)
 * @param menunames	String designating the names of the
 *		menus to load. All the names are in a
 *		single null-terminated string and are
 *		delimited by tabs. Blanks are allowed
 *		in menu names. If this string is NULL,
 *		all the menus in the IDM_AFMENU resource
 *		are added to the application menubar.
 *		If the input string pointer is -1, no menus are loaded.
 *
 * Designates the 3D scene control menus that should be
 * put into the menu bar of the current application. The menus
 * are placed before the "Help" menu.
 *
 * The scene menus are loaded from the IDM_AFMENU menubar
 * resource in afresource.rc2. To get this functionality,
 * applications must edit their MFC resource file to
 * include afresource.rc2.
 *
 * LoadSceneMenus should only be called once and cannot
 * be undone. All menus may be loaded or individual menus
 * from the available set can be selected by name. Menu names
 * include the & to indicate keyboard acceleration.
 *
 * Unfortunately, at this time, keyboard accelerators are
 * NOT loaded and must be explicitly specified by the
 * application in it's accelerator table.
 */
void Demo::LoadSceneMenus(const TCHAR* menunames)
{
	m_MenuNames = menunames;
}

/*!
 * @fn void Demo::SetScene(Scene* scene, Window win)
 * @param scene		new scene to use for display
 * @param hwnd		handle to window whose scene we are replacing
 *
 * This routine is called from the display thread to change the current scene.
 * If it is called directly from another thread, the command is logged by
 * the messenger and deferred until the start of the next frame.
 *
 * @see World3D::SetScene App::OnSetScene Scene::Suspend
 */
void Demo::SetScene(Scene* scene, Window win)
{
	VX_STREAM_ASYNC_BEGIN(s)
		*s << OP(VX_World3D, WORLD3D_SetScene) << this << scene << int32(win);
	VX_STREAM_ASYNC_END();

	Viewer<World3D>::SetScene(scene, win);

	if (m_bCreateLight)
	{
		Group* light = GetModels()->Find(TEXT("viewer.sunlight"), Group::FIND_DESCEND);
		if (light)
			light->SetActive(true);
		light = GetModels()->Find(TEXT("viewer.headlight"), Group::FIND_DESCEND);
		if (light)
			light->SetActive(true);
	}
	// if the user requested a preset viewpoint, jump to it
	if (m_iInitialViewpoint != -1)
	{
		LocationSaver loc_saver;
		loc_saver.SetTarget(GetScene(win)->GetCamera());
		if (loc_saver.ReadStoredLocations(TEXT("default.loc")))
			loc_saver.JumpToSavedLocation(m_iInitialViewpoint);
	}
}

/*!
 * @fn Engine* Demo::MakeEngines()
 * @param naveng	engine to use for navigation, if NULL Flyer is used
 *
 * Creates the default simulation tree for the application.
 * This tree of engines is the root of the simulation tree
 * for all scenes loaded. If the input scene has its own simulation
 * tree, it is added to the default engine tree before display.
 *
 * The root of the default engine tree is assumed to be the navigation
 * engine (which takes mouse, keyboard or voice input and uses it
 * to navigate the viewed scene). The default implementation also
 * provides a frame counter which displays scene statistics in the
 * MFC status bar and a scriptor which can play back camera movement.
 *
 * @return navigation engine to use (root of default simulation tree)
 *
 * @see Flyer App::InitInstance Scriptor
 */
Engine* Demo::MakeEngines()
{
	if ((GetStatLog() == NULL) && m_StatLogFile.IsEmpty())
	{
		FrameCounter* fc = new FrameCounter;
		fc->SetName(TEXT("viewer.statlog"));
		SetStatLog(fc);
	}
	return Viewer<World3D>::MakeEngines();
}

/*!
 * @fn bool Demo::ParseArgs(int argc, TCHAR** argv)
 *
 * Checks for the following command line arguments:
 * @code
 *	-p <n>	specify initial viewpoint 0-5 from viewpoints file
 * @endcode
 *
 * @see App::InitInstance World3D::OnInit
 */
bool Demo::ParseArgs(int argc, TCHAR** argv)
{
	for (int i = 0; i < argc; ++i)
	{
		const TCHAR* p = argv[i];

		if (p == NULL)
			continue;
		if (*p++ != TEXT('-'))
			continue;
		switch (*p++)
		{
			case TEXT('v'): 
			if (*p == 0)
				p = argv[++i];
			else
				++p;
			m_iInitialViewpoint = ATOL(p);
			break;
		}
	}
	return Viewer<World3D>::ParseArgs(argc, argv);
}

#ifdef VX_USE_SCRIPTOR
Animator* Demo::GetAnimator(const TCHAR* name) const
{
	if ((name == NULL) &&
		((name = GetScriptFile()) == NULL))
		return NULL;

	return (Animator*) (Group*) GetScriptor()->FindAnim(name);
}
#endif

}	// end MFC
}	// end Vixen