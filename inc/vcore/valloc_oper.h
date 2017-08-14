#pragma once

#include "vcore/valloc.h"


/*!
 * @fn void* new(size_t size, Allocator* alloc)
 * @relates Allocator
 *
 * Allocates memory using the allocator provided.
 *
 * @param size		Number of bytes to allocate
 * @param pAlloc	Allocator to use
 *
 * @return Pointer to memory block allocated, NULL on error
 */

void* _cdecl operator new(size_t size, Vixen::Core::Allocator* pAlloc);
void  _cdecl operator delete( void *p, Vixen::Core::Allocator* pAlloc);

/*
 * In the debug version, we override the default C-style new/delete pair so as to be able to 
 * keep track of any allocations / dealloations that are done via code that is compiled into
 * the executable.  This information is tracked via the RogueAllocator (facade for GlobalAllocator).
 *
 * QUIRK: on mac, declaring this function as explicit inline avoids linker errors (hmmmmmmmm.....)
 */

#ifdef _DEBUG
inline void* _cdecl operator new (size_t size)		{ return operator new (size, Vixen::Core::RogueAllocator::Get()); }
inline void  _cdecl operator delete (void *p)		{ operator delete (p, Vixen::Core::RogueAllocator::Get()); }
#endif


