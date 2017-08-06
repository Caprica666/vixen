#include "vcore/vcore.h"
#include "vcore/vlock.h"
#include "vcore/vbufq.h"

namespace Vixen {
namespace Core {
BytePool*	ThreadQueue::t_BufAlloc;

#ifndef VX_NOTHREAD
void* BytePool::Alloc(size_t nbytes)
{
	if (!IsLocking())
	{
		if ((m_freeList == NULL) && (m_Options & ALLOC_FreeLater))
			ReallyFree();
		return FixedLenAllocator::Alloc(nbytes);
	}
	Core::Lock lock(m_Lock);
	if ((m_freeList == NULL) && (m_Options & ALLOC_FreeLater))
		ReallyFree();
	return FixedLenAllocator::Alloc(nbytes);
}

void BytePool::Free(void* p)
{
	List*	b = (List*) p;
	int		opts = m_Options;

	if (opts & ALLOC_Lock)			// locking allocator?
	{
		Core::Lock	lock(m_Lock);	// lock around this operation
		if (opts & ALLOC_FreeLater)	// deferred free?
		{
			b->Next = (List*) m_FreeLater;
			m_FreeLater = b;		// save to be freed later
		}
		else
			FixedLenAllocator::Free(p);
	}
	else
	{
		if (opts & ALLOC_FreeLater)	// deferred free?
		{
			b->Next = (List*) m_FreeLater;
			m_FreeLater = b;		// save to be freed later
		}
		else
			FixedLenAllocator::Free(p);
	}
}

void	BytePool::ReallyFree()
{
	Buffer*	freeus;
	Buffer*	nextone;

	if (IsLocking())
	{
		Core::Lock	lock(m_Lock);	// lock around the free
		freeus = (Buffer*) m_FreeLater;
		m_FreeLater = NULL;
		while (nextone = freeus)
		{
			freeus = (Buffer*) freeus->Next;
			nextone->Next = NULL;
			FixedLenAllocator::Free(nextone);
		}
	}
	else							// not a locking allocator
	{
		freeus = (Buffer*) m_FreeLater;
		m_FreeLater = NULL;
		while (nextone = freeus)
		{
			freeus = (Buffer*) freeus->Next;
			nextone->Next = NULL;
			FixedLenAllocator::Free(nextone);
		}
	}

}
#endif

/****
 *
 * class BufferQueue maintains a list of Buffers,
 * allowing you to add and remove buffers from the list.
 * The list is implemented with single links embedded in
 * the buffer headers. The last buffer in the list has
 * a NULL link.
 *
 * class Buffer is a variable-sized structure representing
 * a buffer of data. The buffer has an next pointer for
 * linkage into a list, a byte size and a state. The buffer
 * state is used by BufferQueue for queuing purposes.
 *
 * As buffers are filled by write threads, they
 * are marked as ready, permitting them to be processed
 * and reused. All buffer list manipulation is thread-safe, to
 * permit multiple threads to both produce and consume buffers concurrently.
 *
 * Buffers not in use are marked as free and stored in the free list.
 * When an allocation request is made, an index is provided of
 * which queue you would like the buffer to be submitted to.
 * As they are needed, buffers are taken from the free list.
 * When a buffer from a queue has been processed and released,
 * it is returned to the free list.
 *
 ****/
VX_IMPLEMENT_CLASS(BufferQueue, LockObj);
VX_IMPLEMENT_CLASS(ThreadQueue, BufferQueue);

/*!
 * @fn BufferQueue::BufferQueue(int bufsize, int nqueues)
 * @param bufsize	Number of bytes in a buffer.
 * @param nqueues	Number of separate processing queues.
 *
 * Initializes a set of buffer queues. Fixed size buffers are allocated
 * as needed. When full, the buffers are marked as available for
 * processing. After buffers have been processed, they are put
 * on the free list for use again. This class is designed to be used
 * with cooperating producer and consumer threads.
 *
 * Once you establish the size of the buffers in the pool,
 * you cannot change it. The default buffer size is 1024 bytes.
 */
#define	BUFQ_DefaultBufSize	1024

BufferQueue::BufferQueue(int bufsize, int nqueues)
{
	if (bufsize == 0)
		bufsize = BUFQ_DefaultBufSize;
	m_BufAlloc = NULL;
	m_BufSize = bufsize;
	m_NumQueues = nqueues;
	for (int i = 0; i < BUFQ_MaxQueues; ++i)
		m_Ready[i] = NULL;
}

BufferQueue::~BufferQueue()
{
}

/*!
 * @fn Allocator* BufferQueue::GetBufAlloc()
 *
 * The buffer allocator is used to create and destroy buffers.
 * Subclasses can override this function to provide their own
 * buffer allocators. The default implementation makes a fixed
 * length allocator which uses the global heap and locks around
 * memory management.
 *
 * @return pointer to buffer allocator or NULL if there is none.
 *
 * @see BufferQueue::NewBuffer
 */
Allocator* BufferQueue::GetBufAlloc()
{	return m_BufAlloc; }


/*!
 * @fn Buffer* BufferQueue::NewBuffer()
 *
 * Takes a buffer from the free list or obtains more
 * memory from the global heap and takes from there.
 * This function marks the \b State in the buffer header
 * to mark the buffer as being new (BUF_New).
 * To mark the buffer as full and awaiting processing,
 * set the \b State to BUF_Ready and call Submit.
 * The buffer \b NumBytes field is used to store the
 * number of bytes currently stored in the buffer.
 * It is initialized as the maximum buffer size.
 *
 * The \b Queue field of the buffer indicates the queue index
 * of the queue the buffer will go to when submitted.
 * It is initialized here to zero.
 *
 * @returns buffer allocated or NULL if allocation fails
 *
 * @see BufferQueue::Submit BufferQueue::Free BufferQueue::Submit
 */
Buffer* BufferQueue::NewBuffer()
{
	Buffer*	buffer;
	Allocator* bufalloc = GetBufAlloc();

	if (bufalloc == NULL)
		{ VX_ERROR(("BufferQueue: no buffer allocator"), NULL); }
	buffer = (Buffer*) Buffer::operator new(m_BufSize, bufalloc);
	if (buffer == NULL)
	   { VX_ERROR(("BufferQueue: out of memory\n"), NULL); }
	new (buffer) Buffer();					// do construction ourselves
	VX_TRACE(Debug, ("BufferQueue::NewBuffer @ %p\n", buffer));
	buffer->NumBytes = GetDataSize();			// bytes used by this allocation
	buffer->ID = 0;
	buffer->Queue = 0;
	buffer->State = BUF_New;					// mark as newly created
	return buffer;
}

/*!
 * @fn bool BufferQueue::Free(Buffer* buffer)
 * @param buffer	buffer to free
 *
 * Marks a buffer as free (\b State is BUF_Free)
 * and makes it available for reallocation
 * by adding it to the free list. You should not free a buffer
 * if it has not been processed yet because it is most likely
 * still in a queue.
 *
 * @return  \b true if buffer was successfully freed,
 *			\b false if buffer is on another list
 *
 * @see BufferQueue::Process BufferQueue::CheckFree BufferQueue::Iter::Free
 */
void BufferQueue::Free(Buffer* buffer)
{
	VX_ASSERT(buffer);
	if (buffer->Next != NULL)
	   VX_ERROR_RETURN(("BufferQueue::Free Q=%d ID=%d cannot free, buffer in use\n",
					buffer->Queue, buffer->ID));
	VX_TRACE(Debug, ("BufferQueue::Free  @ %p\n", buffer));
	buffer->SetState(BUF_Free);
	delete buffer;
}


/*!
 * @fn bool BufferQueue::Submit(Buffer* buf)
 * @param	buffer	pointer to buffer to submit
 *
 * Submitting a buffer adds it to the processing queue
 * specified by the buffer \b Queue field. This is a zero-based
 * queue index.
 *
 * @see BufferQueue::Process Buffer
 */
void BufferQueue::Submit(Buffer* buf)
{
	ObjLock	lock(this);

	VX_ASSERT(buf->Next == NULL);
	VX_ASSERT(buf->State >= BUF_Used);
	if (m_Ready[buf->Queue])
		m_Ready[buf->Queue]->Append(buf);
	else
		m_Ready[buf->Queue] = buf;
	VX_TRACE(Debug, ("BufferQueue::Submit %d:%d @ %p\n",
			 buf->Queue, buf->ID, buf));
}
/*!
 * @fn void BufferQueue::Empty()
 *
 * Returns all of the buffers in use by all processing queues to
 * the buffer allocator, which garbage collects them and releases
 * its heap blocks.
 *
 * @see FixedLenAllocator::FreeAll BufferQueue::Free
 */
void BufferQueue::Empty()
{
	ObjLock lock(this);
	Buffer* buflist;

	for (int i = 0; i < m_NumQueues; ++i)
		if (buflist = m_Ready[i])
		{
			buflist->Empty();
			delete buflist;
			m_Ready[i] = NULL;
		}
}

/*!
 * @fn Buffer* BufferQueue::Process(int qnum)
 * @param qnum index of queue to use, defaults to 0
 *
 * Gets the next buffer ready for processing (buffer state
 * is BUF_Ready) from the specified queue. The buffer is
 * removed from the queue and should be freed when the
 * consumer is finished with it.
 *
 * @return next available buffer, or NULL if none ready
 *
 * @see BufferQueue::Free BufferQueue::Submit
 */
Buffer* BufferQueue::Process(int qnum)
{
	ObjLock	lock(this);
	Buffer**	next = (Buffer**) &m_Ready[qnum];
	Buffer*	buf;

	while (buf = *next)
	{
		if (buf->State & BUF_Ready)
		{
			*next = (Buffer*) buf->Next;
			VX_TRACE(Debug, ("BufferQueue::Process %d:%d @ %p %d bytes\n",
					buf->Queue, buf->ID, buf, buf->NumBytes));
			buf->Next = NULL;
			return buf;
		}
		next = (Buffer**) &(buf->Next);
	}
	return NULL;
}

DebugOut& BufferQueue::Print(DebugOut& dbg) const
{
	int n;

	for (int i = 0; i < m_NumQueues; ++i)
	{
		if (n = m_Ready[i]->GetSize())
			dbg << "  Ready[" << i << "] = " << n;
	}
	return dbg;
}

/*!
 * @fn Buffer* ThreadQueue::NewBuffer()
 * @param bufsize	Number of bytes in a buffer.
 *
 * Grabs a new buffer from the allocator for this thread.
 * If there is no buffer allocator, one is created.
 * Each thread has a separate allocator which uses the local heap for
 * that thread and does no locking. This is why it is faster than
 * BufferQueue::NewBuffer.
 *
 * After a buffer has been processed, the consumer thread puts it
 * on a free list targeted for this thread (requires locking).
 * NewBuffer first returns those buffers to the allocator (cheap locking)
 * which puts it on another free list that requires no locking.
 * New buffers are only actually obtained from this final free list.
 *
 * ThreadQueue uses the buffer \b ID field internally to track which
 * allocator each buffer belongs to.
 *
 * @see BufferQueue::BufferQueue ThreadQueue::NewBuffer
 */
Buffer* ThreadQueue::NewBuffer()
{
	BytePool*	alloc = (BytePool*) GetBufAlloc();
	Buffer*	buffer;

	VX_ASSERT(alloc && alloc->IsKindOf(CLASS_(Core::BytePool)));
	buffer = BufferQueue::NewBuffer();
	buffer->ID = alloc - m_Alloc; // save allocator index
	VX_ASSERT(int(buffer->ID) < m_NumThreads);
	return buffer;
}

/*!
 * @fn ThreadQueue::ThreadQueue(int bufsize, int nqueues)
 * @param bufsize	Number of bytes in a buffer, defaults to 4K.
 * @param nqueues	Number of separate processing queues.
 *
 * Initializes a set of buffer queues. Fixed size buffers are allocated
 * as needed by write threads using separate thread allocators
 * that do no locking. When full, the buffers are submitted for
 * processing by one or more read threads. After a buffer has been processed,
 * it is put on a free list associated with the thread it came from.
 *
 * @see BufferQueue::BufferQueue ThreadQueue::NewBuffer
 */

/*!
 * @fn Allocator* ThreadQueue::GetBufAlloc()
 *
 * The buffer allocator is used to create and destroy buffers.
 * Each thread has its own allocator so locking is not needed
 * around allocations. When buffers are freed by read threads
 * during processing, they are added to the free list for that
 * thread (which requires locking). They should be reclaimed by
 * write threads by periodically calling CheckFree (which does
 * very cheap locking).
 *
 * @return pointer to buffer allocator or NULL if there is none.
 *
 * @see BufferQueue::NewBuffer ThreadQueue::CheckFree
 */
Allocator* ThreadQueue::GetBufAlloc()
{
	BytePool*	alloc = t_BufAlloc;
	int			chunksize = 4096;

	if (alloc)
		return alloc;
	if (m_NumThreads >= BUFQ_MaxThreads)
		return NULL;
	alloc = &m_Alloc[m_NumThreads];
	if (m_BufSize * 4 > chunksize)
		chunksize = m_BufSize * 4;
	alloc->SetOptions(ALLOC_FreeLater);
	alloc->SetElementAndBlockSize(m_BufSize, chunksize / m_BufSize);
	t_BufAlloc = alloc;
	InterlockInc(&m_NumThreads);
	return alloc;
}

void ThreadQueue::Empty()
{
	ObjLock	lock(this);
	BufferQueue::Empty();
	for (int t = 0; t < m_NumThreads; ++t)
	{
		BytePool* alloc = &m_Alloc[t];
		alloc->FreeAll();
	}
	m_NumThreads = 0;
}

/*!
 * @fn void BufferQueue::Iter::Free()
 *
 * Frees the most recently accessed buffer, removing it from the processing
 * queue and returning it to the free list.
 *
 * @see BufferQueue::Free
 */
void BufferQueue::Iter::Free()
{
	Buffer*	buf = m_Next;

	if (buf == NULL)
		return;
	if (m_Prev && (m_Prev != buf))
	{
		VX_ASSERT(m_Prev->Next == buf);
		m_Prev->Next = buf->Next;
		m_Next	= (Buffer*) buf->Next;
		buf->Next = NULL;
		m_Queue->Free(buf);
		return;
	}
	VX_ASSERT(buf == m_Queue->m_Ready[m_QNum]);
	m_Queue->m_Ready[m_QNum] = NULL;
	VX_ASSERT(buf->Next == NULL);
	m_Queue->Free(buf);
}

}	// end Core

}	// end Vixen