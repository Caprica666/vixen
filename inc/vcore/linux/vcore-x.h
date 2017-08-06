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
 *	_WIN32			enable compilation for Windows platform
 *	X11_LINUX		enable compilator for X11 under Linux
 *	VX_NOTHREAD		disable multi-threading and locking
 *
 */
#pragma once

#define VX_PTHREAD

#ifdef VX_NOTHREAD
#define	THREAD_LOCAL	static
#else
#define	THREAD_LOCAL	static __declspec(thread)
#endif

// platform-specific confing, mappings, and macro covers
#pragma warning(disable : 64) // Does not declare anything
#pragma warning(disable : 181)
#pragma warning(disable : 457)
#pragma warning(disable : 1125)
#pragma warning(disable : 1224)
#pragma warning(disable : 1595)
#include "vcore/linux/linuxport.h"

namespace Vixen {

#ifndef VX_MaxString
#define	VX_MaxString	8192
#endif

#define	VX_MaxLine		1024
#define VX_MaxName		512

#include "vcore/vobj_macros.h"
#include "vcore/vdebug.h" 		// base headers, in order of appearace
#include "vcore/vobj.h"
#include "vcore/valloc.h"
#include "vcore/vtlsdata.h"
#include "vcore/vstringpool.h"
#include "vcore/linux/vlock-x.h"
#include "vcore/vrefptr.h"
#include "vcore/vrefptr.inl"
#include "vcore/vref.h"		// class Ref in global namespace
#include "vcore/vref.inl"
#include "vcore/vstring.h"
#include "vcore/vobj.inl"	// after vobj/valloc sequence
#include "vcore/varray.h"
#include "vcore/varray.inl"
#include "vcore/vtree.h"
#include "vcore/vtree.inl"
#include "vcore/vdict.h"
#include "vcore/vdict.inl"
#include "vcore/vlock.h"
#include "vcore/vstream.h"
#include "vcore/vfile.h"
#include "vcore/vsocket.h"

#include "pthread.h"
#include "vcore/thread_pt.h"
namespace Core
{
	typedef FileStream NetStream;
}
#endif

#include "vcore/vthread.h"
#include "vcore/vpool.h"
#include "vcore/vbufq.h"

extern DebugOut& vixen_debug;
} // end Vixen

#include "vcore/valloc_oper.h"