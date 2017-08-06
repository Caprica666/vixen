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
 *	VIXEN_EMSCRIPTEN	enable compilation for Emscripten (C++ -> Javascript in browser)
 *	X11_LINUX			enable compilator for X11 under Linux
 *	VX_NOTHREAD			disable multi-threading and locking
 *
 */
#pragma once

#include "vcore/ems/emsport.h"
#include "vcore/ems/vdebug.h"

#ifdef VX_NOTHREAD
#define	THREAD_LOCAL	static
#else
#define	THREAD_LOCAL	static __declspec(thread)
#endif

namespace Vixen {

#define	VX_MaxLine		1024
#define VX_MaxName		512

#include "vcore/vobj_macros.h"
#include "vcore/vobj.h"
#include "vcore/valloc.h"
#include "vcore/vtlsdata.h"
#include "vcore/linux/vlock-x.h"
#include "vcore/vrefptr.h"
#include "vcore/vrefptr.inl"
#include "vcore/vref.h"		// class Ref in global namespace
#include "vcore/vref.inl"

#include "vcore/vstring.h"
#include "vcore/vstringpool.h"
#include "vcore/vobj.inl"	// after vobj/valloc sequence
#include "vcore/varray.h"
#include "vcore/varray.inl"
#include "vcore/vtree.h"
#include "vcore/vtree.inl"
#include "vcore/vdict.h"
#include "vcore/vdict.inl"
#include "vcore/vlock.h"
#include "vcore/vstream.h"
#include "vcore/ems/stream-ems.h"
#include "vcore/vfile.h"

#ifdef VX_PTHREAD
	#include "pthread.h"
#endif
#include "vcore/thread_pt.h"
namespace Core
{
	typedef FileStream NetStream;
}

#include "vcore/vthread.h"
#include "vcore/vpool.h"
#include "vcore/vbufq.h"

extern DebugOut& vixen_debug;
} // end Vixen


//inline void* _cdecl operator new (size_t size, void* where) { return where; }

#include "vcore/valloc_oper.h"