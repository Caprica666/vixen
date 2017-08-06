/*!
 * @file vbufq.h
 *
 * @brief Thread safe buffer pools for input and output.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 * @see vxmess.h vxload.h
 */

#pragma once

namespace Core {

/*!
 * @brief Describes the state of a buffer in the queue.
 *
 * BUF_New, BUF_Locked and BUF_Used are for buffers that are being produced.
 * BUF_Ready and BUF_Pending are for buffers that are being consumed.
 * @see Buffer Buffer::State
 */
enum BufState
{
	BUF_Free = 0,		//!< buffer is free to be reused.
	BUF_New = 1,		//!< buffer has just been allocated.
	BUF_Locked = 2,		//!< buffer locked for write.
	BUF_Used = 4,		//!< buffer has ready to be consumed.
	BUF_Ready = 8,		//!< buffer being processed by reader.
	BUF_Pending = 16,	//!< output buffer waiting to be sent.
};

/*!
 * @class Buffer
 * @brief I/O buffer of fixed size that can be queued.
 *
 * This class is used internally by BufferQueue to represent
 * a list of allocated data buffers.
 *
 * @see ThreadQueue BufferQueue
 * @ingroup vcore
 */
class Buffer : public List
{
public:
	Buffer() : List() { NumBytes = 0; State = BUF_Free; Queue = 0;}
	vint32		NumBytes;		//!< number of bytes used in the buffer
	vint32		State;			//!< state of buffer
	bits		ID : 16;		//!< buffer identifier
	bits		Queue : 16;		//!< queue buffer belongs in

	bool	TestSetState(int32 newval, int32 testval)
	{ return InterlockTestSet(&State, newval, testval) != 0; }

	void	SetState(int32 newval)
	{ InterlockSet(&State, newval); }

	char*	GetData()
	{ return (char*) (this + 1); }
};

/*!
 * @class BufferQueue
 * @brief Thread-safe queues of I/O buffers.
 *
 * Maintains multiple queues of buffers that multiple threads can
 * both produce and consume concurrently.
 * Buffers are fixed length and allocated from global memory by
 * a locking allocator. Write threads allocate buffers, fill them
 * and submit them to one of the input queues. Read threads
 * can process buffers from the input queues simultaneously.
 * After processing, the read thread puts the buffer into a free
 * list where it can be reused again.
 *
 * This class does locking around all buffer manipulation and only
 * supports a single buffer allocation pool. BufferQueue is a descendant that
 * can handle multiple queues and has an allocator for each thread
 * so less locking is needed.
 *
 * @ingroup vcore
 * @see ThreadQueue Buffer
 */
#define	BUFQ_MaxThreads	8
#define	BUFQ_MaxQueues	4

class BufferQueue : public LockObj
{
public:
	VX_DECLARE_CLASS(BufferQueue);

	//! Construct queue for buffers of given size.
	BufferQueue(int bufsize = 1024, int nq = 0);

	//! Destroy all queues and buffers
	~BufferQueue();

	//! Iterates over the buffers in a buffer queue
	class Iter
	{
	public:
		Iter(BufferQueue* bq, int qn = 0); //! Initialize iterator for a buffer queue.
		Buffer*		Next();				//!< Get next buffer from queue.
		void		Free();				//!< Free the buffer most recently gotten.

	protected:
		Buffer*			m_Prev;			// predecessor of current buffer
		Buffer*			m_Next;			// next buffer to be fetched
		BufferQueue*	m_Queue;		// buffer queue
		int				m_QNum;			// queue index
	};
	friend class Iter;

	int 				GetBufSize() const;		//!< Get byte size of buffers.
	int					GetDataSize() const;	//!< Get size of data area.
	int					GetNumQueues() const;	//!< Return number of processing queues.
	virtual void		Empty();				//!< Free all the buffers
	virtual Buffer*		NewBuffer();			//!< Allocate a new buffer.
	virtual void		Free(Buffer* b);		//!< Return buffer to free list.
	virtual void		Submit(Buffer* b);		//!< Submit buffer for processing.
	virtual Buffer*		Process(int q = 0);		//!< Fetch buffer to be processed.
	virtual DebugOut&	Print(DebugOut& dbg) const;

protected:
	virtual Allocator*	GetBufAlloc();			//!< Get queue for this thread

	int32		m_BufSize;						// size of buffers to allocate
	int32		m_NumQueues;					// number of buffer queues
	Allocator*	m_BufAlloc;						// buffer allocators for each thread
	Buffer*		m_Ready[BUFQ_MaxQueues];		// pending input queues
};

/*!
 * @fn int BufferQueue::GetBufSize() const
 *
 * Byte size of the buffers to be allocated.
 * The buffer size must be set once and never changed.
 * Requests to set the buffer size twice will fail.
 */
inline int BufferQueue::GetBufSize() const
{ return m_BufSize; }

inline int BufferQueue::GetDataSize() const
{ return m_BufSize - sizeof(Buffer); }

inline int BufferQueue::GetNumQueues() const
{ return m_NumQueues; }


/*!
 * @fn BufferQueue::Iter::Iter(BufferQueue* bufq, int qnum)
 * @param bufq	buffer queue to examine
 * @param qnum	index of queue to examine
 *
 * Starts an iterator that successively accesses each buffer
 * in the given queue so that each call to \b Next will return a
 * different buffer. Buffers are examined in their queue order.
 */
inline BufferQueue::Iter::Iter(BufferQueue* bufq, int qnum)
{
	m_Queue = bufq;
	m_Prev = NULL;
	m_QNum = qnum;
	m_Next = m_Queue->m_Ready[qnum];
}

/*!
 * @fn Buffer* BufferQueue::Iter::Next()
 *
 * Buffers are returned in their queue order.
 *
 * @return the next buffer in the queue or NULL if no more
 *
 * @see BufferQueue::Iter::Free
 */
inline Buffer* BufferQueue::Iter::Next()
{
	if (m_Next == NULL)
		return NULL;
	if (m_Prev == NULL)
		m_Prev = m_Next;
	else
	{
		m_Prev = m_Next;
		m_Next = (Buffer*) m_Next->Next;
	}
	return m_Next;
}

/*!
 * @class ThreadQueue
 * @brief Thread-safe pool of I/O buffers.
 *
 * Maintains multiple buffer lists, allowing you to allocate
 * variable sized chunks from a buffer in a given list.
 * As buffers are filled, they are marked as ready,
 * permitting them to be processed and reused.
 * All buffer list manipulation is thread-safe, to permit
 * multiple threads to both produce and consume buffers concurrently.
 *
 * Each thread has its own buffer allocator to make and free buffers.
 * There is no locking around allocation. Buffers which are freed
 * during processing are put in the free list for the thread which
 * allocated them. The application can then direct buffers to be
 * garbage collected at a safe time.
 *
 * Buffers may be processed from several different queues. The buffer
 * \b Queue is a zero-based index indicating which queue the
 * buffer should be added to when it is submitted. This permits the
 * same thread to have multiple simultaneous queues.
 *
 * @ingroup vcore
 * @see BufferQueue Buffer
 */
#define	BUFQ_MaxThreads	8
#define	BUFQ_MaxQueues	4


class ThreadQueue: public BufferQueue
{
public:
	friend class Iter;
	VX_DECLARE_CLASS(ThreadQueue);

	ThreadQueue(int bufsize = 4096, int nq = 1)
	: BufferQueue(bufsize, nq)	{ m_NumThreads = 0; }

	void		Empty();					//!< Free all the buffers for all threads.
	Buffer*		NewBuffer();				//!< Allocate a new buffer using this thread's allocator.
	int			GetNumThreads()	const;		//!< Return number of threads.

protected:
	Allocator*	GetBufAlloc();				//!< Get buffer allocator for the current thread.

	vint32		m_NumThreads;				// number of threads providing input
	BytePool	m_Alloc[BUFQ_MaxThreads];	// buffer allocators for each thread
	THREAD_LOCAL BytePool*	t_BufAlloc;		// buffer allocator for this thread
};

inline int ThreadQueue::GetNumThreads() const
{ return m_NumThreads; }

} // end Core