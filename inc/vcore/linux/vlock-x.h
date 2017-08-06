/*!
 * @file vlock-x.h
 *
 * Unix-specific locking classes. These classes require
 * the Unix headers to compile because they rely on
 * Unix operating system objects. This code uses the pthreads API.
 *
 * @author Nola Donato
 * @see vlock-win.h
 */
#pragma once

#define G_BYTE_ORDER	G_LITTLE_ENDIAN

#ifdef VX_NOTHREAD	// non-threaded operation

namespace Core {

inline int32 InterlockGet(vint32* dest) { return *dest; }
inline void InterlockSet(vint32* dest, int32 newval) { *dest = newval; }
inline void InterlockSet(voidptr* dest, void* newval) { *dest = newval; }

inline void* InterlockExch(voidptr* dest, void* newval) { void* tmp = *dest; *dest = newval; return tmp; }

inline int32 InterlockInc(vint32* lpAddend)
{ return ++(*lpAddend); }

inline int32 InterlockDec(vint32* lpAddend)
{ return --(*lpAddend); }

inline int32 InterlockAdd(vint32* dst, int src) { *dst += src; return *dst; }

inline int InterlockTestSet(vint32* i, int set, int test)
{	if (*i == test) { *i = set; return 1; } else return 0; }

inline bool InterlockTestSet(voidptr* p, void* set, void* test)
{	if (*p == test) { *p = set; return 1; } else return 0; }


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

#include <pthread.h>
#include <semaphore.h>
#include <glib/gmacros.h>
#include <glib/gtypes.h>
#include <glib/gatomic.h>

namespace Core {

inline int32 InterlockGet(vint32* dest)
{ return g_atomic_int_get(dest); }

inline void* InterlockGet(void** dest)
{ return g_atomic_pointer_get(dest); }

inline void InterlockSet(void* volatile * dest, void* newval)
{ return g_atomic_pointer_set(dest, newval); }

inline void* InterlockExch(vptr* dest, void* newval)
{ return (void*) _InterlockedExchange((intptr *) dest, (intptr) newval);}

inline void InterlockSet(vint32* dest, int32 newval)
{ g_atomic_int_set(dest, newval); }

inline void InterlockSet(size_t volatile* dest, size_t newval)
{ g_atomic_int_set(dest, newval); }

inline bool InterlockTestSet(vint32* i, int32 set, int32 test)
{ return g_atomic_int_compare_and_exchange(i, test, set); }

inline bool InterlockTestSet(vptr* i, void* set, void* test)
{ return g_atomic_pointer_compare_and_exchange(i, test, set); }

inline int32 InterlockInc(vint32* lpAddend)
{ return g_atomic_int_exchange_and_add(lpAddend, 1); }

inline int32 InterlockInc(intptr volatile* lpAddend)
{ return g_atomic_pointer_exchange_and_add(lpAddend, 1); }

inline int32 InterlockDec(vint32* lpAddend)
{ return g_atomic_int_exchange_and_add(lpAddend, -1); }

inline int32 InterlockDec(intptr volatile* lpAddend)
{ return g_atomic_pointer_exchange_and_add(lpAddend, -1); }

inline int32 InterlockAdd(vint32* Addend, int Value)
{ return g_atomic_int_exchange_and_add(Addend,Value); }

inline size_t InterlockAdd(intptr volatile* Addend, int Value)
{ return g_atomic_pointer_exchange_and_add(Addend,Value); }

class CritSec : BaseObj
{
public:
	VX_DECLARE_CLASS(CritSec);
	CritSec();
	~CritSec();
	void	Enter();
	void	Leave();
	void	Kill();

	static	bool	DoLock;

	pthread_mutex_t		Handle;
	intptr				OwnerThread;
	long				m_LockCount;
	intptr				m_EnterThread;
};

#endif // VX_NOTHREAD

inline void InterlockOr(vint32* i, int x)
{	int o, n; do { o = *i; n = o | x; } while (!InterlockTestSet(i, n, o)); }

inline void InterlockAnd(vint32* i, int x)
{	int o, n; do { o = *i; n = o & x; } while (!InterlockTestSet(i, n, o)); }

class Lock
{
public:
	Lock(CritSec& lock) : m_Lock(lock) { lock.Enter(); }
	Lock(CritSec* lock) : m_Lock(*lock) { lock->Enter(); }
	~Lock() { m_Lock.Leave(); };

protected:
	CritSec&	m_Lock;
};

}	// end Core

