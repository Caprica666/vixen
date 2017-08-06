#pragma once

/*!
 * @file vthread.h
 * @brief Thread handling classes.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 */

namespace Core {

#ifndef VX_NOTHREAD

class Thread;

/*!
 * @class ThreadPool
 * @brief Encapsulates a pool of threads running simultaneously.
 *
 * The thread pool may be independently working threads or they can be coordinated.
 *
 * @ingroup vcore
 * @see Thread
 */
class ThreadPool
{
public:
	ThreadPool() { m_Threads = NULL; NumThreads = 0; DoExit = 0; Synchronized = false; }
	~ThreadPool();
//! Run all threads in the pool using the given thread function.
	virtual void	RunAll(ThreadFunc* = NULL);
//! Resume all threads of given type after suspension.
	virtual bool	ResumeAll(int type = -1);
//! Wait for all threads of the given type to signal done.
	virtual bool	WaitAll(int type = -1);
//! Wait for any one of the threads of the given type to signal done.
	virtual bool	WaitAny(int type = -1);
//! Kill all threads, optionally wait for them to finish first.
	virtual void	KillAll(bool wait = false);
//! Empty the thread pool (assumes all threads have stopped)
	virtual	void	Empty();
//! Add a thread to the pool.
	virtual void	Add(Thread*);
//! Find a thread by index.
	virtual Thread*	Find(int index);
// Remove a thread from the pool and delete it.
//	void	Kill(Thread*);
//! Return number of threads in the pool.
	int		GetNumThreads() const;
//! Set thread priority for all threads in the pool
	void	SetPriority(int p);

	bool	Synchronized;	//!< True if threads in the pool are synchronized.
	vint32	DoExit;			//!< Increment to start shutdown
	vint32	NumThreads;		//!< number of running threads
protected:
	Thread*	m_Threads;		// child threads
};
#endif // VX_NOTHREAD

} // end Core