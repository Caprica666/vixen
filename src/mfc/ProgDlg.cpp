#include "vxmfc.h"
#include "mfc/ProgDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

namespace Vixen {
namespace MFC {

bool g_bProgressDlgIsOpen = false;

ProgressDlg::ProgressDlg(CString caption)
{
	m_strCaption = caption;

    m_nLower=0;
    m_nUpper=200;
    m_nStep=2;
    //{{AFX_DATA_INIT(ProgressDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_bParentDisabled = FALSE;
	g_bProgressDlgIsOpen = true;
}

ProgressDlg::~ProgressDlg()
{
    if(m_hWnd!=NULL)
      DestroyWindow();
	g_bProgressDlgIsOpen = false;
}

BOOL ProgressDlg::DestroyWindow()
{
    ReEnableParent();
    return CDialog::DestroyWindow();
}

void ProgressDlg::ReEnableParent()
{
    if(m_bParentDisabled && (m_pParentWnd!=NULL))
      m_pParentWnd->EnableWindow(TRUE);
    m_bParentDisabled=FALSE;
}

BOOL ProgressDlg::Create(CWnd *pParent)
{
    // Get the true parent of the dialog
    m_pParentWnd = CWnd::GetSafeOwner(pParent);

    // m_bParentDisabled is used to re-enable the parent window
    // when the dialog is destroyed. So we don't want to set
    // it to TRUE unless the parent was already enabled.

    if((m_pParentWnd!=NULL) && m_pParentWnd->IsWindowEnabled())
    {
      m_pParentWnd->EnableWindow(FALSE);
      m_bParentDisabled = TRUE;
    }

    if(!CDialog::Create(ProgressDlg::IDD,pParent))
    {
      ReEnableParent();
      return FALSE;
    }

    return TRUE;
}

void ProgressDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(ProgressDlg)
    DDX_Control(pDX, CG_IDC_PROGDLG_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ProgressDlg, CDialog)
    //{{AFX_MSG_MAP(ProgressDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void ProgressDlg::SetStatus(LPCTSTR lpszMessage)
{
    ASSERT(m_hWnd); // Don't call this _before_ the dialog has
                    // been created. Can be called from OnInitDialog
    CWnd *pWndStatus = GetDlgItem(CG_IDC_PROGDLG_STATUS);

    // Verify that the static text control exists
    ASSERT(pWndStatus!=NULL);
    pWndStatus->SetWindowText(lpszMessage);
}

void ProgressDlg::OnCancel()
{
}

void ProgressDlg::SetRange(int nLower,int nUpper)
{
    m_nLower = nLower;
    m_nUpper = nUpper;
    m_Progress.SetRange(nLower,nUpper);
}
  
int ProgressDlg::SetPos(int nPos)
{
    PumpMessages();
    return m_Progress.SetPos(nPos);
}

int ProgressDlg::SetStep(int nStep)
{
    m_nStep = nStep; // Store for later use in calculating percentage
    return m_Progress.SetStep(nStep);
}

int ProgressDlg::OffsetPos(int nPos)
{
    PumpMessages();
    return m_Progress.OffsetPos(nPos);
}

int ProgressDlg::StepIt()
{
    PumpMessages();
    return m_Progress.StepIt();
}

void ProgressDlg::PumpMessages()
{
    // Must call Create() before using the dialog
    ASSERT(m_hWnd!=NULL);

    MSG msg;
    // Handle dialog messages
	// exclude TIMER events!
    while(PeekMessage(&msg, NULL, WM_MOVE, WM_SYSCOMMAND, PM_REMOVE))
    {
      if(!IsDialogMessage(&msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);  
      }
    }
}


BOOL ProgressDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    m_Progress.SetRange(m_nLower,m_nUpper);
    m_Progress.SetStep(m_nStep);
    m_Progress.SetPos(m_nLower);

    SetWindowText(m_strCaption);

    return TRUE;  
}

}	// end MFC
}	// end Vixen