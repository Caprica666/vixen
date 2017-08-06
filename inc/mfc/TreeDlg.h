#pragma once
/////////////////////////////////////////////////////////////////////////////
// TreeDlg dialog

namespace Vixen
{
	namespace MFC
	{
		/*!
		 * @class TreeDlg
		 * @brief MFC-based tree dialog that shows the Vixen scene hierarchy.
		 *
		 * @see MFC::OrientDlg MFC::DemoView MFC::Demo MFC::CameraDlg MFC::ProgressDlg
		 */
		class TreeDlg : public CDialog
		{
		// Construction
		public:
			void OpenKeyFramerDialog(HTREEITEM hItem);
			void OpenOrientDialog(HTREEITEM hItem);
			void AddModelItemsRecursively(HTREEITEM hParentItem, Model *pModel, int depth);
			void AddEngineItemsRecursively(HTREEITEM hParentItem, Engine *pEngine);
			void RefreshTreeContents(Scene* scene = NULL);
			TreeDlg(CWnd* pParent = NULL);   // standard constructor

		// Dialog Data
			//{{AFX_DATA(TreeDlg)
			enum { IDD = IDD_TREEVIEW };
			CButton	m_Enabled;
			CButton	m_ZoomToNodeButton;
			CButton	m_StartMeButton;
			CTreeCtrl	m_Tree;
			//}}AFX_DATA


		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(TreeDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			CImageList m_ImgList;

			// Generated message map functions
			//{{AFX_MSG(TreeDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnZoomtonode();
			afx_msg void OnStartEngine();
			afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSize(UINT nType, int cx, int cy);
			afx_msg void OnEnabled();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC
}	// end Vixen

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
