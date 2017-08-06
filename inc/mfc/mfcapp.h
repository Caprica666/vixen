#pragma once
#include "stdafx.h"
#include <afxinet.h>

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

namespace Vixen
{
	namespace MFC
	{
		/*!
		 * @class App
		 * @brief Subclass of CWinApp which initializes MFC for use with the scene manager.
		 *
		 * The  InitInstance function is overidden to parse scene manager
		 * arguments, set up the document template for use with Vixen,
		 * initialize the application and support asynchronous loading
		 * of scene manager content files.
		 *
		 * MFC::App works with companion classes MFC::Demo, MFC::View, MFC::Frame
		 * and MFC::Doc to let you:
		 * @code
		 *	route keyboard events to the scene manager
		 *	route mouse events to the scene manager
		 *	support full screen operation
		 *	permit navigator selection
		 *	open .vix files using MFC open dialogs
		 *	add scene manager menus and dialogs to your application
		 * @endcode
		 *
		 * @ingroup vixenmfc
		 * @see View Frame DemoView Demo
		 */
		class App : public CWinApp
		{
		public:
			App() : CWinApp() { };

			//! Called when 3D scene is loaded asynchronously.
			virtual void		OnSetScene(WPARAM hwnd, LPARAM val);
			static const UINT	s_SceneChange;

			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(App)
			public:
			virtual BOOL InitInstance();	//!< Set up document template, initialize 3D application.
			virtual int ExitInstance();		//!< Shutdown 3D processing
			//}}AFX_VIRTUAL

			//{{AFX_MSG(App)
			virtual afx_msg void OnAppAbout();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		class DemoView;
		extern void MakeWorld();

		/*!
		 * @class Demo
		 * @brief directs MFC windows events to the 3D framework.
		 *
		 * Connects the MFC windows message pump to a scene manager application,
		 * allowing display of 3D graphics in the client window of
		 * a single document application.
		 *
		 * Demo works with companion classes MFC::App, MFC::View, MFC::Frame
		 * and MFC::Doc to let you:
		 * @code
		 *	route keyboard events to the scene manager
		 *	route mouse events to the scene manager
		 *	support full screen operation
		 *	permit navigator selection
		 *	open .vix files using MFC open dialogs
		 *	add scene manager menus and dialogs to your application
		 * @endcode
		 *
		 * Demo is intended to encapsulate only scene manager specific application
		 * code. MFC calls should not be made from Demo if possible so that
		 * your demo may be easily ported to other platforms.
		 *
		 * @see Viewer World3D DemoView
		 */
		class Demo : public Viewer<World3D>
		{
			friend class App;
		public:
			Demo();

			//! Enable 3D UI in your application.
#ifdef VX_USE_SCRIPTOR
			Animator*		GetAnimator(const TCHAR* name = NULL) const;
#endif
			Skeleton*		GetSkeleton() const				{ return m_Skeleton; }
			void			SetSkeleton(Skeleton* skel)		{ m_Skeleton = skel; }
			void			LoadSceneMenus(const TCHAR* menuswanted = NULL);
			const TCHAR*	GetSceneMenus() const			{ return m_MenuNames; }
			void			SetOptions(bool bCreateLight)	{ m_bCreateLight = bCreateLight; }
			void			SetStatusText(const TCHAR* txt)	{ m_StatusText = txt; }
			const TCHAR*	GetStatusText() const			{ return m_StatusText; }
			static Demo*	Get()							{ return (Demo*) World3D::Get(); }

			// overrides
			virtual void	Stop();	
			virtual	void	OnExit();
			virtual bool	OnEvent(Event*);
			virtual bool	ParseArgs(const TCHAR* cmdline)	{ return Viewer<World3D>::ParseArgs(cmdline); }
			virtual void	SetScene(Scene*, Window = NULL);

		protected:
			virtual bool	ParseArgs(int argc, TCHAR **argv);
			virtual Engine*	MakeEngines();

			DemoView*		m_pDemoView;
			Core::String	m_StatusText;
			const TCHAR*	m_MenuNames;
			int				m_iInitialViewpoint;
			intptr			m_ThreadID;
			bool			m_bCreateLight;
			Ref<Animator>	m_Animator;
			Ref<Skeleton>	m_Skeleton;
		};
	}	// end MFC

	extern void MakeWorld();
	#define	VIXEN_MAKEWORLD(wclass) namespace Vixen { void MakeWorld() { World::Startup(); wclass* world = new wclass(); world->IncUse(); } }

}	// end Vixen

