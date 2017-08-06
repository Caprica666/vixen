// Taken from MFC to allow customized allocation strategies - thread and
// temporary heap allocations without synchronization calls like
// InterlockedIncrement()/InterlockedDecrement().

#include "vcore/vcore.h"


// static class data
namespace Vixen {
namespace Core {

int StringPool::Debug = 0;

#if 1
StringPool::StringPool(GetAllocatorFunc pfGetAllocator) : PoolAllocator((*pfGetAllocator)(), 64, 6) { }

void String::FreeData(StringData* pData)
{
	// Pass pointer and data size to Free() call
	StringPool*	pPool = (StringPool*)pData->pAlloc;
	if (pData->nAllocLength)
	{
		VX_TRACE(StringPool::Debug, ("String::FreeData '%.512s'\n", pData->data()));
		pPool->Free (pData);
	}
}

void StringPool::AllocString(String& str, size_t newLen)
{
	VX_ASSERT (newLen >= 0);
	VX_ASSERT (newLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	size_t	requestSize = newLen + 1;	// count the extra null
	size_t	curLen = str.GetLength();

	requestSize += sizeof(StringData);		
	// Allocate memory from appropriate pool once determined.
	StringData* pData = (StringData*) Alloc(requestSize);
	VX_ASSERT(pData);
	// Set StringData member variables
	pData->nRefs = 1;
	pData->data()[newLen] = '\0';
	pData->nDataLength = newLen;
	pData->nAllocLength = requestSize;
	pData->pAlloc = this;
	str.m_pchData = pData->data();
}

void String::AllocBuffer(size_t nLen)
{
	VX_ASSERT (nLen >= 0);
	VX_ASSERT (nLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	// Optimization: handle empty string case
	if (nLen == 0)
	{
		Init();
		return;
	}
	Allocator*	pPool;
	size_t		requestSize = nLen;
	size_t		curLen = GetLength();
	StringData*	pData = GetData();
	
	VX_ASSERT(pData);
	if (IsEmpty())
		pPool = _vStringPool;
	else
		pPool = pData->pAlloc;
		
	// Allocate memory from appropriate pool once determined.
	pData = (StringData*) pPool->Alloc(requestSize + sizeof(StringData) + 1);
	VX_ASSERT(pData);
	// Set StringData member variables
	pData->nRefs = 1;
	pData->data()[nLen] = '\0';
	pData->nDataLength = nLen;
	pData->nAllocLength = requestSize;
	pData->pAlloc = pPool;
	m_pchData = pData->data();
}
#else
StringPool::StringPool (GetAllocatorFunc pfGetAllocator)
	: m_resolveMethod (pfGetAllocator),
	m_pBaseAlloc	(NULL),
	m_alloc32		(VSTRINGDATA(32),  157),	//  8K chunks
	m_alloc64		(VSTRINGDATA(64),   97),	//  8K chunks
	m_alloc128		(VSTRINGDATA(128),  55),	//  8K chunks
	m_alloc256		(VSTRINGDATA(256),  29),	//  8K chunks
	m_alloc512		(VSTRINGDATA(512),  30),	// 16K chunks
	m_alloc1024		(VSTRINGDATA(1024), 15),	// 16K chunks
	m_alloc2048		(VSTRINGDATA(2048), 15)		// 32K chunks
{
	// Save addresses of allocators in array

	m_array[0] = &m_alloc32;
	m_array[1] = &m_alloc64;
	m_array[2] = &m_alloc128;
	m_array[3] = &m_alloc256;
	m_array[4] = &m_alloc512;
	m_array[5] = &m_alloc1024;
	m_array[6] = &m_alloc2048;
}

StringPool::~StringPool()
{
	// The StringPool shouldn't try to free any remaining memory,
	// since we cannot guarantee the order in which static destructors
	// are called - this applies to all thread local objects (like this one).
	// We assume that memory contained in this structure is properly handled
	// in the destructor of TLSData<ThreadAllocator>.
}

void* StringPool::Alloc (size_t amount)
{
	// Do we need to resolve the base allocator?
	if (m_pBaseAlloc == NULL)
	{
		Allocator*		pAlloc = m_resolveMethod();
		VX_ASSERT  (pAlloc != NULL);
		SetBaseAllocator (pAlloc);
	}

	// Fatal if not handled correctly
	VX_ASSERT  (m_pBaseAlloc != NULL);

	void*	ptr;

	// Maybe not elegant, but easy for debugging.
	if (amount <= 32)		 ptr = m_alloc32.Alloc   (VSTRINGDATA(m_lastAllocSize = 32));
	else if (amount <= 64)	 ptr = m_alloc64.Alloc   (VSTRINGDATA(m_lastAllocSize = 64));
	else if (amount <= 128)	 ptr = m_alloc128.Alloc  (VSTRINGDATA(m_lastAllocSize = 128));
	else if (amount <= 256)	 ptr = m_alloc256.Alloc  (VSTRINGDATA(m_lastAllocSize = 256));
	else if	(amount <= 512)	 ptr = m_alloc512.Alloc  (VSTRINGDATA(m_lastAllocSize = 512));
	else if	(amount <= 1024) ptr = m_alloc1024.Alloc (VSTRINGDATA(m_lastAllocSize = 1024));
	else if	(amount <= 2048) ptr = m_alloc2048.Alloc (VSTRINGDATA(m_lastAllocSize = 2048));
	else					 ptr = m_pBaseAlloc->Alloc (VSTRINGDATA(m_lastAllocSize = amount));

#ifdef _DEBUG
	UpdateDebugStats (VSTRINGDATA(m_lastAllocSize));
#endif
	return ptr;
}


void StringPool::Free (void* ptr, size_t amount)
{
	// Fatal if not handled correctly
	VX_ASSERT  (m_pBaseAlloc != NULL);

#ifdef _DEBUG
	if (m_pBaseAlloc->IsKindOf(CLASS_(ThreadAllocator)))
	{
		// Ouch - deallocation from a different thread than the one that created
		// the string.  This will most likely lead to heap corruption, since access
		// to the underlying thread heap isn't serialized.
		Allocator* pThreadAlloc = ThreadAllocator::Get();
//		Allocator* pThreadAlloc = _vStringPool;
		assert  (m_pBaseAlloc == pThreadAlloc);
	}
#endif

	if (amount <= 32)			m_alloc32.Free   (ptr);
	else if (amount <= 64)		m_alloc64.Free   (ptr);
	else if (amount <= 128)		m_alloc128.Free  (ptr);
	else if (amount <= 256)		m_alloc256.Free  (ptr);
	else if	(amount <= 512)		m_alloc512.Free  (ptr);
	else if	(amount <= 1024)	m_alloc1024.Free (ptr);
	else if	(amount <= 2048)	m_alloc2048.Free (ptr);
	else						m_pBaseAlloc->Free (ptr);
#ifdef _DEBUG
	intptr tmp = VSTRINGDATA(amount);
	UpdateDebugStats(-tmp);
#endif
}

#ifndef _countof
	#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

void StringPool::FreeAll()
{
	// Free all memory in constituent arrays
	for (int i = 0; i < _countof(m_array); i++)
		if (m_array[i])
			m_array[i]->FreeAll();

#ifdef _DEBUG
	ResetDebugStats();
#endif
}

void StringPool::SetBaseAllocator (Allocator* pBaseAlloc)
{
	// Free up all memory if we are clearing m_pBaseAlloc to NULL.
	if (m_pBaseAlloc != NULL)
		FreeAll();

	// set base allocator for all fixed length allocations
	m_pBaseAlloc = m_array[_countof(m_array) - 1] = pBaseAlloc;
	for (int i = 0; i < _countof(m_array) - 1; i++)
		((FixedLenAllocator*) m_array[i])->SetBlockAllocator (pBaseAlloc);
}


#ifdef _DEBUG
void StringPool::PrintDebugStats()
{
	if (m_resolveMethod == ThreadAllocator::Get)
		VX_PRINTF(("\r\nVStringPool Thread statistics\r\n"));
	else
		VX_PRINTF(("\r\nVStringPool Temporary statistics\r\n"));

	// Print memory statistics for constituent arrays
	for (int i = 0; i < _countof(m_array); i++)
		if (m_array[i])
		{
			char	name[32];

			if (i == _countof(m_array) - 1)
				sprintf(name, "         default heap = ");
			else
				sprintf(name, "    %4ld byte strings = ", 1 << (4 + i));
			VX_PRINTF((name));
			m_array[i]->PrintDebugStats();
		}
}
#endif

void String::FreeData(StringData* pData)
{
	// Pass pointer and data size to Free() call
	StringPool*	pPool = (StringPool*)pData->pAlloc;
	if (pData->nAllocLength)
		pPool->Free (pData, pData->nAllocLength);
}


void StringPool::AllocString(String& str, size_t newLen)
{
	VX_ASSERT (newLen >= 0);
	VX_ASSERT (newLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	size_t	requestSize = newLen + 1;	// count the extra null
	size_t	curLen = str.GetLength();

	if (curLen > 2048)
	{
		size_t delta = newLen - curLen;
		if (delta != 0 && delta < curLen)
			requestSize += curLen;
	}
	requestSize += sizeof(StringData);		
	// Allocate memory from appropriate pool once determined.
	StringData* pData = (StringData*) Alloc(requestSize);
	VX_ASSERT(pData);
	// Set StringData member variables
	pData->nRefs = 1;
	pData->data()[newLen] = '\0';
	pData->nDataLength = newLen;
	pData->nAllocLength = requestSize;
	pData->pAlloc = this;
	str.m_pchData = pData->data();
}


void String::AllocBuffer(size_t nLen)
{
	VX_ASSERT (nLen >= 0);
	VX_ASSERT (nLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	// Optimization: handle empty string case
	if (nLen == 0)
	{
		Init();
		return;
	}
	Allocator*	pPool;

	if (IsEmpty())
		pPool = _vStringPool;
	else
		pPool = GetPool();
	VX_ASSERT(pPool);
	if (pPool == _vStringPool)
		_vStringPool->AllocString(*this, nLen);
	else
	{
		size_t	requestSize = nLen;
		size_t	curLen = GetLength();

		if (curLen > 2048)
		{
			size_t delta = nLen - curLen;
			if (delta != 0 && delta < curLen)
				requestSize += curLen;
		}
		requestSize += sizeof(StringData);
		StringData*	pData = GetData();		VX_ASSERT(pData);
		Allocator*	pPool = pData->pAlloc;	VX_ASSERT(pPool);
		
		// Allocate memory from appropriate pool once determined.
		pData = (StringData*) pPool->Alloc(requestSize);
		VX_ASSERT(pData);
		// Set StringData member variables
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		pData->nAllocLength = requestSize;
		pData->pAlloc = pPool;
		m_pchData = pData->data();
	}
}
#endif


}	// end Core
}	// end Vixen