#pragma once

namespace Core {

/*!
 * @file vlock-win.h
 *
 * Windows specific locking classes. These classes require
 * the Windows headers to compile because they rely on
 * Windows operating system objects.
 *
 * @author Nola Donato
 * @see vlock-x.h
 */
inline void InterlockSet(vint32* dst, int32 val)
{ InterlockedExchange((vuint32*) dst, val); }

inline int32 InterlockExch(vint32* dst, int32 val)
{ return InterlockedExchange((vuint32*) dst, val); }

inline int32	InterlockInc(vint32* dst)
{ return InterlockedIncrement((vuint32*) dst); }

inline int32	InterlockDec(vint32* dst)
{ return InterlockedDecrement((vuint32*) dst); }

inline bool	InterlockTestSet(vint32* dst, int32 set, int32 test)
{
	int32 result = InterlockedCompareExchange((vuint32*) dst, set, test);
	return result == test;
}

inline bool	InterlockTestSet(voidptr* dst, voidptr set, voidptr test)
{ return (test == InterlockedCompareExchangePointer(dst, set, test)); }

inline void* InterlockExch(voidptr* dst, voidptr val)
{ return InterlockedExchangePointer(dst, val); }

inline void InterlockSet(voidptr* dst, voidptr val)
{ InterlockedExchangePointer(dst, val); }

#if defined(_M_X64) || defined(_Wp64)

inline int32	InterlockAdd(vint32* dst, int32 val)
{ return InterlockedAdd((volatile long*) dst, val); }

inline void	InterlockAnd(vint32* dst, int32 val)
{ InterlockedAnd((volatile long*) dst, val); }

inline void	InterlockOr(vint32* dst, int32 val)
{ InterlockedOr((volatile long*) dst, val); }

inline int32	InterlockXor(vint32* dst, int32 val)
{ return InterlockedXor((volatile long*) dst, val); }

inline void InterlockSet(vint64* dst, int64 val)
{ InterlockedExchange64(dst, val); }

inline int64 InterlockExch(vint64* dst, int64 val)
{ return InterlockedExchange64(dst, val); }

inline int64	InterlockInc(vint64* dst)
{ return InterlockedIncrement64(dst); }

inline int64	InterlockDec(vint64* dst)
{ return InterlockedDecrement64(dst); }

inline int64	InterlockAdd(vint64* dst, int64 val)
{ return InterlockedAdd64(dst, val); }

inline void	InterlockAnd(vint64* dst, int64 val)
{ InterlockedAnd64(dst, val); }

inline void	InterlockOr(vint64* dst, vint64 val)
{ InterlockedOr64(dst, val); }

 inline void	InterlockXor(vint64* dst, int64 val)
{ InterlockedXor64(dst, val); }

inline bool	InterlockTestSet(vint64* dst, int64 set, int64 test)
{ return (test == InterlockedCompareExchange((vuint64*) dst, set, test)); }

#else
inline int32	InterlockAdd(vint32* dst, int32 val)
{
	int32 oldval, newval;
	do { oldval = *dst; newval = oldval + val; } while (!InterlockTestSet(dst, newval, oldval));
	return oldval;
}

inline void	InterlockAnd(vint32* dst, int32 val)
{
	int32 oldval, newval;
	do { oldval = *dst; newval = oldval & val; } while (!InterlockTestSet(dst, newval, oldval));
}

inline void	InterlockOr(vint32* dst, int32 val)
{
	int32 oldval, newval;
	do { oldval = *dst; newval = oldval | val; } while (!InterlockTestSet(dst, newval, oldval));
}


#endif

#ifdef VX_NOTHREAD	// non-threaded operation

class CritSec
{
public:
	CritSec()			{ Handle = 0; }
	~CritSec()			{ }
	void	Enter()		{ }
	void	Leave()		{ }
	void	Kill()		{ }
    void*	operator new(size_t, void*);

	static	bool	DoLock;

protected:
	intptr	Handle;
};

#else		// threaded operation

class CritSec : public BaseObj
{
public:
	VX_DECLARE_CLASS(CritSec);
	CritSec();
	~CritSec();
	void	Enter();
	void	Leave();
	void	Kill();

	static	bool	DoLock;

	CRITICAL_SECTION	Handle;
	intptr				OwnerThread;
	intptr				m_EnterThread;
	vint32				m_LockCount;
};

#endif // VX_NOTHREAD

class Lock
{
public:
	Lock(CritSec& lock) : m_Lock(lock) { lock.Enter(); }
	Lock(CritSec* lock) : m_Lock(*lock) { lock->Enter(); }
	~Lock() { m_Lock.Leave(); };

protected:
	CritSec&	m_Lock;
};

} // end Core