/*!
 * @file scenegraphdlg.h
 *
 * WXWindows tree control to describe 3D scene structure
 *
 * @author Ben Discoe
 * Copyright (c) 2001-2009 Intel
 */
#ifndef __SceneGraphDlg_H__
#define __SceneGraphDlg_H__

#include "wx/checkbox.h"
#include "wx/dialog.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"


#define ID_SCENETREE 10020
#define ID_ENABLED 10021
#define ID_ZOOMTO 10022
#define ID_REFRESH 10023
#define ID_SHOWALL 10025

#if defined(_WIN32)
namespace Vixen {
#endif
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

class SceneGraphDlg: public wxDialog
{
public:
    // constructors and destructors
    SceneGraphDlg(wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
	~SceneGraphDlg();

    void OnInitDialog(wxInitDialogEvent& event);
    wxButton*		m_pZoomTo;
    wxCheckBox*		m_pEnabled;
    wxTreeCtrl*		m_pTree;
	Group*			m_pSelectedNode;

    void			CreateImageList(int size = 16);
	void			RefreshTreeContents();
    wxTreeItemId	AddNodeItemsRecursively(wxTreeItemId hParentItem, Group *pNode, int depth);

	wxButton*	GetZoomto()		{ return (wxButton*) FindWindow( ID_ZOOMTO ); }
    wxCheckBox*	GetEnabled()	{ return (wxCheckBox*) FindWindow( ID_ENABLED ); }
    wxTreeCtrl*	GetScenetree()  { return (wxTreeCtrl*) FindWindow( ID_SCENETREE ); }

private:
    wxImageList*	m_imageListNormal;

private:
    // WDR: handler declarations for SceneGraphDlg
    void OnRefresh( wxCommandEvent &event );
    void OnZoomTo( wxCommandEvent &event );
    void OnEnabled( wxCommandEvent &event );
    void OnTreeSelChanged( wxTreeEvent &event );

private:
    DECLARE_EVENT_TABLE()
};

#if defined(_WIN32)
}
#endif
#endif
