#pragma once

#ifdef new
    #undef new
#endif

#if defined(_WIN32)
namespace Vixen {
#endif
	class SceneGraphDlg;

	class WXApp : public wxApp
	{
	public:
		// default constructor
		WXApp() : wxApp() { }

		virtual bool	OnInit();
	};

	/*!
	 * @define VIXEN_MAKEWORLD(appclass)
	 * @param appclass	class of world to instantiate
	 * Instantiates the 3D world container for the current platform.
	 * This version of the function is for wxWindows.
	 */
	extern void _vixen_MakeWorld();

	#define	VIXEN_MAKEWORLD(wclass) void _vixen_MakeWorld() { World::Startup(); wclass* world = new wclass(); world->IncUse(); }

	// #define	VIXEN_MAKEWORLD(appclass) void _vixen_MakeWorld() { Core::CoreInit(); appclass* world = new appclass(); world->IncUse(); }

DECLARE_APP( WXApp )

#if defined(_WIN32)
}	// end Vixen
#endif
