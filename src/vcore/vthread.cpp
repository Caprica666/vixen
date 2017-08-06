#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

#ifndef VX_NOTHREAD
/*!
 * @fn void ThreadPool::Add(Thread* addme)
 * @param addme	Thread to add.
 *
 * Adds a thread to this thread pool. It will not
 * start executing until ThreadPool::RunAll is called.
 * Threads added after the pool has been started need
 * to be started by Thread::Run.
 *
 * @see ThreadPool::RunAll Thread::Run ThreadPool::KillAll
 */
void ThreadPool::Add(Thread* last)
{
	if (m_Threads == NULL)
	{
		m_Threads = last;
		last->Next = NULL;
		return;
	}
	m_Threads->Append(last);
}

/*!
 * @fn Thread* ThreadPool::Find(int index)
 * @param index	zero based index of thread to find
 *
 * Searches the thread list for the thread with ThreadIndex
 * equal to the input index.
 *
 * @return pointer to thread or NULL if not found.
 *
 * @see ThreadPool::Add Thread::GetThreadIndex
 */
Thread* ThreadPool::Find(int index)
{
	Thread* thread = m_Threads;

	while (thread)
	{
		if (thread->GetThreadIndex() == index)
			return thread;
		thread = (Thread*) thread->Next;
	}
	return NULL;
}

ThreadPool::~ThreadPool()
{
	KillAll(true);
}

/*!
 * @fn void ThreadPool::RunAll(ThreadFunc* func)
 * @param func	Thread function to execute.
 *
 * Start all the threads in this pool. The default implementation
 * executes them with the given thread function.
 *
 * @see Thread::Run ThreadPool::KillAll ThreadPool::WaitAll
 */
void ThreadPool::RunAll(ThreadFunc* func)
{
	Thread*	thread = m_Threads;

	InterlockSet(&NumThreads, 0);
	while (thread)						// start worker threads
	{
		if (!thread->IsRunning())
		{
			if (func)
				thread->Run(func);
			else
				thread->Run();
		}
		InterlockInc(&NumThreads);
		thread = (Thread*) thread->Next;
	}
}

/*!
 *
 * @fn bool ThreadPool::ResumeAll(int threadtype)
 *
 * Resumes all the threads of a given type by signalling
 * them individually. If the thread type is omitted, all
 * threads are resumed.
 *
 * @see Thread::Resume ThreadPool::WaitAll Thread::Suspend
 */
bool ThreadPool::ResumeAll(int threadtype)
{
	Thread*	thread = m_Threads;

	if (NumThreads == 0)
		return false;
	while (thread)
	{
		if ((threadtype == -1) || thread->GetOptions() & threadtype)
			thread->Resume();
		thread = (Thread*) thread->Next;
	}
	return true;
}

/*!
 * @fn bool ThreadPool::WaitAll(int opts)
 * @param threadtype	Thread types to check for.
 *
 * Waits for all one of the threads of the given type
 * to signal they are done (with their done event).
 * The done event is reset to the non-signalled state
 * by this function.
 *
 * @see ThreadPool::ResumeAll Thread::GetDoneEvent  Thread::SignalDone
 */
bool ThreadPool::WaitAll(int threadtype)
{
	Semaphore*	done_events[64];
	int			i = 0;
	Thread*		thread = m_Threads;
	while (thread)
	{
		if ((threadtype == -1) || (thread->GetOptions() & threadtype))
		{
			if (i >= 64)
				return false;
			if (thread->IsRunning())
				done_events[i++] = thread->GetDoneEvent();
		}
		thread = (Thread*) thread->Next;
	}
	if (i == 0)
		return false;
	return Semaphore::WaitAll(done_events, i);
}

/*!
 * @fn bool ThreadPool::WaitAny(int threadtype)
 * @param threadtype	Thread types to check for.
 *
 * Waits for any one of the threads of the given type
 * to signal it is done (with the done event).
 * The done event is reset to the non-signalled state
 * by this function.
 *
 * @see ThreadPool::ResumeAll Thread::GetDoneEvent  Thread::SignalDone
 */
bool ThreadPool::WaitAny(int threadtype)
{
	Semaphore*	done_events[64];
	int			i = 0;
	Thread*		thread = m_Threads;
	while (thread)
	{
		if ((threadtype == -1) || (thread->GetOptions() & threadtype))
		{
			if (i >= 64)
				return false;
			if (thread->IsRunning())
				done_events[i++] = thread->GetDoneEvent();
		}
		thread = (Thread*) thread->Next;
	}
	if (i == 0)
		return false;
	return Semaphore::WaitAny(done_events, i);
}

/*!
 * @fn void ThreadPool::Empty()
 *
 * Removes all the threads in the pool and reclaims their memory.
 * This function does nothing unless all of the threads have stopped.
 *
 * @see ThreadPool::ResumeAll  Thread::SignalDone ThreadPool::WaitAll
 */
void ThreadPool::Empty()
{
	if ((NumThreads == 0) && m_Threads)
	{
		m_Threads->Empty();
		delete m_Threads;
		m_Threads = NULL;
	}
}

/*!
 * @fn bool ThreadPool::KillAll(bool wait)
 * @param wait	If \b true, wait for threads to signal done.
 *
 * Kills all the threads in the pool and reclaims their memory.
 * If the \b wait option is used, threads are not killed
 * until they have signalled they are done.
 *
 * @see ThreadPool::ResumeAll  Thread::SignalDone ThreadPool::WaitAll
 */
void ThreadPool::KillAll(bool wait)
{
	if (DoExit)
		return;
	if (NumThreads == 0)
	{
		DoExit = true;
		return;
	}
	while (wait)
	{
		Semaphore*	done_events[64];
		int			i = 0;
		Thread*		thread = m_Threads;

		while (thread)
		{
			if (i >= 64)
				break;
			if (thread->IsRunning())
				done_events[i++] = thread->GetDoneEvent();
			thread = (Thread*) thread->Next;
		}
		VX_ASSERT(i == NumThreads);
		if (!InterlockTestSet(&DoExit, 1L, 0))
			return;
		if (i == 0)
			break;
		ResumeAll();
		Semaphore::WaitAll(done_events, i);
	}
	Empty();
	DoExit = true;
}


/*!
 * @fn int ThreadPool::GetNumThreads() const
 *
 * Returns the number of threads in this poolo.
 *
 * @see ThreadPool::RunAll ThreadPool::Add
 */
int ThreadPool::GetNumThreads() const
{
	if (m_Threads != NULL)
		return m_Threads->GetSize();
	return 0;
}

/*!
 * @fn bool Thread::Suspend()
 *
 * Suspends the thread if it is running, until the resume event occurs.
 *
 * @return \b true if thread was suspended, \b false on timeout or error
 *
 * @see Thread::Resume Thread::GetDoneEvent Thread::SignalDone
 */
bool Thread::Suspend()
{
	if (!IsRunning())
		return false;
	return (m_ResumeEvent.Wait() != VX_WaitTimeOut);
}

/*!
 * @fn bool Thread::Resume()
 *
 * Resumes the thread if it is suspended by signalling a resume event.
 *
 * @return \b true if thread was resumed, \b false on timeout or error
 *
 * @see Thread::Suspend Thread::SignalDone
 */
bool Thread::Resume()
{
	m_ResumeEvent.Release();
	return false;
}

/*!
 * @fn void Thread::SignalDone()
 *
 * Signals the thread has completed its task by triggering
 * the done event. This event is exposed and can be waited on
 * by requesting the event handle used for the done event.
 *
 * @see Thread::GetDoneEvent ThreadPool::WaitAll
 */
void Thread::SignalDone()
{
	m_DoneEvent.Release();
}
#endif

}	// end Core
}	// end Vixen