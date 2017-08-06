
namespace Vixen {

	class ComputeThreadPool : public Core::ThreadPool
	{
	public:
		ComputeThreadPool(int nthreads = 0, int opts = THREAD_TYPE_WORKER);

		virtual bool	WaitAll(int type = -1);
		virtual void	KillAll(bool wait = false);
		virtual int		AddTask(Engine* engine, int tasktype);
		virtual int		NextTask(ComputeThread& thread);
		virtual int		NumTasks() const	{ return (int) m_TaskQueue.GetSize(); }
		float			Time;

		enum
		{
			DATA_PARALLEL = 1,
			TASK_PARALLEL = 2,
			THREAD_TYPE_MAIN = 1,
			THREAD_TYPE_WORKER = 2,
		};
	protected:
		Array<Engine* volatile>	m_TaskQueue;
		IntArray		m_TaskTypes;
	};

	class ComputeThread : public Core::Thread
	{
	public:
		ComputeThread(ComputeThreadPool& threadpool, int threadopts = THREAD_TYPE_WORKER, int index = 0);
		virtual bool			Resume();
		static Core::ThreadFunc	ComputeThreadFunc;
		enum
		{
			THREAD_TYPE_MAIN = 1,
			THREAD_TYPE_WORKER = 2,
		};
		ComputeThreadPool&	ThreadPool;
		int32				NextTask;
	};

}	// end Vixen