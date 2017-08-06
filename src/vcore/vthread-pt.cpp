#include "vcore/vcore.h"


namespace Vixen {
namespace Core {
#ifndef VX_NOTHREAD

Semaphore::Semaphore()
{
	pthread_mutex_init(&Handle, NULL);
	pthread_cond_init(&Signal, NULL);
}

Semaphore::~Semaphore()
{
	pthread_mutex_destroy(&Handle);
	pthread_cond_destroy(&Signal);
}

int	Semaphore::Wait()
{
	int		rc = pthread_cond_wait(&Signal, &Handle);

	if (rc == ETIMEDOUT)
		return VX_TimeOut;
	return rc;
}

bool Semaphore::WaitAll(Semaphore** thread_handles, int nhandles)
{
	// i = max,
	// n = number left
	// m = counter, current one we're trying
	int m, n;
	n = nhandles;
	while (n > 0)
	{
		for (int m = 0; m < nhandles; ++m)
		{
			if( thread_handles[m] )
			{
				if (thread_handles[m]->Wait() >= 0)	// decremented sem
				{
					n--;
					thread_handles[m] = NULL;
				}
			}
		}
	}
	// TODO: implement a timeout if it takes too long
	return true;
}

bool Semaphore::WaitAny(Semaphore** thread_handles, int nhandles)
{
	// i = max,
	// n = number left
	// m = counter, current one we're trying
	int m, n;
	n = nhandles;
	while (n > 0)
	{
		for (int m = 0; m < nhandles; ++m)
		{
			if( thread_handles[m])
			{
				if (thread_handles[m]->Wait() >= 0)	// decremented sem
				{
					n--;
					thread_handles[m] = NULL;
					return true;
				}
			}
		}
	}
	// TODO: implement a timeout if it takes too long
	return false;
}

/*!
 * @fn void ThreadPool::SetPriority(int p)
 * @param p	priority to set for threads (0 is lowest, 4 is highest)
 *
 * Sets the thread priority of all of the threads in this pool
 */
void ThreadPool::SetPriority(int p)
{
#if 0	// TODO: implement for Linux - this is Windows implementation
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
		if (thread->ThreadHandle)
			::SetThreadPriority(thread->m_ThreadHandle, tp);
		thread = (Thread*) thread->Next;
	}
#endif
}

Thread::Thread(int opts)
	: m_Options(opts)
{
	m_ThreadIndex = 0;
	m_IsRunning = false;
	m_ThreadFunc = NULL;
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

	pthread_create(&m_ThreadHandle, NULL, func, this);
	m_IsRunning = true;
}

Semaphore*	Thread::GetDoneEvent()
{
	return &m_DoneEvent;
}

void* Thread::GetCurrentThread()
{
	pthread_t handle = pthread_self();
	return (void*) handle;
}

#endif

}	// end Core
}	// end Vixen
