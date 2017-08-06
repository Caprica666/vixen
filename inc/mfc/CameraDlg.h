//
// Header for the Camera Dialog
//

#pragma once

#include "LocationSaver.h"
#include "afresource.h"

namespace Vixen
{
	namespace MFC
	{
		/*!
		 * @class CameraDlg
		 * @brief MFC dialog to change Vixen 3D camera parameters.
		 *
		 * @see MFC::TreeDlg MFC::DemoView MFC::Demo MFC::OrientDlg MFC::ProgressDlg MFC::LocationSaver
		 */
		class CameraDlg : public CDialog
		{
		// Construction
		public:
			void UpdateSliders();
			void SliderMoved(CSliderCtrl *pSlider, int nPos);
			void UpdateValues();
			void SetFlightspeed();
			void GetFlightspeed();
			CameraDlg(CWnd* pParent = NULL);   // standard constructor
			void SetCamera(Camera *);
			void JumpToSavedViewpoint(int num);
			void SyncNavigator();
			void SetLocationSaver(LocationSaver *pSaver);
			LocationSaver *GetLocationSaver() { return m_pLSaver; }

		// Dialog Data
			//{{AFX_DATA(CameraDlg)
			enum { IDD = IDD_CAMDIALOG };
			CSliderCtrl	m_FlightSpeed;
			CSliderCtrl	m_Yon;
			CSliderCtrl	m_ViewAngle;
			CSliderCtrl	m_Hither;
			float	m_fHither;
			float	m_fViewAngle;
			float	m_fYon;
			float	m_fFlightSpeed;
			float	m_fAspect;
			//}}AFX_DATA

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CameraDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			Camera *m_pCamera;
			LocationSaver *m_pLSaver;

			// Generated message map functions
			//{{AFX_MSG(CameraDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnChangeHither();
			afx_msg void OnChangeYon();
			afx_msg void OnChangeVangle();
			afx_msg void OnZoomtoscene();
			afx_msg void OnCubeViews();
			afx_msg void OnButtonStore1();
			afx_msg void OnButtonStore2();
			afx_msg void OnButtonStore3();
			afx_msg void OnButtonStore4();
			afx_msg void OnButtonStore5();
			afx_msg void OnButtonStore6();
			afx_msg void OnButtonRecall1();
			afx_msg void OnButtonRecall2();
			afx_msg void OnButtonRecall3();
			afx_msg void OnButtonRecall4();
			afx_msg void OnButtonRecall5();
			afx_msg void OnButtonRecall6();
			afx_msg void OnChangeFspeed();
			afx_msg void OnChangeAspect();
			afx_msg void OnNavFlyer();
			afx_msg void OnUpdateNavFlyer(CCmdUI* pCmdUI);
			afx_msg void OnNavDriver();
			afx_msg void OnUpdateNavDriver(CCmdUI* pCmdUI);
			afx_msg void OnNavArcball();
			afx_msg void OnUpdateNavArcball(CCmdUI* pCmdUI);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	 // end MFC
}		// end Vixen
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
