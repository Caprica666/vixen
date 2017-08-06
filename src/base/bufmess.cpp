#include "vixen.h"

namespace Vixen {
using namespace Core;

int32 BufMessenger::MaxBufSize = MESS_MaxBufSize - sizeof(Core::Buffer);
BufMessenger*	BufMessenger::s_OnlyOne = NULL;

char*		BufMessenger::t_LastOp;		// last opcode this thread wrote
int			BufMessenger::t_ReadLogType;	// the log this thread is reading from
int			BufMessenger::t_WriteLogType;	// the log this thread is writing to
Core::Buffer*	BufMessenger::t_WriteBuf[MESS_MaxLogs];

/*!
 * @fn BufMessenger::BufMessenger(Core::ThreadQueue* bufpool)
 *
 * Description:
 * Constructs a buffering messenger and establishes the initial buffer size.
 * The messenger cannot log a message larger than this maximum buffer size.
 * Buffers are allocated as needed for logging so this does not
 * restrict the amount of information that can be logged, just the
 * individual message size. A message cannot be split across buffers.
 *
 * @see VXBufferPool
 */
BufMessenger::BufMessenger(Core::ThreadQueue* bufpool)
 :	Messenger(NULL, true), m_BufPool(bufpool)
{
	Open(NULL, Stream::OPEN_RW);
	VX_ASSERT(s_OnlyOne == NULL);
	s_OnlyOne = this;
}

BufMessenger::~BufMessenger()
{
	Close();
	s_OnlyOne = NULL;
}

BufMessenger*	BufMessenger::Get()
{	return s_OnlyOne; }

bool BufMessenger::Open(const TCHAR* name, int mode)
{
	if (m_BufPool == NULL)
	{
		m_BufPool = new Core::ThreadQueue(MESS_MaxBufSize, MESS_MaxLogs);
		m_BufPool->MakeLock();
	}
	m_ReadPtr = NULL;
	m_ReadBuf = NULL;
	m_OpenMode = (mode != 0);
	for (int log = 0; log < m_BufPool->GetNumQueues(); ++log)
	{
		SelectWriteLog(log);
		SetWriteBuf(NULL);
		SelectReadLog(log);
		SetReadBuf(NULL);
	}
	return true;
}

bool BufMessenger::Close()
{
	ThreadQueue* bufpool = m_BufPool;
	if (bufpool == NULL)
		return true;
	m_ReadBuf = NULL;
	m_ReadPtr = NULL;
	bufpool->Empty();
	for (int log = 0; log < m_BufPool->GetNumQueues(); ++log)
	{
		SelectWriteLog(log);
		SetWriteBuf(NULL);
		SelectReadLog(log);
		SetReadBuf(NULL);
	}
	m_Names->Empty();
	return Messenger::Close();
}

size_t BufMessenger::Read(char* buffer, int n)
{
	int	ncopy = 0;
	int	ofs = 0;

	while ((n - ncopy) > 0)
	{
		Core::Buffer* buf = GetReadBuf();	// have a read buffer?
		if (buf == NULL)					// error, should have one
			return 0;
		ncopy = buf->NumBytes;
		if (ncopy > n)
			ncopy = n;
		if (buffer)
			memcpy(buffer, m_ReadPtr, ncopy);// read data from buffer
		m_ReadPtr += ncopy;					// advance input ptr
		if (Core::InterlockAdd(&(buf->NumBytes), -ncopy) <= 0)
		{									// consume the buffer space
			m_ReadBuf = NULL;				// indicate no read buffer
			m_ReadPtr = NULL;
			if (buf->State == 0)
				m_BufPool->Free(buf);		// free buffer for re-use
		}
	}
	return n;
}

bool BufMessenger::BeginOp(int logtype, const SharedObj* obj)
{
	SelectWriteLog(logtype);			// select output log
	Core::Buffer*	buf = GetWriteBuf();// get the current write buffer

	if (buf)							// can we use the current write buffer?
	{
		int state = buf->State;
		if (!buf->TestSetState(BUF_Locked, BUF_Used))
			buf->TestSetState(BUF_Locked, BUF_Free);
		if (state & (BUF_New | BUF_Used))
		{
			t_LastOp = buf->GetData() + buf->NumBytes; // update command start
			return true;				// can use this buffer
		}
	}
	if (m_OpenMode == 0)
		return false;
	t_LastOp = NULL;
	SetWriteBuf(NULL);					// no valid write buffer now
	buf = m_BufPool->NewBuffer();
	if (buf == NULL)					// can't allocate new buffer?
		return false;
	buf->Queue = logtype;				// indicate which log in the buffer
	buf->NumBytes = 0;
	t_LastOp = buf->GetData();
	SetWriteBuf(buf);					// make it current output buffer
	return true;
}

void BufMessenger::EndOp()
{
	Core::Buffer*	buf = GetWriteBuf();		// get current write buffer

	if (buf->TestSetState(BUF_Used, BUF_New))	// not already submitted?
		m_BufPool->Submit(buf);					// submit it now
	else
	{
		VX_ASSERT(buf->State == BUF_Locked);
		buf->SetState(BUF_Used);				// unlock it
	}
}

Messenger&	BufMessenger::OutOp(const VXOpcode& op)
{
	Core::Buffer* buf = GetWriteBuf();
	if (buf)
		t_LastOp = buf->GetData() + buf->NumBytes;
	return *this << int32(uint32(uint16(op.ClassID) << 16) | op.Opcode);
}

void BufMessenger::DoEvent()
{
	bool savelog = t_NoLog;
	t_NoLog = false;				// enable logging during events
	Messenger::DoEvent();
	t_NoLog = savelog;
}

/****
 *
 * Write will fail if the message is larger than the largest
 * buffer size. Otherwise, it will attempt to put the data
 * in the current buffer. If it cannot be added, we allocate
 * another buffer. We don't break commands over buffer boundaries
 * so the data from the command already in the existing buffer
 * has to be copied into the new one
 *
 ****/
size_t BufMessenger::Write(const char* buffer, int n)
{
	if ((n <= 0) || (n > MaxBufSize))			// request size invalid?
		VX_ERROR(("Messenger::Write %d exceeds maximum buffer size of %d\n", n, MaxBufSize), false);

	Core::Buffer*	buf = GetWriteBuf();		// get write buffer
	if ((buf == NULL) ||
		(n + buf->NumBytes > MaxBufSize))		// less than what we want
		buf = SwitchBuffers(buf);				// get a new buffer
	if (!buf)
		VX_ERROR(("Messenger::Write buffer full %d bytes\n", n), false);
	char*	bufptr = buf->GetData() + buf->NumBytes;
	if (buffer)
		memcpy(bufptr, buffer, n);				// save data in buffer
	Core::InterlockAdd(&(buf->NumBytes), n);	// consume the space
	return n;
}

/****
 *
 * Allocate a new buffer, copy the fragment of the last command
 * (if any) into the new buffer. Upon entry, NumBytes in the old
 * buffer is the number of bytes left.
 *
 ****/
Core::Buffer* BufMessenger::SwitchBuffers(Core::Buffer* oldbuf)
{
	int32				log = GetWriteLog();
	Core::Buffer*		newbuf = m_BufPool->NewBuffer();
	char*				lastop = t_LastOp;

	if (m_OpenMode == 0)
		return NULL;
	if (newbuf == NULL)
		{ VX_ERROR(("Messenger::Write out of buffer space\n"), NULL); }
	newbuf->NumBytes = 0;
	newbuf->Queue = log;						// tell it which log to go to
	SetWriteBuf(newbuf);						// make this our new write buffer
	if (oldbuf && lastop)
	{
		size_t	n = lastop - oldbuf->GetData();	// bytes in completed commands
		size_t	m = oldbuf->NumBytes;		

	// # bytes used in buffer
		m -= n;
		if (m > 0)								// have partial command buffered?
		{
			if (n == 0)							// entire command buffered?
				return NULL;
			oldbuf->NumBytes -= (uint32) m;
			if (lastop)							// copy to new buffer
			{
				memcpy(newbuf->GetData(), lastop, m);
				newbuf->NumBytes += (uint32) m;
			}
		}
		VX_TRACE(Debug > 2, (("Messenger: SwitchBuffers: %d:%d READY %d bytes\n"),
			oldbuf->Queue, oldbuf->ID, oldbuf->NumBytes));
		if (oldbuf->State == BUF_New)
		{
			oldbuf->State = BUF_Used;			// mark as ready for processing
			m_BufPool->Submit(oldbuf);			// submit for input
		}
		else
			oldbuf->SetState(BUF_Used);
	}
	newbuf->State = BUF_New;
	newbuf->Queue =(uint16) log;				// designate which log
	t_LastOp = newbuf->GetData();
	VX_TRACE(Debug > 2, (("Messenger: SwitchBuffers: %d:%d NEW %d copied\n"),
		newbuf->Queue, newbuf->ID, newbuf->NumBytes));
	return newbuf;
}

/****
 *
 * IsEmpty returns <true> if there are no more buffers
 * to be processed. The input buffers have NumBytes set
 * to the number of bytes LEFT in the buffer. We change
 * it to the number of bytes USED in the buffer.
 *
 ****/
bool BufMessenger::IsEmpty() const
{
	BufMessenger*	cheat = (BufMessenger*) this;
	Core::Buffer*	buf = GetReadBuf();

	if (buf)						// current buffer has data?
		if (buf->NumBytes > 0)
			return false;			// return not empty
		else
		{
			if (buf->State == 0)	// buffer exhausted
				cheat->m_BufPool->Free(buf);
			buf = NULL;				// free it and get new one
		}
	if (buf == NULL)
	{
		if (m_OpenMode == 0)
			return true;
		buf = cheat->m_BufPool->Process(GetReadLog());
		if (buf == NULL)			// new buffer available?
		{
			cheat->SetReadBuf(NULL);
			cheat->m_ReadPtr = NULL;
			return true;			// no, we are empty
		}
		VX_ASSERT(buf->Next == NULL);
		InterlockAnd(&(buf->State), ~BUF_Ready);
		cheat->SetReadBuf(buf);		// clear "ready" status and use this one
		cheat->m_ReadPtr = buf->GetData();
	}
	VX_ASSERT(buf->NumBytes > 0);
	return buf == NULL;
}

bool BufMessenger::Load()
{
	t_NoLog = true;				// disable logging for this thread
	for (int log = 0; log < m_BufPool->GetNumQueues(); ++log)
	{
		SelectReadLog(log);		// select which log to read
		Messenger::Load();	// load from only that log
		SetReadBuf(NULL);		// don't reuse read buffer from this log
		m_ReadPtr = NULL;
	}
	t_NoLog = false;			// enable logging for this thread
	return true;
}

/*!
 * @fn bool BufMessenger::Flush()
 *
 * Examines all buffers for each log and sends those that are ready to
 * the remove processors. The logs differ in their behavior with respect
 * to sending and keeping buffers.
 * - MESS_FastLog	high priority updates to remote machines
 *					transactions on this log are ignored locally
 * - MESS_UpdateLog	object updates sent to remote machines
 *					updates have already been made locally so
 *					buffers can be discarded
 * - MESS_EventLog	events may be sent to remote machines
 *					they are processed locally too
 *
 * \b SendUpdates and \b SendEvents control whether the updates / events are
 * sent to remote processors (whether OnSend is called). The buffer state
 * determines whether it should be consumed or not
 *	- BUF_Used	buffer written last frame
 *	- BUF_Ready	buffer ready for reading
 *	- BUF_Pending	buffer waiting to be sent
 */
void BufMessenger::Flush()
{
	ObjLock	lock((LockObj*) m_BufPool);

	for (int log = 0; log < m_BufPool->GetNumQueues(); ++log)
	{
		ThreadQueue::Iter iter(m_BufPool, log);
		Core::Buffer* buf = NULL;

		while (buf = iter.Next())			// for all buffers in this log
		{
		/*
		 * Examine all the update buffers. If SendUpdates is set, send
		 * each buffer to the remote processors by calling OnSend.
		 * These updates are never kept and processed locally because
		 * the objects have already been changed.
		 */
			buf->TestSetState(BUF_Ready, BUF_Used);
			if (buf->State >= BUF_Used)
			{
				VX_TRACE(Debug, ("BufMessenger::Flush %d:%d @ %p %d bytes\n",
						buf->Queue, buf->ID, buf, buf->NumBytes));
				switch (log)
				{
					case MESS_FastLog:
					case MESS_UpdateLog:
					if (SendUpdates)
						OnSend(buf);
					InterlockAnd(&(buf->State), ~BUF_Ready);
					break;
		/*
		 * Examine all the event buffers. If SendEvents is set, send
		 * each buffer to the remote processors by calling OnSend.
		 * Events are always left in the buffer pool to be processed locally
		 */
					case MESS_EventLog:
					if (SendEvents)
						OnSend(buf);
					break;
		/*
		 * Examine the local log which is both processed locally and sent.
		 * These are commands which are deferred until the display thread.
		 * Events are always left in the buffer pool to be processed locally
		 */
					case MESS_LocalLog:
					if (SendUpdates)
						OnSend(buf);
					break;
				}						// end switch
			}
			if (buf->State == 0)		// buffer no longer used?
				iter.Free();			// remove from input Q and free
		}								// end while
	}									// end for
}

void	BufMessenger::SetReadBuf(Core::Buffer* b)
{
	m_ReadBuf = b;
}

void	BufMessenger::SetWriteBuf(Core::Buffer* b)
{
	t_WriteBuf[t_WriteLogType] = b;
}

int	BufMessenger::GetBuffer(char** buffer) 
{
	Core::Buffer*	buf;

	if (buf = m_ReadBuf)
	{
		*buffer = (char*) buf->GetData();
		return buf->NumBytes;
	 }
	*buffer = NULL;
	return 0;
}

}	// end Vixen