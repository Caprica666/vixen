
/*!
 * The Visual Experience Engine wxWindows Framework connects the scene manager to
 * the platform independent wxWindows library, allowing a single document application
 * to display 3D graphics in a client view.
 *
 * To support 3D viewing under wxWindows, the following classes are subclassed in
 * this framework:
 *
 * @code
 * Vixen		wxWindows	Feature
 * WXW::Frame	wxFrame		full screen support
 * WXW::App		wxApp		scene manager startup and argument parsing
 * WXW::View	wxCanvas	3D display and event linkage
 * @endcode
 *
 * To make a 3D application, you need only subclass Viewer<World3D> to add
 * your own 3D functionality and use the VIXEN_MAKEWORLD macro to declare it.
 *
 * The wxWindows support classes provide support for these features in your application:
 *	- route mouse and keyboard events to scene manager engines
 *	- full screen 3D operation
 *	- 3 degrees of freedom navigation
 *	- open scene manager (.VIX) files from system dialogs
 *
 *
 * @author Nola Donato
 * @author Danny Chen
 */
#pragma once

#ifdef _WIN32
#include <windows.h>
#include "wx/wxprec.h"
#include "wx/object.h"

#else



#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	64
#endif
#ifndef _LARGE_FILES
#define _LARGE_FILES
#endif
#ifndef __WXX11__
#define __WXX11__
#endif
#ifndef __WXUNIVERSAL__
#define __WXUNIVERSAL__
#endif

#include "wx/wx.h"

#endif

#include "wxwin/wxapp.h"
#include "wxwin/wxview.h"

#include "vixen.h"
#include "vxutil.h"
#include "util/vxviewerapp.h"

