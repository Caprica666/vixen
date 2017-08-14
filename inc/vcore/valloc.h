
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

/*!
 * @brief Internal structure that chains together allocated memory blocks.
 * @ingroup vcore
 * @internal
 */
//===========================================================================
struct Chain
//===========================================================================
{
	Chain*			pNext;
	Allocator*		pAlloc;
	size_t			nLength;

	void*			data();

	// quasi-constructor/destructor
	static Chain*	Create(Allocator* pAlloc, Chain*& head, size_t nMax, size_t cbElement);
	void			FreeDataChain(Chain* skipMe = NULL);
	void			VisitAllElements(size_t elemsize, void VisitFunc(void*));

	// Debug checks ensure proper recycling during Free()
#ifdef _DEBUG
	bool			ContainsPtr(void* ptr);
	size_t			TotalSize();
	int				NumBlocks();
#endif
};

// inline methods
inline void*	Chain::data()	{ return this + 1; }

#define	ALLOC_ZeroMem	1
#define	ALLOC_Lock		2
#define	ALLOC_FreeLater	4
// #define	ALLOC_DefaultAlign	8L

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
//===========================================================================
class Allocator : public BaseObj
//===========================================================================
{
	VX_DECLARE_CLASS(Allocator);
    virtual ~Allocator();

public:
//! Allocate \b amount bytes of memory.
	virtual	void*	Alloc(size_t amount)            { VX_ASSERT(false); return NULL; /* = 0 pure virtual */ }
//! Return memory used by input object to the heap.
	virtual void	Free(void* ptr)                 { VX_ASSERT(false); /* = 0 pure virtual */ }
//! Returns the allocator used to grab heap blocks.
    Allocator*		GetBlockAllocator() const       { return m_BlockAlloc; }
//! Sets the allocator used to grab heap blocks.
	virtual void	SetBlockAllocator (Allocator* pAlloc);
//! Gets the allocator options.
	int				GetOptions() const              { return m_Options; }
//! Sets the allocation options.
	virtual void	SetOptions(int o)               { m_Options = o; }
//! Return \b true if locking is enabled for this allocator.
	bool			IsLocking() const               { return (m_Options & ALLOC_Lock) != 0; }

#ifdef _DEBUG
//! Print debugging statistic information
	void            PrintDebugStats();
	void            ResetDebugStats();
	void			UpdateDebugStats(long amount, void* ptr = NULL);
#endif

protected:
//! Constructor is private, must subclass this one.
	Allocator();

//! Align memory to integer boundary
	size_t          AlignMem (size_t address);
    void            Alignment (int align);

// heap block allocator
	Allocator*      m_BlockAlloc;
    size_t          m_AlignmentMask;
	int             m_Options;

// Debug-only metrics tracking
#ifdef _DEBUG
protected:
	long            m_currentSize;
	long            m_totalSize;
	long            m_maxSize;
	long			m_totalAllocs;
	long			m_remainingFrees;
	
	long			m_blockAllocs;
	long			m_blockFrees;
	char			m_lastWords[32];
#endif
};

// Make these functions go away in non-debug build, eliminates the need for extraneous #ifdef _DEBUG in the code
#ifndef _DEBUG
#define PrintDebugStats()
#define ResetDebugStats()
#define UpdateDebugStats(x)
#define UpdateDebugStats(x,p)
#endif
    
inline Allocator::~Allocator()
{
    PrintDebugStats();
}
    
inline void Allocator::SetBlockAllocator(Allocator* a)
{
	m_BlockAlloc = a;
}

// inline methods
inline size_t Allocator::AlignMem (size_t address)	{ return ((address + m_AlignmentMask) & ~m_AlignmentMask); }



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
//===========================================================================
class GlobalAllocator : public Allocator
//===========================================================================
{
	VX_DECLARE_CLASS(GlobalAllocator);
public:
	GlobalAllocator();
    ~GlobalAllocator();

	// overrides
	virtual	void*       Alloc(size_t amount);
	virtual void        Free(void* ptr);
	size_t				SizeOfPtr(void* ptr);
	
	static GlobalAllocator*	Get()	{ return s_ptheOneAndOnly; }

protected:
	static GlobalAllocator* s_ptheOneAndOnly;
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
//===========================================================================
class ThreadAllocator : public Allocator
//===========================================================================
{
	VX_DECLARE_CLASS(ThreadAllocator);
public:
//! Make thread heap of given size
	ThreadAllocator (int initialSize = DEFAULT_TLS_HEAP_SIZE, intptr threadid = 0);
//! Destroy heap resources
    ~ThreadAllocator();

	// overrides
	virtual	void*       Alloc(size_t amount);
	virtual void        Free(void* ptr);
	virtual void        SetOptions(int);

	static Allocator*	Get();	//!< Gets the thread allocator for the current thread.

protected:
	intptr              m_threadID;
	HANDLE              m_heap;
	DWORD               m_heapOpts;
};

#ifdef _DEBUG
extern void EnableMemoryDebug();
extern void DisableMemoryDebug();
#endif


//===========================================================================
class FastContext
//===========================================================================
{
public:
	FastContext (size_t initialSize = 8192, size_t incrementSize = 4096);
	~FastContext();
	
	operator FastAllocator*() const;
};
	
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
//===========================================================================
class FastAllocator : public Allocator
//===========================================================================
{
	VX_DECLARE_CLASS(FastAllocator);
public:
	// Constructor/destructor
	FastAllocator (size_t initialSize = 8192, size_t incrementSize = 4096);
	FastAllocator (void* buffer, size_t initialSize, bool deleteBlock = true, size_t incrementSize = 2048);
    ~FastAllocator();

//! Reclaim storage area to be reused
	virtual	void        Empty();
	// overrides
	virtual void*       Alloc(size_t size);
	virtual void        Free(void* ptr);
	void				FreeAll();

//! Returns currently active fast allocator
	static FastAllocator* Get();

	// attributes
//!	Returns the initial size of heap.
	size_t              InitialSize() const;
//! Returns the amount heap will grow when exhausted.
	size_t				IncrementSize() const;
//! Sets the memory used for the initial heap.
	void				SetInitialBlock (void* buffer, size_t initialSize, bool deleteBlock = true);

protected:
	// Utility method to reset pointer/counter variables
	void				Init();

	// data members used to hold the pooled allocations
	size_t				m_bufUsed;
	size_t				m_bufSize;
	size_t				m_initialSize;
	size_t				m_incrementSize;
	struct Chain*		m_keepBlock;
	struct Chain*		m_blocks;
	struct Chain*		m_freeBlocks;
};

// inline methods
inline size_t FastAllocator::InitialSize() const			{ return m_initialSize; }
inline size_t FastAllocator::IncrementSize() const			{ return m_incrementSize; }
#ifdef _DEBUG
inline void	  FastAllocator::Free (void* ptr)				{ VX_ASSERT (m_blocks == NULL || m_blocks->ContainsPtr (ptr)); }
#else
inline void	  FastAllocator::Free (void* ptr)				{ /* do nothing, block chain will be destroyed in ~FastAllocator() */ }
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
//===========================================================================
class FixedLenAllocator : public Allocator
//===========================================================================
{
//	friend class TLSData;
	VX_DECLARE_CLASS(FixedLenAllocator);
public:
//! Creates an allocator for fixed length elements.
	FixedLenAllocator(size_t elementSize = 16, size_t blockSize = 16);
    ~FixedLenAllocator();

	// Overrides
	virtual void*       Alloc (size_t size);
    virtual void        Free (void* ptr);
	void				FreeAll();

//! Returns size of fixed elements allocated.
	size_t              ElementSize() const;
//! Returns size of heap blocks allocated.
	size_t              BlockSize() const;
	void                SetElementAndBlockSize (size_t eSize, size_t bSize);

protected:
	// Utility method to reset pointer/counter variables
	void                Init();

	// data members used to hold the pooled allocations
	size_t              m_elementSize;
	size_t              m_blockSize;
	struct Chain*       m_blocks;
	struct Chain*       m_freeList;
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

//===========================================================================
class PoolAllocator : public Allocator
//===========================================================================
{


VX_DECLARE_CLASS(PoolAllocator);
public:
	PoolAllocator (Allocator* blockalloc = NULL, size_t smallest = 32, size_t numpools = ALLOC_MaxPools, int options = 0);
    ~PoolAllocator();

	void*               Alloc (size_t amount);
	void                Free (void* p);
	void                SetBlockAllocator (Allocator* blockalloc);

protected:
	int                 m_NumPools;			// number of fixed length pools
	size_t              m_MaxSize;			// maximum size for pooled allocation
	FixedLenAllocator*	m_MemPool[ALLOC_MaxPools];
};


#ifdef _DEBUG
/*!
 * @class RogueAllocator
 *
 * @brief Rogue memory allocator that returns memory allocated from override of
 * operator new() and operator delete()
 *
 * It tracks alloc/free operations, and defers to the Global Allocator to do its work
 *
 * The rogue allocator is a singleton - only one may be created per application.
 * @ingroup vcore
 */
//===========================================================================
class RogueAllocator : public Allocator
//===========================================================================
{
	VX_DECLARE_CLASS(RogueAllocator);
public:
	RogueAllocator();
	~RogueAllocator();
	
	// overrides
	virtual	void*       Alloc(size_t amount)	{ GlobalAllocator* og = GlobalAllocator::Get(); UpdateDebugStats(amount); return og->Alloc(amount); }
	virtual void        Free(void* ptr)			{ GlobalAllocator* og = GlobalAllocator::Get(); int amount = og->SizeOfPtr(ptr); UpdateDebugStats(-amount); og->Free (ptr); }
	
	static RogueAllocator*	Get()				{ return s_ptheBeanCounter; }
	
protected:
	static RogueAllocator* s_ptheBeanCounter;
};
#endif
	

} // end Core
