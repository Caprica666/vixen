#pragma once

#ifndef __ICL
#pragma managed(push, off)
#endif

/*!
 * @file vxmfc.h
 * @brief Classes for using Vixen with MFC.
 *
 * The Visual Experience Engine MFC Framework connects the scene manager to MFC,
 * allowing a single document application to display 3D graphics in a client view.
 * To support 3D viewing under MFC, the following MFC classes are subclassed in
 * this framework:
 *
 * @code
 * Vixen			MFC			Feature
 * MFC::Frame		CFrameWnd	full screen support
 * MFC::Doc			CDocument	open .VIX files from dialogs
 * MFC::App			CWinApp		scene manager startup and argument parsing
 * MFC::View		CView		3D display and event linkage
 * MFC::DemoView	View		3D Viewer user interface
 * MFC::Demo		World3D		3D Viewer user interface
 * @endcode
 *
 * To make a 3D application, you need only subclass MFC::Demo to add
 * your 3D functionality.
 *
 * The MFC support classes provide support for these features in your application: 
 *	- route mouse and keyboard events to scene manager engines
 *	- full screen 3D operation
 *	- 3 degrees of freedom navigation
 *	- open scene manager (.VIX) files from system dialogs
 *	- add scene manager menus to your application
 *
 * The @htmlonly <A><HREF="oview/viewer_help.htm">3D Viewer</A> @endhtmlonly
 * application is built from the capabilities in this framework
 * Using these classes, you can add the menus and dialogs from the viewer to your application.
 * To use the scene manager with MFC, include vixenmfc.h instead of vixen.h and link with vixenmfc.lib
 *
 * @ingroup vixenmfc
 * @author Nola Donato
 * @author Ben Discoe (inspired by his 3D Viewer framework)
 */
#include "mfc/stdafx.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "vixen.h"
#include "vxutil.h"
#include "util/vxviewerapp.h"

#include "mfc/framectr.h"
#include "mfc/mfcapp.h"
#include "mfc/mfcframe.h"
#include "mfc/mfcview.h"
#include "mfc/mfcdoc.h"
#include "mfc/afresource.h"
#include "mfc/demoview.h"
#include "mfc/afresource.h"       // main symbols

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#ifndef __ICL
#pragma managed(pop)
#endif