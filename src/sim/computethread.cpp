#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"

namespace Vixen {
#if defined(_WIN32) && !defined(VX_PTHREAD)
	#include "vcore/win32/vtlsdata.inl"
#else
	#include "vcore/vtlsdata_pt.inl"
#endif

ComputeThread::ComputeThread(ComputeThreadPool& threadpool, int threadopts, int index)
: Thread(threadopts), ThreadPool(threadpool)
{
	m_ThreadIndex = index;
	if (threadopts & THREAD_TYPE_MAIN)
	{
#if defined(_WIN32)
		m_ThreadHandle = GetCurrentThread();
#else
		m_ThreadHandle = (pthread_t)GetCurrentThread();
#endif
		m_IsRunning = true;
	}
}

#if defined(_WIN32) && !defined(VX_PTHREAD)
void ComputeThread::ComputeThreadFunc( void *arg )
#else
void* ComputeThread::ComputeThreadFunc( void *arg )
#endif
{
    _mm_setcsr(_mm_getcsr() | /*FTZ:*/ (1<<15) | /*DAZ:*/ (1<<6)); // Needed per thread
	VX_ASSERT(arg);
	ComputeThread&		thread = *((ComputeThread*) arg);
	ComputeThreadPool&	threadpool = thread.ThreadPool;
	Core::TLSData*		tls = Core::TLSData::Get();
	int					threadindex = thread.GetThreadIndex();
	bool				ismain = thread.GetOptions() & THREAD_TYPE_MAIN;

	thread.NextTask = 0;
	VX_ASSERT(thread.IsRunning());
	VX_TRACE2(Engine::Debug, ("ComputeThreadPool::Start %d", threadindex));
	while (!threadpool.DoExit)
	{
		if (threadpool.NextTask(thread) < 0)
		{
			if (ismain)
			{
				thread.NextTask = -1;
				break;
			}
			thread.SignalDone();
			VX_TRACE2(Engine::Debug, ("ComputeThreadPool::Suspend %d", threadindex));
			thread.Suspend();
		}
	}
	if (!ismain)
	{
		thread.Stop();
		VX_ASSERT(!thread.IsRunning());
		VX_TRACE2(Engine::Debug, ("ComputeThreadPool::End %d", threadindex));
	}
#if !defined(_WIN32) || defined(VX_PTHREAD)
	return NULL;
#endif
}

bool ComputeThread::Resume()
{
	NextTask = 0;
	return Core::Thread::Resume();
}

ComputeThreadPool::ComputeThreadPool(int nthreads, int opts)
{
	Time = 0;
	m_TaskQueue.MakeLock();
	DoExit = false;
	for (int i = 0; i < nthreads; ++i)
		Add(new ComputeThread(*this, opts, i));
}

void ComputeThreadPool::KillAll(bool wait)
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
		int				num_running = 0;
		ComputeThread*	thread = (ComputeThread*) m_Threads;
		DoExit = true;
		while (thread)
		{
			if (thread->IsRunning())
				++num_running;
			thread = (ComputeThread*) thread->Next;
		}
		if (num_running == 0)
			break;
	}
	delete m_Threads;
	m_Threads = NULL;
}

bool ComputeThreadPool::WaitAll(int type)
{
#if 1
	bool running = false;
	do
	{
		_mm_pause(); // Sleep(0);
		ComputeThread*	thread = (ComputeThread*) m_Threads;
		running = false;
		while (thread)
		{
			if (thread->NextTask >= 0)
				running = true;
			thread = (ComputeThread*) thread->Next;
		}
	}
	while (running);
#else
	bool rc = ThreadPool::WaitAll(type);
#endif
	m_TaskQueue.Empty();
	m_TaskTypes.Empty();
	return true;
}

int ComputeThreadPool::AddTask(Engine* engine, int tasktype)
{
	VX_TRACE2(Engine::Debug, ("ComputeThreadPool::Add: %s", engine->GetName()));
	m_TaskQueue.Lock();
	int taskindex = (int) m_TaskQueue.Append(engine);
	m_TaskTypes.Append(tasktype);
	m_TaskQueue.Unlock();
	return taskindex;
}


/*
 * Each thread executes a different engine, each gets a different task
 */
int ComputeThreadPool::NextTask(ComputeThread& thread)
{
	int		curtask;

	if (thread.NextTask < 0)					// no more tasks for this thread?
		return -1;
	m_TaskQueue.Lock();
	while ((curtask = thread.NextTask++) < NumTasks())	// try to claim the next task
	{
		int32	tasktype = m_TaskTypes[curtask];
		Engine* eng = NULL;
		int		tid = thread.GetThreadIndex();
		int		n = NumThreads;

		if (tasktype == TASK_PARALLEL)			// only one thread should work on this task?
		{
			eng = (Engine*) Core::InterlockExch((vptr*) m_TaskQueue[curtask], NULL);
			tid = 0;
			n = 1;
		}
		else if	(tasktype == DATA_PARALLEL)		// all threads should work on this task?
			eng = (Engine*) m_TaskQueue[curtask];
		if (eng != NULL)						// if task not already claimed
		{
			m_TaskQueue.Unlock();
			eng->Eval(Time, tid, n);			// execute it
			return curtask;
		}
	}
	m_TaskQueue.Unlock();
	thread.NextTask = -1;
	return -1;
}


} // end Vixen

#endif
