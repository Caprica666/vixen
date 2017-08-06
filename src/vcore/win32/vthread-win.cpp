#include "vcore/vcore.h"
#include "vcore/vlock.h"
#include <process.h>

namespace Vixen {
namespace Core {

#ifndef VX_NOTHREAD

/*!
 * @fn void ThreadPool::SetPriority(int p)
 * @param p	priority to set for threads (0 is lowest, 4 is highest)
 *
 * Sets the thread priority of all of the threads in this pool
 */
void ThreadPool::SetPriority(int p)
{
	int		i = 0, tp = THREAD_PRIORITY_NORMAL;
	Thread*	thread = m_Threads;

	switch (p)
	{
		case 0: tp = THREAD_PRIORITY_IDLE; break;
		case 1: tp = THREAD_PRIORITY_BELOW_NORMAL; break;
		case 2: tp = THREAD_PRIORITY_NORMAL; break;
		case 3: tp = THREAD_PRIORITY_ABOVE_NORMAL; break;
		case 4: tp = THREAD_PRIORITY_HIGHEST; break;
	}
	while (thread)
	{
		if (thread->m_ThreadHandle)
			::SetThreadPriority(thread->m_ThreadHandle, tp);
		thread = (Thread*) thread->Next;
	}
}


Thread::Thread(int opts)
	: m_Options(opts)
{
	m_ThreadHandle = NULL;
	m_IsRunning = false;
	m_ThreadFunc = NULL;
	m_ThreadIndex = 0;
}

Thread::~Thread()
{
}

/*!
 * @fn void Thread::Run(ThreadFunc* func)
 * @param func	Thread function to execute.
 *
 * Creates and executes a thread running the specified function.
 * The argument passed to the thread function is the
 * pointer to this Thread. If this function pointer is NULL,
 * the function specified by \b SetThreadFunc is used.
 *
 * @see Scene::EndThread Scene::InitThread ThreadPool::RunAll Thread::SetThreadFunc
 */
void Thread::Run(ThreadFunc* func)
{
	if (func == NULL)
		func = m_ThreadFunc;
	if (func == NULL)
		return;
	if (!m_IsRunning)
	{
		m_IsRunning = true;
		m_ThreadHandle = (HANDLE) _beginthread(func, 0, this);
	}
}

int Thread::GetNumProcessors()
{
#ifdef _OPENMP
	return omp_get_max_threads();
#elif VX_USE_CILK
	return __cilkrts_get_nworkers();
#else
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#endif
}

Semaphore*	Thread::GetDoneEvent()
{
	return &m_DoneEvent;
}

void* Thread::GetCurrentThread()
{
	intptr threadid = ::GetCurrentThreadId();
	return (void*) threadid;
}

Semaphore::Semaphore(uint32 timeout)
{
	TimeOut = timeout;
	Handle = ::CreateSemaphore(NULL, 0, 1, NULL);
}

Semaphore::~Semaphore()
{
	::CloseHandle(Handle);
}

int	Semaphore::Wait()
{
	DWORD err = ::WaitForSingleObject(Handle, TimeOut);
	if (err == 0)
		return 0;
	if (err == WAIT_TIMEOUT)
	{
		VX_TRACE(1, ("Semaphore::Wait Timeout expired"));
		return VX_WaitTimeOut;
	}
	return err;
}

void	Semaphore::Release()
{
	::ReleaseSemaphore(Handle, 1, NULL);
}

bool Semaphore::WaitAll(Semaphore** semaphores, int n)
{
	DWORD	err;

	if (n == 1)
		err = ::WaitForSingleObject(semaphores[0]->Handle, VX_TimeOut);
	else
	{
		HANDLE*	event_handles = (HANDLE*) alloca(n * sizeof(HANDLE));

		for (int i = 0; i < n; ++i)
			event_handles[i] = semaphores[i]->Handle;
		err = ::WaitForMultipleObjects(n, (HANDLE*) event_handles, TRUE, VX_TimeOut);
	}
	VX_TRACE(err == WAIT_TIMEOUT, ("Semaphore::WaitAll Timeout expired"));
	return (err == 0);
}

bool Semaphore::WaitAny(Semaphore** semaphores, int n)
{
	DWORD	err;

	if (n == 1)
		err = ::WaitForSingleObject(semaphores[0]->Handle, VX_TimeOut);
	else
	{
		HANDLE*		event_handles = (HANDLE*) alloca(n * sizeof(HANDLE));
		for (int i = 0; i < n; ++i)
			event_handles[i] = semaphores[i]->Handle;
		err = ::WaitForMultipleObjects(n, (HANDLE*) event_handles, FALSE, VX_TimeOut);
	}
	VX_TRACE(err == WAIT_TIMEOUT, ("Semaphore::WaitAny Timeout expired"));
	return (err == 0);
}

#endif


}	// end Core
}	// end Vixen