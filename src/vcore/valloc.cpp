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
	
VX_IMPLEMENT_CLASS(RogueAllocator, Allocator);

RogueAllocator* RogueAllocator::s_ptheBeanCounter = NULL;

RogueAllocator	g_daGodFather;

RogueAllocator::RogueAllocator()
  : Allocator()
{
	VX_ASSERT (s_ptheBeanCounter == NULL);
	s_ptheBeanCounter = this;
}

RogueAllocator::~RogueAllocator()
{
//	VX_PRINTF(("~RogueAllocator      "));
	VX_PRINTF(("operator new/delete  "));
	s_ptheBeanCounter = NULL;
}
	
}
}
#endif


void* _cdecl operator new(size_t size, Vixen::Core::Allocator* pAlloc)
{
	// Use allocator if present, otherwise, straight allocation
	if (pAlloc)
		return pAlloc->Alloc (size);
	else
		return Vixen::Core::GlobalAllocator::Get()->Alloc (size);
}

void _cdecl operator delete( void *p, Vixen::Core::Allocator* pAlloc)
{
	// Handle delete of null pointer
	if (p)
	{
		// Use allocator if present, otherwise, straight delete operation
		if (pAlloc)
			pAlloc->Free (p);
		else
			Vixen::Core::GlobalAllocator::Get()->Free (p);
	}
}

#if defined(_DEBUG) && !defined(__APPLE__)
void* _cdecl operator new (size_t size) { return operator new (size, Vixen::Core::RogueAllocator::Get()); }
void  _cdecl operator delete (void *p) { operator delete (p, Vixen::Core::RogueAllocator::Get()); }
#endif

namespace Vixen {
namespace Core {

VX_IMPLEMENT_CLASS(Allocator, BaseObj);
VX_IMPLEMENT_CLASS(GlobalAllocator, Allocator);
VX_IMPLEMENT_CLASS(FastAllocator, Allocator);
VX_IMPLEMENT_CLASS(FixedLenAllocator, Allocator);
VX_IMPLEMENT_CLASS(PoolAllocator, Allocator);


//============================================================
// struct Chain
//============================================================

Chain* Chain::Create(Allocator* pAlloc, Chain*& pHead, size_t nMax, size_t cbElement)
{
	VX_ASSERT(nMax > 0 && cbElement > 0);
	VX_ASSERT(pAlloc != NULL);

	// NULL allocator passed - use the one used on other elements
	// in the chain if non-NULL, or just get the Thread allocator;
//	if (pAlloc == NULL)
//		if (pHead == NULL || (pAlloc = pHead->pAlloc) == NULL)
//			pAlloc = ThreadAllocator::Get();

	size_t amount = sizeof(Chain) + nMax * cbElement;
	Chain* p = (Chain*) pAlloc->Alloc (amount);

	if (p)
	{
		p->nLength = nMax * cbElement;
		p->pNext = pHead;
		p->pAlloc = pAlloc;
		pHead = p;
	}
	return p;
}

void Chain::FreeDataChain(Chain* skipMe /* = NULL */)
{
	Chain* p = this;
	while (p != NULL)
	{
		char* data = (char*) p;
		Chain* pNext = p->pNext;
		VX_ASSERT(p->pAlloc != NULL || data == (char*) skipMe);

		// Avoid deleting block that was explicitely mentioned.
		if (data != (char*) skipMe)
			p->pAlloc->Free(data);
		p = pNext;
	}
}

/*
 * Visit all elements in all chained blocks whether they
 * are allocated or free. Call VisitFunc at each element.
 */
void Chain::VisitAllElements(size_t elemsize, void VisitFunc(void*))
{
	Chain* p = this;
	while (p != NULL)
	{
		Chain*	pNext = p->pNext;
		size_t	nbytes = p->nLength;
		char*	data = (char*) (p + 1);

		VX_ASSERT(p->pAlloc != NULL);
		while (nbytes >= elemsize)
		{
			VisitFunc(data);
			data += elemsize;
			nbytes -= elemsize;
		}
		p = pNext;
	}
}

#ifdef _DEBUG
bool Chain::ContainsPtr (void* ptr)
{
	Chain* p = this;
	while (p != NULL)
	{
		char* bytes = (char*) p;
		Chain* pNext = p->pNext;

		// Does pointer fall inside the block length?
		if (bytes + sizeof(Chain) <= ptr && (bytes + (sizeof(Chain) + p->nLength)) > ptr)
			return true;
		else
			p = pNext;
	}

	return false;
}


size_t Chain::TotalSize()
{
	size_t total = 0;
	for (Chain* p = this; p != NULL; p = p->pNext)
		total += p->nLength;
	
	return total;
}
	
int Chain::NumBlocks()
{
	int total = 0;
	for (Chain* p = this; p != NULL; p = p->pNext)
		total++;
	
	return total;
}
	
#endif


//============================================================
// Allocator class
//============================================================

Allocator::Allocator()
:   m_BlockAlloc	(NULL),
    m_Options		(/*ALLOC_ZeroMem*/)
{
    Alignment (4);
    ResetDebugStats();
}

void Allocator::Alignment (int align)
{
#ifdef _DEBUG
    for (int i = 1; i < 8*sizeof(int); i++)
        if ((1 << i) == align)
            goto AllOK;
    
    // Alignment should be a power of 2
    VX_ASSERT (false);
#endif
	// yeah.yeah.yeah all you 'purists' ... eat shit
AllOK:
    m_AlignmentMask = align - 1;
}

void* Allocator::Alloc(size_t amount)
{
	void* ptr;
	if (m_Options & ALLOC_ZeroMem)
		ptr = calloc(1, amount);
	else
		ptr = malloc(amount);

	UpdateDebugStats((long)SizeOfPtr(ptr), ptr);
	VX_TRACE2(Allocator::Debug, ("Allocator::Alloc(%d) %p\n", amount, ptr));

	return ptr;
}

void Allocator::Free(void* ptr)
{
	UpdateDebugStats(-(long)SizeOfPtr(ptr), ptr);
	VX_TRACE2(Allocator::Debug, ("Allocator::Free(%d) %p\n", amount, ptr));

	free(ptr);
}


size_t Allocator::SizeOfPtr(void* ptr)
{
#ifdef _WIN32
	size_t	amount = _msize(ptr);
#elif defined(__APPLE__)
	size_t  amount = malloc_size(ptr);
#else
	VX_ASSERT(false);
	size_t	amount = 0;
#endif
	return	amount;
}
    

#ifdef _DEBUG
void Allocator::UpdateDebugStats(long amount, void* ptr /* = NULL */)
{
	m_currentSize += amount;

	if (amount > 0)
	{
		m_totalAllocs++;
		m_remainingFrees++;
		m_totalSize += amount;

		if (m_currentSize > m_maxSize)
			m_maxSize = m_currentSize;
	}
	else
	{
		m_remainingFrees--;
	}
}

void Allocator::ResetDebugStats()
{
	m_totalAllocs		= 0;
	m_remainingFrees	= 0;
	m_currentSize		= 0;
	m_maxSize			= 0;
	m_totalSize			= 0;
	
	m_blockAllocs		= 0;
	m_blockFrees		= 0;
	m_lastWords[0]		= 0;
}


void Allocator::PrintDebugStats()
{
	VX_PRINTF(("\talloc:%6ld, !free:%6ld, size:%8ld, maxSize:%8ld, totalSize:%8ld%s\r\n",
			m_totalAllocs, m_remainingFrees, m_currentSize, m_maxSize, m_totalSize, m_lastWords));
}
#endif
    
    
//============================================================
// GlobalAllocator class
//============================================================

GlobalAllocator*	GlobalAllocator::s_ptheOneAndOnly = NULL;
	
GlobalAllocator		g_grandpa;

GlobalAllocator::GlobalAllocator()
{
    VX_ASSERT (s_ptheOneAndOnly == NULL);
    s_ptheOneAndOnly = this;
}

GlobalAllocator::~GlobalAllocator()
{
    s_ptheOneAndOnly = NULL;
    VX_PRINTF(("~GlobalAllocator      "));
}
	


//============================================================================
// FastContext
//============================================================================

FastContext::FastContext (size_t initial, size_t increment)
{
	TLSData::Get()->FastStack.Push (new FastAllocator(initial, increment));		// memory leak prone???
}
	
FastContext::~FastContext()
{
	delete TLSData::Get()->FastStack.Pop();
};

FastContext::operator FastAllocator*() const
{
	return CAST_(FastAllocator, TLSData::Get()->FastStack.Top());
}
	
//============================================================================
// FastAllocator
//============================================================================

FastAllocator::FastAllocator(size_t initialSize, size_t incrementSize)
:	m_bufUsed		(0),
	m_bufSize		(0),
	m_initialSize	(initialSize),
	m_incrementSize	(incrementSize),
	m_keepBlock		(NULL),
	m_freeBlocks	(NULL),
	m_blocks		(NULL)
{
	Init();
}

FastAllocator::FastAllocator(void* buffer, size_t initialSize, bool deleteBlock /*= true */, size_t incrementSize /* = 2048 */)
:	m_bufUsed		(0),
	m_bufSize		(0),
	m_initialSize	(initialSize),
	m_incrementSize	(incrementSize),
	m_keepBlock		(NULL),
	m_freeBlocks	(NULL),
	m_blocks		(NULL)
{
	SetInitialBlock (buffer, initialSize, deleteBlock);
	Init();
}

FastAllocator::~FastAllocator()
{
	// Free memory if not already done
	if (m_blocks)
		m_blocks->FreeDataChain (m_keepBlock);
	if (m_freeBlocks)
		m_freeBlocks->FreeDataChain (NULL);
    
    VX_PRINTF(("~FastAllocator         "));
}

FastAllocator* FastAllocator::Get()
{
	// It is OK if this method returns a ThreadAllocator (placed as default onto
	// stack when thread is instantiated).  It is only slightly less efficient at
	// memory management, but still gets the job done.
	// FastAllocator* pFastAlloc = CAST_(FastAllocator, tlsState->FastStack.Top());
	FastAllocator* pFastAlloc = CAST_(FastAllocator, TLSData::Get()->FastStack.Top());
	VX_ASSERT (pFastAlloc != NULL);
	return pFastAlloc;
}


void* FastAllocator::Alloc(size_t size)
{
	// Does allocation size exceed our block size?
	if (size >= m_incrementSize)
	{
		// allocate block that is large enough
		Chain* newBlock = Chain::Create (m_BlockAlloc, m_blocks, 1, size);

		// Make sure this isn't the first and only block
		if (newBlock->pNext)
		{
			// switch new block to be head->pNext, preserve existing
			// head, m_bufUsed, m_bufSize for subsequent allocations
			m_blocks = newBlock->pNext;
			newBlock->pNext = m_blocks->pNext;
			m_blocks->pNext = newBlock;
		}

		// return pointer to new block allocation
		return newBlock->data();

	}
	// Do we need to allocate another buffer?
	else if (m_bufUsed + size >= m_bufSize)
	{
		// add another block
		Chain* newBlock = m_freeBlocks;

		if (newBlock)
		{
			m_freeBlocks = m_freeBlocks->pNext;
			newBlock->pNext = m_blocks;
			m_blocks = newBlock;
		}
		else
		{
			size_t delta = (m_blocks ? m_incrementSize : m_initialSize);
			Chain::Create(m_BlockAlloc, m_blocks, 1, delta);
		}
		// Reset internal buffer pointers
		m_bufUsed = 0;
		m_bufSize = m_incrementSize;
	}

	// We have to have a memory buffer at this point...
	VX_ASSERT (m_blocks != NULL);

	// Get pointer to new chunk and update buffer variable
	char*  mem = (char*) m_blocks->data() + m_bufUsed;
	m_bufUsed += AlignMem (size);
	UpdateDebugStats ((long)size, mem);

	// Return pointer to memory chunk
	return mem;
}

void FastAllocator::FreeAll()
{
	Chain**	reused = &m_freeBlocks;
	Chain*	p = m_freeBlocks;
	size_t	size = m_initialSize;

	while (p != NULL)						// find end of free block list
	{
		reused = &(p->pNext);
		p = p->pNext;
	}
	p = m_blocks;							// determine which blocks can be reused
	while (p != NULL)
	{
		Chain*	next = p->pNext;
		if (p->nLength != size)				// a big block with one object?
		{
			Chain* tmp = p;					// free this one, don't reuse
			p->pAlloc->Free(tmp);
		}
		else if (p != m_keepBlock)
		{
			*reused = p;					// reuse this block
			reused = &(p->pNext);			// next reused block linked here
			*reused = NULL;
		}
		size = m_incrementSize;
		p = next;
	}
	*reused = NULL;							// end reused block list
	m_blocks = m_keepBlock;
}

void FastAllocator::Empty()
{
	FreeAll();
	Init();
}

void FastAllocator::Init()
{
	// Reset internal buffer pointers
	m_bufUsed = 0;

	// Special handling required if keepblock is retained.
	if (m_keepBlock)
	{
		m_bufSize = m_initialSize;
		m_blocks = m_keepBlock;

		// since this block of memory may come from the stack, zero initialize.
		memset (m_blocks, 0, m_bufSize);
	}
	else
	{
		m_bufSize = 0;
		m_blocks = NULL;
	}
}


void FastAllocator::SetInitialBlock (void* buffer, size_t initialSize, bool deleteBlock /* = true */)
{
	// We can't have an initial block already set - this class can currently only manage
	// a single block that has special "don't delete" attribute.
	VX_ASSERT (m_keepBlock == NULL && m_blocks == NULL && m_bufUsed == 0);

	// Save the new initial size - overwrite old setting
	m_initialSize = initialSize;

	// Save m_keepBlock pointer to indicate this shouldn't be deleted
	m_keepBlock = (Chain*)buffer;

	// Reset takes care of the nitty-gritty settings and initializing the block
	Init();

	// If we are to delete the block, reset m_keepblock
	if (deleteBlock)
		m_keepBlock = NULL;
}



//============================================================================
// FixedLenAllocator
//============================================================================

// Ensure that elements are correctly aligned for host operating system.
FixedLenAllocator::FixedLenAllocator (size_t elementSize, size_t blockSize)
: m_elementSize	(AlignMem (elementSize)),
  m_blockSize		(blockSize)
{
	// Need to be ensure we aren't requesting zero-sized elements.
	VX_ASSERT (elementSize != 0);
	Init();
	VX_TRACE2(Allocator::Debug, ("FixedLenAllocator::Create(%d, %d)\n", elementSize, blockSize));
}


FixedLenAllocator::~FixedLenAllocator()
{
	// Free memory if not already done
	if (m_blocks != NULL)
	{
#ifdef _DEBUG
		m_blockFrees -= m_blocks->NumBlocks();
		sprintf (m_lastWords, "   %ld*%zd", m_blockAllocs, (long)m_elementSize * m_blockSize + sizeof(Chain));
#endif
		m_blocks->FreeDataChain();
	}
	VX_TRACE2(Allocator::Debug, ("FixedLenAllocator::Delete(%d, %d)\n", m_elementSize, m_blockSize));
    VX_PRINTF (("  ~FixedLenAlloc-%-4d", m_elementSize));
}

void* FixedLenAllocator::Alloc (size_t size)
{
	bool isLocking = IsLocking();
	if (isLocking) m_Lock.Enter();
	// Make sure the correct allocation size is requested
	VX_ASSERT (AlignMem (size) <= m_elementSize);

	// Do we need to replenish the free blocks?
	if (m_freeList == NULL)
	{
		// add another block
		if (m_BlockAlloc == NULL)
			m_BlockAlloc = GlobalAllocator::Get();
		Chain* newBlock = Chain::Create (m_BlockAlloc, m_blocks, m_blockSize, m_elementSize);
		VX_ASSERT(newBlock != NULL);
#ifdef _DEBUG
//      this would keep track of the block allocations, not the sub-allocations (see below call
//      to UpdateDebugStats() which registers the requested size.
//		UpdateDebugStats (m_blockSize * m_elementSize);
		m_blockAllocs++;
#endif
		// chain in reverse order to make it easier to debug
		char*	pRaw = (char*) newBlock->data();
		pRaw += (m_blockSize - 1) * m_elementSize;

		for (long ii = (long) m_blockSize - 1; ii >= 0; ii--, pRaw -= m_elementSize)
		{
			Chain* pVChain = (Chain*)pRaw;
			pVChain->pNext = m_freeList;
			m_freeList = pVChain;
		}
	}

	// we must have something
	VX_ASSERT (m_freeList != NULL);
	VX_ASSERT (m_freeList->nLength >= 0);

    // Extract next element from head of list and zero-init pointer
	Chain* pVChain = m_freeList;
	m_freeList = m_freeList->pNext;

    UpdateDebugStats((long)size, pVChain);
	VX_ASSERT ((m_freeList == NULL) || (m_freeList->nLength >= 0));
//	pVChain->pNext = NULL;		// should be OK, since ALLOC_ZeroMem is being phased out

	// Return pointer to memory block
	if (isLocking) m_Lock.Leave();
	return pVChain;
}

void FixedLenAllocator::Free(void* aPtr)
{
	bool isLocking = IsLocking();
	if (isLocking) m_Lock.Enter();
#ifdef _DEBUG
	UpdateDebugStats(-(long)m_elementSize, aPtr);

    // Make sure that we are freeing a pointer which actually beloings to this allocator.
    VX_ASSERT (m_blocks && m_blocks->ContainsPtr (aPtr));
#endif
    if (m_Options & ALLOC_ZeroMem)
        memset(aPtr, 0, m_elementSize);

	// Chain this back into the free list.  Note the unsafe cast to VChain pointer -
    // it is assumed that we are indeed passing a valid VChain pointer!
    Chain*	pVChain = (Chain*)aPtr;
    pVChain->pNext = m_freeList;
    m_freeList = pVChain;
	if (isLocking) m_Lock.Leave();
}

void FixedLenAllocator::FreeAll()
{
	bool isLocking = IsLocking();
	if (isLocking) m_Lock.Enter();

	Chain* blockptr = m_blocks;

	m_freeList = NULL;
	while (blockptr)
	{
		char*	pRaw = (char*) blockptr->data();
		pRaw += (m_blockSize - 1) * m_elementSize;

		for (long ii = (long) m_blockSize - 1; ii >= 0; ii--, pRaw -= m_elementSize)
		{
			Chain* pVChain = (Chain*) pRaw;
			pVChain->pNext = m_freeList;
			m_freeList = pVChain;
		}
		blockptr = blockptr->pNext;
	}
	if (isLocking) m_Lock.Leave();
	VX_TRACE(Allocator::Debug, ("FixedLenAllocator::FreeAll()\n"));
}

void FixedLenAllocator::Init()
{
	// Reset internal buffer pointers
	m_freeList = NULL;
	m_blocks = NULL;

	// Reset debugging stats
	ResetDebugStats();
}

void FixedLenAllocator::SetElementAndBlockSize(size_t eSize, size_t bSize)
{
	// Need to be able to hold at least a pointer
	VX_ASSERT (eSize >= sizeof (void*));
	VX_ASSERT (m_blocks == NULL && m_freeList == NULL);

	m_elementSize = eSize;
	m_blockSize   = bSize;
}

//============================================================================
// PoolAllocator
//============================================================================
    
/*!
 * @fn PoolAllocator::PoolAllocator(Allocator* blockalloc, int npools)
 * @param blockalloc	Block allocator to use to allocate memory for pools.
 * @param smallest		Size of the smallest element pool. Others pools are multiples of this base size.
 * @param npools		Number of memory pools to use.
 * @param options		Allocator options.
 *
 * A pool allocator creates several internal memory pools which
 * are each maintained by a fixed length allocator. The block
 * allocator supplied here is used to obtain the memory for these pools.
 * Blocks too large for the pools are allocated directly using the block allocator.
 *
 * @see ThreadAllocator GlobalAllocator Allocator::SetBlockAllocator
 */

PoolAllocator::PoolAllocator(Allocator* blockalloc, size_t smallest, size_t npools, int options) : Allocator()
{
    if (blockalloc == NULL)
        blockalloc = GlobalAllocator::Get();
    m_BlockAlloc = blockalloc;
    m_NumPools = (int) npools;

    for (size_t i = 0; i < m_NumPools && i < ALLOC_MaxPools; ++i)
    {
        m_MaxSize = smallest << i;
        m_MemPool[i] = new FixedLenAllocator (m_MaxSize, (8192 - sizeof(Chain)) / m_MaxSize);
		m_MemPool[i]->SetOptions(options);
    }
}
    
PoolAllocator::~PoolAllocator()
{
    for (int i = 0; i < m_NumPools; ++i)
    {
        VX_TRACE(Allocator::Debug, ("Free pool %d\n", m_MemPool[i]->ElementSize()));
        delete m_MemPool[i];
    }
    VX_PRINTF (("    ~PoolAllocator     "));
}
    
/*!
 * @fn void PoolAllocator::SetBlockAllocator(Allocator* blockalloc)
 * @param blockalloc Block allocator to use to allocate memory for pools.
 *
 * This allocator creates several internal memory pools which
 * are each maintained by a fixed length allocator. The block allocator
 * supplied here is used to obtain the memory for these pools.
 * Usually the global or thread allocator is used as a block allocator.
 *
 * Establishing a block allocator will also allocate the memory pools
 * if they have not already been established with PoolAllocator::InitPools
 *
 * @see ThreadAllocator GlobalAllocator FixedLenAllocator::SetBlockAllocator PoolAllocator::InitPools
 */
void PoolAllocator::SetBlockAllocator(Allocator* blockalloc)
{
	if (blockalloc == NULL)
		blockalloc = GlobalAllocator::Get();
	m_BlockAlloc = blockalloc;
	if (m_MemPool[0] == NULL)
		return;
	for (int i = 0; i < m_NumPools; ++i)
		m_MemPool[i]->SetBlockAllocator(blockalloc);
}

/*!
 * @fn void* PoolAllocator::Alloc(size_t bytes)
 * @param bytes	Number of bytes to allocate.
 *
 * Allocates a block of memory at least \b bytes long.
 * The allocator of the object (first word) is set to the pool from
 * which the memory came. When the object is deleted, it will be
 * its memory will be automatically returned to this pool.
 *
 * Memory is allocated in large chunks and organized into size pools.
 * Objects near a given size are allocated from the same pool.
 * This has the side effect of improving caching behavior because all
 * the objects of the same size allocated at the same time will be in
 * the same memory page. Instead of returning memory to the OS when
 * it is no longer used, this allocator keeps the objects freed and reuses them.
 * All memory in the pools is freed when this allocator is deleted.
 *
 * @return Pointer to allocated memory, NULL if no memory could be allocated
 *
 * @see Allocator::Free
 * @see BaseObj::operator new
 */
void* PoolAllocator::Alloc(size_t byte_size)
{
	void*	ptr = NULL;

	byte_size += sizeof(size_t);			// make room for size
	if (byte_size > m_MaxSize)				// too big for pools?
		ptr = m_BlockAlloc->Alloc(byte_size);
	else for (int i = 0; i < m_NumPools; ++i)
	{
		FixedLenAllocator* pool = m_MemPool[i];
		if (byte_size <= pool->ElementSize())
		{
            byte_size = pool->ElementSize();
			ptr = pool->Alloc(pool->ElementSize());	// grab some bytes
			break;
		}
	}
	if (ptr == NULL)
		VX_ERROR(("PoolAllocator::Alloc %ld ERROR out of memory\n", byte_size), NULL);
	VX_TRACE2(Allocator::Debug, ("PoolAllocator::Alloc(%d) %p\n", byte_size, ptr));//
    UpdateDebugStats((long)byte_size);			// do NOT pass pointer, should be handled
    
	*((size_t*) ptr) = byte_size;			// save the true size
	return ((size_t*) ptr) + 1;				// point past the size
}

/*!
 * @fn void PoolAllocator::Free(void* ptr)
 * @param ptr	Pointer to memory block to free.
 *
 * If the pointer came from this allocator, it is reclaimed by
 * the allocator. This memory will be reused. Do not call this
 * routine directly unless you are sure the object came from this
 * allocator.
 *
 * @see BaseObj::operator delete
 * @see Allocator::Alloc
 */
void PoolAllocator::Free(void* ptr)
{
	VX_ASSERT(ptr);

    size_t*	p = (size_t*) ptr;
	size_t byte_size = *--p;			// grab byte size
	if (byte_size > m_MaxSize)			// not in the pools?
	{
		VX_TRACE2(Allocator::Debug, ("PoolAllocator::Free(%d)\n", byte_size));
		m_BlockAlloc->Free(p);			// give it back to block allocator
		return;
	}
	for (int i = 0; i < m_NumPools; ++i)
	{
		FixedLenAllocator* pool = m_MemPool[i];
		if (byte_size <= pool->ElementSize())
		{
			pool->Free(p);						// put it back in the pool it came from
			VX_TRACE2(Allocator::Debug, ("PoolAllocator::Free(%d)\n", byte_size));
            UpdateDebugStats (-(long)byte_size);		// do NOT pass pointer, should be handled
			return;
		}
	}
	VX_ERROR_RETURN(("PoolAllocator::Free %p does not come from this pool", ptr));
}

    
}	// end Core
}	// end Vixen
