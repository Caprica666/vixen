#include "vcore/vcore.h"
#include <malloc.h>

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

	// NULL allocator passed - use the one used on other elements
	// in the chain if non-NULL, or just get the Thread allocator;
	if (pAlloc == NULL)
		if (pHead == NULL || (pAlloc = pHead->pAlloc) == NULL)
			pAlloc = ThreadAllocator::Get();

	Chain* p = (Chain*) pAlloc->Alloc (sizeof(Chain) + nMax * cbElement);

	if (p)
	{
		p->nLength = nMax * cbElement;
		p->pNext = pHead;
		p->pAlloc = pAlloc;
		pHead = p;  // change head (adds in reverse order for simplicity)
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
#endif

TempAllocScope::TempAllocScope (FastAllocator* pAlloc)
{
	assert (pAlloc != NULL);
	TLSData::Get()->FastStack.Push(pAlloc);
}

TempAllocScope::~TempAllocScope()
{
	TLSData::Get()->FastStack.Pop();
}

//============================================================
// Allocator class
//============================================================

void	Allocator::FreeAll()
{
#ifdef _DEBUG
	if (Allocator::Debug) PrintDebugStats();
#endif
	VX_ASSERT(false);
}

#ifdef _DEBUG
Allocator::Allocator()
	: m_totalAllocs	(0),
	m_remainingFrees(0),
	m_currentSize	(0),
	m_maxSize		(0),
	m_totalSize		(0),
	m_Options		(ALLOC_ZeroMem),
	m_BlockAlloc	(NULL)
{
}

void Allocator::UpdateDebugStats(intptr amount)
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
}


void Allocator::PrintDebugStats()
{
	VX_PRINTF(("\ttotalAllocs : %ld, remainingFrees : %ld, currentSize : %ld, maxSize : %ld, totalSize : %ld\r\n",
			m_totalAllocs, m_remainingFrees, m_currentSize, m_maxSize, m_totalSize));
}
#else
Allocator::Allocator()
  : m_BlockAlloc	(NULL),
	m_Options		(ALLOC_ZeroMem)
{
}
#endif


//============================================================
// GlobalAllocator class
//============================================================

GlobalAllocator*	GlobalAllocator::s_ptheOneAndOnly;

GlobalAllocator::GlobalAllocator(int unused)
{
}

void* GlobalAllocator::Alloc (size_t amount)
{
#ifdef _DEBUG
	UpdateDebugStats (amount);
#endif
	void* ptr;
	if (m_Options & ALLOC_ZeroMem)
		ptr = calloc(1, amount);
	else
		ptr = malloc(amount);
	VX_TRACE2(GlobalAllocator::Debug, ("GlobalAllocator::Alloc(%d) %p\n", amount, ptr));
	return ptr;
}

void GlobalAllocator::Free (void* ptr)
{
#if defined(_DEBUG) && defined(_WIN32)
	intptr	amount = _msize(ptr);
	UpdateDebugStats(-amount);
	VX_TRACE2(GlobalAllocator::Debug, ("GlobalAllocator::Free(%d) %p\n", amount, ptr));
#else						// TODO: figure out what to do on Linux
	VX_TRACE2(GlobalAllocator::Debug, ("GlobalAllocator_Free %p\n", ptr));
#endif
	free (ptr);
}

void* GlobalAllocator::Grow (void* ptr, size_t newsize)
{
	#if defined(_DEBUG) && defined(_WIN32)
	size_t	amount = _msize(ptr);
	UpdateDebugStats (newsize - amount);
	VX_TRACE2(GlobalAllocator::Debug, ("GlobalAllocator::Free(%d) %p\n", amount, ptr));
#else 						// TODO: figure out what to do on Linux
	VX_TRACE2(GlobalAllocator::Debug, ("GlobalAllocator_Free %p\n", ptr));
#endif
	return realloc(ptr, newsize);
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
}

Allocator* FastAllocator::Get()
{
	// It is OK if this method returns a ThreadAllocator (placed as default onto
	// stack when thread is instantiated).  It is only slightly less efficient at
	// memory management, but still gets the job done.
	// FastAllocator* pFastAlloc = CAST_(FastAllocator, tlsState->FastStack.Top());
	Allocator* pFastAlloc = TLSData::Get()->FastStack.Top();
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
			newBlock = Chain::Create(m_BlockAlloc, m_blocks, 1, delta);
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
#ifdef _DEBUG
	UpdateDebugStats (size);
#endif

	// Return pointer to memory chunk
	return mem;
}


void FastAllocator::Empty()
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
	Init();
}

void FastAllocator::FreeAll()
{
	if (m_freeBlocks)
		m_freeBlocks->FreeDataChain(NULL);
	if (m_blocks)
	{
		// Free the entire memory chain at once
		m_blocks->FreeDataChain (m_keepBlock);
		Init();
	}

	// Make sure we are truly in a default state
	VX_ASSERT (m_bufUsed == 0 && ((m_bufSize == 0 && m_blocks == NULL) || m_keepBlock));
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
#ifdef _DEBUG
	ResetDebugStats();
#endif
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
		m_blocks->FreeDataChain();
	VX_TRACE2(Allocator::Debug, ("FixedLenAllocator::Delete(%d, %d)\n", m_elementSize, m_blockSize));
}

void* FixedLenAllocator::Alloc (size_t size)
{
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
		UpdateDebugStats (m_blockSize * m_elementSize);
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
#ifdef _DEBUG
	VX_ASSERT (m_freeList->nLength >= 0);
#endif
	// Extract next element from head of list and zero-init pointer
	Chain* pVChain = m_freeList;
	m_freeList = m_freeList->pNext;
#ifdef _DEBUG
	VX_ASSERT ((m_freeList == NULL) || (m_freeList->nLength >= 0));
#endif
	pVChain->pNext = NULL;

	// Return pointer to memory block
	return pVChain;
}

void* FixedLenAllocator::Grow(void* ptr, size_t size)
{
	return NULL;
}

void FixedLenAllocator::FreeAll()
{
	if (m_blocks)
	{
		// Remove all the memory at once
		m_blocks->FreeDataChain();
		Init();
	}
	VX_TRACE(Allocator::Debug, ("FixedLenAllocator::FreeAll(%d, %d)\n", m_elementSize, m_blockSize));
	// Make sure we are truly in a default state
	VX_ASSERT (m_freeList == NULL && m_blocks == NULL);
}


void FixedLenAllocator::Init()
{
	// Reset internal buffer pointers
	m_freeList = NULL;
	m_blocks = NULL;

	// Reset debugging stats
#ifdef _DEBUG
	ResetDebugStats();
#endif
}

void FixedLenAllocator::SetElementAndBlockSize(size_t eSize, size_t bSize)
{
	// Need to be able to hold at least a pointer
	VX_ASSERT (eSize >= sizeof (void*));
	VX_ASSERT (m_blocks == NULL && m_freeList == NULL);

	m_elementSize = eSize;
	m_blockSize   = bSize;
}

PoolAllocator*	PoolAllocator::s_ptheOneAndOnly;

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

	if (m_MaxSize == 0)
		InitPools(new (m_BlockAlloc) FixedLenAllocator(m_BaseSize, m_BaseSize * 64), m_NumPools);
	byte_size += sizeof(size_t);			// make room for size
	if (byte_size > m_MaxSize)				// too big for pools?
		ptr = m_BlockAlloc->Alloc(byte_size);
	else for (int i = 0; i < m_NumPools; ++i)
	{
		FixedLenAllocator* pool = m_MemPool[i];
		if (byte_size <= pool->ElementSize())
		{
			ptr = pool->Alloc(pool->ElementSize());	// grab some bytes
			break;
		}
	}
	if (ptr == NULL)
		VX_ERROR(("PoolAllocator::Alloc %d ERROR out of memory\n", byte_size), NULL);
	VX_TRACE2(Allocator::Debug || PoolAllocator::Debug, ("PoolAllocator::Alloc(%d) %p\n", byte_size, ptr));
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
	if (m_MaxSize == 0)
		VX_ERROR_RETURN(("PoolAllocator::Free %p does not come from this pool\n", ptr));
	size_t*	p = (size_t*) ptr;
	size_t byte_size = *--p;			// grab byte size
	if (byte_size > m_MaxSize)			// not in the pools?
	{
		VX_TRACE2(Allocator::Debug || PoolAllocator::Debug, ("PoolAllocator::Free(%d)\n", byte_size));
		m_BlockAlloc->Free(p);			// give it back to block allocator
		return;
	}
	for (int i = 0; i < m_NumPools; ++i)
	{
		FixedLenAllocator* pool = m_MemPool[i];
		VX_ASSERT(pool);
		if (byte_size <= pool->ElementSize())
		{
			pool->Free(p);				// put it back in the pool it came from
			VX_TRACE2(Allocator::Debug || PoolAllocator::Debug, ("PoolAllocator::Free(%d)\n", byte_size));
			return;
		}
	}
	VX_ERROR_RETURN(("PoolAllocator::Free %p does not come from this pool", ptr));
}

/*!
 * @fn void* PoolAllocator::Grow(void* ptr, size_t amount)
 * @param ptr	Pointer to memory block to enlarge or replace.
 * @param amount New size for memory block
 *
 * If the object can still fit in place, we just return the
 * existing pointer. Otherwise, we allocate a new block and
 * copy the memory over to it.
 *
 * @see Allocator::Free
 * @see Allocator::Alloc
 */
void* PoolAllocator::Grow(void* ptr, size_t amount)
{
	size_t*	p = ((size_t*) ptr) - 1;
	size_t	byte_size = *p;					// grab previous byte size
	FixedLenAllocator* pool;

	VX_ASSERT(byte_size > 0);
	amount += sizeof(size_t);				// include the size field
	VX_TRACE(PoolAllocator::Debug, ("PoolAllocator_Grow(%d)\n", amount));
	if (byte_size > m_MaxSize)				// from block allocator?
	{
		ptr = m_BlockAlloc->Grow(p, amount);// try to grow here
		if (ptr)
		{
			p = (size_t*) ptr;
			*p++ = amount;					// store new size
			return p;						// point past size
		}
		return NULL;						// cannot grow
	}
	for (int i = 0; i < m_NumPools; ++i)	// find pool the object is in
	{
		pool = m_MemPool[i];
		VX_ASSERT(pool);
		if (byte_size <= pool->ElementSize())
			if (amount <= pool->ElementSize())
			{
				*p++ = amount;				// save new current size
				return p;
			}
			else break;
	}
	ptr = Alloc(amount - sizeof(size_t));	// make a new area
	if (ptr == NULL)
		return NULL;
	byte_size -= sizeof(size_t);			// dont include size field
	memcpy(ptr, p + 1, byte_size);			// copy old data, not size
	pool->Free(p);							// free the old data area
	return ptr;
}

/*!
 * @fn bool PoolAllocator::InitPools(FixedLenAllocator* pool, int n)
 *
 * @param pool	fixed length allocator for smallest element size
 * @param n		number of pools to create
 *
 * The object allocator keeps a set of fixed size allocators to satisfy
 * requests for a small amounts of memory. The input \b pool establishes the type
 * of allocator to use and the size of the smallest elements. The function
 * constructs the remaining \b n - 1 allocators by doubling the element
 * size each time.
 *
 * You can make the object allocator thread-safe by providing it
 * with pools that can perform locking around allocation and free.
 * The default condition is to do no locking.
 *
 * @see FixedLenAllocator PoolAllocator::SetBlockAllocator
 */
bool PoolAllocator::InitPools(FixedLenAllocator* pool, int npools)
{
	BaseObj*		ptr = NULL;
	int			i = 0;
	size_t		blocksize;

	VX_ASSERT(m_MemPool[0] == NULL);
	VX_ASSERT(pool);
	m_MaxSize = pool->ElementSize();
	blocksize = pool->BlockSize();
	m_MemPool[i] = pool;
	if (m_MaxSize == 0)
		m_MaxSize = 128;
	if (blocksize == 0)
		blocksize = 8192;
	pool->SetElementAndBlockSize(m_MaxSize, blocksize / m_MaxSize);
	pool->SetBlockAllocator(m_BlockAlloc);
	pool->SetOptions(m_Options);
	while (++i < npools)
	{
		m_MaxSize <<= 1;
		pool = (FixedLenAllocator*) pool->GetClass()->CreateObject(m_BlockAlloc);
		while (blocksize / m_MaxSize < 4)
			blocksize *= 2;
		pool->SetElementAndBlockSize(m_MaxSize, blocksize / m_MaxSize);
		pool->SetBlockAllocator(m_BlockAlloc);
		pool->SetOptions(m_Options);
		VX_ASSERT(m_MemPool[i] == NULL);
		m_MemPool[i] = pool;
	}
	m_NumPools = npools;
	return true;
}

#ifdef _DEBUG
void PoolAllocator::ResetDebugStats()
{
	for (int i = 0; i < m_NumPools; ++i)
		m_MemPool[i]->ResetDebugStats();
}

void PoolAllocator::PrintDebugStats()
{
	for (int i = 0; i < m_NumPools; ++i)
	{
		FixedLenAllocator* pool = m_MemPool[i];
		if (pool == NULL)
			continue;
		VX_PRINTF(("Pool %d\n", pool->ElementSize()));
		pool->PrintDebugStats();
	}
}
#endif

}	// end Core
}	// end Vixen