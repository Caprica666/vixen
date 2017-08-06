
#include "vcore/vcore.h"

namespace Vixen {

DebugOut& DebugOut::operator<<(const TCHAR* s)	{ fputs(s, stdout); return *this; }
DebugOut& DebugOut::operator<<(TCHAR c)			{ putchar(c); return *this; }
DebugOut& DebugOut::operator<<(int v)			{ Core::String s(v); fputs(s, stdout); return *this; }
DebugOut& DebugOut::operator<<(float v)			{ Core::String s(v); fputs(s, stdout); return *this; }
DebugOut& DebugOut::operator<<(void* p)			{ Core::String s((int) p); fputs(s, stdout); return *this; }
DebugOut& endl(DebugOut& out)					{ putchar('\n');  return out; }


static DebugOut error_out;
DebugOut& vixen_debug = error_out;

namespace Core {
	
double	GetTime()
{
	float t = emscripten_get_now() / 1000.0f;
	return t;
}

/*!
 * @fn bool CoreInit()
 *
 * Initializes the core features. After this call, allocators and run-time typing
 * will be available. It should be called before instantiating anything derived
 * from BaseObj.
 *
 * @relates BaseObj
 */
extern	StringPool*	_vStringPool;
extern	StringData	_vInitThreadData[];
extern	LPCTSTR		_vEmptyStr;
char	galloc[sizeof(GlobalAllocator)];
TLSData	TLSData::t_TLSData;

bool _cdecl CoreInit()
{
	if (GlobalAllocator::s_ptheOneAndOnly)
		return false;					// cannot call this one twice
	GlobalAllocator* globalalloc = (GlobalAllocator*) galloc;
	new ((void*) globalalloc) GlobalAllocator;
	GlobalAllocator::s_ptheOneAndOnly = globalalloc;
	if (PoolAllocator::s_ptheOneAndOnly)
		return false;					// cannot call this one twice
	PoolAllocator::s_ptheOneAndOnly = new (GlobalAllocator::s_ptheOneAndOnly) PoolAllocator;

	_vThreadData = (StringData*)&_vInitThreadData;
	_vEmptyStr = _vThreadData->data();
	_vStringPool = new StringPool(GlobalAllocator::Get);
	_vInitThreadData[0].pAlloc = _vStringPool;
#ifndef VX_NOTHREAD
	Class* csec = CLASS_(CritSec);
	csec->SetAllocator(&(vtls->GetLockPool));
#endif
	Class::LinkClassHierarchy();
	return true;
}

void _cdecl CoreExit()
{
	if (GlobalAllocator::s_ptheOneAndOnly == NULL)
		return;
	_vStringPool->FreeAll();
	delete _vStringPool;
	_vStringPool = NULL;
	delete PoolAllocator::s_ptheOneAndOnly;
	GlobalAllocator::s_ptheOneAndOnly->FreeAll();
	free(GlobalAllocator::s_ptheOneAndOnly);
	PoolAllocator::s_ptheOneAndOnly = NULL;
	GlobalAllocator::s_ptheOneAndOnly = NULL;
}

void FixedLenAllocator::Free(void* aPtr)
{
#ifdef _DEBUG
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
}

}	// end Core
}	// end Vixen

