#pragma once


namespace Vixen
{

	namespace MFC
	{
	/*!
	 * @class EnvironDlg
	 * @brief MFC dialog to change Vixen 3D scene environment.
	 *
	 * @see MFC::TreeDlg MFC::DemoView MFC::Demo MFC::CameraDlg MFC::ProgressDlg
	 */
		class EnvironDlg : public CDialog
		{
		// Construction
		public:
			EnvironDlg(CWnd* pParent = NULL);   // standard constructor

		// Dialog Data
			//{{AFX_DATA(EnvironDlg)
			enum { IDD = IDD_ENVIRON };
			CSliderCtrl	m_Density;
			CEdit	m_FogStart1;
			CEdit	m_FogEnd1;
			CSliderCtrl	m_FogStart2;
			CSliderCtrl	m_FogEnd2;
			CButton	m_FogColor;
			CButton	m_BgColor;
			float	m_fFogEnd;
			float	m_fFogStart;
			float	m_fDensity;
			int		m_iType;
			//}}AFX_DATA

			void SliderMoved(CSliderCtrl *pSlider, int nPos);
			void SetFog(Fog *pFog);
			void UpdateValues();
			void UpdateSliders();

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(EnvironDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			Fog	*m_pFog;

			// Generated message map functions
			//{{AFX_MSG(EnvironDlg)
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeEnd1();
			afx_msg void OnChangeStart1();
			afx_msg void OnButtonFogcolor();
			afx_msg void OnButtonBgcolor();
			afx_msg void OnButtonAmbient();
			afx_msg void OnChangeDensity1();
			afx_msg void OnLinear();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC
}		// end Vixen
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

