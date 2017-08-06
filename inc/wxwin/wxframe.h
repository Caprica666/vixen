
#pragma once

#ifdef new
    #undef new
#endif

#if defined(_WIN32)
namespace Vixen {
#endif
	class SceneGraphDlg;

	class WXFrame : public wxFrame
	{
	public:
		WXFrame( const wxString &title, wxPoint pos, wxSize size );

		bool OnNewDocument( void );

		// Functions to handle events
		void OnOpen( wxCommandEvent &event );
		void OnExit( wxCommandEvent &event );
		void OnShowAll( wxCommandEvent &event );
		void OnAbout( wxCommandEvent &event );
		void OnSceneGraph(wxCommandEvent& event);
		void InitGUI();

	protected:
		wxString m_FileName;
		wxString m_FileRoot;
		wxString m_ModelName;
		SceneGraphDlg*	m_pSceneGraphDlg;
	private:

		DECLARE_EVENT_TABLE()
	};

#if defined(_WIN32)
}	// end Vixen
#endif
