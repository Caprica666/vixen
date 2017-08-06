#pragma once

namespace Vixen
{
	namespace MFC
	{
	/*!
	 * @class View
	 * @brief Subclass of the MFC CView which allows a 3D scene to 
	 * be displayed in the view.
	 *
	 * Mouse and keyboard events from the view are mapped to the appropriate SharedObj::OnEvent calls.
	 * If keyboard or mouse engines are present, they will respond to events
	 * from the MFC view.
	 *
	 * View also refreshes the main status bar with the frame counter 
	 * that provides rendering statistics.
	 *
	 * View does not implement the 3D Viewer user interface.
	 * DemoView, a subclass of View, adds this functionality.
	 * Unless you are NOT using the 3D menus, you should use DemoView
	 * rather than View, to get your application to respond to
	 * 3D menus and dialogs. 
	 *
	 * The following MFC functions are overriden by View:
	 * @code
	 *		OnDraw			displays current 3D scene in window
	 *		OnSize			resets 3D viewport based on window size
	 *		OnEraseBkgnd	suppresses background erase
	 *		PreCreateWindow	uses special 3D window class
	 *		OnChar			passes key events to scene manager key engines
	 *		OnLButtonDown	passes mouse events to scene manager mouse engines
	 *		OnRButtonDown	
	 *		OnMButtonDown	
	 *		OnLButtonUp		
	 *		OnRButtonUp		
	 *		OnMButtonUp		
	 *		OnMouseMove		
	 * @endcode
	 *
	 * @ingroup vixenmfc
	 * @see App World3D SharedObj::OnEvent FrameCounter
	 */
	class View : public CView
	{
	public:
		View();
		~View();

		DECLARE_DYNCREATE(View)

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(View)
		public:
		virtual void OnDraw(CDC* pDC);  // overridden to draw this view
		protected:
		//}}AFX_VIRTUAL

	protected:
		Scene*	GetScene()	{ return World3D::Get()->GetScene((Window) GetSafeHwnd()); }

		//{{AFX_MSG(View)
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnDestroy();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnMove(int x, int y);
		afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnLButtonDblClick(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDblClick(UINT nFlags, CPoint point);
		afx_msg void OnMButtonDblClick(UINT nFlags, CPoint point);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	protected:
 		void		Resize();

		UINT_PTR	m_StatusTimer;
		CWnd*		m_ClientWnd;
	};
	}	// end MFC
} // end Vixen


