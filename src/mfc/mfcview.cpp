/*
 * View
 *
 * MFC CView subclass for use with the Visual Experience Engine.
 * You can inherit from View anywhere you can use an MFC
 * single-document view. We supply implementations of the
 * following CView functions:
 *	OnDraw			display current scene
 *	OnSize			handle window resize
 *	OnMove			handle window move
 *	OnEraseBkgnd	inhibits background erase
 *	PreCreateWindow	setup for OpenGL
 *	OnLButtonDown	filter mouse events to scene manager
 *	OnRButtonDown
 *	OnMButtonDown
 *	OnLButtonUp
 *	OnLButtonDown
 *	OnMButtonDown
 *	OnMouseMove
 *	OnKeyUp			filter key events to scene manager
 *
 * In the scene manager view of the world, there is a single window owned
 * by the global World3D object. This class provides the glue
 * to graft the scene manager onto an MFC CView.
 */
#include "vxmfc.h"
#include "mmsystem.h"

namespace Vixen {
namespace MFC {

IMPLEMENT_DYNCREATE(View, CView)

BEGIN_MESSAGE_MAP(View, CView)
	//{{AFX_MSG_MAP(View)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_MOVE()
	ON_WM_CHAR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

View::View() :CView()
{
	m_StatusTimer = 0;
	m_ClientWnd = NULL;
}

View::~View()
{
	assert(m_StatusTimer == 0);
	if (m_ClientWnd)
	{
		m_ClientWnd->DestroyWindow();
		delete m_ClientWnd;
	}
}

/****
 *
 * Scene manager takes care of erasing the background
 *
 ****/
BOOL View::OnEraseBkgnd(CDC* pDC) 
{
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

void View::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	Resize();
}

void View::OnMove(int x, int y) 
{
	CView::OnMove(x, y);
	Resize();
}

/****
 *
 * Scene manager refreshes the status text based on a timer. The timer is set
 * to go off approx 10 times / sec. It also checks to see if the
 * scene manager thread has quit and posts a quit message if necessary.
 *
 ****/
#define STATUS_TIMER 1
#define	STATUS_TIME	100

#ifndef _M_X64
static void CALLBACK EXPORT StatusFunc(HWND hwnd, UINT msg, UINT timerid, DWORD time)
#else
static void CALLBACK EXPORT StatusFunc(HWND hwnd, UINT msg, UINT_PTR timerid, DWORD time)
#endif
{
	Demo*	app = Demo::Get();
	const TCHAR*	status = app->GetStatusText();
	if (*status)
	{
		((CFrameWnd*) AfxGetMainWnd())->SetMessageText(status);
		app->SetStatusText(NULL);
	}
}

void View::OnDraw(CDC* pDC)
{
	static bool entered = false;
	World3D*	w = World3D::Get();
	Window	hwnd = (Window) GetSafeHwnd();
	
	if (!w->IsRunning() && entered == false)
	{
		entered = true;
		w->Run((Window) hwnd);
		//m_StatusTimer = SetTimer(STATUS_TIMER, STATUS_TIME, &StatusFunc);
	}
	Scene* scene = w->GetScene(hwnd);
	if (scene)
	{
		scene->Repaint();
#ifdef VX_NOTHREAD
		if ((scene->GetOptions() & Scene::REPAINT) == 0)
			Invalidate();
#endif
	}
}

void View::Resize()
{
	Scene*	scene = GetScene();

	if (scene)
	{
		scene->Suspend();
		scene->SetViewport();
		scene->Resume();
	}
} 

int View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);
	return CView::OnCreate(lpCreateStruct);
}

void View::OnDestroy() 
{
	KillTimer(m_StatusTimer);
	m_StatusTimer = 0;
	Scene*	scene = GetScene();
	if (scene)
		scene->SetWindow(NULL);
	CView::OnDestroy();
}

void View::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDown(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnLButtonUp(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnLButtonDblClick(UINT nFlags, CPoint point) 
{
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags | MouseEvent::DOUBLE;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnMouseMove(UINT nFlags, CPoint point) 
{
	CView::OnMouseMove(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnRButtonDown(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnRButtonUp(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnRButtonDblClick(UINT nFlags, CPoint point) 
{
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags | MouseEvent::DOUBLE;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnMButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnMButtonDown(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnMButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnMButtonUp(nFlags, point);
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnMButtonDblClick(UINT nFlags, CPoint point) 
{
	MouseEvent* m = new MouseEvent;
	Scene* scene = GetMainScene();
	m->MousePos.x = float(point.x);
	m->MousePos.y = float(point.y);
	m->MouseFlags = nFlags | MouseEvent::DOUBLE;
	m->Sender = World3D::Get();
	m->Log();
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	World3D*	w = World3D::Get();
	Frame*	main_frame = (Frame*) AfxGetMainWnd();
	CRect			rect;

	switch (nChar)
	{
		case VK_ESCAPE:
		if (main_frame->IsKindOf(RUNTIME_CLASS(Frame)))
			main_frame->ToggleFullScreen();
			return;

		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_INSERT:
		case VK_DELETE:
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
		case VK_ADD:
		case VK_SUBTRACT:
		if (!w->RawInput)
			nChar += 0x100;				// avoid ANSI conflict
		break;

		default:
		if (!w->RawInput)
			return;
	   }
	KeyEvent* k = new KeyEvent;
	Scene* scene = GetMainScene();
	k->KeyCode = nChar;
	k->KeyFlags = nFlags & KeyEvent::MASK;
	k->Sender = w;
	k->Log();				// log event to output stream
	if (scene)
		scene->Repaint();	// force event loop to execute
}

void View::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	World3D*	w = World3D::Get();
	KeyEvent* k;

	if (!w->RawInput)
		return;
	switch (nChar)
	{
		case VK_ESCAPE:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_INSERT:
		case VK_DELETE:
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
		case VK_ADD:
		case VK_SUBTRACT:
		if (!w->RawInput)
			nChar += 0x100;				// avoid ANSI conflict

		default:
		Scene* scene = GetMainScene();
		k = new KeyEvent;
		k->KeyCode = nChar;
		k->KeyFlags = nFlags & KeyEvent::MASK;
		k->Sender = w;
		k->Log();				// log event to output stream
		if (scene)
			scene->Repaint();	// force event loop to execute
	}
}

void View::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	World3D*	w = World3D::Get();
	KeyEvent*	k;

	if (w->RawInput)
		return;

	DemoView*	main_view = (DemoView*) this;
	if (main_view->IsKindOf(RUNTIME_CLASS(DemoView)))
	{
		switch (nChar)
		{
			case 1:		// Ctrl-A
			main_view->ZoomAll();
			return;

			case 23:	// Ctrl-W
			main_view->ToggleWireframe();
			return;
		}
	}
	Scene* scene = GetMainScene();
	k = new KeyEvent;
	k->KeyCode = nChar;
	k->KeyFlags = nFlags & KeyEvent::MASK;
	k->Sender = w;
	k->Log();				// log event to output stream
	if (scene)
		scene->Repaint();	// force event loop to execute
}

}	// end MFC
}	// end Vixen