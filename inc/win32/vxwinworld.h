
#pragma once

namespace Vixen {
/*!
 * @class WinWorld
 * @brief Windows 3D application container.
 *
 * Handles communication with operating system and the underlying
 * display device. Controls message passing and task management.
 *
 * There is only one global world object which acts
 * as an event dispatcher. Input events from the mouse,
 * keyboard or other devices are directed toward specific
 * scene manager objects. Window events like repaint and resize are
 * communicated to the underlying 3D display platform.
 *
 * If you are using WinWorld, your application controls WinMain,
 * the message pump and the display. WinWorld routes mouse events to
 * the scene manager and calls Scene::DoFrame in response to the
 *  WM_PAINT message.
 *
 * Methods:
 *	InitInstance	Initialize this instance of the app
 *  GetWinThread	Returns thread ID of windows message pump thread.
 *
 * @see Demo App World3D
 */
class WinWorld : public World3D
{
public:
	WinWorld();

//! Returns the single instance of the 3D world.
	static	WinWorld*	Get()		{ return (WinWorld*) World3D::Get(); }
//! Returns the Windows thread ID.
	DWORD		GetWinThread()		{ return m_ThreadID; }
//! Returns the Windows \b instance passed to WinMain
	HINSTANCE	GetInstance() const { return m_Instance; }
//! Function to call as Windows entry point
	virtual	int	WinMain(HINSTANCE hInstance, TCHAR* lpCmdLine);
//! Windows message handling procedure
	virtual	bool WndProc(MSG& msg);

	static LRESULT CALLBACK DefaultWndProc(HWND, UINT, WPARAM, LPARAM);
	virtual bool OnInit();
	virtual	void OnExit();

protected:
	HINSTANCE			m_Instance;
	DWORD				m_ThreadID;
};

} // end Vixen


#include "win32/vxjoystick.h"
/*!
 * @define VIXEN_MAKEWORLD(appclass)
 * @param appclass	class of world to instantiate
 * Instantiates the 3D world container for the current platform.
 * This version of the function is for Microsoft Windows. 
 */
#define	VIXEN_MAKEWORLD(appclass) appclass _vixenWorld;		\
	int APIENTRY ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )\
	{														\
		_vixenWorld.IncUse();								\
		return _vixenWorld.WinMain(hInstance, lpCmdLine);	\
	}