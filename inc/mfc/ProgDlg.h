// ProgDlg.h : header file
// CG: This file was added by the Progress Dialog component

/////////////////////////////////////////////////////////////////////////////
// ProgressDlg dialog

#pragma once

namespace Vixen
{
	namespace MFC
	{
		/*!
		 * @class ProgressDlg
		 * @brief MFC-based progress dialog to use with Vixen applications.
		 * @see MFC::TreeDlg MFC::DemoView MFC::Demo MFC::OrientDlg MFC::CameraDlg
		 */
		class ProgressDlg : public CDialog
		{
		// Construction / Destruction
		public:
			ProgressDlg(CString caption);   // standard constructor
			~ProgressDlg();

			BOOL Create(CWnd *pParent=NULL);

			// Progress Dialog manipulation
			void SetStatus(LPCTSTR lpszMessage);
			void SetRange(int nLower,int nUpper);
			int  SetStep(int nStep);
			int  SetPos(int nPos);
			int  OffsetPos(int nPos);
			int  StepIt();
        
		// Dialog Data
			//{{AFX_DATA(ProgressDlg)
			enum { IDD = CG_IDD_PROGRESS };
			CProgressCtrl	m_Progress;
			//}}AFX_DATA

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(ProgressDlg)
			public:
			virtual BOOL DestroyWindow();
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			CString m_strCaption;
			int m_nLower;
			int m_nUpper;
			int m_nStep;
			BOOL m_bParentDisabled;

			void ReEnableParent();

			virtual void OnCancel();
			virtual void OnOK() {}; 
			void PumpMessages();

			// Generated message map functions
			//{{AFX_MSG(ProgressDlg)
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC
}	// end Vixen
