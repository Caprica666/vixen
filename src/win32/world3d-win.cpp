#include "vixen.h"

#ifndef VIXEN_GLFW

#include "win32/vxwinworld.h"

namespace Vixen {

WinWorld::WinWorld() : World3D()
{
	m_ThreadID = ::GetCurrentThreadId();
}

void WinWorld::OnExit()
{
	::PostThreadMessage(m_ThreadID, WM_QUIT, 0, 0);
	World3D::OnExit();
}

/*
 * @fn WinWorld::DefaultWndProc
 *
 *	Called to process a single Windows message. The Scene manager handles
 *	window move and resize, repaint and mouse events in the
 *	default WinWorld::WndProc.
 *
 *	You can override World3D::WndProc to change windows message
 *	processing for individual events or to respond to specific
 *	windows messages
 */
LRESULT CALLBACK WinWorld::DefaultWndProc(
						 HWND hWnd,         // window handle
						 UINT message,      // type of message
						 WPARAM wParam,     // additional information
						 LPARAM lParam      // additional information
						 )
{
	MSG			msg;
	msg.hwnd = hWnd;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;

	if (!((WinWorld*) World3D::Get())->WndProc(msg))				
		return (DefWindowProc(hWnd, message, wParam, lParam));
   return 0;
}

bool WinWorld::WndProc(MSG& msg)
{
	KeyEvent*	k;
	MouseEvent*	m;
	int			flags = 0;
	Scene*		scene = GetScene((Window) msg.hwnd);

	switch (msg.message)
	{
		case WM_ERASEBKGND:
		return true;

		case WM_MOVE:
		case WM_SIZE:
		if (scene)
		{
			scene->Suspend();
			scene->SetViewport();
			scene->Resume();
		}
		break;

		case WM_PAINT:
		if (scene)
		{
			scene->Repaint();
	#ifdef VX_NOTHREAD
			if ((scene->GetOptions() & Scene::REPAINT) == 0)
				::InvalidateRect(msg.hwnd, NULL, false);
	#endif
		}	
		break;

		case WM_DESTROY:
		World3D::Get()->Stop();
		return false;		

		case WM_KEYUP:
		if (!RawInput)
			break;

		case WM_KEYDOWN:
		switch (msg.wParam)
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
			if (!RawInput)
				msg.wParam += 0x100;
			k = new KeyEvent;
			k->KeyCode = LOWORD(msg.wParam);
			k->KeyFlags = HIWORD(msg.lParam);
			k->Sender = this;
			k->Log();				// log event to output stream
			if (scene)
				scene->Repaint();
			break;

			default:
			if (RawInput)
			{
				k = new KeyEvent;
				k->KeyCode = LOWORD(msg.wParam);
				k->KeyFlags = HIWORD(msg.lParam);
				k->Sender = this;
				k->Log();				// log event to output stream
				if (scene)
					scene->Repaint();
				return true;
			}
			return false;
		}
		break;

		case WM_CHAR:
		if (RawInput)
			break;
		k = new KeyEvent;
		k->KeyCode = LOWORD(msg.wParam);
		k->KeyFlags = HIWORD(msg.lParam);
		k->Sender = this;
		k->Log();				// log event to output stream
		if (scene)
			scene->Repaint();
		break;

		
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		flags = MouseEvent::DOUBLE;
		// fall thru

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:  
		case WM_RBUTTONUP:  
		case WM_MOUSEMOVE:
		::GetCursorPos(&msg.pt);
		m = new MouseEvent;
		m->MousePos.x = float(LOWORD(msg.lParam));
		m->MousePos.y = float(HIWORD(msg.lParam));
		m->MouseFlags = flags | (msg.wParam & 0X1F);
		m->Sender = this;
		m->Log();
		if (scene)
			scene->Repaint();
		break;
				
        default:          // Passes it on if unproccessed
		return false;
   }
   return true;
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
	HWND		wHdl;
	WNDCLASS	wc;
	
	wc.style = (CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);
	wc.lpfnWndProc   = (WNDPROC) WinWorld::DefaultWndProc;// Window Procedure
	wc.cbClsExtra    = 0;                           		// No per-class extra data.
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_Instance;                   		// Owner of this class
	wc.hIcon         = LoadIcon(m_Instance, TEXT("Vixen"));	// Icon name from .RC 
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 		// Cursor
	wc.hbrBackground = NULL;								// Default color
	wc.lpszMenuName  = NULL;                   				// Menu name from .RC
	wc.lpszClassName = TEXT("Vixen");                  		// Name to register as
	
	if (!World3D::OnInit())								// Initialize application
		return false;
	if (!RegisterClass(&wc))
		return false;

	// Full screen window size
	HDC	hdc = GetDC (NULL);
	if (WinRect.Width() == 0.0f)
		WinRect.max.x = (float) GetDeviceCaps (hdc, HORZRES);
	if (WinRect.Height() == 0.0f)
		WinRect.max.y = (float) GetDeviceCaps (hdc, VERTRES);
	ReleaseDC (NULL, hdc);

	// Main window for this app instance.
	wHdl = ::CreateWindow(
		wc.lpszClassName,       // See RegisterClass()
		TEXT("Vixen Window"),   // Window title bar
		WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,	// Window style.
		(int) WinRect.min.x,	// init x pos
		(int) WinRect.min.y,	// init y pos
		(int) WinRect.Width(),	// init x size
		(int) WinRect.Height(),	// init y size
		NULL,                   // Overlapped windows have no parent.
		NULL,                   // Use the window class menu.
		m_Instance,             // This instance owns this window.
		NULL                    // We don't use any data in our WM_CREATE
        );
	
	if (!wHdl)
		return false;
	// Make the window visible; update its client area; and return "success"
	::ShowWindow(wHdl, TRUE); 		// Show the window
	::SetActiveWindow(wHdl);
	::PostMessage(wHdl, WM_SETFOCUS, 0, 0L);
	return Run((Window) wHdl);	// make the scene
}

/*++++
 *
 * Name: WinWorld::WinMain
 *	int WinMain(HINSTANCE instance, LPSTR cmdline, int cmdshow)
 *
 * Description:
 *	Windows message pump function called when a Windows application
 *	starts up. The default behavior is to parse a filename argument,
 *	initialize a single window and process windows messages.
 *
 *	You can override this function to process events between messages.
 *	You can override WinWorld::WndProc to change windows message
 *	processing for individual events or to respond to specific
 *	windows messages
 *
 *	To use the default WinMain implementation, define your
 *	application's WinMain as follows:
 *		int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
 *							 LPSTR lpCmdLine, int nCmdShow )
 *		{
 *			return ((WinWorld*) World3D::Get())->WinMain(hInstance, lpCmdLine, nCmdShow);
 *		}
 *
 * Also: WinWorld::WndProc
 *
 ---*/
int WinWorld::WinMain(HINSTANCE hInstance, TCHAR* lpCmdLine)
{
	MSG				msg;
	Core::TLSData*	tls = Core::TLSData::Get();

	m_Instance = hInstance;
	ParseArgs(lpCmdLine);
	Startup();
	if (!OnInit())
		return FALSE;
	while (TRUE)
	{
		if (!GetMessage(&msg, NULL, 0, 0))
			return FALSE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClass(TEXT("Vixen"), hInstance);
	Shutdown();
	return FALSE;
}

}	// end Vixen

#endif