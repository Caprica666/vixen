#include "vcore/vcore.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif
namespace Vixen {

namespace Core {


VX_IMPLEMENT_CLASS(ThreadAllocator, Allocator);

#ifdef _DEBUG
void EnableMemoryDebug()
{
	int tmpDbgFlag;
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	tmpDbgFlag |= _CRTDBG_ALLOC_MEM_DF;
	tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpDbgFlag |= _CRTDBG_CHECK_CRT_DF;
	tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
}

void DisableMemoryDebug()
{
	_CrtSetDbgFlag(0);
}
#endif


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
	m_heap = HeapCreate (HEAP_NO_SERIALIZE, initialSize, 0);
	VX_ASSERT (m_heap != NULL);
}

ThreadAllocator::~ThreadAllocator()
{
	VX_TRACE(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Delete(%d) %d", m_threadID));
	HeapDestroy (m_heap);
	m_heap = NULL;
}

void ThreadAllocator::SetOptions (int opts)
{
	m_heapOpts = HEAP_NO_SERIALIZE;
	if (opts & ALLOC_ZeroMem)
		m_heapOpts |= HEAP_ZERO_MEMORY;
	Allocator::SetOptions(opts);
}


void* ThreadAllocator::Alloc (size_t amount)
{
#ifdef _DEBUG
	UpdateDebugStats(amount);
#endif
	VX_TRACE2(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Alloc(%d) %d", amount, m_threadID));
	return HeapAlloc(m_heap, m_heapOpts, amount);
}


void ThreadAllocator::Free (void* ptr)
{
#ifdef _DEBUG
	intptr amount = HeapSize(m_heap, HEAP_NO_SERIALIZE, ptr);
	UpdateDebugStats(-amount);
	VX_TRACE2(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Free(%d) %d", amount, m_threadID));
#endif
	HeapFree(m_heap, HEAP_NO_SERIALIZE, ptr);
}

}	// end Core
}	// end Vixen