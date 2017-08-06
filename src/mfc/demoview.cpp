#include "vxmfc.h"
#include "mfc/framectr.h"
#include "util/vxarcball.h"

namespace Vixen {
namespace MFC {

IMPLEMENT_DYNCREATE(DemoView, View)

DemoView::DemoView()
{
	m_bSpinScene = false;
	m_bSpinLight = false;
	m_bWireframe = false;
	m_bShowInfo = false;
	m_sceneMenu = NULL;
}

DemoView::~DemoView()
{
}

void DemoView::SetWireframe(bool bOn)
{
	Scene *scene = GetMainScene();
	Model* root;

	scene->Suspend();
	root = scene->GetModels();
	if (root)
		if (bOn)
			Appearance::Apply(root, Appearance::SHADING, Appearance::WIRE); 
		else
			Appearance::Apply(root, Appearance::SHADING, Appearance::SMOOTH); 
	scene->Resume();
}

void DemoView::ToggleWireframe()
{
	m_bWireframe = !m_bWireframe;
	SetWireframe(m_bWireframe);
}

void DemoView::ZoomAll()
{
	Demo*		app = Demo::Get();
	Scene*	scene = GetScene();
	Camera*	cam;
	Transformer* eng;

	if (!scene)
		return;
	cam = scene->GetCamera();
	eng = (Transformer*) app->GetNavigator();
	scene->ShowAll();
	if (eng && eng->IsClass(VX_Transformer))
		eng->SetTransform(cam->GetTransform());
}


BEGIN_MESSAGE_MAP(DemoView, View)
	//{{AFX_MSG_MAP(DemoView)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_VIEW_SCENEGRAPH, OnViewScenegraph)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCENEGRAPH, OnUpdateViewScenegraph)
	ON_COMMAND(ID_VIEW_ZOOMALL, OnViewZoomall)
	ON_COMMAND(ID_VIEW_FRAMERATECOUNTER, OnViewFrameratecounter)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRAMERATECOUNTER, OnUpdateViewFrameratecounter)
	ON_COMMAND(ID_VIEW_CAMERA, OnCamdialog)
	ON_COMMAND(ID_VIEW_WIREFRAME, OnViewWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME, OnUpdateViewWireframe)
	ON_COMMAND(ID_VIEW_DISABLELIGHTING, OnDisableLighting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISABLELIGHTING, OnUpdateDisableLighting)
	ON_COMMAND(ID_SCPT_OPEN, OnScptOpen)
	ON_COMMAND(ID_SCPT_RECORD, OnScptRecord)
	ON_COMMAND(ID_SCPT_RUN, OnScptPlay)
	ON_COMMAND(ID_SCPT_STOP, OnScptStop)
	ON_UPDATE_COMMAND_UI(ID_SCPT_RECORD, OnUpdateScptRecord)
	ON_COMMAND(ID_SCPT_REPLAY, OnScptReplay)
	ON_UPDATE_COMMAND_UI(ID_SCPT_REPLAY, OnUpdateScptReplay)
	ON_UPDATE_COMMAND_UI(ID_SCPT_STOP, OnUpdateScptStop)
	ON_COMMAND(ID_SCPT_REVERSE, OnScptReverse)
	ON_UPDATE_COMMAND_UI(ID_SCPT_REVERSE, OnUpdateScptReverse)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_NAV_ARCBALL, OnNavArcball)
	ON_COMMAND(ID_NAV_FLYER, OnNavFlyer)
	ON_COMMAND(ID_NAV_DRIVER, OnNavDriver)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef _DEBUG
void DemoView::AssertValid() const
{
	CView::AssertValid();
}

void DemoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

void DemoView::OnDropFiles(HDROP hDropInfo) 
{
	TCHAR buf[256];

	UINT result = DragQueryFile(hDropInfo,
		0,		// index of file to query 
		buf,	// buffer for returned filename 
		256);	// size of buffer for filename 
}

void DemoView::OnViewScenegraph() 
{
	if (m_wndTreeDlg.GetSafeHwnd() == NULL)
	{
		if (!m_wndTreeDlg.Create(IDD_TREEVIEW, this))
		{
			TRACE0("Failed to create tree dialog\n");
		}
		m_wndTreeDlg.ShowWindow(SW_HIDE);
	}
	m_wndTreeDlg.RefreshTreeContents(GetMainScene());	
	if (m_wndTreeDlg.IsWindowVisible())
		m_wndTreeDlg.ShowWindow(SW_HIDE);
	else
		m_wndTreeDlg.ShowWindow(SW_SHOW);
}

void DemoView::OnUpdateViewScenegraph(CCmdUI* pCmdUI) 
{
	if (m_wndTreeDlg.GetSafeHwnd() == NULL)
	{
		pCmdUI->SetCheck(FALSE);
	}
	else
	{
		pCmdUI->SetCheck(m_wndTreeDlg.IsWindowVisible());
	}
}

void DemoView::OnViewZoomall() 
{
	ZoomAll();
}

void DemoView::OnViewFrameratecounter() 
{
	Demo*	app = Demo::Get();
	FrameCounter* framectr = (FrameCounter*) app->GetStatLog();
	if (!framectr)
		return;
	if (framectr->IsRunning())
		framectr->Stop();
	else
		framectr->Start();
}

void DemoView::OnUpdateViewFrameratecounter(CCmdUI* pCmdUI) 
{
	Demo*	app = Demo::Get();
	Engine *pCounter = app->GetStatLog();
	if (pCounter)
		pCmdUI->SetCheck(pCounter->IsRunning());
}

void DemoView::OnCamdialog() 
{
	if (m_wndCameraDlg.GetSafeHwnd() == NULL)
	{
		if (!m_wndCameraDlg.Create(IDD_CAMDIALOG, this))
		{
			TRACE0("Failed to create camera dialog\n");
		}
		else
		{
			Camera* cam = GetMainScene()->GetCamera();
			Transformer* xform = (Transformer*) Demo::Get()->GetNavigator();
			m_DefaultLocSaver.ReadStoredLocations(TEXT("default.loc"));
			m_wndCameraDlg.SetLocationSaver(&m_DefaultLocSaver);
			m_wndCameraDlg.SetCamera(cam);
			if (xform && xform->IsClass(VX_Transformer))
				xform->SetTransform(cam->GetTransform());
		}
	}
	m_wndCameraDlg.ShowWindow(SW_SHOW);
}


void DemoView::OnViewWireframe() 
{
	ToggleWireframe();
}

void DemoView::OnUpdateViewWireframe(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bWireframe);
}

void DemoView::OnDisableLighting() 
{
	Appearance::DoLighting = !Appearance::DoLighting;
}

void DemoView::OnUpdateDisableLighting(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(!Appearance::DoLighting);
}

void DemoView::OnScptOpen() 
{
#ifdef VX_USE_SCRIPTOR
	Demo*			app = Demo::Get();
	const TCHAR*	dir = app->GetMediaDir();
	CFileDialog		openDialog(TRUE, TEXT("*.bvh"), TEXT("test.bvh"));

	openDialog.m_ofn.lpstrFilter = TEXT("BVH Animation Files (*.bvh)\0*.bvh\0Vixen Scripts (*.scp)\0*.scp\0Vixen Files (*.vix)\0*.vix\0All Files (*.*)\0*.*\0\0");
	if (dir)
		openDialog.m_ofn.lpstrInitialDir = dir;
	else
		openDialog.m_ofn.lpstrInitialDir = TEXT("..\\..\\Data");
	if (openDialog.DoModal() == IDOK)
	{
		CString			filename(openDialog.GetPathName());
		Scriptor*		scp = app->GetScriptor();
		const TCHAR*	engname = NULL;

		if (scp)
		{
			Scene*		scene = GetScene();
			Engine*		simroot = scene->GetEngines();
			Skeleton*	skel = app->GetSkeleton();
			SharedObj*	target = NULL;

			app->SetScriptFile(filename);
			if (filename.Right(3).CompareNoCase(TEXT("scp")) == 0)
			{
				scp->LoadScript(filename);
				return;
			}
			if (skel == NULL)
			{
				skel = (Skeleton*) simroot->Find(TEXT("skeleton"), Group::FIND_ACTIVE | Group::FIND_END | Group::FIND_DESCEND);
				app->SetSkeleton(skel);
			}
			target = skel;
			if (skel)
			{
				engname = skel->GetName();
				engname = STRCHR(engname, TEXT('.')) + 1;
			}
			else
				target = scp->GetTarget();
			scp->Load(filename, engname, Animator::AUTO_PLAY, target);
		}
	}
#endif
}

void DemoView::OnScptPlay() 
{
	Demo*			app = Demo::Get();
	Scene*			scene = GetScene();
	Engine*			simroot = scene->GetEngines();
	NavRecorder*	nav = GetNavRecorder();
	SharedObj*		targ = NULL;

	if (nav && nav->GetMessenger())
	{
		nav->Stop();
		nav->SetMessenger(NULL);
	}
#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = app->GetScriptor();
	Skeleton*	skel = app->GetSkeleton();

	if (scp)
	{
		scp->Begin();
		return;
	}
#endif
	else if (simroot)
	{
		simroot = simroot->GetAt(1);
		if (simroot)
			simroot->Start(Engine::CONTROL_CHILDREN);
	}
	m_wndTreeDlg.RefreshTreeContents(scene);
}


void DemoView::PlayAnimation(const TCHAR* filename, SharedObj* target) 
{
	Demo*		app = Demo::Get();
	Engine*		simroot = GetMainScene()->GetEngines();
	int32		opts = 0;

	Scriptor*	scp = app->GetScriptor();
	if (filename)
	{
		Animator*	anim = app->GetAnimator();
		app->SetScriptFile(filename);
		if (anim)
			opts = anim->GetOptions();
		anim = scp->Load(filename, NULL, opts | Animator::AUTO_PLAY, target);
		return;
	}
	if (scp)
	{
		scp->Begin();
		return;
	}
	if (simroot)
		simroot = simroot->GetAt(1);
	if (simroot && (simroot = simroot->GetAt(1)))
		simroot->Start(Engine::CONTROL_CHILDREN);
}

NavRecorder* DemoView::GetNavRecorder() const
{
	Navigator*	nav = (Navigator*) Demo::Get()->GetNavigator();
	NavRecorder*	rec = NULL;

	if (nav == NULL)
		return NULL;
	GroupIter<Engine>	iter(nav, Group::CHILDREN);
	while (rec = (NavRecorder*) (Engine*) iter.Next())
		if (rec->IsClass(VX_NavRecorder))
			return rec;
	return NULL;
}

void DemoView::OnScptRecord() 
{
	NavRecorder*	rec = GetNavRecorder();
	Messenger*	m = NULL;

	if (rec == NULL)
	{
		rec = new NavRecorder;
		Scene* scene = GetMainScene();
		if (scene)
		{
			scene->Suspend();		// lock around hierarchy changes
			Engine*	nav = Demo::Get()->GetNavigator();
			nav->Append(rec);
			scene->Resume();
		}
		else
			Demo::Get()->GetNavigator()->Append(rec);
	}
	else
		m = rec->GetMessenger();

	if (m == NULL)
	{
		m = new FileMessenger;
		rec->SetMessenger(m);
		m->SetOutStream(new Core::FileStream);
	}
#ifdef VX_USE_SCRIPTOR
	const TCHAR*	filename = Demo::Get()->GetScriptFile();
	if (filename == NULL)
		return;
#else
	const TCHAR*	filename = "demo.dat";
#endif
	m->Open(filename, Messenger::OPEN_WRITE);
	if (!rec->IsRunning())
	{
		rec->SetTarget(GetMainScene()->GetCamera());
		rec->Start();
	}
}

void DemoView::OnUpdateScptRecord(CCmdUI* pCmdUI) 
{
	NavRecorder*	nav = GetNavRecorder();
	bool			isrecording = nav && nav->IsRunning();

	pCmdUI->Enable(!isrecording);
}

void DemoView::OnScptStop() 
{
	Demo*			app = Demo::Get();
	NavRecorder*	nav = GetNavRecorder();

	if (nav)
	{
		nav->Stop();
		nav->SetMessenger(NULL);
	}
#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = app->GetScriptor();
	if (scp)
	{
		scp->Stop();
		return;
	}
	else
#endif
	{
		Engine*	simroot = GetMainScene()->GetEngines();
		if (simroot)
			simroot = simroot->GetAt(1);
		if (simroot)
			simroot->Stop(Engine::CONTROL_CHILDREN);
	}
}

void DemoView::OnUpdateScptStop(CCmdUI* pCmdUI) 
{
	HMENU		hMenu = AfxGetApp()->m_pMainWnd->GetMenu()->m_hMenu;
	Scene*		scene = GetScene();
	Engine*		simroot = scene->GetEngines();
	NavRecorder*	nav = GetNavRecorder();

	CheckMenuItem(hMenu,ID_SCPT_STOP, MF_UNCHECKED);
#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = Demo::Get()->GetScriptor();
	if (scp && scp->IsRunning())
	{
		Animator*	anim = scp->FindAnim(NULL, scp->GetTarget());
		if (anim)
		{
			int opts = anim->GetOptions();
			if (anim->IsRunning())
				CheckMenuItem(hMenu, ID_SCPT_STOP, MF_CHECKED);
			return;
		}
	}
#endif
	if (nav && nav->IsRunning())
		CheckMenuItem(hMenu,ID_SCPT_STOP, MF_CHECKED);	
}

void DemoView::OnScptReplay() 
{
	Scene*		scene = GetScene();
	Engine*		simroot = scene->GetEngines();

#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = Demo::Get()->GetScriptor();
	if (scp)
	{
		Animator*	anim = scp->FindAnim(NULL, scp->GetTarget());
		if (anim)
		{
			int opts = anim->GetOptions();
			if (anim->GetControl() & Engine::CYCLE)
				anim->Disable(Engine::CYCLE);
			else
				anim->Enable(Engine::CYCLE);
			return;
		}
	}
#endif
	if (simroot)
		simroot = simroot->GetAt(1);
	if (simroot)
	{
		if (simroot->GetControl() & Engine::CYCLE)
			simroot->Disable(Engine::CYCLE, Engine::CONTROL_CHILDREN);
		else
			simroot->Enable(Engine::CYCLE, Engine::CONTROL_CHILDREN);
	}
}

void DemoView::OnUpdateScptReplay(CCmdUI* pCmdUI) 
{
	HMENU		hMenu = AfxGetApp()->m_pMainWnd->GetMenu()->m_hMenu;
	Scene*		scene = GetScene();
	Engine*		simroot = scene->GetEngines();

	CheckMenuItem(hMenu,ID_SCPT_REPLAY,MF_UNCHECKED);			return;
#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = Demo::Get()->GetScriptor();
	if (scp)
	{
		Animator*	anim = scp->FindAnim(NULL, scp->GetTarget());
		if (anim)
		{
			if (anim->GetControl() & Engine::CYCLE)
				CheckMenuItem(hMenu,ID_SCPT_REPLAY, MF_CHECKED);	
			return;
		}
	}
#endif
	if (simroot)
		simroot = simroot->GetAt(1);
	if (simroot)
	{
		if (simroot->GetControl() & Engine::CYCLE)
			CheckMenuItem(hMenu,ID_SCPT_REPLAY, MF_CHECKED);	
	}
}

void DemoView::OnUpdateScptReverse(CCmdUI* pCmdUI) 
{
	HMENU		hMenu = AfxGetApp()->m_pMainWnd->GetMenu()->m_hMenu;
	Scene*		scene = GetScene();
	Engine*		simroot = scene->GetEngines();

	CheckMenuItem(hMenu,ID_SCPT_REVERSE,MF_UNCHECKED);
#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = Demo::Get()->GetScriptor();
	if (scp)
	{
		Animator*	anim = scp->FindAnim(NULL, scp->GetTarget());
		if (anim)
		{
			if (anim->GetControl() & Engine::REVERSE)
				CheckMenuItem(hMenu, ID_SCPT_REVERSE, MF_CHECKED);	
			return;
		}
	}
#endif
	if (simroot)
		simroot = simroot->GetAt(1);
	if (simroot)
	{
		if (simroot->GetControl() & Engine::REVERSE)
			CheckMenuItem(hMenu,ID_SCPT_REVERSE, MF_CHECKED);	
	}
}


void DemoView::OnScptReverse() 
{
	Scene*		scene = GetScene();
	Engine*		simroot = scene->GetEngines();

#ifdef VX_USE_SCRIPTOR
	Scriptor*	scp = Demo::Get()->GetScriptor();
	if (scp)
	{
		Animator*	anim = scp->FindAnim(NULL, scp->GetTarget());
		if (anim)
		{
			int opts = anim->GetOptions();
			if (anim->GetControl() &  Engine::REVERSE)
				opts &= ~ Engine::REVERSE;
			else
				opts |=  Engine::REVERSE;
			anim->SetOptions(opts);
			return;
		}
	}
#endif
	if (simroot)
		simroot = simroot->GetAt(1);
	if (simroot)
	{
		if (simroot->GetControl() & Engine::REVERSE)
			simroot->Disable(Engine::REVERSE, Engine::CONTROL_CHILDREN);
		else
			simroot->Enable(Engine::REVERSE, Engine::CONTROL_CHILDREN);
	}
}

//
// Refresh tree dialog whenever document (scene graph) is updated
//
void DemoView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
	CView::OnUpdate (pSender, lHint, pHint);
	DemoView::m_wndTreeDlg.RefreshTreeContents(GetMainScene());	
}


int DemoView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (View::OnCreate(lpCreateStruct) == -1)
		return -1;
	LoadSceneMenus(Demo::Get()->GetSceneMenus());
	return 0;
}

/*++++
 *
 * Name: DemoView_LoadSceneMenus
 *	bool LoadSceneMenus(char* menunames)
 *
 * Description:
 *	Dynamically loads the 3D scene control menus into
 *	the menu bar of the current application. The menus
 *	are placed before the "Help" menu.
 *
 *	The scene menus are loaded from the IDM_AFMENU menubar
 *	resource in afresource.rc2. To get this functionality,
 *	applications must edit their MFC resource file to
 *	include afresource.rc2.
 *
 *	LoadSceneMenus should only be called once and cannot
 *	be undone. All menus may be loaded or individual menus
 *	from the available set can be selected by name. Menu names
 *	include the & to indicate keyboard acceleration.
 *
 *	Unfortunately, at this time, keyboard accelerators are
 *	NOT loaded and must be explicitly specified by the
 *	application in it's accelerator table.
 *
 *	Args:
 *		<menunames>	String designating the names of the
 *					menus to load. All the names are in a
 *					single null-terminated string and are
 *					delimited by tabs. Blanks are allowed
 *					in menu names. If this string is NULL,
 *					all the menus in the IDM_AFMENU resource
 *					are added to the application menubar.
 *
 * Returns: bool
 *	<true> if menus successfully loaded
 *	<false> if problems finding resource or a particular named menu
 *
 ----*/
bool DemoView::LoadSceneMenus(const TCHAR* menunames)
{
	// Do not load menus at all if -1 is string pointer
	if (menunames == ((const TCHAR*) -1))
		return true;

	// Add menus to the top level frame window
	CFrameWnd* frameWindow = GetTopLevelFrame();
	if (frameWindow == NULL)
		return false;

	// Get frameMenu
	CMenu*	frameMenu = frameWindow->GetMenu();
	if (frameMenu == NULL || frameMenu->GetSafeHmenu() == NULL)
		return false;

	// Create CMenu to retrieve submenus - 
	// use new so menu isn't purged from memory upon exit.
	m_sceneMenu = new CMenu();
	m_sceneMenu ->LoadMenu (IDM_AFMENU);
	if (m_sceneMenu ->GetSafeHmenu() == NULL)
		return false;
	
	// See if last submenu is "Help" and insert before.
	CString	menuname;
	int		mbindex = frameMenu->GetMenuItemCount();
	frameMenu->GetMenuString (mbindex - 1, menuname, MF_BYPOSITION);
	if (menuname == "&Help")
		mbindex--;

	// Copy the menus the user designated, all if menunames is NULL
	CString	wanted(menunames);
	CMenu*	menu = NULL;
	int		menuindex = 0;

	while (menu = m_sceneMenu->GetSubMenu(menuindex))
	{
		m_sceneMenu->GetMenuString(menuindex++, menuname, MF_BYPOSITION);
		if (menunames)
			if (wanted.Find(menuname) < 0)
				continue;
		frameMenu->InsertMenu(mbindex, MF_POPUP | MF_STRING | MF_BYPOSITION, (UINT_PTR) menu->GetSafeHmenu(), menuname);
	}
	return true;	
}

void DemoView::OnDestroy() 
{
	View::OnDestroy();
	
	delete m_sceneMenu;
}

//
// Select the arcball for navigation
// Mouse movement causes the scene to rotate
//
void DemoView::OnNavArcball() 
{
	Demo*				world = Demo::Get();
	Engine*				navroot = (Engine*) world->GetNavigator();
	Navigator*			naveng;
	Camera*				cam = (Camera*) navroot->GetTarget();
	ArcBall*			arcball;
	GroupIter<Engine>	iter(navroot, Group::CHILDREN);
	Core::String		engname;
	Model*				sceneroot = world->GetUserModels();

	VX_ASSERT(navroot->IsClass(VX_Transformer));
	VX_ASSERT(cam && cam->IsClass(VX_Camera));
	while (naveng = (Navigator*) iter.Next())
	{
		if (naveng->IsClass(VX_Navigator))
		{
			naveng->ClearFlags(SharedObj::DOEVENTS);
			naveng->SetActive(false);
		}
	}
	arcball = (ArcBall*) navroot->Find(TEXT(".arcball"), Group::FIND_END | Group::FIND_CHILD);
	if (arcball)
	{
		arcball->SetActive(true);
		arcball->SetFlags(SharedObj::DOEVENTS);
		return;
	}
	arcball = new ArcBall();
	arcball->SetFlags(SharedObj::DOEVENTS);
	if (sceneroot)
	{
		arcball->SetTarget(sceneroot);
		engname = sceneroot->GetName();
	}
	else
		engname = cam->GetName();
	engname += TEXT(".arcball");
	arcball->SetName(engname);
	GetMessenger()->Observe(arcball, Event::MOUSE, NULL);
	navroot->Append(arcball);
}

void DemoView::OnNavFlyer() 
{
	Demo*				world = Demo::Get();
	Engine*				navroot = (Engine*) world->GetNavigator();
	Navigator*			naveng;
	Flyer*				flyer;
	GroupIter<Engine>	iter(navroot, Group::CHILDREN);

	VX_ASSERT(navroot->IsClass(VX_Transformer));
	while(naveng = (Navigator*) iter.Next())
	{
		if (naveng->IsClass(VX_Flyer))
		{
			naveng->SetActive(true);
			naveng->SetFlags(SharedObj::DOEVENTS);
		}
		else if (naveng->IsClass(VX_Navigator) || naveng->IsClass(VX_ArcBall))
		{
			naveng->ClearFlags(SharedObj::DOEVENTS);
			naveng->SetActive(false);
		}
	}
	flyer = (Flyer*) navroot->Find(TEXT(".flyer"), Group::FIND_END | Group::FIND_CHILD);
	VX_ASSERT(flyer);
}

void DemoView::OnNavDriver() 
{
	Demo*				world = Demo::Get();
	Transformer*		navroot = (Transformer*) world->GetNavigator();
	Navigator*			naveng;
	Camera*				cam = (Camera*) navroot->GetTarget();
	Navigator*			driver;
	GroupIter<Engine>	iter(navroot, Group::CHILDREN);
	Core::String		engname;

	VX_ASSERT(navroot->IsClass(VX_Transformer));
	VX_ASSERT(cam && cam->IsClass(VX_Camera));
	while(naveng = (Navigator*) iter.Next())
	{
		if (naveng->IsClass(VX_Flyer) || naveng->IsClass(VX_ArcBall))
		{
			naveng->ClearFlags(SharedObj::DOEVENTS);
			naveng->SetActive(false);
		}
	}
	driver = (Navigator*) navroot->Find(TEXT(".driver"), Group::FIND_END | Group::FIND_CHILD);
	if (driver)
	{
		driver->SetActive(true);
		driver->SetFlags(SharedObj::DOEVENTS);
		return;
	}
	driver = new Navigator();
	driver->SetFlags(SharedObj::DOEVENTS);
	driver->SetSpeed(0.2f);
	driver->SetTurnSpeed(0.2f);
	engname = cam->GetName();
	GetMessenger()->Observe(driver, Event::MOUSE, NULL);
	navroot->Append(driver);
	FindTerrain(navroot, engname);
	engname += TEXT(".driver");
	driver->SetName(engname);
}

void DemoView::FindTerrain(Transformer* navroot, const TCHAR* targname) 
{
	Demo*				world = Demo::Get();
	const Shape*		mod = (const Shape*) world->Find(TEXT("@terr"));
	TerrainCollider*	tcoll = (TerrainCollider*) navroot->Find(TEXT(".terraincollider"), Group::FIND_CHILD | Group::FIND_END);

	if (tcoll != NULL)
		return;
	VX_ASSERT(navroot->IsClass(VX_Transformer));
	if (mod && mod->IsClass(VX_Shape))
	{
		Core::String name(targname);
		tcoll = new TerrainCollider;
		tcoll->BuildTerrain(mod, 5, 5);
		tcoll->SetHeight(5.0f);
		name += TEXT(".terraincollider");
		tcoll->SetName(name);
		navroot->Append(tcoll);
	}
}

}	// end MFC
}	// end Vixen