#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*!
 * @file vtlsdata.h
 * @brief Implements thread local storage under Windows.
 *
 * Because operating systems differ as to how thread
 * specific storage is implemented, this class is Windows-specific.
 *
 * @ingroup vcore
 * @author Kim Kinzie
 */
namespace Core {

class Allocator;
class String;

/*!
 * @class AllocStack
 *
 * @brief Internal class used by allocators.
 *
 * @see Allocator
 */
class AllocStack
{
public:
	AllocStack();

	// stack methods
	void				Push (Allocator* alloc);
	Allocator*			Pop();
	Allocator*			Top();

protected:
	Allocator*			m_stack[10];
	size_t				m_stackTop;
};

// Create thread-based heap (64K default mem) and initialize the stack
inline AllocStack::AllocStack()
{
	memset(&m_stack, 0, sizeof(m_stack)); 
	m_stackTop = -1;
}

inline void AllocStack::Push (Allocator* pAlloc)
{
	// Watch for overrun of the allocator stack
	VX_ASSERT(m_stackTop == -1 || m_stackTop < sizeof (m_stack)/sizeof(Allocator*));
	m_stack[++m_stackTop] = pAlloc;
}

inline Allocator* AllocStack::Pop()
{
	// Zero out previous memory pointer before pop operation
	Allocator* pAlloc = m_stack[m_stackTop]; 
	m_stack[m_stackTop--] = NULL; 
	
	// Since we push the CGlobalAllocator onto the stack during
	// construction, a value of zero indicates one too many pops.
	VX_ASSERT(m_stackTop >= 0);
	return pAlloc;
}

inline Allocator*	AllocStack::Top()				
{ 
	return m_stack[m_stackTop]; 
}

/*
 * Thread local storage for the Core layer
 */
class TLSData
{
public:
	voidptr				ThreadID;		//! thread ID of owning thread
	ThreadAllocator		ThreadAlloc;
	Allocator*			EventAlloc;
	AllocStack			FastStack;
#ifndef VX_NOTHREAD
	FixedLenAllocator	LockPool;		// critical section pool

	Allocator*			GetLockPool()	{ return &LockPool; }
#endif

	//! Return pointer to this thread's storage block, create one if it does not exist.
	static TLSData*		Get()			{ return &t_TLSData; }

	//! Initializes a new thread local storage area.
	TLSData();

private:
	THREAD_LOCAL TLSData	t_TLSData;
};

inline TLSData::TLSData()
	: ThreadAlloc(DEFAULT_TLS_HEAP_SIZE),
	FastStack(),
	EventAlloc(NULL),
#ifndef VX_NOTHREAD
	LockPool(sizeof(Core::CritSec)),
#endif
	ThreadID(0)
{
	FastStack.Push(&ThreadAlloc);
#ifndef VX_NOTHREAD
	LockPool.SetBlockAllocator(ThreadAllocator::Get());
#endif
}

} // end Core
