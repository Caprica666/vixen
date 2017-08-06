#pragma once

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "mfc/TreeDlg.h"
#include "mfc/CameraDlg.h"

namespace Vixen
{
	namespace MFC
	{
	/*!
	 * @class DemoView
	 * @brief Adds support for the scene manager 3D Viewer user interface to your application.
	 * This subclass of CView responds to the message IDs in the
	 * Vixen menus and dialogs added by Demo::LoadSceneMenus.
	 *
	 * The scene manager menus let you
	 *	- view scene as shaded or wireframe
	 *	- zoom camera to view entire scene
	 *	- manipulate scene graph with a tree control
	 *	- set camera speed and store camera viewpoints
	 *	- record, save and run camera scripts
	 *
	 * @ingroup vixenmfc
	 * @see View Demo
	 */
		class DemoView : public View
		{
		protected:
			DemoView();           // protected constructor used by dynamic creation
			DECLARE_DYNCREATE(DemoView)
			bool			LoadSceneMenus(const TCHAR* menunames = NULL);
			NavRecorder*	GetNavRecorder() const;
			Scene*			GetScene() const	{ return ((Demo*) World::Get())->GetScene((Window) GetSafeHwnd()); }
#ifdef VX_USE_SCRIPTOR
			Animator*		GetAnimator() const	{ return ((Demo*) World::Get())->GetAnimator(); }
#endif

		// Attributes
		public:
			TreeDlg			m_wndTreeDlg;
			CameraDlg		m_wndCameraDlg;
			LocationSaver	m_DefaultLocSaver;
			CMenu*			m_sceneMenu;

			bool m_bSpinScene;
			bool m_bSpinLight;
			bool m_bWireframe;
			bool m_bShowInfo;
			bool m_bViewSkin;

		// Operations
		public:
			void SetWireframe(bool on);
			void ToggleWireframe();
			void ZoomAll();
			void PlayAnimation(const TCHAR* name, SharedObj* target);
			void SetCamera(Camera *pCam) { m_wndCameraDlg.SetCamera(pCam); }

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CFrameworkView)
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			virtual ~DemoView();
		#ifdef _DEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif

			virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);	
			void		FindTerrain(Transformer* navroot, const TCHAR* targname);

		// Generated message map functions
		protected:
			//{{AFX_MSG(DemoView)
			afx_msg void OnDropFiles(HDROP hDropInfo);
			afx_msg void OnViewScenegraph();
			afx_msg void OnUpdateViewScenegraph(CCmdUI* pCmdUI);
			afx_msg void OnViewZoomall();
			afx_msg void OnViewFrameratecounter();
			afx_msg void OnUpdateViewFrameratecounter(CCmdUI* pCmdUI);
			afx_msg void OnCamdialog();
			afx_msg void OnViewWireframe();
			afx_msg void OnUpdateViewWireframe(CCmdUI* pCmdUI);
			afx_msg void OnDisableLighting();
			afx_msg void OnUpdateDisableLighting(CCmdUI* pCmdUI);
			afx_msg void OnScptOpen();
			afx_msg void OnScptRecord();
			afx_msg void OnScptPlay();
			afx_msg void OnScptStop();
			afx_msg void OnUpdateScptRecord(CCmdUI* pCmdUI);
			afx_msg void OnScptReplay();
			afx_msg void OnUpdateScptReplay(CCmdUI* pCmdUI);
			afx_msg void OnUpdateScptStop(CCmdUI* pCmdUI);
			afx_msg void OnScptReverse();
			afx_msg void OnUpdateScptReverse(CCmdUI* pCmdUI);
			afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnDestroy();
			afx_msg void OnNavArcball();
			afx_msg void OnNavFlyer();
			afx_msg void OnNavDriver();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC
}	// end Vixen

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

