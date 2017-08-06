// EnvironDlg.cpp : implementation file
//

#include "vxmfc.h"
#include "mfc/EnvironDlg.h"

#define MIN_EXP		-2.0f
#define EXP_RANGE	6.0f

#define MIN_DEN		0.00f
#define DEN_RANGE	0.10f

namespace Vixen {
namespace MFC {

EnvironDlg::EnvironDlg(CWnd* pParent /*=NULL*/)
	: CDialog(EnvironDlg::IDD, pParent)
{
	m_pFog = NULL;

	//{{AFX_DATA_INIT(EnvironDlg)
	m_fFogEnd = 0.0f;
	m_fFogStart = 0.0f;
	m_fDensity = 0.0f;
	m_iType = -1;
	//}}AFX_DATA_INIT
}


void EnvironDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EnvironDlg)
	DDX_Control(pDX, IDC_DENSITY2, m_Density);
	DDX_Control(pDX, IDC_START1, m_FogStart1);
	DDX_Control(pDX, IDC_END1, m_FogEnd1);
	DDX_Control(pDX, IDC_START2, m_FogStart2);
	DDX_Control(pDX, IDC_END2, m_FogEnd2);
	DDX_Control(pDX, IDC_FOGCOLOR, m_FogColor);
	DDX_Control(pDX, IDC_BGCOLOR, m_BgColor);
	DDX_Text(pDX, IDC_END1, m_fFogEnd);
	DDX_Text(pDX, IDC_START1, m_fFogStart);
	DDX_Text(pDX, IDC_DENSITY1, m_fDensity);
	DDX_Radio(pDX, IDC_LINEAR, m_iType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EnvironDlg, CDialog)
	//{{AFX_MSG_MAP(EnvironDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_END1, OnChangeEnd1)
	ON_EN_CHANGE(IDC_START1, OnChangeStart1)
	ON_BN_CLICKED(IDC_FOGCOLOR, OnButtonFogcolor)
	ON_BN_CLICKED(IDC_BGCOLOR, OnButtonBgcolor)
	ON_BN_CLICKED(IDC_AMBIENT, OnButtonAmbient)
	ON_EN_CHANGE(IDC_DENSITY1, OnChangeDensity1)
	ON_BN_CLICKED(IDC_LINEAR, OnLinear)
	ON_BN_CLICKED(IDC_EXPONENTIAL, OnLinear)
	ON_BN_CLICKED(IDC_EXPONENTIAL2, OnLinear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EnvironDlg message handlers

BOOL EnvironDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateValues();

	m_FogStart2.SetRange(0, 1000);
	m_FogStart2.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_FogEnd2.SetRange(0, 1000);
	m_FogEnd2.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_Density.SetRange(0, 1000);
	m_Density.SetTicFreq(100);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void EnvironDlg::SetFog(Fog *pFog)
{
	m_pFog = pFog;

	m_FogStart1.EnableWindow(m_pFog != NULL);
	m_FogEnd1.EnableWindow(m_pFog != NULL);
	m_FogStart2.EnableWindow(m_pFog != NULL);
	m_FogEnd2.EnableWindow(m_pFog != NULL);
	m_FogColor.EnableWindow(m_pFog != NULL);
	m_Density.EnableWindow(m_pFog != NULL);

	UpdateValues();
	UpdateSliders();
	Invalidate(false);
}


void EnvironDlg::UpdateValues()
{
	if (!m_pFog) return;

	m_fFogStart = m_pFog->GetStart();
	m_fFogEnd = m_pFog->GetEnd();
	m_fDensity = m_pFog->GetDensity();
	m_iType = m_pFog->GetKind();

	UpdateData(FALSE);
}


void EnvironDlg::UpdateSliders()
{
	int iMin, iMax;
	float fNewVal;

	m_FogStart2.GetRange(iMin, iMax);
	fNewVal = (float)((log10(m_fFogStart)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_FogStart2.SetPos((int)fNewVal);

	m_FogEnd2.GetRange(iMin, iMax);
	fNewVal = (float)((log10(m_fFogEnd)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_FogEnd2.SetPos((int)fNewVal);

	m_Density.GetRange(iMin, iMax);
	fNewVal = (float)((m_fDensity-MIN_DEN)/DEN_RANGE) * (iMax-iMin);
	m_Density.SetPos((int)fNewVal);
}


void EnvironDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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


void EnvironDlg::SliderMoved(CSliderCtrl *pSlider, int nPos)
{
	int iMin, iMax;
	pSlider->GetRange(iMin, iMax);
	float fNewVal = (float)nPos / (iMax-iMin);

	if (pSlider == &m_FogStart2)
	{
		// exponential range from 0.1 to 1000.0
		m_fFogStart = (float)pow(10.0f, float(fNewVal * EXP_RANGE + MIN_EXP));
		if (m_pFog) m_pFog->SetStart(m_fFogStart);
		UpdateData(FALSE);
	}
	if (pSlider == &m_FogEnd2)
	{
		// exponential range from 0.1 to 1000.0
		m_fFogEnd = (float)pow(10.0f, float(fNewVal * EXP_RANGE + MIN_EXP));
		if (m_pFog) m_pFog->SetEnd(m_fFogEnd);
		UpdateData(FALSE);
	}
	if (pSlider == &m_Density)
	{
		// exponential range from 0.1 to 1000.0
		m_fDensity = MIN_DEN + fNewVal * DEN_RANGE;
		if (m_pFog) m_pFog->SetDensity(m_fDensity);
		UpdateData(FALSE);
	}
}


void EnvironDlg::OnChangeEnd1() 
{
	UpdateData(TRUE);
	if (m_pFog) m_pFog->SetEnd(m_fFogStart);
	UpdateSliders();
}

void EnvironDlg::OnChangeStart1() 
{
	UpdateData(TRUE);
	if (m_pFog) m_pFog->SetStart(m_fFogStart);
	UpdateSliders();
}

void EnvironDlg::OnChangeDensity1() 
{
	UpdateData(TRUE);
	if (m_pFog) m_pFog->SetDensity(m_fDensity);
	UpdateSliders();
}


void EnvironDlg::OnButtonFogcolor() 
{
	if (m_pFog == NULL)
		AfxMessageBox(TEXT("No fog"));
	else
	{
		Col4 pscol = m_pFog->GetColor();
		COLORREF clrInit = RGB(pscol.r * 255, pscol.g * 255, pscol.b * 255);
		CColorDialog dlg(clrInit, CC_FULLOPEN, this);

		if (dlg.DoModal() == IDOK)
		{
			pscol.r = (float) GetRValue(dlg.m_cc.rgbResult) / 255.0f;
			pscol.g = (float) GetGValue(dlg.m_cc.rgbResult) / 255.0f;
			pscol.b = (float) GetBValue(dlg.m_cc.rgbResult) / 255.0f;
			m_pFog->SetColor(pscol);
		}
	}
}

void EnvironDlg::OnButtonBgcolor() 
{
	Scene *pScene = GetMainScene();
	Col4 pscol = pScene->GetBackColor();

	COLORREF clrInit = RGB(pscol.r * 255, pscol.g * 255, pscol.b * 255);
	CColorDialog dlg(clrInit, CC_FULLOPEN, this);

	if (dlg.DoModal() == IDOK)
	{
		pscol.r = (float) GetRValue(dlg.m_cc.rgbResult) / 255.0f;
		pscol.g = (float) GetGValue(dlg.m_cc.rgbResult) / 255.0f;
		pscol.b = (float) GetBValue(dlg.m_cc.rgbResult) / 255.0f;
		pScene->SetBackColor(pscol);
	}
}

void EnvironDlg::OnButtonAmbient() 
{
	Scene *pScene = GetMainScene();
	Light*	light = pScene->GetAmbient();
	Col4	pscol(0, 0, 0);

	if (light)
		pscol = light->GetColor();
	COLORREF clrInit = RGB(pscol.r * 255, pscol.g * 255, pscol.b * 255);
	CColorDialog dlg(clrInit, CC_FULLOPEN, this);

	if (dlg.DoModal() == IDOK)
	{
		pscol.r = (float) GetRValue(dlg.m_cc.rgbResult) / 255.0f;
		pscol.g = (float) GetGValue(dlg.m_cc.rgbResult) / 255.0f;
		pscol.b = (float) GetBValue(dlg.m_cc.rgbResult) / 255.0f;
		pScene->SetAmbient(pscol);
	}
}

void EnvironDlg::OnLinear() 
{
	UpdateData(TRUE);
	if (m_pFog) m_pFog->SetKind(m_iType);
	UpdateSliders();
}

}	// end MFC
}	// end Vixen