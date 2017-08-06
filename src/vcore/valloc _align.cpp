#include "vcore/vcore.h"
#include <malloc.h>

namespace Vixen {
namespace Core {


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