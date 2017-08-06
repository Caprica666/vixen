/*!
 * @file thread-win.h
 *
 * Windows-specific threading support. These classes require
 * the Windows headers to compile because they rely on
 * Windows operating system objects.
 *
 * If VX_NOTHREAD is defined, the scene manager is built with
 * single threaded runtime libraries and cannot use
 * threads at all
 *
 * @ingroup vcore
 * @see thread-x.h vlock-win.h
 */

#pragma once

namespace Core {

//	ThreadFunc	-> function executed by thread
typedef void _cdecl ThreadFunc(void* arg);
#define	VX_TimeOut	10000

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
	Semaphore(uint32 timeout = VX_TimeOut) { Handle = 0; }
	~Semaphore()		{ }
	int		Wait()		{ return 0; }
	void	Release()	{ }

	void*	Handle;
};

#else	// VX_NOTHREAD

#define	VX_WaitTimeOut	STATUS_TIMEOUT

class Semaphore
{
public:
	Semaphore(uint32 timeout = VX_TimeOut);
	~Semaphore();
	int			Wait();
	void		Release();
	static bool	WaitAll(Semaphore** thread_handles, int n);
	static bool	WaitAny(Semaphore** semaphores, int n);

	uint32		TimeOut;
	void*		Handle;
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
	~Thread();
//! Set function to run when new thread is spawned.
	void			SetThreadFunc(ThreadFunc* func)	{ m_ThreadFunc = func; }
//! Run this thread with the given thread function.
	virtual void	Run(ThreadFunc* = NULL);
//! Suspend this thread, wait for resume event to be signalled.
	virtual bool	Suspend();
//! Resume this thread, signal a resume event.
	virtual bool	Resume();
//! Signal this thread is done.
	virtual void	SignalDone();
//! Stop this thread, signal done.
	virtual void	Stop()					{ InterlockSet(&m_IsRunning, 0); SignalDone(); }
//! Return event indicating thread is done.
	Semaphore*		GetDoneEvent();
//! Return thread options.
	int				GetOptions() const		{ return m_Options; }
//! Return thread handle, NULL if thread is not running.
	void*			GetThreadHandle() const	{ return m_ThreadHandle; }
//! Return zero-based thread index
	int				GetThreadIndex() const	{ return m_ThreadIndex; }
//! Returns \b true if thread is running.
	bool			IsRunning() const		{ return m_IsRunning != 0; }	
//! Returns the handle to the currently running thread.
	static void*	GetCurrentThread();
//! Returns number of hardware processors
	static int		GetNumProcessors();

protected:
	vint32			m_IsRunning;	// true when thread is running
	int				m_Options;		// thread options
	int				m_ThreadIndex;	// 0-based index of thread
	ThreadFunc*		m_ThreadFunc;	// function to run
	Semaphore		m_DoneEvent;	// event indicating thread has finished
	Semaphore		m_ResumeEvent;	// event indicating thread should resume
	void*			m_ThreadHandle;	//!< handle to thread
};


#endif // VX_NOTHREAD

} // end Core