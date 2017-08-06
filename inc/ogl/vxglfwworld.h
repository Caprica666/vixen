
#pragma once
namespace Vixen {
/*!
 * @class WinWorld
 * @brief GLFW 3D application container.
 *
 * Handles communication with operating system and the underlying
 * OpenGL display device using GLFW.
 *
 * There is only one global world object which acts
 * as an event dispatcher. Input events from the mouse,
 * keyboard or other devices are directed toward specific
 * scene manager objects. Window events like repaint and resize are
 * communicated to the underlying 3D display platform.
 *
 * If you are using WinWorld, GLFW routes and keyboard events to
 * the scene manager and refreshes the screen.
 *
 * @see Demo App World3D
 */
class WinWorld : public World3D
{
public:
//! Returns the single instance of the 3D world.
	static	WinWorld*	Get()	{ return (WinWorld*) World3D::Get(); }
	virtual bool	OnInit();
	virtual	void	OnExit();

#ifdef _WIN32
	virtual	int		WinMain(HINSTANCE instance, TCHAR* lpCmdLine);
#endif
	virtual	int		Main(int argc, TCHAR **argv);
	static	void	ForceRefresh(Scene* scene);
	static	void	OnPaint();
	static	int		OnClose();
	static	void	OnResize(int width, int height);
	static	void	OnKey(int key, int action);
	static	void	OnChar(int key, int action);
	static	void	OnMouseButton(int button, int action);
	static	void	OnMouseMove(int x, int y);

	Vec2			MousePos;
	int				MouseButtons;
	int				MouseMods;

	static	double	CurTime;
	static	double	FrameTime;
};


} // end Vixen

#if defined(_WIN32)
/*!
 * @define VIXEN_MAKEWORLD(appclass)
 * @param appclass	class of world to instantiate
 * Instantiates the 3D world container for the current platform.
 * This version of the function is for Microsoft Windows. 
 */
#define	VIXEN_MAKEWORLD(appclass) appclass _vixenWorld;	\
	int APIENTRY ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )		\
	{														\
		_vixenWorld.IncUse();								\
		return _vixenWorld.WinMain(hInstance, lpCmdLine);	\
	}

#else
/*!
 * @define VIXEN_MAKEWORLD(appclass)
 * @param appclass	class of world to instantiate
 * Instantiates the 3D world container for the current platform.
 * This version of the function is for non-Windows platforms.
 */
#define	VIXEN_MAKEWORLD(appclass)				\
	appclass _vixenWorld;						\
	int main(int argc, TCHAR **argv)			\
	{											\
		int rc = _vixenWorld.Main(argc, argv);	\
		exit(rc);								\
	}

#endif