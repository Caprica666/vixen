#pragma once
namespace Core
{
#if 0
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
	virtual void*		Alloc (size_t amount);
	virtual void		AllocString(String& str, size_t newLen);
	virtual void		Free (void* ptr);
	virtual void		Free (void* ptr, size_t amount);
	virtual void		FreeAll();

	// Additional interfaces
	void				SetBaseAllocator (Allocator* pBaseAlloc);
	size_t				LastAllocSize();

	// Static notification callback
	static void			ResetPoolAllocator (Allocator* pDyingAllocator);

#ifdef _DEBUG
	// Debug statistic printout
	void				PrintDebugStats();
#endif

	static	int			Debug;

protected:
	size_t				m_lastAllocSize;
	Allocator*			m_pBaseAlloc;
	Allocator*			m_array[8];
	FixedLenAllocator	m_alloc32;
	FixedLenAllocator	m_alloc64;
	FixedLenAllocator	m_alloc128;
	FixedLenAllocator	m_alloc256;
	FixedLenAllocator	m_alloc512;
	FixedLenAllocator	m_alloc1024;
	FixedLenAllocator	m_alloc2048;
	GetAllocatorFunc	m_resolveMethod;
};

inline void StringPool::Free (void* ptr)	{ /* call Free (ptr, amount); */ VX_ASSERT(false); }
inline size_t StringPool::LastAllocSize()	{ return m_lastAllocSize; }

#else
/*!
 * @class StringPool
 *
 * @brief Internal class used to manage string allocations.
 *
 * @see String Allocator
 */
class StringPool : public PoolAllocator
{
public:
	StringPool(GetAllocatorFunc func);
	virtual void	AllocString(String& str, size_t newLen);
	static	int			Debug;
};

#endif

extern StringPool*	_vStringPool;

} // end Core
