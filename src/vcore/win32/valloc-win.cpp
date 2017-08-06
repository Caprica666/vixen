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

void FixedLenAllocator::Free (void* aPtr)
{
	// Make sure that we are freeing a pointer which actually beloings to this allocator.
#ifdef _DEBUG
	VX_ASSERT (m_blocks && m_blocks->ContainsPtr (aPtr));
#endif
	// zero-init memory for next Alloc() call - equivalent to memset (pVChain, 0, m_elementSize);
	// Assembly code is optimized based on knowledge of the data - it is always aligned 
	// to a 4-byte boundary, and therefore can be zero-initialized a DWORD (4 bytes) a time.
	if (m_Options & ALLOC_ZeroMem)
	{
#ifdef _M_IX86
		_asm
		{
			mov		edi, dword ptr [aPtr]
			mov		ebx, dword ptr [this]
			mov		ecx, dword ptr [ebx].m_elementSize
			shr		ecx, 2
			xor		eax, eax
			rep	stos dword ptr [edi]
		}
#else
		memset(aPtr, 0, m_elementSize);
#endif
	}

	// Chain this back into the free list.  Note the unsafe cast to Chain pointer -
	// it is assumed that we are indeed passing a valid Chain pointer!
	Chain*	pVChain = (Chain*)aPtr;
	pVChain->pNext = m_freeList;
	m_freeList = pVChain;
	VX_ASSERT ((m_freeList == NULL) || (m_freeList->nLength >= 0));
}


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

void* ThreadAllocator::Grow (void* ptr, size_t amount)
{
#ifdef _DEBUG
	size_t growth = amount - HeapSize(m_heap, HEAP_NO_SERIALIZE, ptr);

	UpdateDebugStats(growth);
	VX_TRACE2(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::Grow(%d) %d", growth, m_threadID));
#endif

	return HeapReAlloc(m_heap, m_heapOpts, ptr, amount);
};

void ThreadAllocator::FreeAll()
{
	VX_TRACE(ThreadAllocator::Debug || Allocator::Debug, ("ThreadAllocator::FreeAll() %d", m_threadID));
	HeapCompact(m_heap, HEAP_NO_SERIALIZE);
}

VX_IMPLEMENT_CLASS(AlignedAllocator, Allocator);



AlignedAllocator::AlignedAllocator(int align)
{
	if (align < 4)
		align = ALLOC_DefaultAlign;
	m_Align = align;
	m_BlockAlloc = GlobalAllocator::Get();
}

void* AlignedAllocator::Alloc(size_t amount)
{
#ifdef _DEBUG
	UpdateDebugStats (amount);
#endif
	void* ptr = _aligned_malloc(amount, m_Align);

	if (m_Options & ALLOC_ZeroMem)
		memset(ptr, 0, amount);
	VX_TRACE2(GlobalAllocator::Debug, ("AlignedAllocator::Alloc(%d) %p", amount, ptr));
	return ptr;
}

void AlignedAllocator::Free(void* ptr)
{
#if defined(_DEBUG) && defined(_WIN32)
	intptr	amount = _aligned_msize(ptr, m_Align, 0);
	UpdateDebugStats(-amount);
	VX_TRACE2(GlobalAllocator::Debug, ("AlignedAllocator::Free(%d) %p", amount, ptr));
#else						// TODO: figure out what to do on Linux
	VX_TRACE2(GlobalAllocator::Debug, ("AlignedAllocator %p", ptr));
#endif
	_aligned_free(ptr);
}

void* AlignedAllocator::Grow(void* ptr, size_t newsize)
{
	#if defined(_DEBUG) && defined(_WIN32)
	size_t	amount = _aligned_msize(ptr, m_Align, 0);
	UpdateDebugStats (newsize - amount);
	VX_TRACE2(GlobalAllocator::Debug, ("AlignedAllocator::Free(%d) %p", amount, ptr));
#else 						// TODO: figure out what to do on Linux
	VX_TRACE2(GlobalAllocator::Debug, ("AlignedAllocator %p", ptr));
#endif
	return _aligned_realloc(ptr, newsize, m_Align);
}

}	// end Core
}	// end Vixen