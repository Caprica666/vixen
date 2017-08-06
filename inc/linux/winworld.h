/*!
 * @class WinWorld
 * @brief Windowed 3D application container for Linux.
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
 * If you are using WinWorld, your application controls
 * the message pump and the display. WinWorld routes mouse events to
 * the scene manager and calls Scene::DoFrame in response to the
 * PAINT message.
 *
 * @see MFC::Demo MFC::App World3D
 */
#pragma once

namespace Vixen {
class WinWorld : public World3D
{
public:
	WinWorld();

//! Returns the single instance of the 3D world.
	static	WinWorld*	Get()		{ return (WinWorld*) World::Get(); }

//! Returns the Window management thread ID.
	uint32		GetWinThread()		{ return m_ThreadID; }

//! Function to call as entry point
	virtual	int	WinMain(int argc, char** argv);

	virtual bool OnInit();
	virtual	void OnExit();

protected:
	Window		MakeWindow();
	bool		ProcessEvent();

	uint32		m_ThreadID;
};

/*!
 * @define VIXEN_MAKEWORLD(appclass)
 * @param appclass	class of world to instantiate
 * Instantiates the 3D world container for the current platform.
 * This version of the function is for Microsoft Windows.
 */
#define	VIXEN_MAKEWORLD(appclass) appclass _vixenWorld;  \
    int main( int argc, char **argv)  {					 \
		_vixenWorld.IncUse();							\
        return (Vixen::WinWorld::Get())->WinMain(argc, argv); \
    }

}	// end Vixen
