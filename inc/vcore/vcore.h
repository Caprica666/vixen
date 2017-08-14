/*!
 * @defgroup vcore Vixen Core Layer
 *
 * The Vixen Core Layer is a set of classes to provide an operating system
 * independent base for Visual Experience Engine projects. It includes:
 *
 * - A common base object with for run-time type checking and
 *	memory allocation control. 
 * - smart-pointer to encapsulate a reference-count based garbage collector.
 * - memory management infrastructure
 * - customizable collection classes for arrays, lists, hierarchies and dictionaries
 * - stream classes for file and socket I/O
 * - thread support including locking, thread specific storage and thread pools
 *
 * @par Compile Options
 *	_WIN32				enable compilation for Windows platform
 *	VIXEN_EMSCRIPTEN	enable compilation with Emscripten
 *	X11_LINUX			enable compilator for X11 under Linux
 *	VX_NOTHREAD			disable multi-threading and locking
 *
 */
#pragma once

// platform-specific confing, mappings, and macro covers
#ifdef _WIN32
	#include "vcore/win32/vcore-win.h"

#else
	#include "vcore/linux/vcore-x.h"
#endif

