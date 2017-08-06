/*
 * scenegraphdlg.cpp
 *
 * WXWindows tree control to describe 3D scene structure
 *
 * @author Ben Discoe
 * Copyright (c) 2001-2010 Intel
 */
#include "vxwxwin.h"
#include "wx/image.h"
#include "wxwin/SceneGraphDlg.h"


#define	icon_CAMERA	0
#define	icon_LIGHT	4
#define	icon_SHAPE	2
#define	icon_MODEL	3
#define	icon_ENGINE	1
#define	icon_UNKNOWN 8


#if defined(__WXGTK__) || defined(__WXX11__)
#include "wxwin/icons/icon1.xpm"	// camera
#include "wxwin/icons/icon2.xpm"	// engine
#include "wxwin/icons/icon3.xpm"	// shape
#include "wxwin/icons/icon4.xpm"	// model
#include "wxwin/icons/icon5.xpm"	// light
#include "wxwin/icons/icon6.xpm"
#include "wxwin/icons/icon7.xpm"	// geometry
#include "wxwin/icons/icon8.xpm"
#include "wxwin/icons/icon9.xpm"	// unknown
#include "wxwin/icons/icon10.xpm"
#endif

#if defined(_WIN32)
namespace Vixen {
#endif

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(Group *pNode)
	{
		m_pNode = pNode;
	}
	Group *m_pNode;
};


// WDR: class implementations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

// WDR: event table for SceneGraphDlg

BEGIN_EVENT_TABLE(SceneGraphDlg,wxDialog)
	EVT_INIT_DIALOG (SceneGraphDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_SCENETREE, SceneGraphDlg::OnTreeSelChanged )
	EVT_CHECKBOX( ID_ENABLED, SceneGraphDlg::OnEnabled )
	EVT_BUTTON( ID_ZOOMTO, SceneGraphDlg::OnZoomTo )
	EVT_BUTTON( ID_REFRESH, SceneGraphDlg::OnRefresh )
END_EVENT_TABLE()

SceneGraphDlg::SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	SceneGraphFunc( this, TRUE );

	m_pZoomTo = GetZoomto();
	m_pEnabled = GetEnabled();
	m_pTree = GetScenetree();

	m_pZoomTo->Enable(false);

	m_imageListNormal = NULL;
	CreateImageList(16);
}

SceneGraphDlg::~SceneGraphDlg()
{
	delete m_imageListNormal;
}

void SceneGraphDlg::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}
	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	// should correspond to TreeCtrlIcon_xxx enum
	wxIcon icons[10];
	icons[0] = wxICON(icon1);
	icons[1] = wxICON(icon2);
	icons[2] = wxICON(icon3);
	icons[3] = wxICON(icon4);
	icons[4] = wxICON(icon5);
	icons[5] = wxICON(icon6);
	icons[6] = wxICON(icon7);
	icons[7] = wxICON(icon8);
	icons[8] = wxICON(icon9);
	icons[9] = wxICON(icon10);

	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		int sizeOrig = icons[i].GetWidth();
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
		{
			wxBitmap bmap;
			bmap.CopyFromIcon(icons[i]);
			wxImage image = bmap.ConvertToImage();
			image.Rescale(size, size);
			m_imageListNormal->Add(image);
		}
	}
	m_pTree->SetImageList(m_imageListNormal);
}


void SceneGraphDlg::RefreshTreeContents()
{
	Scene* scene = GetMainScene();
	if (!scene)
		return;
	m_pTree->DeleteAllItems();	// start with a blank slate

	Model *pRoot = scene->GetModels();
	Engine* pSimRoot = scene->GetEngines();
	wxTreeItemId treeRoot = m_pTree->AddRoot(TEXT("Scene"));

	if (pRoot)						// Fill in the tree with nodes
		AddNodeItemsRecursively(treeRoot, pRoot, 0);
	if (pSimRoot)
		AddNodeItemsRecursively(treeRoot, pSimRoot, 0);
	m_pSelectedNode = NULL;
}


wxTreeItemId SceneGraphDlg::AddNodeItemsRecursively(wxTreeItemId hParentItem, Group *pNode, int depth)
{
	wxString	str;
	int			nImage;
	wxTreeItemId hNewItem;
	TCHAR		objclass[256];

	if (!pNode)
		return 0;
	STRCPY(objclass, pNode->ClassName());
	if ((objclass[0] == TEXT('V')) && (objclass[1] == TEXT('X')))
		str = objclass + 2;
	VX_ASSERT(pNode->IsClass(VX_Group));
	if (pNode->IsClass(VX_Model))
	{
		nImage = icon_MODEL;
		if (pNode->IsClass(VX_Light))
			nImage = icon_LIGHT;
		else if (pNode->IsClass(VX_Shape))
			nImage = icon_SHAPE;
		else if (pNode->IsClass(VX_Camera))
			nImage = icon_CAMERA;
	}
	else if (pNode->IsClass(VX_Engine))
		nImage = icon_ENGINE;
	else
		nImage = icon_UNKNOWN;
	if (pNode->GetName())
	{
		str += TEXT(" \"");
		str += pNode->GetName();
		str += TEXT("\"");
	}

	hNewItem = m_pTree->AppendItem(hParentItem, str, nImage, nImage);

	if (pNode->IsClass(VX_Shape))
	{
		Shape*		pGeom = (Shape*) pNode;
		wxTreeItemId	hGeomItem;
		Geometry*		pMesh = pGeom->GetGeometry();

		if (pMesh->IsClass(VX_TextGeometry))
			str.Printf(TEXT("Text"));
		else if (pMesh->IsClass(VX_Mesh))
			str.Printf(TEXT("Mesh %d verts"), pMesh->GetNumVtx());
		hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
	}

	m_pTree->SetItemData(hNewItem, new MyTreeItemData(pNode));

	int num_children = pNode->GetSize();
	if (num_children > 200)
	{
		wxTreeItemId	hSubItem;
		str.Printf(TEXT("(%d children)"), num_children);
		hSubItem = m_pTree->AppendItem(hNewItem, str, 8, 8);
	}
	else if (num_children > 0)
	{
		GroupIter<Group> iter(pNode, Group::CHILDREN);
		Group* child;

		while (child = iter.Next())
			AddNodeItemsRecursively(hNewItem, child, depth + 1);
	}
	// expand a bit so that the tree is initially partially exposed
	if (depth < 3)
		m_pTree->Expand(hNewItem);
	return hNewItem;
}


// WDR: handler implementations for SceneGraphDlg

void SceneGraphDlg::OnRefresh( wxCommandEvent &event )
{
	RefreshTreeContents();
}

void SceneGraphDlg::OnZoomTo( wxCommandEvent &event )
{
	if (m_pSelectedNode && m_pSelectedNode->IsClass(VX_Model))
		GetMainScene()->ZoomToModel((Model*) m_pSelectedNode);
}

void SceneGraphDlg::OnEnabled( wxCommandEvent &event )
{
	if (m_pSelectedNode)
		m_pSelectedNode->SetActive(m_pEnabled->GetValue());
}

void SceneGraphDlg::OnTreeSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

	m_pEnabled->Enable(data != NULL);
	m_pSelectedNode = NULL;
	if (data && data->m_pNode)
	{
		m_pSelectedNode = data->m_pNode;
		m_pEnabled->SetValue(m_pSelectedNode->IsActive());
		m_pZoomTo->Enable(true);
	}
	else
		m_pZoomTo->Enable(false);
}

void SceneGraphDlg::OnInitDialog(wxInitDialogEvent& event)
{
	RefreshTreeContents();

	wxWindow::OnInitDialog(event);
}


wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{
    wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxTreeCtrl *item1 = new wxTreeCtrl( parent, ID_SCENETREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxSUNKEN_BORDER );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxBoxSizer *item2 = new wxBoxSizer( wxHORIZONTAL );

    wxCheckBox *item3 = new wxCheckBox( parent, ID_ENABLED, TEXT("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, ID_ZOOMTO, TEXT("Zoom To"), wxDefaultPosition, wxSize(60,-1), 0 );
    item2->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item5 = new wxButton( parent, ID_REFRESH, TEXT("Refresh"), wxDefaultPosition, wxSize(55,-1), 0 );
    item2->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item2, 0, wxALIGN_CENTRE|wxALL, 0 );

    if (set_sizer)
    {
        parent->SetAutoLayout( TRUE );
        parent->SetSizer( item0 );
        if (call_fit)
        {
            item0->Fit( parent );
            item0->SetSizeHints( parent );
        }
    }

    return item0;
}

#if defined(_WIN32)
}	// end Vixen
#endif