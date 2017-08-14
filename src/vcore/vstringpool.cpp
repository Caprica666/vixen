// Taken from MFC to allow customized allocation strategies - thread and
// temporary heap allocations without synchronization calls like
// InterlockedIncrement()/InterlockedDecrement().

#include "vcore/vcore.h"


// static class data
namespace Vixen {
namespace Core {

//============================================================================
// StringPool
//============================================================================

VX_IMPLEMENT_CLASS(StringPool, PoolAllocator);

StringPool* StringPool::s_singleton;

StringPool g_spaghettiFactory;
	
StringPool::~StringPool()
{
    // The StringPool shouldn't try to free any remaining memory,
    // since we cannot guarantee the order in which static destructors
    // are called - this applies to all thread local objects (like this one).
    // We assume that memory contained in this structure is properly handled
    // in the destructor of TLSData<ThreadAllocator>.
    
    VX_PRINTF (("~StringPool\n"));
}


StringPool::StringPool(Allocator* pAlloc)
  : PoolAllocator(pAlloc, 32, 7)			// create pool ranging from 32 bytes -> 2k
{
	ASSERT (pAlloc != NULL);
    StringPool::s_singleton = this;
}

void String::FreeData(StringData* pData)
{
	// Pass pointer and data size to Free() call
	StringPool*	pPool = (StringPool*)pData->pAlloc;
	if (pData->nAllocLength)
		pPool->Free (pData);

}

void StringPool::AllocString(String& str, size_t newLen)
{
	VX_ASSERT (newLen >= 0);
	VX_ASSERT (newLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	size_t	requestSize = newLen + 1;	// count the extra null
//	size_t	curLen = str.GetLength();

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
//	size_t		curLen = GetLength();
	StringData*	pData;
	
	if (IsEmpty())
        pPool = StringPool::Get();
	else
	{
		pData = GetData();
		pPool = pData->pAlloc;
	}
		
	// Allocate memory from appropriate pool once determined.
    size_t  data = sizeof(StringData);
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


}	// end Core
}	// end Vixen
