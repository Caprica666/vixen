#pragma once

/*!
 * @file vtlsdata.h
 * @brief Implements thread local storage under Unix.
 *
 * Because operating systems differ as to how thread
 * specific storage is implemented, this class may not
 * be useable asis on some platforms. This API is compatible
 * with the pthread-specific implementation.
 *
 * @ingroup vcore
 */
namespace Core {
class Allocator;

/*!
 * @class AllocStack
 *
 * @brief Internal class used by allocators.
 *
 * @see VAllocator
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


/*!
 * @class StringPool
 *
 * @brief Internal class used to manage string allocations.
 *
 * @see String Allocator
 */
class StringPool : public Allocator
{
public:
	StringPool (GetAllocatorFunc pfGetAllocator);
	~StringPool();

	// Overridden allocator methods
	void*				Alloc (size_t amount);
	void				Free (void* ptr);
	void				Free (void* ptr, size_t amount);
	void				FreeAll();

	// Additional interfaces
	void				SetBaseAllocator (Allocator* pBaseAlloc);
	int					LastAllocSize();

#ifdef _DEBUG
	// Debug statistic printout
	void				PrintDebugStats();
#endif

protected:
	int					m_lastAllocSize;
	Allocator*			m_array[8];
	Allocator*			m_pBaseAlloc;
	FixedLenAllocator	m_alloc16;
	FixedLenAllocator	m_alloc32;
	FixedLenAllocator	m_alloc64;
	FixedLenAllocator	m_alloc128;
	FixedLenAllocator	m_alloc256;
	FixedLenAllocator	m_alloc512;
	FixedLenAllocator	m_alloc1024;
};

extern StringPool*	_vStringPool;

/*!
 * @class TLSData
 *
 * @brief Holder for all per-thread globals.
 *
 * @note this structure doesn't only need to hold public variables,
 * but also can have explicit methods which can be called.
 * The only thing to remember is to call TLSData::Get()->YourMethod() to
 * make sure that the underlying data is handled in a thread-isolated manner.
 *
 * @ingroup vcore
 */
class TLSData
{
public:

	// allocators
	voidptr				ThreadID;		//! thread ID of owning thread
	ThreadAllocator		ThreadAlloc;
	Allocator*			EventAlloc;
	AllocStack			FastStack;
#ifndef VX_NOTHREAD
	Allocator*			LockPool;		// critical section pool

	static Allocator*	GetLockPool();
#endif

	//! Return pointer to this thread's storage block, create one if it does not exist.
	static TLSData*		Get();

	//! Initializes a new thread local storage area.
	TLSData();

private:
	THREAD_LOCAL TLSData	t_TLSData;
};


}	// end Core
