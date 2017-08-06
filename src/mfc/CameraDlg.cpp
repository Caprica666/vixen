
// CameraDlg.cpp : implementation file
//
#include "vxmfc.h"
#include "mfc/CameraDlg.h"

#define MIN_EXP		-2.0f
#define EXP_RANGE	6.0f
#define MIN_ANG		0.05f
#define ANG_RANGE	1.60f


namespace Vixen {
namespace MFC {

CameraDlg::CameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CameraDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CameraDlg)
	m_fHither = 0.0f;
	m_fViewAngle = 0.0f;
	m_fYon = 0.0f;
	m_fFlightSpeed = 10.0f;
	m_fAspect = 1.0f;
	//}}AFX_DATA_INIT

	m_pCamera = NULL;
	m_pLSaver = NULL;
}


void CameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CameraDlg)
	DDX_Control(pDX, IDC_FSPEED_S, m_FlightSpeed);
	DDX_Control(pDX, IDC_YON_S, m_Yon);
	DDX_Control(pDX, IDC_VANGLE_S, m_ViewAngle);
	DDX_Control(pDX, IDC_HITHER_S, m_Hither);
	DDX_Text(pDX, IDC_HITHER, m_fHither);
	DDV_MinMaxFloat(pDX, m_fHither, 1.e-002f, 10.f);
	DDX_Text(pDX, IDC_VANGLE, m_fViewAngle);
	DDV_MinMaxFloat(pDX, m_fViewAngle, 0.1f, 3.f);
	DDX_Text(pDX, IDC_YON, m_fYon);
	DDV_MinMaxFloat(pDX, m_fYon, 0.1f, 10000.f);
	DDX_Text(pDX, IDC_FSPEED, m_fFlightSpeed);
	DDV_MinMaxFloat(pDX, m_fFlightSpeed, 1.0f, 500.f);
	DDX_Text(pDX, IDC_ASPECT, m_fAspect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CameraDlg, CDialog)
	//{{AFX_MSG_MAP(CameraDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_HITHER, OnChangeHither)
	ON_EN_CHANGE(IDC_YON, OnChangeYon)
	ON_EN_CHANGE(IDC_VANGLE, OnChangeVangle)
	ON_BN_CLICKED(IDC_ZOOMTOSCENE, OnZoomtoscene)
	ON_BN_CLICKED(IDC_CUBEMAPVIEWS, OnCubeViews)
	ON_BN_CLICKED(IDC_BUTTON_STORE1, OnButtonStore1)
	ON_BN_CLICKED(IDC_BUTTON_STORE2, OnButtonStore2)
	ON_BN_CLICKED(IDC_BUTTON_STORE3, OnButtonStore3)
	ON_BN_CLICKED(IDC_BUTTON_STORE4, OnButtonStore4)
	ON_BN_CLICKED(IDC_BUTTON_STORE5, OnButtonStore5)
	ON_BN_CLICKED(IDC_BUTTON_STORE6, OnButtonStore6)
	ON_BN_CLICKED(IDC_BUTTON_RECALL1, OnButtonRecall1)
	ON_BN_CLICKED(IDC_BUTTON_RECALL2, OnButtonRecall2)
	ON_BN_CLICKED(IDC_BUTTON_RECALL3, OnButtonRecall3)
	ON_BN_CLICKED(IDC_BUTTON_RECALL4, OnButtonRecall4)
	ON_BN_CLICKED(IDC_BUTTON_RECALL5, OnButtonRecall5)
	ON_BN_CLICKED(IDC_BUTTON_RECALL6, OnButtonRecall6)
	ON_EN_CHANGE(IDC_FSPEED, OnChangeFspeed)
	ON_EN_CHANGE(IDC_ASPECT, OnChangeAspect)
	ON_COMMAND(ID_NAV_FLYER, OnNavFlyer)
	ON_UPDATE_COMMAND_UI(ID_NAV_FLYER, OnUpdateNavFlyer)
	ON_COMMAND(ID_NAV_DRIVER, OnNavDriver)
	ON_UPDATE_COMMAND_UI(ID_NAV_DRIVER, OnUpdateNavDriver)
	ON_COMMAND(ID_NAV_ARCBALL, OnNavArcball)
	ON_UPDATE_COMMAND_UI(ID_NAV_ARCBALL, OnUpdateNavArcball)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CameraDlg::SetCamera(Camera *pCam) 
{
	m_pCamera = pCam;
	if (m_pLSaver) m_pLSaver->SetTarget(m_pCamera);

	if (GetSafeHwnd() != NULL)
	{
		UpdateValues();
		UpdateSliders();
	}
}

void CameraDlg::SetLocationSaver(LocationSaver *pSaver)
{
	m_pLSaver = pSaver;
	if (m_pLSaver) m_pLSaver->SetTarget(m_pCamera);
}

/////////////////////////////////////////////////////////////////////////////
// CameraDlg message handlers

BOOL CameraDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateValues();
	m_Hither.SetRange(0, 1000);
	m_Hither.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_Yon.SetRange(0, 1000);
	m_Yon.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_ViewAngle.SetRange(0, 1000);
	m_ViewAngle.SetTicFreq((int)(1000.0f / ANG_RANGE / 16));

	m_FlightSpeed.SetRange(1, 100);
	m_FlightSpeed.SetTicFreq(10);
	m_FlightSpeed.SetPos((int) m_fFlightSpeed);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CameraDlg::UpdateValues()
{
	if (!m_pCamera) return;

	m_fHither = m_pCamera->GetHither();
	m_fYon = m_pCamera->GetYon();
	m_fViewAngle = m_pCamera->GetFOV();
	m_fAspect = m_pCamera->GetAspect();

//	GetFlightspeed();
	UpdateData(FALSE);
}

void CameraDlg::SetFlightspeed()
{
	Demo*	app = Demo::Get();
	Flyer*	fly = (Flyer*) app->GetNavigator();

	if (fly == NULL)
		return;

	Flyer::Iter iter(fly);
	while (fly = (Flyer*) iter.Next())
		if (fly->IsClass(VX_Flyer))
		{
			float	speed = m_fFlightSpeed;
			fly->SetSpeed(speed);
			return;
		}
}

void CameraDlg::GetFlightspeed()
{
	Demo*	app = Demo::Get();
	Flyer* fly = (Flyer*) app->GetNavigator();
	if (fly) m_fFlightSpeed = fly->GetSpeed();
}

void CameraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl *pSlider = (CSliderCtrl *)pScrollBar;

	switch (nSBCode)
	{
		case SB_PAGELEFT:
		case SB_PAGERIGHT:
			SliderMoved(pSlider, pSlider->GetPos());
			break;
		case SB_LEFT:
		case SB_RIGHT:
		case SB_LINELEFT:
		case SB_LINERIGHT:
		case SB_ENDSCROLL:
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
//			SliderMoved(pSlider, nPos);
			SliderMoved(pSlider, pSlider->GetPos());
			break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CameraDlg::SliderMoved(CSliderCtrl *pSlider, int nPos)
{
	int iMin, iMax;
	pSlider->GetRange(iMin, iMax);
	float fNewVal = (float)nPos / (iMax-iMin);

	if (pSlider == &m_Hither)
	{
		// exponential range from 0.1 to 1000.0
		m_fHither = (float)pow(10.0f, float(fNewVal * EXP_RANGE + MIN_EXP));
		m_pCamera->SetHither(m_fHither);
		UpdateData(FALSE);
	}
	if (pSlider == &m_Yon)
	{
		// exponential range from 0.1 to 1000.0
		m_fYon = (float)pow(10.0f, float(fNewVal * EXP_RANGE + MIN_EXP));
		m_pCamera->SetYon(m_fYon);
		UpdateData(FALSE);
	}
	if (pSlider == &m_ViewAngle)
	{
		// linear angular range from MIN_ANG to MIN_ANG + ANG_RANGE
		m_fViewAngle = MIN_ANG + fNewVal * ANG_RANGE;
		m_pCamera->SetFOV(m_fViewAngle);
		UpdateData(FALSE);
	}
	if (pSlider == &m_FlightSpeed)
	{
		m_fFlightSpeed = float(nPos);
		UpdateData(FALSE);
		SetFlightspeed();
	}
}

void CameraDlg::OnChangeHither() 
{
	UpdateData(TRUE);

	// range check
	if (m_fHither > m_fYon)
	{
		m_fHither = m_fYon;
		UpdateData(FALSE);
	}

	// update camera and slider
	m_pCamera->SetHither(m_fHither);
	UpdateSliders();
}

void CameraDlg::OnChangeYon() 
{
	UpdateData(TRUE);

	// range check
	if (m_fYon < m_fHither)
	{
		m_fYon = m_fHither;
		UpdateData(FALSE);
	}

	// update camera and slider
	m_pCamera->SetYon(m_fYon);
	UpdateSliders();
}

void CameraDlg::OnChangeVangle() 
{
	UpdateData(TRUE);

	// update camera and slider
	m_pCamera->SetFOV(m_fViewAngle);
	UpdateSliders();
}

void CameraDlg::UpdateSliders()
{
	int iMin, iMax;
	float fNewVal;

	m_Hither.GetRange(iMin, iMax);
	fNewVal = (float)((log10(m_fHither)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_Hither.SetPos((int)fNewVal);

	fNewVal = (float)((log10(m_fYon)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_Yon.SetPos((int)fNewVal);

	fNewVal = (float)((m_fViewAngle-MIN_ANG)/ANG_RANGE) * (iMax-iMin);
	m_ViewAngle.SetPos((int)fNewVal);

	fNewVal = m_fFlightSpeed;
	m_FlightSpeed.SetPos((int)fNewVal);
}

void CameraDlg::OnZoomtoscene() 
{
	Scene *scene = GetMainScene();
	if (scene)
		scene->ShowAll();
}

void CameraDlg::OnCubeViews() 
{
	Camera* cam = (Camera*) m_pLSaver->GetTarget();
	if ((cam == NULL) || !cam->IsClass(VX_Camera))
		return;
	((Frame*) AfxGetMainWnd())->ResizePower2(GetParent());
	Vec3 pos = cam->GetTranslation();
	cam->SetType(Camera::ORTHOGRAPHIC);
	cam->SetFOV(PI / 2);
	cam->Reset();
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(0);	// FRONT
	cam->Reset();
	cam->Rotate(Model::YAXIS, PI / 2);
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(1);	// LEFT
	cam->Reset();
	cam->Rotate(Model::YAXIS, -PI / 2);
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(2);	// RIGHT
	cam->Reset();
	cam->Rotate(Model::YAXIS, PI);
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(3);	// BACK
	cam->Reset();
	cam->Rotate(Model::XAXIS, PI / 2);
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(4);	// TOP
	cam->Reset();
	cam->Rotate(Model::XAXIS, -PI / 2);
	cam->SetTranslation(pos);
	m_pLSaver->StoreCurrentLocationTo(5);	// BOTTOM
	cam->Reset();
	cam->SetType(Camera::PERSPECTIVE);
	cam->SetTranslation(pos);
}

void CameraDlg::OnButtonStore1() 
{
	m_pLSaver->StoreCurrentLocationTo(0);
}

void CameraDlg::OnButtonStore2() 
{
	m_pLSaver->StoreCurrentLocationTo(1);
}

void CameraDlg::OnButtonStore3() 
{
	m_pLSaver->StoreCurrentLocationTo(2);
}

void CameraDlg::OnButtonStore4() 
{
	m_pLSaver->StoreCurrentLocationTo(3);
}

void CameraDlg::OnButtonStore5() 
{
	m_pLSaver->StoreCurrentLocationTo(4);
}

void CameraDlg::OnButtonStore6() 
{
	m_pLSaver->StoreCurrentLocationTo(5);
}

/////////////////

void CameraDlg::OnButtonRecall1() 
{
	m_pLSaver->RecallStoredLocation(0);
	SyncNavigator();
}

void CameraDlg::OnButtonRecall2() 
{
	m_pLSaver->RecallStoredLocation(1);
	SyncNavigator();
}

void CameraDlg::OnButtonRecall3() 
{
	m_pLSaver->RecallStoredLocation(2);
	SyncNavigator();
}

void CameraDlg::OnButtonRecall4() 
{
	m_pLSaver->RecallStoredLocation(3);
	SyncNavigator();
}

void CameraDlg::OnButtonRecall5() 
{
	m_pLSaver->RecallStoredLocation(4);
	SyncNavigator();
}

void CameraDlg::OnButtonRecall6() 
{
	m_pLSaver->RecallStoredLocation(5);
	SyncNavigator();
}

void CameraDlg::OnChangeFspeed() 
{
	UpdateData(TRUE);

	// range check
	if (m_fFlightSpeed < 1.0f)
	{
		m_fFlightSpeed = 1.0f;
		UpdateData(FALSE);
	}

	// update flight engine and slider
	SetFlightspeed();
	UpdateSliders();
}

void CameraDlg::OnChangeAspect() 
{
	UpdateData(TRUE);
	
	GetMainScene()->SetAutoAdjust(false);
	m_pCamera->SetAspect(m_fAspect);
}

void CameraDlg::JumpToSavedViewpoint(int num)
{
	switch (num)
	{
		case 1: OnButtonRecall1(); break;
		case 2: OnButtonRecall2(); break;
		case 3: OnButtonRecall3(); break;
		case 4: OnButtonRecall4(); break;
		case 5: OnButtonRecall5(); break;
		case 6: OnButtonRecall6(); break;
	}
}

void CameraDlg::SyncNavigator()
{
	Demo*	app = Demo::Get();
	Camera* cam = (Camera*) m_pLSaver->GetTarget();
	Transformer* eng = (Transformer*) app->GetNavigator();
	if (cam->IsClass(VX_Camera))
		if (eng && eng->IsClass(VX_Transformer))
			eng->SetTransform(cam->GetTransform());
}


void CameraDlg::OnNavFlyer() 
{
	// TODO: Add your command handler code here
	
}

void CameraDlg::OnUpdateNavFlyer(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CameraDlg::OnNavDriver() 
{
	// TODO: Add your command handler code here
	
}

void CameraDlg::OnUpdateNavDriver(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CameraDlg::OnNavArcball() 
{
	// TODO: Add your command handler code here
	
}

void CameraDlg::OnUpdateNavArcball(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

}	// end MFC
}	// end Vixen