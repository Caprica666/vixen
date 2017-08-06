#include "vcore/vcore.h"
//#include "vcore/vlock.h"
//#include "asm/errno.h"

namespace Vixen {
namespace Core {

bool	CritSec::DoLock = false;

float GetTime()
{
	timeval tv;
	unsigned long tv_ms;
	gettimeofday( &tv, NULL );
	tv_ms = (tv.tv_sec * 1000) + tv.tv_usec;
	return (float)tv_ms * 0.001f;
}


#ifndef VX_NOTHREAD

VX_IMPLEMENT_CLASS(CritSec, BaseObj);


CritSec::CritSec()
{
	VX_ASSERT(DoLock);
	pthread_mutex_unlock(&Handle);
}

CritSec::~CritSec() { 
	Kill();
}

void	CritSec::Enter()
{
	if (DoLock)
	{
		int rc = pthread_mutex_trylock(&Handle);
		ulong id = pthread_self();
		if (rc == 0)
			return;
		if ((rc == EDEADLK) || (rc == EBUSY))
		{
			++ m_LockCount;
			return;
		}
		pthread_mutex_lock(&Handle);
	}
}


void	CritSec::Leave()
{
	if (DoLock)
	{
		--m_LockCount;
		pthread_mutex_unlock(&Handle);
	}
}

void	CritSec::Kill()
{
	if (DoLock)
		pthread_mutex_destroy(&Handle);
}


bool LockObj::TryLock() const
{
	if (m_Lock && (m_Lock->m_LockCount > 0))	// object is locked
		return false;
	Lock();
	return true;
}

/*
 * @fn int LockObj::Lock() const
 *
 * This function will wait until it is safe to access the object
 * and will not let others at it until PSObj::Unlock is called.
 * If locking is disabled globally or this object does not have
 * a lock, this function returns 0 and does nothing.
 *
 * Although it changes the locking status of the object,
 * we designate this function \b const so it can be used
 * for locking around read operations more easily.
 *
 * @return lock count for object, 0 if it could not be locked
 *
 * @see LockObj::Unlock LockObj::MakeLock LockObj::KillLock
 */
bool LockObj::Lock() const
{
	if (!CritSec::DoLock)
		return false;
	CritSec* plock = (CritSec*) m_Lock;
	if (plock == NULL)					// lock already created?
		return false;
	plock->Enter();						// wait for lock
	return true;
}

/*
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

	CritSec*		plock = new CritSec;	// allocate lock

	if (plock == NULL)					// could not allocate lock?
		VX_ERROR(("LockObj::MakeLock ERROR cannot make lock"), false);
	plock->m_LockCount = 0;
	InterlockTestSet((void* volatile*) &m_Lock, plock, NULL);

	return true;
}

/*
 * @fn bool LockObj::KillLock() const
 *
 * Detaches the operating system lock from the object,
 * freeing it to be reused. This also disables locking on the object.
 *
 * @see LockObj::MakeLock LockObj::Unlock LockObj::Lock
 */
void LockObj::KillLock() const
{
	if (!CritSec::DoLock)
	{
		m_Lock = NULL;
		return;
	}
	CritSec*	plock = (CritSec*) InterlockExch((void* volatile*) &m_Lock, NULL);

	if (plock == NULL)
		return;
	VX_ASSERT(plock->m_LockCount == 0);
// we do not need to delete the lock,
// it is deleted when this thread's storage is freed
}

/*
 * @fn int LockObj::Unlock() const
 *
 * This function indicates to others that we have finished
 * accessing an object. If locking is disabled globally, this
 * object does not have a lock, or it is not locked,
 * this function returns 0 and does nothing.
 *
 * @return lock count for object, 0 if completely unlocked
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

#endif
}
}
