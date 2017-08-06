#include "vcore/vcore.h"

namespace Vixen {	

namespace Core {

#if defined(WIN32) && !defined(VX_NOTHREAD)
class LockAllocator : public BytePool
{
	friend class TLSData;
public:
	LockAllocator(size_t blockSize = 1024);

	virtual void	FreeAll();
	static void		FreeCritSec(void* p);
};

LockAllocator::LockAllocator(size_t blockSize)
 :	BytePool(sizeof(CritSec), blockSize)
{
}

void LockAllocator::FreeCritSec(void* p)
{
	CritSec* cs = (CritSec*) p;
	if (cs->GetAllocator())
		cs->Kill();
}

void LockAllocator::FreeAll()
{
	m_blocks->VisitAllElements(sizeof(CritSec), FreeCritSec);
	FixedLenAllocator::FreeAll();
}

#else

typedef BytePool LockAllocator;
#endif



//============================================================
// TLSData class
//============================================================

size_t	TLSData::s_maxofs = 0;
bool	TLSData::s_shutdown = 0;
intptr	TLSData::s_tlsSlot = 0;	
int		TLSData::Debug = 0;

TLSData* TLSData::Init()
{
	TLSData::s_tlsSlot = (intptr) TlsAlloc();
	TLSData::s_shutdown = false;
	VX_TRACE(Debug, ("TLSData::Init(%d)", ::GetCurrentThreadId()));
	return TLSData::Get();
}

void TLSData::Shutdown()
{ 
	VX_TRACE(Debug, ("TLSData::Shutdown(%d)", ::GetCurrentThreadId()));
	TempTLS::FreeAll();
#ifndef VX_NOTHREAD
	if (LockPool)
	{
		LockPool->FreeAll();
		delete LockPool;
	}
#endif	TLSData::Free();
	s_shutdown = TRUE;
	TLSData::s_maxofs = 0;
}

#ifndef VX_NOTHREAD
Allocator*	TLSData::LockPool = NULL;

Allocator*	TLSData::GetLockPool()
{
	if (LockPool == NULL)
	{
		LockPool = new LockAllocator();
		LockPool->SetOptions(ALLOC_Lock);
	}
	return LockPool;
}
#endif

TLSData::TLSData() 
  : ThreadAllocator(DEFAULT_TLS_HEAP_SIZE, ::GetCurrentThreadId()),
	FastStack(),
	m_tlsPool(ThreadAllocator::Get),
	EventAllocator(NULL),
#ifndef VX_NOTHREAD
//	LockPool(),
#endif
	m_tempPool(FastAllocator::Get)
{
	ThreadID = (void*) ::GetCurrentThreadId();
	FastStack.Push(&ThreadAllocator);
#ifndef VX_NOTHREAD
//	LockPool.SetBlockAllocator(ThreadAllocator::Get()); 
#endif
	memset(m_userData, 0, sizeof(m_userData));
	VX_TRACE(Debug, ("TLSData::Create(%d)", ThreadID));
}

TLSData::~TLSData()
{
}

size_t	TLSData::NextOffset(int n)
{
	size_t o = s_maxofs + (size_t) &((TLSData*) 0)->m_userData[0];
	n = (n + 3) & ~3;
	VX_ASSERT(n < TLS_MaxOffset);
	s_maxofs += n;
	return o;
}


void TLSData::Free()
{
	TLSData* tlsptr = GetPtr();
	if (tlsptr)
	{
		VX_TRACE(TLSData::Debug, ("TLSData::Free(%d)", tlsptr->ThreadID));
		if (tlsptr->EventAllocator)
		{
			delete tlsptr->EventAllocator;
			tlsptr->EventAllocator = NULL;
		}
#ifndef VX_NOTHREAD
//		tlsptr->LockPool.FreeAll();
#endif
		tlsptr->~TLSData();
		free(tlsptr);
		Set(NULL);
	}
}

vint32		TempTLS::s_numTemp = 0;
TLSData*	TempTLS::s_tempTls[TLS_MaxTemp];

TempTLS::TempTLS()
{
	TLSData* tlsdata = TLSData::GetPtr();	// get current TLS data
	if (tlsdata)							// have thread data already?
		return;								// leave existing TLS data alone

	void* threadid = (void*) ::GetCurrentThreadId();// get current thread ID
	for (int i = 0; i < s_numTemp; ++i)		// look for a pre-existing area
	{										// to reuse if possible
		tlsdata = s_tempTls[i];
		if (tlsdata &&						// ThreadID == NULL is unused
			InterlockTestSet(&(tlsdata->ThreadID), threadid, NULL))
		{
			TLSData::Set(tlsdata);			// grab this one and go
			return;
		}
	}
	VX_ASSERT(s_numTemp < TLS_MaxTemp);		// can we make another?
	tlsdata = TLSData::Get();				// make a new TLS area
	tlsdata->ThreadID = threadid;			// attach it to our thread
	s_tempTls[s_numTemp] = tlsdata;			// allow it to be reused later
	InterlockInc(&s_numTemp);				// bump number of reusable areas
}

TempTLS::~TempTLS()
{
	TLSData* tlsdata = TLSData::GetPtr();	// get TLS pointer
	if (tlsdata)
		tlsdata->ThreadID = NULL;			// free it for reuse
}

void TempTLS::FreeAll()
{
	int numtemp = InterlockExch(&s_numTemp, 0);
	for (int i = 0; i < numtemp; ++i)		// look for a pre-existing area
	{										// to reuse if possible
		TLSData* tlsdata = s_tempTls[i];
		if (tlsdata)						// ThreadID == 0 is unused
		{
			VX_ASSERT(tlsdata->ThreadID == NULL);
			s_tempTls[i] = NULL;
			tlsdata->Free();
		}
	}
}
}	// end Core
}	// end Vixen