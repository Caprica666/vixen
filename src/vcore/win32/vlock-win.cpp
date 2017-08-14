#include "vcore/vcore.h"
#include <process.h>
#include <MMSystem.h>

namespace Vixen {
namespace Core {

double GetTime()
{
	return (double) ::timeGetTime() * 0.001f;
}

bool	CritSec::DoLock = false;

#ifdef VX_NOREFCOUNT
VX_IMPLEMENT_CLASS(LockObj, BaseObj);
#else
VX_IMPLEMENT_CLASS(LockObj, RefObj);
#endif

#ifndef VX_NOTHREAD

VX_IMPLEMENT_CLASS(CritSec, BaseObj);

#ifdef _DEBUG

/*!
 * @fn CritSec::CritSec()
 * @brief Constructor for critical sections.
 *
 * The constructor saves the ID of the current thread when the critical
 * section is initialized for later error checking
 *
 * @see CritSec::Kill
 */
CritSec::CritSec()
{
	::InitializeCriticalSection(&Handle);
	m_LockCount = 0;
	m_EnterThread = 0;
	OwnerThread = ::GetCurrentThreadId();
	VX_TRACE2(CritSec::Debug, ("CritSec::Create(%p) thread = %d", this, OwnerThread));
}

CritSec::~CritSec()
{
	Kill();
}

/*!
 * @fn void CritSec::Kill()
 * @brief Destroys a critical sections.
 *
 * The saved ID of the constructing thread is compared against the current thread ID.
 * If they do not match, the critical section is not deleted.
 * It can only be deleted by the thread which created it.
 *
 * @see CritSec::CritSec
 */
void CritSec::Kill()
{
	if (CritSec::DoLock)
	{
		VX_ASSERT(m_LockCount == 0);
		VX_ASSERT(m_EnterThread == 0);
	}
	if (OwnerThread == ::GetCurrentThreadId())
	{
		::DeleteCriticalSection(&Handle);
		VX_TRACE2(CritSec::Debug, ("CritSec::Destroy(%p) Thread %d", this, OwnerThread));
	}
	OwnerThread = NULL;
	m_EnterThread = 0;
	m_LockCount = 0;
}

/*!
 * @fn void	CritSec::Enter()
 *
 * @brief Enters a critical section, waiting if necessary for all other
 * threads to leave it.
 *
 * The debug version saves the ID of the current
 * thread and maintains a count of the number of Enter calls
 * for later error checking
 *
 * @see CritSec::Leave
 */
void	CritSec::Enter()
{
	if (!DoLock)
		return;
	::EnterCriticalSection(&Handle);
	++m_LockCount;
	intptr id = ::GetCurrentThreadId();
	if (m_EnterThread == 0)
		m_EnterThread = id;
}

/*!
 * @fn void	CritSec::Leave()
 *
 * @brief Leaves a critical section, unlocking it so other threads can use it.
 *
 * @see CritSec::Enter
 */
void	CritSec::Leave()
{
	if (!DoLock)
		return;
	VX_ASSERT(m_LockCount > 0);
	--m_LockCount;
	intptr id = ::GetCurrentThreadId();
	VX_ASSERT(m_EnterThread == id);
	if (m_LockCount == 0)
		m_EnterThread = 0;
	::LeaveCriticalSection(&Handle);
}
#else	// non-DEBUG

CritSec::CritSec()
{
	VX_ASSERT(DoLock);
	::InitializeCriticalSection(&Handle);
}

CritSec::~CritSec()
	{ Kill(); }

void	CritSec::Enter()
{
	if (DoLock)
	{
		::EnterCriticalSection(&Handle);
		++m_LockCount;
	}
}

void	CritSec::Leave()
{
	if (DoLock)
	{
		 --m_LockCount;
		::LeaveCriticalSection(&Handle);
	}
}

void	CritSec::Kill()
{
	::DeleteCriticalSection(&Handle);
}

#endif _DEBUG

bool LockObj::TryLock() const
{
	Core::CritSec* lock = m_Lock;

	if (lock == NULL)
		return true;
	if (lock->m_LockCount > 0)	// object is locked
		return false;
	Lock();
	return true;
}

/*!
 * @fn bool LockObj::Lock() const
 *
 * This function will wait until it is safe to access the object
 * and will not let others at it until SharedObj::Unlock is called.
 * If locking is disabled globally or this object does not have
 * a lock, this function returns 0 and does nothing.
 *
 * Although it changes the locking status of the object,
 * we designate this function \b const so it can be used
 * for locking around read operations more easily.
 *
 * @return true if locked successfully, false if it could not be locked
 *
 * @see LockObj::Unlock LockObj::MakeLock LockObj::KillLock
 */
bool LockObj::Lock() const
{
	if (!CritSec::DoLock)
		return false;
	CritSec* plock = m_Lock;	// do we have a lock?
	if (plock == NULL)			// no, don't lock then
		return false;
	plock->Enter();				// wait for lock
	return true;
}

/*!
 * @fn bool LockObj::MakeLock() const
 *
 * Attaches an operating system lock to the object to enable thread-safe access.
 * This also enables locking on the object.
 *
 * @return \b true if successful, else \b false
 *
 * @see SharedObj::Unlock SharedObj::Lock
 */
bool LockObj::MakeLock() const
{
	if (m_Lock || !CritSec::DoLock)		// object already has lock?
		return false;

	CritSec*	plock = new CritSec;	// allocate lock

	if (plock == NULL)					// could not allocate lock?
		VX_ERROR(("LockObj::MakeLock ERROR cannot make lock"), false);
	plock->m_LockCount = 0;
	Core::InterlockSet((voidptr*) &m_Lock, plock);
	return true;
}
			
/*!
 * @fn bool LockObj::KillLock() const
 *
 * Detaches the operating system lock from the object,
 * freeing it to be reused. This also disables locking on the object.
 *
 * @see LockObj::MakeLock LockObj::Unlock LockObj::Lock
 */
void LockObj::KillLock() const
{
// we do not need to delete the critical section,
// it is deleted when this thread's storage is freed
	CritSec* cs = (CritSec*) InterlockExch((voidptr*) &m_Lock, NULL);
}

/*!
 * @fn bool LockObj::Unlock() const
 *
 * This function indicates to others that we have finished
 * accessing an object. If locking is disabled globally, this
 * object does not have a lock, or it is not locked,
 * this function returns false and does nothing.
 *
 * @return true if object was successfully unlocked, else false
 *
 * @see LockObj::Lock LockObj::MakeLock LockObj::KillLock
 */
bool LockObj::Unlock() const
{
	CritSec*plock = m_Lock;

	if (plock == NULL)
		return false;
	if (plock->m_LockCount > 0)
	{
		plock->Leave();
		return true;
	}
	return false;
}

#endif	// VX_NOTHREAD

}	// end Core
}	// end Vixen