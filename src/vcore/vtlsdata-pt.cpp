
#include "vcore/vcore.h"

//============================================================
// VTlsData class
//============================================================
namespace Vixen {

namespace Core {

int		TLSData::Debug = 0;

TLSData::TLSData()
	: ThreadAllocator (DEFAULT_TLS_HEAP_SIZE),
	FastStack(),
	EventAllocator(NULL),
#ifndef VX_NOTHREAD
	m_lockPool	  (),
#endif
	ThreadID(0)
{
	FastStack.Push(&ThreadAllocator);
#ifndef VX_NOTHREAD
	m_lockPool.SetBlockAllocator(ThreadAllocator::Get());
#endif
	VX_TRACE(Debug, ("TLSData::Create(%d)", ThreadID));
}

}	// end Core
}	// end Vixen
