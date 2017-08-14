#include "vcore/vcore.h"

#ifdef __APPLE__
#include <malloc/malloc.h>  // OSX
#endif

//============================================================
// global operator new and delete
//============================================================

#ifdef _DEBUG
namespace Vixen {
namespace Core {
	
//============================================================
// ThreadAllocator class
//============================================================

Allocator* ThreadAllocator::Get()
{
    TLSData* tls = TLSData::Get();
    return &(tls->ThreadAlloc);
}

ThreadAllocator::ThreadAllocator (int initialSize, intptr threadid)
:	Allocator(), m_threadID(threadid)
{
    SetOptions(ALLOC_ZeroMem);
    VX_TRACE(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Create(%d) %d", m_threadID, initialSize));
//    m_heap = HeapCreate (HEAP_NO_SERIALIZE, initialSize, 0);
//    VX_ASSERT (m_heap != NULL);
}

ThreadAllocator::~ThreadAllocator()
{
//    VX_TRACE(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Delete(%d) %d", m_threadID));
//    HeapDestroy (m_heap);
    m_heap = NULL;
    
    VX_PRINTF (("~ThreadAllocator@%-4d", m_threadID));
}

void ThreadAllocator::SetOptions (int opts)
{
//    m_heapOpts = HEAP_NO_SERIALIZE;
//    if (opts & ALLOC_ZeroMem)
//        m_heapOpts |= HEAP_ZERO_MEMORY;
    Allocator::SetOptions(opts);
}


void* ThreadAllocator::Alloc (size_t amount)
{
    UpdateDebugStats(amount);

//    VX_TRACE2(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Alloc(%d) %d", amount, m_threadID));
//    return HeapAlloc(m_heap, m_heapOpts, amount);
    return NULL;
}


void ThreadAllocator::Free (void* ptr)
{
#ifdef _DEBUG
//    intptr amount = HeapSize(m_heap, HEAP_NO_SERIALIZE, ptr);
//    UpdateDebugStats(-amount);
//    VX_TRACE2(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Free(%d) %d", amount, m_threadID));
#endif
//    HeapFree(m_heap, HEAP_NO_SERIALIZE, ptr);
}
    
}	// end Core
}	// end Vixen
