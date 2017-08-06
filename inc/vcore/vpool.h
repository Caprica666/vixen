#pragma once

/*!
 * @file vpool.h
 * @brief Defines thread-safe pooled memory allocators.
 *
 * @author Nola Donato
 *
 * @ingroup vcore
 * @see vbufq.h valloc.h
 */

namespace Core {
#ifdef VX_NOTHREAD

	typedef FixedLenAllocator BytePool;

#else
/*!
 * @class BytePool
 * @brief Locking allocator which returns memory blocks of fixed sizes.
 *
 * If locking is enabled for this allocators (ALLOC_Lock set),
 * multiple threads may use the same allocator safely. If your
 * application uses one thread to allocate but multiple threads may free,
 * you can enable deferred free (ALLOC_FreeLater). This causes Free
 * calls from foriegn threads to put free blocks on a thread-safe
 * free list. Blocks on this list can be garbage collected for real later
 * by the thread which owns the allocator. This mode of operation
 * uses much cheaper locking than ALLOC_Lock. 
 *
 * You can only allocate memory for objects which are derived from BaseObj
 * and all allocation requests must be for the same amount.
 * A pointer to this allocator is stored in the BaseObj header.
 *
 * @ingroup vcore
 * @see BaseObj FixedLenAllocator Allocator BaseObj::SetClassAllocator Allocator::SetOptions
 */
class BytePool : public FixedLenAllocator
{
	VX_DECLARE_CLASS(BytePool);
public:
	BytePool(size_t esize = 16, size_t bsize = 8192) : FixedLenAllocator(esize, bsize) { }
	~BytePool()	{ FreeAll(); }

	void*		Alloc(size_t amount);
	void*		Grow(void* p, size_t amount);
	void		ReallyFree();
	void		FreeAll();
	void		Free(void* ptr);
protected:
	CritSec		m_Lock;	
	void*		m_FreeLater;		// blocks from foreigners to be freed
};

inline void* BytePool::Grow(void* p, size_t nbytes)
{
	if (IsLocking())
	{
		Core::Lock	lock(m_Lock);
		return FixedLenAllocator::Grow(p, nbytes);
	}
	return FixedLenAllocator::Grow(p, nbytes);
}

inline void BytePool::FreeAll()
{
	Core::Lock	lock(m_Lock);
	ReallyFree();
	FixedLenAllocator::FreeAll();
}
#endif


/*!
 * @class Pool
 * @param OBJ class for pool objects
 *
 * @brief Pool of homogeneous objects of that grows dynamically.
 *
 * Items are allocated from separate chunks of contiguous
 * memory that is grabbed as needed. When an item is freed, it
 * is added to a free list. Memory can only be completely freed
 * when the pool is no longer in use (all items are free).
 *
 * Class OBJ must have a special operator new to support
 * being used in a pool:
 * @code
 *	T::operator new(size_t size, void* addr)
 *		{ return addr; }
 * @endcode
 *
 * @ingroup vcore
 * @see FixedLenAllocator VBufferPool
 */
template <class OBJ> class Pool : public BytePool
{
public:
	Pool(int chunksize = 4096, int objsize = sizeof(OBJ))
	: BytePool(objsize, chunksize) { }
};

} // end Core