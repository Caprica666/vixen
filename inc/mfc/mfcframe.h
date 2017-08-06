#pragma once

namespace Vixen
{
	namespace MFC
	{
	/*!
	 * @class Frame
	 * @brief Subclass of CFrameWnd that helps MFC applications
	 * co-exist peacefully with the scene manager.
	 *
	 * Full screen operation eliminates all evidence of MFC and Windows
	 * so that only the 3D client window is visible. To implement the full
	 * screen toggle capabilities, Vixen subclasses the main frame window
	 * (CFrameWnd) and overrides some of the underlying Windows message handlers.
	 * If you are using AppWizard, to get the full screen capabilities
	 * you need to derive your frame window class from ISMFrameWnd instead
	 * of CFrameWnd.
	 *
	 * To put a 3D application into full screen mode, you either hit ESCAPE
	 * or maximize the main window. Frame then hides the status bar,
	 * menu and other window adornments and sets the size of the main frame
	 * window so that the client window appears full screen. Keyboard and
	 * mouse events are still routed to the application via VXFCMView.
	 *
	 * @ingroup vixenmfc
	 * @see View App DemoView
	 */
		class Frame : public CFrameWnd
		{
		protected: // create from serialization only
			Frame();
			DECLARE_DYNCREATE(Frame)

		// Attributes
		public:

		// Operations
		public:

		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(Frame)
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			//}}AFX_VIRTUAL

		// Implementation
		public:
			virtual ~Frame();
		#ifdef _DEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif

		protected:  // control bar embedded members
			static Frame	*sm_Self;
			CStatusBar  m_wndStatusBar;
			LONG		m_normalStyle;
			DWORD		m_normalExStyle;
			CRect		m_normalSize;
			bool		m_FullScreen;

		public:
			void		ResizePower2(CWnd* view);	//! Resize to closest power of 2.
			void		SetFullScreen(bool enable);	//!< Enable / disable full screen mode.
			void		ToggleFullScreen()			//!< Roggle full screen mode.
						{ SetFullScreen(!m_FullScreen); }

		// Generated message map functions
		protected:
			//{{AFX_MSG(Frame)
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnMove(int x, int y);
			afx_msg void OnClose();
			afx_msg void OnSize( UINT nType, int cx, int cy );
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
	}	// end MFC

} // end Vixen
