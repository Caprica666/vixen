#include "vxwxwin.h"

#ifdef _WIN32
#include "win32/vixenw.h"
namespace Vixen {
#else
#include "linux/vixenx.h"
#endif


#define VX_REFRESH_TIMER 1
#define VX_REFRESH_TIME 20

BEGIN_EVENT_TABLE(WXView, wxScrolledWindow )
	EVT_PAINT( WXView::OnPaint )
	EVT_SIZE( WXView::OnSize )
	EVT_CHAR( WXView::OnChar )
	EVT_MOUSE_EVENTS( WXView::OnMouseEvent )
	EVT_ERASE_BACKGROUND( WXView::OnEraseBackground )
	EVT_TIMER(VX_REFRESH_TIMER, WXView::OnRefreshTimer )
END_EVENT_TABLE()


WXView::WXView( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
						  long style, const wxString &name ) :
			 wxScrolledWindow( parent, id, pos, size, style, name )
{
	parent->Show(TRUE);
	doResize = false;
	m_RefreshTimer = NULL;
}


// Destructor called when frame is destroyed
WXView::~WXView()
{
	Scene *scene = World3D::Get()->GetScene((Window) GetHandle());
	if (m_RefreshTimer)
	{
		m_RefreshTimer->Stop();
		delete m_RefreshTimer;
	}
	if (scene)						// now that we're NOT rendering, turn off Vixen
		scene->SetWindow(NULL);
}

void WXView::OnEraseBackground( wxEraseEvent &event )
{
	// do nothing to avoid flashing
	return;
}


void WXView::OnSize( wxSizeEvent &event )
{
#ifdef VX_NOTHREAD
	doResize = true;
#else
	Resize();
#endif
}

void WXView::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	World3D* world = World3D::Get();
	if (world == NULL)
		return;
	DeviceInfo* devinfo = (DeviceInfo*) world->GetProp(PROP_DevInfo);

	if (devinfo == NULL)
	{
		Core::TLSData*	tls = Core::TLSData::Get();	// establish thread storage
		WXApp&	app = wxGetApp();					// get WX application

		devinfo = new DeviceInfo;					// remember low-level info
		world->AddProp(devinfo);

#ifndef _WIN32
		devinfo->WinHandle = (Window) GetHandle();
		devinfo->DispHandle = (Display*) wxGetDisplay();
		devinfo->VisHandle = (XVisualInfo*) app.GetVisualInfo(wxGetDisplay());
#else
		devinfo->WinHandle = (HWND) GetHWND();
#endif
		world->Run((Window) devinfo->WinHandle);	// start scene threads
#ifdef VX_NOTHREAD
		m_RefreshTimer = new wxTimer(this, VX_REFRESH_TIMER );
		m_RefreshTimer->Start(VX_REFRESH_TIME);
#endif
	}
#ifndef VX_NOTHREAD
	Scene* scene = world->GetScene((Vixen::Window) devinfo->WinHandle);
	if (scene)
		scene->Repaint();
#endif
}

// This timer will call DoFrame as fast as possible
// It should be the only real thing that talks to the scene manager
// once a scene is loaded.
void WXView::OnRefreshTimer( wxTimerEvent &event )
{
#ifdef VX_NOTHREAD
	Window winhandle = (Window) GetHandle();
	Scene* scene = World3D::Get()->GetScene(winhandle);

	if (doResize)
	{
		#ifdef __WIN32
			if (!GetContext())
				return;
		#endif
		Resize();
		doResize = false;
	}
	if (scene)
		scene->Repaint();			// force display refresh
#endif
}

void WXView::Resize()
{
	World3D*	world = World3D::Get();
	if (world == NULL)
		return;

	Window	winhandle = (Window) GetHandle();
	Scene*	scene = world->GetScene(winhandle);
	if (scene)
	{
		scene->Suspend();
		scene->SetViewport();
		scene->Resume();
	}
}

void WXView::OnChar( wxKeyEvent &event )
{
	long key = event.GetKeyCode();

	if ( key == WXK_ESCAPE || key == 'q' || key == 'Q' )
	{
		World3D* world = World3D::Get();
		if (world)
			world->Stop();
		exit(0);
	}
	Messenger* mess = GetMessenger();
	KeyEvent* ke = new KeyEvent();
	Scene* scene = GetMainScene();

	ke->KeyCode = key;
	ke->KeyFlags = 0;
	ke->Sender = World3D::Get();
	ke->Log();
	if (scene)
		scene->Repaint();			// if not refreshing continuously, force event loop
}

void WXView::OnMouseEvent( wxMouseEvent &event )
{
	Messenger* mess = GetMessenger();
	MouseEvent* me = new MouseEvent();
	Scene* scene = GetMainScene();

	me->MouseFlags = 0;
	me->MousePos.x = (float)event.GetX();
	me->MousePos.y = (float)event.GetY();

	if( event.LeftDown() || event.LeftIsDown() )
		me->MouseFlags |= MouseEvent::LEFT;
	if( event.RightDown() || event.RightIsDown() )
		me->MouseFlags |= MouseEvent::RIGHT;
	if( event.MiddleDown() || event.MiddleIsDown() )
		me->MouseFlags |= MouseEvent::MIDDLE;

	me->Sender = World3D::Get();
	me->Log();
	if (scene)
		scene->Repaint();			// if not refreshing continuously, force event loop
}

#if defined(_WIN32)
}	// end Vixen
#endif