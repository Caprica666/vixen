#include "vxmfc.h"

namespace Vixen {
namespace MFC {

Frame	*Frame::sm_Self;

IMPLEMENT_DYNCREATE(Frame, CFrameWnd)

BEGIN_MESSAGE_MAP(Frame, CFrameWnd)
	//{{AFX_MSG_MAP(Frame)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

Frame::Frame()
{
	sm_Self = this;
	m_FullScreen = false;
	m_normalStyle = 0;
	m_normalExStyle = 0;
}

Frame::~Frame()
{
}

BOOL Frame::PreCreateWindow(CREATESTRUCT& cs)
{
	World3D* w = World3D::Get();

	if (w->WinRect.Width() > 0)
	{
		cs.x = (int) w->WinRect.min.x;
		cs.y = (int) w->WinRect.min.y;
		cs.cx = (int) w->WinRect.Width();
		cs.cy = (int) w->WinRect.Height();
	}
	return CFrameWnd::PreCreateWindow(cs);
}

/*!
 * @fn void Frame::SetFullScreen(bool enable)
 *
 * In full screen mode, all MFC window adornments are suppressed
 * and the main frame is maximized and positioned so that only
 * the 3D client window is visible
 */
void Frame::SetFullScreen(bool enable)
{
	HWND	hWnd = GetSafeHwnd();
	static	bool	busy = false;	// re-entrancy lock

	if (busy || (enable == m_FullScreen))
		return;
	// Hide window to avoid herky-jerky
	busy = true;
	m_FullScreen = enable;

	if (enable)						// enable full screen?
	{
		// Save for restore
		WINDOWPLACEMENT wp;

		GetWindowPlacement(&wp);
		m_normalSize = wp.rcNormalPosition;
		m_normalStyle = ::GetWindowLong (hWnd, GWL_STYLE);
		m_normalExStyle = ::GetWindowLong (hWnd, GWL_EXSTYLE);

		ShowWindow (SW_HIDE);
		::SetWindowLong (hWnd, GWL_STYLE, WS_POPUP);
		::SetWindowLong (hWnd, GWL_EXSTYLE, 0);

		// Hide status and menu
		m_wndStatusBar.ShowWindow (SW_HIDE);
		SetMenu (NULL);

		// explode size to cover entire screen
		CRect	size (-1, -1, ::GetSystemMetrics (SM_CXSCREEN) + 3, ::GetSystemMetrics (SM_CYSCREEN) + 3);
		MoveWindow (size, true);
		ShowWindow (SW_SHOW);
	}	
	else							// disable full screen?
	{
		// Change window attributes, hide status and menu
		ShowWindow (SW_HIDE);
		::SetWindowLong (hWnd, GWL_STYLE, m_normalStyle);
		::SetWindowLong (hWnd, GWL_EXSTYLE, m_normalExStyle);
		m_wndStatusBar.ShowWindow (SW_SHOWNORMAL);

		CMenu	loadMenu;
		loadMenu.LoadMenu (IDR_MAINFRAME);
		SetMenu (&loadMenu);
		((DemoView*) GetActiveView())->LoadSceneMenus();

		MoveWindow (m_normalSize, true);
		ShowWindow (SW_SHOW);
		SetFocus();
	}
	busy = false;
}

void Frame::ResizePower2(CWnd* clientwin)
{
	HWND		hWnd = GetSafeHwnd();
	Scene*		scene = GetMainScene();
	CRect		clientrect;
	CRect		framerect;
	int32		cw = 16;
	int32		ch = 16;
	int32		w, h;

	if (clientwin == NULL)
		clientwin = GetActiveView();
	GetWindowRect(&framerect);				// get frame window dimensions
	clientwin->GetClientRect(&clientrect);	// get client window dimensions
	w = clientrect.Width();					// subtract 2 pixel wide borders
	h = clientrect.Height();
	clientwin->GetWindowRect(&clientrect);
	while (cw < w)							// find smallest power of 2 ..
		cw <<= 1;							// dimensions that contain the ..
	while (ch < h)							// client window completely
		ch <<= 1;
	cw += 4;								// add borders back in
	ch += 4;
	w = framerect.Width() - clientrect.Width();
	h = framerect.Height() - clientrect.Height();
	w += cw;
	h += ch;
	ShowWindow(SW_HIDE);
	SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
	clientwin->SetWindowPos(NULL, 0, 0, cw, ch, SWP_NOMOVE | SWP_NOZORDER);
	ShowWindow(SW_SHOW);
	if (scene)
	{
		scene->Suspend();
		scene->SetViewport();
		scene->Resume();
	}
}

#ifdef _DEBUG
void Frame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void Frame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

int Frame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	
	return 0;
}

void Frame::OnClose()
{
	World3D::Get()->Stop();
	CFrameWnd::OnClose();
}

void Frame::OnMove(int x, int y) 
{
	Scene*	scene = GetMainScene();

	if (scene)
	{
		scene->Suspend();
		scene->SetViewport();
		scene->Resume();
	}
}

/****
 *
 *	Frame traps WM_SIZE messages to determine when the window
 *	is being maximized or restored.	A full screen 3D application
 *	removes all evidence of Windows (See ToggleFullScreen)
 *
 ****/
void Frame::OnSize( UINT nType, int cx, int cy )
{
	if (nType == SIZE_MAXIMIZED)
		SetFullScreen(true);
	else if (nType == SIZE_RESTORED)
		SetFullScreen(false);
	CFrameWnd::OnSize (nType, cx, cy);
}

}	// end MFC
}	// end Vixen