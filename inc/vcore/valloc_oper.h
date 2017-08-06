#pragma once

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
inline void* _cdecl operator new(size_t size, Vixen::Core::Allocator* pAlloc)
{
	// Use allocator if present, otherwise, straight allocation
	if (pAlloc)
		return pAlloc->Alloc (size);
	else
		return calloc (1, size);
}

//-----------------------------------------------
// Don't expect this to work!  For compiler only!.
// ...Delete your objects this way!
//		pObj->~Delete();
//-----------------------------------------------
inline void _cdecl operator delete( void *p, Vixen::Core::Allocator* pAlloc)
{
	// Handle delete of null pointer
	if (p)
	{
		// Use allocator if present, otherwise, straight delete operation
		if (pAlloc)
			pAlloc->Free (p);
		else
			free (p);
	}
}
