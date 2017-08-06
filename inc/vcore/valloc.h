
#pragma once

namespace Core {

/*!
 * @file valloc.h
 * @brief Memory allocation classes.
 *
 * @ingroup vcore
 */

//! Default size of thread allocator heap
#define DEFAULT_TLS_HEAP_SIZE		(64 * 1024)

// forward declaration
struct Chain;
class Allocator;
class FastAllocator;
class GlobalAllocator;

// function pointers, callbacks
typedef Allocator* (*GetAllocatorFunc)();

/*!
 * @brief Internal structure that chains together allocated memory blocks.
 * @ingroup vcore
 * @internal
 */
struct Chain
{
	Chain*			pNext;
	Allocator*		pAlloc;
	size_t			nLength;

	void* data();

	// quasi-constructor/destructor
	static Chain*	Create(Allocator* pAlloc, Chain*& head, size_t nMax, size_t cbElement);
	void			FreeDataChain(Chain* skipMe = NULL);
	void			VisitAllElements(size_t elemsize, void VisitFunc(void*));
	// Debug checks ensure proper recycling during Free()
#ifdef _DEBUG
	bool			ContainsPtr(void* ptr);
#endif
};

// inline methods
inline void*	Chain::data()	{ return this + 1; }


/*!
 * @class TempAllocScope
 * @brief Internal exception-safe class to handle setting a fast allocation context.
 */
class TempAllocScope
{
public:
	TempAllocScope(FastAllocator* pAlloc);
	~TempAllocScope ();
};


#define	ALLOC_ZeroMem	1
#define	ALLOC_Lock		2
#define	ALLOC_FreeLater	4
#define	ALLOC_DefaultAlign	8L

/*!
 * @class Allocator
 *
 * @brief Memory allocation base class.
 *
 * Allocators provide a powerful framework for memory management
 * by allowing you to maintain separate pools of memory for different
 * classes. The \b global allocator gets its memory from the system
 * global heap and performs locking around allocation and free operations
 * for thread-safe access. Other allocators are built on top of
 * the global one that can grab large heap blocks and manage them
 * for you to cut down on locking overhead. For example, a \b thread
 * allocator does no locking on individual requests and returns its
 * global heap blocks when the thread exits. A \b fast allocator
 * never frees anything until the allocator itself is deleted.
 *
 * Each class or object can be given its own allocator. You can also
 * make your own allocators for application-specific memory management.
 *
 * The base class exists to establish the allocator interfaces.
 * This class cannot be instantiated - it must be subclassed.
 *
 * @ingroup vcore
 * @see GlobalAllocator ThreadAllocator Class::SetAllocator BaseObj::operator new
 */
class Allocator : public BaseObj
{
	VX_DECLARE_CLASS(Allocator);
	virtual ~Allocator()	{ }

public:
//! Allocate \b amount bytes of memory.
	virtual	void*	Alloc(size_t amount)	{ VX_ASSERT(false); return NULL; }
//! Enlarge already allocated area to \b  bytes.
	virtual	void*	Grow(void* ptr, size_t amount)	{ VX_ASSERT(false); return NULL; }
//! Return memory used by input object to the heap.
	virtual void	Free(void* ptr)		{ VX_ASSERT(false); }
//! Free all system memory used by this allocator.
	virtual void	FreeAll();
//! Returns the allocator used to grab heap blocks.
	Allocator*		GetBlockAllocator() const;
//! Sets the allocator used to grab heap blocks.
	virtual void	SetBlockAllocator (Allocator* pAlloc);
//! Gets the allocator options.
	int				GetOptions() const	{ return m_Options; }
//! Sets the allocation options.
	virtual void	SetOptions(int o)	{ m_Options = o; }
//! Return \b true if locking is enabled for this allocator.
	bool			IsLocking() const	{ return (m_Options & ALLOC_Lock) != 0; }

	bool	DoLock;
#ifdef _DEBUG
//! Print debugging statistic information
	virtual void	PrintDebugStats();
	virtual void	ResetDebugStats();
#endif

protected:
//! Constructor is private, must subclass this one.
	Allocator();

//! Align memory to integer boundary
	size_t		AlignMem (size_t address);

// heap block allocator
	Allocator*	m_BlockAlloc;
	int			m_Options;

// Debug-only metrics tracking
#ifdef _DEBUG

protected:
	void			UpdateDebugStats(intptr amount);

	size_t			m_currentSize;
	size_t			m_totalSize;
	size_t			m_maxSize;
	int				m_totalAllocs;
	int				m_remainingFrees;
#endif
};

inline void Allocator::SetBlockAllocator(Allocator* a)
{
	m_BlockAlloc = a;
}

inline Allocator*	Allocator::GetBlockAllocator() const
{
	return m_BlockAlloc;
}




// inline methods
inline size_t Allocator::AlignMem (size_t address)	{ return ((address + 3L) & ~3L); }



/*!
 * @class GlobalAllocator
 *
 * @brief Global memory allocator that returns memory allocated from global heap.
 *
 * It is the default allocator for all classes unless otherwise specified.
 * This is not the most efficient allocator because it locks around all
 * allocation and free operations to provide thread safety.
 * The global allocator is a singleton - only one may be created per application.
 * @ingroup vcore
 */
class GlobalAllocator : public Allocator
{
	friend bool _cdecl CoreInit();
	friend void _cdecl CoreExit();
	VX_DECLARE_CLASS(GlobalAllocator);
public:
	GlobalAllocator(int unused = 0);

	// overrides
	virtual	void*	Alloc(size_t amount);
	virtual void*	Grow(void* ptr, size_t amount);
	virtual void	Free(void* ptr);
	virtual	void	FreeAll();

	static Allocator*	Get()	//!< Get the one and only global allocator
	{ return s_ptheOneAndOnly; }

protected:
	static GlobalAllocator* s_ptheOneAndOnly;
};

// inline methods
inline void GlobalAllocator::FreeAll()			{}

#ifdef _WIN32
/*!
 * @class AlignedAllocator
 *
 * @brief Global memory allocator that returns memory allocated from global heap
 * aligned to a specific power of 2 boundary.
 *
 * This is not the most efficient allocator because it locks around all
 * allocation and free operations to provide thread safety. It shares
 * the same heap as the global allocator.
 * @ingroup vcore
 */
class AlignedAllocator : public Allocator
{
	friend bool _cdecl CoreInit();
	friend void _cdecl CoreExit();
	VX_DECLARE_CLASS(AlignedAllocator);

public:
	AlignedAllocator(int align = ALLOC_DefaultAlign);

	// overrides
	virtual	void*	Alloc(size_t amount);
	virtual void*	Grow(void* ptr, size_t amount);
	virtual void	Free(void* ptr);
	virtual	void	FreeAll()	{ };
protected:
	size_t			m_Align;
};


/*!
 * @class ThreadAllocator
 *
 * @brief Allocates memory allocated from a thread-bound heap.
 *
 * The memory is reclaimed when the thread exits.
 * The thread allocator does no locking.
 * @ingroup vcore
 */
class ThreadAllocator : public Allocator
{
	VX_DECLARE_CLASS(ThreadAllocator);
public:
//! Make thread heap of given size
	ThreadAllocator (int initialSize = DEFAULT_TLS_HEAP_SIZE, intptr threadid = 0);
//! Destroy heap resources
	virtual ~ThreadAllocator();

	// overrides
	virtual	void*	Alloc(size_t amount);
	virtual void*	Grow(void* ptr, size_t amount);
	virtual void	Free(void* ptr);
	virtual void	FreeAll();
	virtual void	SetOptions(int);

	static Allocator*	Get();	//!< Gets the thread allocator for the current thread.

protected:
	intptr		m_threadID;
	HANDLE		m_heap;
	DWORD		m_heapOpts;
};

#ifdef _DEBUG
extern void EnableMemoryDebug();
extern void DisableMemoryDebug();
#endif

#else
typedef GlobalAllocator ThreadAllocator;
#endif // _WIN32


/*!
 * @class FastAllocator
 *
 * @brief Allocates variable length chunks contained in a larger memory block chain.
 *
 * The memory returned is suitable for any contained object type.
 * The fast allocator does no locking and cannot be used simultaneously
 * by multiple threads.
 * @ingroup vcore
 */
class FastAllocator : public Allocator
{
	VX_DECLARE_CLASS(FastAllocator);
public:
	// Constructor/destructor
	FastAllocator (size_t initialSize = 8192, size_t incrementSize = 4096);
	FastAllocator (void* buffer, size_t initialSize, bool deleteBlock = true, size_t incrementSize = 2048);
	virtual ~FastAllocator();

//! Reclaim storage area to be reused
	virtual	void	Empty();
	// overrides
	virtual void*	Alloc(size_t size);
	virtual void	Free(void* ptr);
	virtual void	FreeAll();

//! Returns currently active fast allocator
	static Allocator*		Get();

	// attributes
//!	Returns the initial size of heap.
	size_t					InitialSize() const;
//! Returns the amount heap will grow when exhausted.
	size_t					IncrementSize() const;
//! Sets the memory used for the initial heap.
	void					SetInitialBlock (void* buffer, size_t initialSize, bool deleteBlock = true);


protected:
	// Utility method to reset pointer/counter variables
	void					Init();

	// data members used to hold the pooled allocations
	size_t					m_bufUsed;
	size_t					m_bufSize;
	size_t					m_initialSize;
	size_t					m_incrementSize;
	struct Chain*			m_keepBlock;
	struct Chain*			m_blocks;
	struct Chain*			m_freeBlocks;
};

// inline methods
inline size_t FastAllocator::InitialSize() const			{ return m_initialSize; }
inline size_t FastAllocator::IncrementSize() const			{ return m_incrementSize; }
#ifdef _DEBUG
inline void	  FastAllocator::Free (void* ptr)				{ VX_ASSERT (m_blocks == NULL || m_blocks->ContainsPtr (ptr)); }
#else
inline void	  FastAllocator::Free (void* ptr)				{ /* do nothing */ }
#endif


/*! @class FixedLenAllocator
 *
 * @brief Allocates memory blocks of the same fixed size.
 *
 * Items are allocated from separate chunks of contiguous
 * memory that is grabbed as needed. When an item is freed, it
 * is added to a free list. Memory can only be completely freed
 * when the pool is no longer in use (all items are free).
 * This allocator does no locking and cannot be used simultaneously
 * by multiple threads.
 * @ingroup vcore
 */
class FixedLenAllocator : public Allocator
{
	friend class TLSData;
	VX_DECLARE_CLASS(FixedLenAllocator);
public:
//! Creates an allocator for fixed length elements.
	FixedLenAllocator(size_t elementSize = 16, size_t blockSize = 16);
	virtual ~FixedLenAllocator();

	// Overrides
	virtual void*	Alloc(size_t size);
	virtual void*	Grow(void* ptr, size_t size);
	virtual void	Free(void* ptr);
	virtual void	FreeAll ();

//! Returns size of fixed elements allocated.
	size_t			ElementSize() const;
//! Returns size of heap blocks allocated.
	size_t			BlockSize() const;
	void			SetElementAndBlockSize(size_t eSize, size_t bSize);

protected:
	// Utility method to reset pointer/counter variables
	void			Init();

	// data members used to hold the pooled allocations
	size_t			m_elementSize;
	size_t			m_blockSize;
	struct Chain*	m_blocks;
	struct Chain*	m_freeList;
};

// inline methods
inline size_t FixedLenAllocator::BlockSize() const		{ return m_blockSize; }
inline size_t FixedLenAllocator::ElementSize() const	{ return m_elementSize; }

/*!
 * @class PoolAllocator
 *
 * @brief Memory allocator that keeps a set of fixed size allocators to satisfy
 * requests for a small amounts of memory.
 *
 * Each fixed allocator has its own memory pool which allocates chunks of a specific size.
 * When an object is freed, it is put into a free list for its pool so that objects can be
 * garbage collected quickly and reused.
 *
 * The pool allocator minimizes the number of system calls made to allocate
 * memory for small objects. System memory chunks are not reclaimed until
 * the allocator is destroyed. However, system memory already allocated is reused.
 *
 * To allocate system memory or to satisfy allocation requests outside the size
 * range of the pools, the parent allocator is used (usually the global or thread
 * allocator).
 *
 * When a small object is allocated by the pool allocator, it is really be allocated
 * by one of the internal fixed length allocators. It is the allocator that really
 * allocated the memory which is associated with the object - not this allocator.
 * You can only allocate memory for objects which are derived from BaseObj. The
 * pointer to the real allocator is stored in the BaseObj header.
 *
 * Although this allocator does no locking, it can be made thread-safe by
 * initializing it with pools which can lock.
 *
 * @ingroup vcore
 * @see BaseObj FixedLenAllocator Allocator GlobalAllocator ThreadAllocator Class::SetAllocator
 */
#define	ALLOC_MaxPools	8

class PoolAllocator : public Allocator
{
	friend bool _cdecl CoreInit();
	friend void _cdecl CoreExit();
	VX_DECLARE_CLASS(PoolAllocator);
public:
	PoolAllocator(Allocator* blockalloc = NULL, int smallest = 128, int numpools = 6);
	~PoolAllocator()	{ FreeAll(); }

	void*		Alloc(size_t amount);
	void*		Grow(void* ptr, size_t size);
	void		Free(void* p);
	void		FreeAll();
	void		SetBlockAllocator(Allocator* blockalloc);
	bool		InitPools(FixedLenAllocator* firstpool, int numpools);

#ifdef _DEBUG
	void		PrintDebugStats();
	void		ResetDebugStats();
#endif

	static Allocator*		Get()	//!< Get the one and only global allocator
	{ return s_ptheOneAndOnly; }

protected:
	static PoolAllocator* s_ptheOneAndOnly;
	int		m_NumPools;			// number of fixed length pools
	size_t	m_MaxSize;			// maximum size for pooled allocation
	size_t	m_BaseSize;			// size of memory blocks in smallest pool
	FixedLenAllocator*	m_MemPool[ALLOC_MaxPools];
};

/*!
 * @fn PoolAllocator::PoolAllocator(Allocator* blockalloc, int npools)
 * @param blockalloc	Block allocator to use to allocate memory for pools.
 * @param smallest		Size of the smallest element pool. Others pools are multiples of this base size.
 * @param npools		Number of memory pools to use.
 *
 * A pool allocator creates several internal memory pools which
 * are each maintained by a fixed length allocator. The block
 * allocator supplied here is used to obtain the memory for these pools.
 * Blocks too large for the pools are allocated directly using the block allocator.
 *
 * @see ThreadAllocator GlobalAllocator Allocator::SetBlockAllocator
 */
inline PoolAllocator::PoolAllocator(Allocator* blockalloc, int smallest, int npools) : Allocator()
{
	if (blockalloc == NULL)
		blockalloc = GlobalAllocator::Get();
	m_BlockAlloc = blockalloc;
	m_NumPools = npools;
	m_BaseSize = smallest;
	m_MaxSize = 0;
	VX_ASSERT(npools > 0);
	VX_ASSERT(smallest > 0);
	for (int i = 0; i < npools; ++i)
		m_MemPool[i] = NULL;
}

/*!
 * @fn void PoolAllocator::FreeAll()
 *
 * Frees all of the memory in the memory pools. If objects in these
 * pools are still being used, the data in them may get corrupted.
 *
 * @see PoolAllocator::Free Allocator::FreeAll
 */
inline void PoolAllocator::FreeAll()
{
#ifdef _DEBUG
	PrintDebugStats();
#endif
	if (m_MemPool[0] == NULL)
		return;
	for (int i = 0; i < m_NumPools; ++i)
	{
		VX_TRACE(Allocator::Debug, ("Free pool %d\n", m_MemPool[i]->ElementSize()));
		delete m_MemPool[i];
		m_MemPool[i] = NULL;
	}
	m_MaxSize = 0;
}

} // end Core
