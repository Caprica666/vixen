#include "vixen.h"

namespace Vixen {
using namespace Core;

#ifndef VX_NOTHREAD
/*
 * @class SharedStringPool
 * This subclass of string pool provides thread safety because
 * it locks around all string pool accesses. This is necessary
 * if you will have multiple threads making or deleting strings.
 * @internal
 * @ingroup vixenint
 */
class SharedStringPool : public StringPool
{
public:
	SharedStringPool (GetAllocatorFunc func) : StringPool(func) { }

	void	Lock()						{ m_Lock.Enter(); }
	void	Unlock()					{ m_Lock.Leave(); }
	virtual void*	Alloc (size_t n)	{ m_Lock.Enter(); void* p = StringPool::Alloc(n); m_Lock.Leave(); return p; }
	virtual void	Free (void* ptr)	{ m_Lock.Enter(); StringPool::Free(ptr); m_Lock.Leave(); }
	virtual void	FreeAll()			{ m_Lock.Enter(); StringPool::FreeAll(); m_Lock.Leave(); }
//	virtual void	Free (void* ptr, size_t amount)		{ m_Lock.Enter(); StringPool::Free(ptr, amount); m_Lock.Leave(); }
	virtual void	AllocString(String& str, size_t n)	{ m_Lock.Enter(); StringPool::AllocString(str, n); m_Lock.Leave(); }

protected:
	CritSec	m_Lock;
};
#endif

void _cdecl World::Shutdown()
{
	if (s_OnlyOne != NULL)
		s_OnlyOne->Delete();
	s_OnlyOne = NULL;
#ifndef VX_NOTHREAD
	CritSec::DoLock = false;	// disable locking
#endif
	CoreExit();
}

bool World::Startup()
{
	if (!CoreInit())
		return true;

	s_OnlyOne = NULL;
	BufMessenger::s_OnlyOne = NULL;

#ifdef VX_NOTHREAD
	Core::PoolAllocator* objalloc = (Core::PoolAllocator*) Core::PoolAllocator::Get();
	objalloc->SetOptions(ALLOC_ZeroMem);
	CLASS_(BaseObj)->SetAllocator(objalloc);

#else
	/*
	 * replace old string pool with locking version
	 */
//	delete _vStringPool;
	CritSec::DoLock = true;
//	_vStringPool = new SharedStringPool(GlobalAllocator::Get);
//	_vStringPool->SetOptions(ALLOC_Lock);
//	_vInitThreadData[0].pAlloc = _vStringPool;
//	_vThreadData = (StringData*) &_vInitThreadData[0];
//	_vEmptyStr = _vThreadData->data();
	/*
	 * replace object allocator with locking one
	 */
	PoolAllocator* objalloc = (PoolAllocator*) PoolAllocator::Get();
	BytePool*	pool = new (GlobalAllocator::Get()) BytePool(64, 64 * 64);
#ifdef _DEBUG
	pool->SetOptions(ALLOC_ZeroMem | ALLOC_Lock);
	objalloc->SetOptions(ALLOC_ZeroMem | ALLOC_Lock);
#else
	pool->SetOptions(ALLOC_Lock);
	objalloc->SetOptions(ALLOC_Lock);
#endif
	objalloc->InitPools(pool, 6);
	CLASS_(BaseObj)->SetAllocator(objalloc);

#ifdef _DEBUG
//	_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF );
#endif
#endif
	return true;
}

} // end Vixen