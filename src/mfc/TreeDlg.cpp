#include "vxmfc.h"
#include "mfc/afresource.h"
#include "mfc/TreeDlg.h"
#include "mfc/CameraDlg.h"

namespace Vixen {
namespace MFC {


#define	TREEID_LIGHT	1
#define	TREEID_SHAPE	2
#define	TREEID_CAMERA	3
#define	TREEID_GROUP	4
#define TREEID_GEOMETRY	5
#define	TREEID_ENGINE	6
#define TREEID_KEYFRAMER 7
#define	TREEID_MODEL	8
#define TREEID_TOOMANY	10


TreeDlg::TreeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TreeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(TreeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TreeDlg)
	DDX_Control(pDX, IDC_ENABLED, m_Enabled);
	DDX_Control(pDX, IDC_ZOOMTONODE, m_ZoomToNodeButton);
	DDX_Control(pDX, IDC_PLAYANIM, m_StartMeButton);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TreeDlg, CDialog)
	//{{AFX_MSG_MAP(TreeDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnDblclkTree)
	ON_BN_CLICKED(IDC_ZOOMTONODE, OnZoomtonode)
	ON_BN_CLICKED(IDC_PLAYANIM, OnStartEngine)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL TreeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ImgList.Create(IDB_TREEIMAGES, 16, 16, (COLORREF)0xFFFFFF);
	m_Tree.SetImageList(&m_ImgList, TVSIL_NORMAL);	// TVSIL_STATE?

	Scene* scene = GetMainScene();
	if (scene)
	{
		scene->Suspend();		// lock around hierarchy changes
		RefreshTreeContents(scene);
		scene->Resume();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TreeDlg::RefreshTreeContents(Scene* scene)
{
	if (!scene)
	{
		scene = GetMainScene();
		if (!scene)
			return;
	}
	if (!m_Tree)
		return;

	// start with a blank slate
	m_Tree.DeleteAllItems();

	HTREEITEM hModels = m_Tree.InsertItem(TEXT("Models"));
	HTREEITEM hEngines = m_Tree.InsertItem(TEXT("Engines"));
	m_Tree.SetItemImage(hModels, 0, 0);
	m_Tree.SetItemImage(hEngines, 0, 0);

	Model* pMod = scene->GetModels();
	if (pMod) AddModelItemsRecursively(hModels, pMod, 0);

	// Fill in the tree with engines
	Engine *pEng = scene->GetEngines();
	if (pEng) AddEngineItemsRecursively(hEngines, pEng);

	m_Tree.Expand(hModels, TVE_EXPAND);
}


void TreeDlg::AddModelItemsRecursively(HTREEITEM hParentItem,
										Model *pModel, int depth)
{
	if (!pModel)
		return;

	const TCHAR* classname = pModel->ClassName();
	CString name(classname);
	CString	str;
	int		nImage;
	HTREEITEM hNewItem = TVI_ROOT;

	if (pModel->IsClass(VX_Light))
		nImage = TREEID_LIGHT;
	else if (pModel->IsClass(VX_Camera))
		nImage = TREEID_CAMERA;
	else if (pModel->IsClass(VX_Shape))
		nImage = TREEID_SHAPE;
	else if (pModel->IsClass(VX_Model)) 
		nImage = TREEID_MODEL;
	else
		nImage = TREEID_GROUP;
	if (pModel->GetName())
	{
		name += " ";
		name += pModel->GetName();
	}
	str.Format(TEXT("%s "), name);
	hNewItem = m_Tree.InsertItem(str, hParentItem);
	m_Tree.SetItemImage(hNewItem, nImage, nImage);

	if (pModel->IsClass(VX_Shape))
	{
		const TriMesh*	pMesh;
		HTREEITEM		hGeomItem;
		intptr			iNumPrim;
		intptr			nverts;

		pMesh = (const TriMesh*) ((Shape*)pModel)->GetGeometry();
		if (!pMesh)
			hGeomItem = m_Tree.InsertItem(TEXT("no surfaces"), hNewItem);
		else
		{
			nverts = pMesh->GetNumVtx();
			if (pMesh->IsClass(VX_TriMesh))
			{
				if (pMesh->GetNumIdx() > 0)
					iNumPrim = pMesh->GetNumFaces();
				else
					iNumPrim = nverts / 3;
				VX_ASSERT(iNumPrim < INT_MAX);
				VX_ASSERT(nverts < INT_MAX);
				str.Format(TEXT("TriMesh (%d faces)"), (int) iNumPrim);
			}
			else
			{
				str.Format(TEXT("Geometry (%d verts)"), (int) nverts);
			}
			hGeomItem = m_Tree.InsertItem(str, hNewItem);
			m_Tree.SetItemImage(hGeomItem, TREEID_GEOMETRY, 5);
		}
	}

	if (hNewItem == TVI_ROOT) return;
	m_Tree.SetItemData(hNewItem, (DWORD_PTR) pModel);

	int num_children = pModel->GetSize();
	if (num_children > 300)
	{
		HTREEITEM	hSubItem;
		str.Format(TEXT("(%d children)"), num_children);
		hSubItem = m_Tree.InsertItem(str, hNewItem);
		m_Tree.SetItemImage(hSubItem, TREEID_TOOMANY, 10);
	}
	else
	{
		GroupIter<Model>	iter((Model*) pModel, Group::CHILDREN);

		Model *pShape;
		while (pShape = iter.Next())
		{
			AddModelItemsRecursively(hNewItem, pShape, depth+1);
		}
	}

	if (depth < 2)
		m_Tree.Expand(hNewItem, TVE_EXPAND);
}


void TreeDlg::AddEngineItemsRecursively(HTREEITEM hParentItem, Engine *pEngine)
{
	if (!pEngine) return;

	int				nImage = TREEID_ENGINE;
	const TCHAR*	classname = pEngine->ClassName();
	CString			name(classname);
	CString			str1;
	CString			str2;
	char			eng_state = TEXT('?');

	switch (pEngine->GetState() & ~Engine::REVERSE)
	{
		case Engine::IDLE: eng_state = TEXT('I'); break;
		case Engine::DONE: eng_state = TEXT('D'); break;
		case Engine::RUNNING: eng_state = TEXT('R'); break;
	}
	if (pEngine->GetName())
	{
		name += " ";
		name += pEngine->GetName();
	}
	str1.Format(TEXT("%s %c "), name, eng_state);
	if (pEngine->IsClass(VX_Interpolator))
	{
		const Interpolator*	pInterp = (const Interpolator*) pEngine;
		intptr			nkeys = pInterp->GetSize();

		str2.Format(TEXT(" (%d keys)"), (int) nkeys);
		str1 += str2;
	}
	Model *pTarget = (Model*) pEngine->GetTarget();
	if (pTarget && pTarget->GetName())
	{
		str1+= " -> ";
		str1 += pTarget->GetName();
	}

	HTREEITEM hNewItem = m_Tree.InsertItem(str1, hParentItem);
	m_Tree.SetItemImage(hNewItem, nImage, nImage);
	m_Tree.SetItemData(hNewItem, (DWORD_PTR) pEngine);

	Engine *pChild;
	GroupIter<Engine>	iter((Engine*) pEngine, Group::CHILDREN);

	while (pChild = iter.Next())
	{
		AddEngineItemsRecursively(hNewItem, pChild);
	}
//	m_Tree.Expand(hNewItem, TVE_EXPAND);
}

void TreeDlg::OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if (hItem != NULL)
	{
		Model* root = (Model*) m_Tree.GetItemData(hItem);
		int		imageNum;

		m_Tree.GetItemImage(hItem, imageNum, imageNum);
		if (root && (imageNum <= 5))
		{
			bool active = root->IsActive();
			Model* m;
			Model::Iter iter(root, Group::DEPTH_FIRST);
			while (m = (Model*) iter.Next())
				m->SetActive(!active);
		}
	}
	*pResult = 0;
}

void TreeDlg::OnZoomtonode() 
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	Model*	pModel = (Model *)(m_Tree.GetItemData(hItem));
	Demo*	app = Demo::Get();
	Scene*	scene = GetMainScene();
	Camera*	cam;
	Transformer* eng;

	if (!pModel)
		return;
	if (!scene)
		return;
	eng = (Transformer*) app->GetNavigator();
	if (pModel->IsClass(VX_Camera))
	{
		app->SetCamera((Camera*) pModel);
		return;
	}
	cam = scene->GetCamera();
	scene->ZoomToModel(pModel);
	if (eng && eng->IsClass(VX_Transformer))
		eng->SetTransform(cam->GetTransform());
}

void TreeDlg::OnStartEngine() 
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	Engine *eng = (Engine *)(m_Tree.GetItemData(hItem));

	if (eng)
		eng->Start();
}

void TreeDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	TV_ITEM itemNew = pNMTreeView->itemNew;

	Model *pModel = NULL;
	Engine *pEngine = NULL;

	int iImage, iDummy;
	m_Tree.GetItemImage(itemNew.hItem, iImage, iDummy);
	switch (iImage)
	{
		case TREEID_LIGHT:		// light
		case TREEID_SHAPE:		// shape
		case TREEID_CAMERA:		// camera
		case TREEID_GROUP:		// group
		case TREEID_MODEL:		// model
		pModel = (Model *)(m_Tree.GetItemData(itemNew.hItem));
		break;

		case TREEID_ENGINE:		// engine
		case TREEID_KEYFRAMER:	// keyframer
		pEngine = (Engine *)(m_Tree.GetItemData(itemNew.hItem));
		break;
	}

	if (pModel || pEngine)
		m_Enabled.EnableWindow(TRUE);
	else
	{
		m_Enabled.SetCheck(false);
		m_Enabled.EnableWindow(FALSE);
	}

	if (pModel)
	{
		m_ZoomToNodeButton.EnableWindow(TRUE);
		m_StartMeButton.EnableWindow(FALSE);
		m_Enabled.SetCheck(pModel->IsActive());
	}
	if (pEngine)
	{
		m_ZoomToNodeButton.EnableWindow(FALSE);
		m_StartMeButton.EnableWindow(TRUE);
		m_Enabled.SetCheck(pEngine->IsActive());
	}

	*pResult = 0;
}

void TreeDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	m_Tree.SetWindowPos(NULL, 0, 0, cx-24, cy-55, SWP_NOZORDER|SWP_NOMOVE);

	RECT rectB;
	m_ZoomToNodeButton.GetWindowRect(&rectB);
	DWORD w = rectB.right-rectB.left + 16;
	m_ZoomToNodeButton.SetWindowPos(NULL, cx - 2 * w, cy-31, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_StartMeButton.GetWindowRect(&rectB);
	m_StartMeButton.SetWindowPos(NULL, cx - w, cy-31, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_Enabled.SetWindowPos(NULL, 16, cy-31, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

void TreeDlg::OnEnabled() 
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	int iImage, iDummy;
	m_Tree.GetItemImage(hItem, iImage, iDummy);
	switch (iImage)
	{
		case TREEID_LIGHT:		// light
		case TREEID_SHAPE:		// shape
		case TREEID_CAMERA:		// camera
		case TREEID_GROUP:		// group
		case TREEID_MODEL:		// model
		{
			Model *pModel = (Model *)(m_Tree.GetItemData(hItem));
			if (pModel)
				pModel->SetActive(m_Enabled.GetCheck() != 0);
		}
		break;

		case TREEID_ENGINE:		// engine
		case TREEID_KEYFRAMER:	// keyframer
		{
			Engine *pEngine = (Engine *)(m_Tree.GetItemData(hItem));
			if (!pEngine) return;
			if (m_Enabled.GetCheck())
			{
				pEngine->SetActive(true);
				pEngine->Start();
			}
			else
			{
				pEngine->Stop();
				pEngine->SetActive(false);
			}
		}
		break;
	}
}

}	// end MFC
}	// end Vixen