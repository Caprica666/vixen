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
 *	VX_NOTHREAD		disable multi-threading and locking
 *
 */


// platform-specific confing, mappings, and macro covers
#include <windows.h>
#include <WinSock.h>
#include "vcore/win32/pcport.h"

namespace Vixen {

#ifndef VX_MaxString
#define	VX_MaxString	8192
#endif

#define	VX_MaxLine		1024
#define VX_MaxName		512

#include "vcore/vobj_macros.h"
#include "vcore/vdebug.h" 		// base headers, in order of appearace
#include "vcore/vobj.h"
#include "vcore/win32/vlock-win.h"
#include "vcore/valloc.h"
#include "vcore/vtlsdata.h"
#include "vcore/vstringpool.h"
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

#ifdef VX_PTHREAD
	#include "vcore/thread_pt.h"
#else
	#include "vcore/win32/thread-win.h"
#endif
#include "vcore/win32/stream-win.h"
#include "vcore/vthread.h"
#include "vcore/vpool.h"
#include "vcore/vbufq.h"

extern DebugOut& vixen_debug;
} // end Vixen

#include "vcore/valloc_oper.h"
#include "vcore/cast.h"