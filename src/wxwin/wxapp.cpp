#include "vxwxwin.h"
#include "wxwin/wxframe.h"

#if defined(_WIN32)
namespace Vixen {
#endif

IMPLEMENT_APP(  WXApp )

bool WXApp::OnInit(void)
{
	wxString	title(TEXT("wxViewer"));
	// create a frame in our App
	WXFrame* vixenFrame = new WXFrame(title, wxPoint( 50, 50 ), wxSize( 500, 500 ));
	World3D* world = World3D::Get();

	if (world == NULL)				// create Vixen World
		_vixen_MakeWorld();
	world = World3D::Get();
	if (world == NULL)
		return false;
	world->ParseArgs(wxApp::argc, wxApp::argv);
	if (world->OnInit())			// initialize Vixen World
	{
		vixenFrame->Show(TRUE);		// show the frame
		SetTopWindow(vixenFrame);
		return true;
	}
	return false;
}

#if defined(_WIN32)
}
#endif


