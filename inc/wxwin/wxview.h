#pragma once

#include "wx/timer.h"

#if defined(_WIN32)
namespace Vixen {
#endif

class WXView : public wxScrolledWindow
{
public:
	WXView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
		long style, const wxString &name);
	~WXView();

	// Functions to handle events
	void OnPaint( wxPaintEvent &event );
	void OnSize( wxSizeEvent &event );
	void OnEraseBackground( wxEraseEvent &event );
	void OnChar( wxKeyEvent &event );
	void OnMouseEvent( wxMouseEvent &event );
	void OnRefreshTimer( wxTimerEvent &event );
	DECLARE_EVENT_TABLE()

protected:
	void Resize(void);
	bool doResize;

	wxTimer*	m_RefreshTimer;
};

#if defined(_WIN32)
}
#endif

