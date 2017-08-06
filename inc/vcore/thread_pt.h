/*!
 * @file thread-pt.h
 *
 * These classes abstract threading constructs in an
 * operating system independent way but may not work
 * on all platforms. This specific header is compatible
 * with the Unix-specific implementation (pthread API).
 *
 * If VX_NOTHREAD is defined, the scene manager is built with
 * single threaded runtime libraries and cannot use
 * threads at all
 *
 * @see thread-win.h vlock-x.h
 */

#pragma once

namespace Core {
/*!
 * Type defining a worker callback for a thread pool.
 * This declaration is compatible with the pthread callback type.
 */
typedef void* ThreadFunc(void* arg);

#ifdef VX_NOTHREAD

/*!
 * @class Semaphore
 * Wrapper for operating system semaphore used internally to synchronize
 * the operation of cooperating threads.
 *
 * @ingroup vcore
 *
 * @see CritSec
 */
class Semaphore
{
public:
	Semaphore(uint32 timeout = VX_TimeOut)	{ Handle = 0; }
	~Semaphore()	{ }
	int		Wait()	{ return 0; }
	void	Release()	{ }

	void *Handle;
};

#else	// VX_NOTHREAD

class Semaphore
{
public:
	Semaphore();
	~Semaphore();
	int			Wait();
	void		Release()	{ pthread_cond_signal( &Signal ); }

	static bool	WaitAll(Semaphore** thread_handles, int n);
	static bool	WaitAny(Semaphore** semaphores, int n);

	uint32		TimeOut;
	pthread_mutex_t	Handle;
	pthread_cond_t	Signal;
};


/*!
 * @class Thread
 * @brief Encapsulates a working thread that can be stopped and started
 * externally using operating system signal events.
 *
 * @ingroup vcore
 *
 * @see ThreadPool
 */
class Thread : public List
{
	friend class ThreadPool;
public:
//! Construct a thread object, save the thread options.
	Thread(int opts = 0);
//! Set function to run when new thread is spawned.
	void			SetThreadFunc(ThreadFunc* func)	{ m_ThreadFunc = func; }
//! Run this thread with the given thread function.
	virtual void	Run(ThreadFunc* = NULL);
//! Suspend this thread, wait for resume event to be signalled.
	virtual bool	Suspend();
//! Resume this thread, signal a resume event.
	virtual bool	Resume();
//! Stop this thread, signal a done event.
	virtual void	Stop()					{ m_IsRunning = false; SignalDone(); }
//! Signal this thread is done.
	virtual void	SignalDone();
//! Return event indicating thread is done.
	Semaphore*		GetDoneEvent();
//! Return thread options.
	int				GetOptions() const		{ return m_Options; }
//! Return thread handle, NULL if thread is not running.
	void*			GetThreadHandle() const	{ return (void*)m_ThreadHandle; }
//! Return zero-based thread index
	int				GetThreadIndex() const	{ return  m_ThreadIndex; }
//! Returns \b true if thread is running.
	bool			IsRunning() const		{ return m_IsRunning; }	
//! Returns the handle to the currently running thread.
	static void*	GetCurrentThread();

protected:
	bool			m_IsRunning;
	int				m_Options;		//!< thread options
	int				m_ThreadIndex;	// 0-based index of thread
	ThreadFunc*		m_ThreadFunc;	// function to run
	Semaphore		m_DoneEvent;	// event indicating thread has finished
	Semaphore		m_ResumeEvent;	// event indicating thread should resume
	pthread_t		m_ThreadHandle;	//!< handle to thread
};

#define	VX_TimeOut	10000
#define	VX_WaitTimeOut	10000
#endif // VX_NOTHREAD

}	// end Core
