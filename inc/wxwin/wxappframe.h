#ifdef new
    #undef new
#endif

#if defined(_WIN32)
namespace Vixen {
#endif
	#define VX_REFRESH_TIMER	1
	#define VX_REFRESH_TIME		1

	class WXApp : public wxApp
	{
	public:
		// default constructor
		WXApp() : wxApp() { }

		bool	OnInit();
	};

	class WXFrame : public wxFrame
	{
	public:
		WXFrame( const wxString &title, wxPoint &pos, wxSize &size );
	
		bool OnNewDocument( void );

		// Functions to handle events
		void OnOpen( wxCommandEvent &event );
		void OnExit( wxCommandEvent &event );
		void OnAbout( wxCommandEvent &event );
	//	void OnCloseWindow( wxCloseEvent &event );
	
	protected:
		wxString m_FileName;
		wxString m_FileRoot;
		wxString m_ModelName;

	private:

		DECLARE_EVENT_TABLE()
	};

	/*!
	 * @define VIXEN_MAKEWORLD(appclass)
	 * @param appclass	class of world to instantiate
	 * Instantiates the 3D world container for the current platform.
	 * This version of the function is for wxWindows. 
	 */
	extern void _vixen_MakeWorld();

	#define	VIXEN_MAKEWORLD(appclass) void _vixen_MakeWorld() { Core::CoreInit(); appclass* world = new appclass(); world->IncUse(); }

#if defined(_WIN32)
}	// end Vixen
#endif
