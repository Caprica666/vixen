
#include "vcore/vcore.h"
#
//============================================================
// VTlsData class
//============================================================

namespace Vixen {
namespace Core {

    
// int		TLSData::Debug = 0;
	
#ifdef VX_NOTHREAD
TLSData				 TLSData::t_TLSData;
#else
TLSData thread_local TLSData::t_TLSData;
#endif
	
TLSData::TLSData()
	: ThreadAlloc (DEFAULT_TLS_HEAP_SIZE),
	FastStack(),
	EventAlloc(NULL),
#ifndef VX_NOTHREAD
//	m_lockPool	  (sizeof(CritSec)),			// !!!!!
#endif
	ThreadID(0)
{
    FastStack.Push(ThreadAllocator::Get());
#ifndef VX_NOTHREAD
//	m_lockPool.SetBlockAllocator(ThreadAllocator::Get());		// !!!!!!!!
#endif
//	VX_TRACE(Debug, ("TLSData::Create(%d)", ThreadID));
}

}	// end Core
}	// end Vixen
