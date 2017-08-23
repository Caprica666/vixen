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
	SharedStringPool (Allocator* allocator) : StringPool(allocator)
	{
		s_singleton = this;
	}

	void	Lock()						{ m_Lock.Enter(); }
	void	Unlock()					{ m_Lock.Leave(); }
	virtual void*	Alloc (size_t n)	{ m_Lock.Enter(); void* p = StringPool::Alloc(n); m_Lock.Leave(); return p; }
	virtual void	Free (void* ptr)	{ m_Lock.Enter(); StringPool::Free(ptr); m_Lock.Leave(); }
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
	if (s_ObjectAllocator)
		delete s_ObjectAllocator;
#ifndef VX_NOTHREAD
	CritSec::DoLock = false;	// disable locking
#endif
}

bool World::Startup()
{
	if (s_ObjectAllocator)
		return true;
	CoreInit();
	s_OnlyOne = NULL;
	BufMessenger::s_OnlyOne = NULL;

#ifdef VX_NOTHREAD
	s_ObjectAllocator = new PoolAllocator(GlobalAllocator::Get(), 64, 6);

#else
	/*
	 * replace old string pool with locking version
	 * replace object allocator with locking one
	 */
	CritSec::DoLock = true;
	s_ObjectAllocator = new PoolAllocator(GlobalAllocator::Get(), 64, 6, ALLOC_Lock);

#ifdef _DEBUG
//	_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF );
#endif
#endif
	CLASS_(BaseObj)->SetAllocator(s_ObjectAllocator);
	return true;
}

} // end Vixen