
#pragma once
namespace Vixen
{
	namespace MFC
	{
		class COrientDlg : public CDialog
		{
		// Construction
		public:
			COrientDlg(CWnd* pParent = NULL);   // standard constructor

			void SliderMoved(CSliderCtrl *pSlider, int nPos);
			void ApplyRotationToTarget();
			void SetTargetModel(Model *m) { m_pTargetModel = m; }

		// Dialog Data
			//{{AFX_DATA(COrientDlg)
			enum { IDD = IDD_ORIENT };
			CSliderCtrl	m_pYaw;
			CSliderCtrl	m_pPitch;
			//}}AFX_DATA

			void UpdateSliders();

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(COrientDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			float	m_fPitch, m_fYaw;
			Model*	m_pTargetModel;

			// Generated message map functions
			//{{AFX_MSG(COrientDlg)
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

	}	// end MFC
}		// end Vixen

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

