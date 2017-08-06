/*!
 * @file vlock.h
 *
 * @brief Reference counted, locking object.
 * This class provides support for thread-safe locking of an object
 * using operating system semaphores.
 *
 * @author Nola Donato
 * @ingroup vcore
 * @see vobj.h vref.h
 */

#pragma once

namespace Core {

class CritSec;

/*!
 * @class LockObj
 * @brief Base class to provide object level locking.
 *
 * @ingroup vcore
 * @see BaseObj RefObj VRef Core::Lock
 */
#ifdef VX_NOREFCOUNT
class LockObj : public BaseObj
#else
class LockObj : public RefObj
#endif
{
public:
	friend class ObjLock;
	VX_DECLARE_CLASS(LockObj);

#ifdef VX_NOREFCOUNT
	void	Delete() { }
	void	IncUse() { }
#endif

#ifdef VX_NOTHREAD
	bool		MakeLock() const { return true; }
	void		KillLock() const { }
	int			Lock() const { return 1; }
	int			Unlock() const { return 0; }
	bool		TryLock() const { return true; }
#else
#ifdef VX_NOREFCOUNT
	LockObj() : BaseObj() { m_Lock = 0; }
#else
	LockObj() : RefObj() { m_Lock = 0; }
#endif
	~LockObj()	{ KillLock(); }					//!< Kill lock on destruction
	bool		MakeLock() const;				//!< Enable locking and make a lock
	void		KillLock() const;				//!< Disable locking and delete the lock
	bool		Lock() const;					//!< Lock this object, wait until success
	bool		Unlock() const;					//!< Unlock this object
	bool		TryLock() const;				//!< Try to lock this object, return on failure

	mutable CritSec* volatile m_Lock;			//!< object lock
#endif
};


/*!
 * @class ObjLock
 * @brief Internal class used to lock around object updates
 * or accesses of shared data structures.
 *
 * @ingroup vcore
 * @see LockObj::Lock LockObj::Unlock
 */
#ifdef VX_NOTHREAD
class ObjLock
{
public:
	ObjLock(LockObj* optr) : ObjPtr(optr) {}
	ObjLock(const LockObj* optr) : ObjPtr(optr) {}
	void Unlock() {}
	~ObjLock() {}
protected:
	const LockObj* ObjPtr;
};

#else
class ObjLock
{
public:
	ObjLock(LockObj* optr) : ObjPtr(optr)
	{ if (optr) { optr->Lock(); m_Lock = optr->m_Lock; } else m_Lock = NULL; }

	ObjLock(const LockObj* optr) : ObjPtr(optr)
	{ if (optr) { optr->Lock(); m_Lock = optr->m_Lock; } else m_Lock = NULL; }

	~ObjLock()	{ Unlock(); }

	void Unlock()
	{
		if (CritSec::DoLock && m_Lock && m_Lock->m_LockCount > 0)
			m_Lock->Leave();
	}

protected:
	const LockObj* ObjPtr;
	CritSec* volatile m_Lock;

};
#endif

} // end Core