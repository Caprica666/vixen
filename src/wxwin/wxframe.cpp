#include "vxwxwin.h"
#include "wxwin/SceneGraphDlg.h"
#include "wxwin/wxframe.h"
#include "wxwin/wxview.h"

#if defined(_WIN32)
namespace Vixen {
#endif

BEGIN_EVENT_TABLE( WXFrame, wxFrame )
	EVT_MENU( wxID_OPEN, WXFrame::OnOpen )
	EVT_MENU( ID_SCENETREE, WXFrame::OnSceneGraph )
	EVT_MENU( ID_SHOWALL, WXFrame::OnShowAll )
	EVT_MENU( wxID_EXIT, WXFrame::OnExit )
	EVT_MENU( wxID_ABOUT, WXFrame::OnAbout )
//	EVT_CLOSE( WXFrame::OnCloseWindow )
END_EVENT_TABLE()


WXFrame::WXFrame( const wxString &title, wxPoint pos, wxSize size ) :
	wxFrame( (wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	InitGUI();
	(void) new WXView( this, -1, wxPoint( 0, 0 ), wxSize( 500, 500 ), 0, "WXView");
	CreateStatusBar(1);
	SetStatusText(TEXT("WXWindows Visual Experience Engine"));
}

void WXFrame::InitGUI()
{
	// set the icon
	wxIcon	appicon(TEXT("wxwin/icons/wxview.ico"), wxBITMAP_TYPE_ICO_RESOURCE, -1, -1);
	SetIcon(appicon);

	// make menu's
	wxMenu *fileMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;

	fileMenu->Append( wxID_OPEN, TEXT("&Open") );
	fileMenu->Append( ID_SCENETREE, TEXT("&Scene") );
	fileMenu->Append( ID_SHOWALL, TEXT("&Show All") );
	fileMenu->Append( wxID_EXIT, TEXT("E&xit") );
	helpMenu->Append( wxID_ABOUT, TEXT("&About") );

	// make a menu bar
	wxMenuBar *menuBar = new wxMenuBar;

	menuBar->Append( fileMenu, TEXT("&File") );
	menuBar->Append( helpMenu, TEXT("&Help") );

	// put the menu bar in the frame
	SetMenuBar( menuBar );

	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, TEXT("Scene Graph"),
										wxDefaultPosition, wxDefaultSize,
										wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(250, 350);
}


void WXFrame::OnOpen( wxCommandEvent &event )
{
    wxFileDialog file(this, TEXT("Open an existing Vixenfile"), TEXT("."), TEXT(""), TEXT("*.vix"), wxFD_DEFAULT_STYLE, wxPoint(20, 20));

    if (file.ShowModal() == wxID_OK)
    {
		wxArrayString filenames;
		file.GetPaths(filenames);
		m_FileName = filenames[0];
		m_FileRoot = filenames[0];
	   // If there is a '.' in the file name, return all that is left of the last dot.
	   if( m_FileRoot.Find( '.', TRUE ) )
		   m_FileRoot = m_FileRoot.Left( m_FileRoot.Find( '.', TRUE ) );
	   OnNewDocument();
    }
}

bool WXFrame::OnNewDocument()
{
	World3D* world = World3D::Get();
	Messenger* mess = world->GetMessenger();
	const TCHAR *fname = world->GetFileName();

	if (strlen(m_FileName) > 0)
		world->SetFileName(fname = m_FileName);
	else if(fname == NULL)
		return TRUE;
	world->LoadAsync(fname, world->GetScene());
	return TRUE;
}

void WXFrame::OnExit( wxCommandEvent &event )
{
	delete m_pSceneGraphDlg;
	m_pSceneGraphDlg = NULL;
	Close(TRUE);
	World3D* world = World3D::Get();
	if (world)
	{
		world->DelProp(PROP_DevInfo);
		world->Stop();
		world->Delete();
	}
}

void WXFrame::OnShowAll(wxCommandEvent& event)
{
	World3D* world = World3D::Get();
	Scene* scene = world->GetScene();
	scene->ShowAll();
}


void WXFrame::OnAbout( wxCommandEvent &event )
{
    wxMessageBox(TEXT("Vixen wxWindows Implementation"),
                 TEXT("About VXWXWorld"), wxOK | wxICON_INFORMATION, this);
}

void WXFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(TRUE);
}

#if defined(_WIN32)
}	// end Vixen
#endif