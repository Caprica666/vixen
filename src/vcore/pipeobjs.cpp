//***************************************************************************
//
//		File:	PipeObjs.cpp - Data pipeline objects
//
//***************************************************************************

#include <stdio.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/shm.h>	// shared memory routines
#endif
#include <curl/multi.h>

#include "stdafx.h"
#include "pipeobjs.h"

namespace Vixen {
namespace Core {

IMPLEMENT_DYNAMIC (MemoryNode, PipeNode);
IMPLEMENT_DYNAMIC (SeekAdapterNode, MemoryNode);
IMPLEMENT_DYNAMIC (FileNode, PipeNode);
IMPLEMENT_DYNAMIC (ExecNode, FileNode);
IMPLEMENT_DYNAMIC (HttpNode, MemoryNode);
IMPLEMENT_DYNAMIC (NamedPipeNode, FileNode);
IMPLEMENT_DYNAMIC (BitBucketNode, PipeNode);
IMPLEMENT_DYNAMIC (ZLibCompressNode, PipeNode);
	
String StripQuotes (String& str)
{
	if (str.GetLength() >= 2)
		if ((str[0] == '"'  && str[str.GetLength()] == '"') ||
			(str[0] == '\'' && str[str.GetLength()] == '\'' ))
			return str.Mid (1, str.GetLength() - 2);
	
	return str;
}

//============================================================================
// MemoryNode
//============================================================================

// Growable Sink: client calls Detach()
MemoryNode::MemoryNode ()
  : PipeNode()
{
	CommonInit ((Byte*)GlobalAllocator::Get()->Alloc (OPTIMAL_BUFFERSIZE), 0);
}

// Source: non-NULL maxCount
MemoryNode::MemoryNode (unsigned char* buffer, ULONG maxCount)
  : PipeNode()
{
	CommonInit (buffer, maxCount);
}


void MemoryNode::CommonInit (unsigned char* buffer, ULONG maxCount)
{
	m_total_in		= 0;
	m_total_out     = 0;
	m_next_in		=
	m_next_out		=
	m_bufferStart	= buffer;
	m_avail_in		=
	m_avail_out		=
	m_bufferSize	=
	m_optimal_in	=
	m_optimal_out   = maxCount ? maxCount : OPTIMAL_BUFFERSIZE;
	m_allocDelta    = OPTIMAL_BUFFERSIZE;

	// Can grow only if we are a sink, kill sink memory in 
	// destructor unless Detach() is called.
	m_canGrow       = maxCount == 0;
	m_killBuffer	= maxCount == 0;
}

MemoryNode::~MemoryNode()
{
	// Sink - kill memory block if Detach() wasn't called
	if (m_killBuffer && m_bufferStart != NULL)
	{
		GlobalAllocator::Get()->Free (m_bufferStart);
		m_bufferStart = NULL;
	}
}


unsigned char* MemoryNode::Attach (unsigned char* b, ULONG max)
{
	unsigned char*	prevBuffer = m_bufferStart;

	// Use initialization routine to handle nitty-gritty.
	ASSERT (m_status == PIPE_Ok && m_total_in == 0); 
	CommonInit (b, max);

	// Return previous buffer
	return prevBuffer; 
}

unsigned char* MemoryNode::Detach()
{ 
//	ASSERT (m_status == PIPE_Stream_End);

	// Shrink resulting buffer to total size read
//	if (m_canGrow && m_bufferSize != Size())
//	{
//		m_bufferStart = (unsigned char*) realloc (m_bufferStart, Size());
//		m_bufferSize = Size();
//	}

	// Allocate a new buffer, return pointer to previous version
	unsigned char*	prevBuffer = m_bufferStart;
	CommonInit ((Byte*)GlobalAllocator::Get()->Alloc (OPTIMAL_BUFFERSIZE), 0);
	return prevBuffer;
}
	
	
void MemoryNode::Flush()
{
	if (!m_fileName.IsEmpty())
	{
#ifdef __APPLE__
		int		key = m_fileName.toInt();
		int		shmid;
		void*	pointer;
		
		if ((shmid = shmget (key, m_total_out, 0666 | IPC_CREAT)) != -1)
		{
			if ((pointer = shmat (shmid, NULL, 0)) != (void*)-1)
			{
				memmove (pointer, m_bufferStart, m_total_out);
				shmdt (pointer);
			}
		}
#endif
	}
}


ULONG MemoryNode::Read (unsigned char* where, ULONG amount)
{
	// Keep a running total and watch to error return
	ULONG	total = 0;
	while (amount && m_status == PIPE_Ok)
	{
		// Have we exhausted the buffer?
		if (m_avail_out == 0)
		{
			// Growable sink: enlarge buffer
			if (m_canGrow )
			{
				// determine total size required to hold new data
				m_bufferSize = m_total_out + amount;
				m_bufferStart = (unsigned char*) realloc (m_bufferStart, m_bufferSize);
#ifdef _DEBUG
				// WARNING: do not pass the pointer in the second parameter to UpdateDebugStats()
				// since that can cause it to wipe out memory (for debugging sake it may get
				// overwritten by the hex code 0xdeadbeef when freed) .. sooo .. just don't do it.
				GlobalAllocator::Get()->UpdateDebugStats(-m_total_out);
				GlobalAllocator::Get()->UpdateDebugStats(m_bufferSize);
#endif
				// Reset pointer, also, just in case a new mem block was returned
				m_next_out  = m_bufferStart + m_total_out;
				m_avail_out = m_bufferSize  - m_total_out;
			}
			else
			{
				// Fixed-size source: we are at input end.
				ASSERT (m_prev == NULL);
				m_status = PIPE_Stream_End;
				return total;
			}

			// Input buffer exhausted- read next buffer
			if (m_avail_in == 0 && m_prev)
			{
				// Do the Read() and cascade status from node
				m_avail_in = m_prev->Read (m_next_out, m_avail_out);
				m_status   = m_prev->Status();

				// Increment total count, set amount of trasnferable data
				m_total_in += m_avail_in;
				m_avail_out = m_avail_in;

				// If at end of input, don't transfer any more - just return
				if (m_avail_in == 0)
					return total;
			}
		}

		// Shuffle information from input buffer, decrements amount, increments total
		ULONG increment =  TransferData (where, &amount, &total, PIPE_Read);

		// update totals, counters, and pointers
		m_total_out += increment;
		m_next_in	+= increment;
		m_avail_in  -= increment;
		if (where) where += increment;
		
		if (m_status == PIPE_Stream_End)
			Flush();
	}

	// return actual amount read
	return total;
}


ULONG MemoryNode::Write (unsigned char* what,  ULONG amount)
{
	// Keep a running total and watch to error return
	ULONG	total = 0;
	while (amount && m_status == PIPE_Ok)
	{
		// Have we exhausted the buffer?
		if (m_avail_in == 0 || m_avail_in < amount)
		{
			// Growable sink: enlarge buffer
			if (m_canGrow)
			{
				// determine total size required to hold new data
				m_bufferSize = m_total_in + amount;
				m_bufferStart = (unsigned char*) realloc (m_bufferStart, m_bufferSize);
#ifdef _DEBUG
				// WARNING: do not pass the pointer in the second parameter to UpdateDebugStats()
				// since that can cause it to wipe out memory (for debugging sake it may get
				// overwritten by the hex code 0xdeadbeef when freed) .. sooo .. just don't do it.
				GlobalAllocator::Get()->UpdateDebugStats(-m_total_out);
				GlobalAllocator::Get()->UpdateDebugStats(m_bufferSize);
#endif
				// Reset pointer, also, just in case a new mem block was returned
				m_next_in   = m_bufferStart + m_total_in;
				m_avail_in  = m_bufferSize  - m_total_in;
			}
			else
			{
				// Fixed-sized sink: buffer exhausted, indicates PIPE_Stream_End
				ASSERT (m_next == NULL);
				m_status = PIPE_Stream_End;
				return 0;
			}
		}
		
		// Shuffle information from output buffer, decrements amount, increments total
		ULONG increment =  TransferData (what, &amount, &total, PIPE_Write);

		// update totals, counters, and pointers
		m_total_in  += increment;
		if (what) what += increment;
	}

	// return amount read
	return total;
}


ULONG MemoryNode::Size()
{
	if (m_canGrow)
	{
		// Do we need to exhaust the input source in order to determine
		// the overall size of this node?
		if (m_prev && m_status != PIPE_Stream_End)
		{
			// Save current offset
			ULONG		save_total_out = m_total_out;

			// Read the remainder into in buffer
			while (m_status == PIPE_Ok)
				MemoryNode::Read (NULL, OPTIMAL_BUFFERSIZE);
			
			// Reset original pointers/counts and return total size.
			m_avail_out = m_total_out - save_total_out;
			m_next_out  = m_bufferStart + save_total_out;
			m_total_out = save_total_out;
		}

		// return the buffered file size.
		return m_total_in; 
	}
	else
	{
		// Non-growable source - return the amount of data available
		return m_bufferSize;
	}
}


//============================================================================
// SeekAdapterNode
//============================================================================

// SeekAdapters are always filter elements
SeekAdapterNode::SeekAdapterNode ()
  : MemoryNode	(),
	m_currentPos    (0),
	m_totalBuffered (0)
{
}


// Simulate seek operation by resetting position pointer.  If necessary,
// the output buffer is enlarged, and data blocks are read up to the
// seek position.  If PIPE_Stream_End is encountered before the seek
// position, the seek operation is indeterminate, and returns PIPE_Stream_End.
ULONG SeekAdapterNode::Seek (ULONG offset)
{
	// Shouldn't TRY anything when in error mode, but we can be at Stream_End.
	ASSERT (StatusOK());
	
	// Beyond the reach of the current buffer?
	if (offset > m_bufferSize && m_status == PIPE_Ok)
	{
		if (m_prev)
		{
			// Move the current pointer to buffer end, set avail_out to zero 
			// in anticipation of the loop to fill remainder of buffer.
			m_currentPos = m_bufferSize;
			m_next_out   = m_bufferStart + m_bufferSize;
			m_avail_out  = 0;

			// Continue feeding the buffer until offset is reached or stream end found
			while (m_currentPos < offset && m_status == PIPE_Ok)
			{
				// read the next block of data using base class code
				m_currentPos += MemoryNode::Read (NULL, offset - m_currentPos);
			}
		}
		else if (m_next)
		{
			// Move the current pointer to buffer end, set avail_out to zero 
			// in anticipation of the loop to fill remainder of buffer.
			m_currentPos = m_bufferSize;
			m_next_in    = m_bufferStart + m_bufferSize;
			m_avail_in   = 0;

			// Continue feeding the buffer until offset is reached or stream end found
			while (m_currentPos < offset && m_status == PIPE_Ok)
			{
				// read the next block of data using base class code
				m_currentPos += MemoryNode::Write (NULL, offset - m_currentPos);
			}
		}
	}

	// Reset current position and return seek offset
	if (offset <= m_bufferSize)
	{
		// reset pointers for the next read/write operation
		if (m_prev)
		{
			ASSERT (offset <= m_total_in);

			m_next_out  = m_bufferStart + offset;
			m_avail_out = m_total_in - offset;
		}
		else if (m_next)
		{
			m_next_in   = m_bufferStart + offset;
			m_avail_in  = m_bufferSize - offset;
		}

		// Update the new position
		m_currentPos = offset;
	}

	return m_currentPos;
}


ULONG SeekAdapterNode::Read (unsigned char* where, ULONG amount)
{
	// Enlarge buffer by seeking to the desired offset to enlarge
	if (m_currentPos + amount > m_bufferSize)
	{
		ULONG	returnTo = m_currentPos;
		Seek (m_currentPos + amount);
		Seek (returnTo);
	}

	// Transfer data from out buffer until amount is exhausted
	ULONG	total  = 0;
	while (amount && Status() == PIPE_Ok)
	{
		ULONG increment = TransferData (where, &amount, &total, PIPE_Read); 
		m_currentPos +=	increment;
		if (where) where += increment;
	}

	return total;
}


ULONG SeekAdapterNode::Write(unsigned char* what, ULONG amount)
{
	// Enlarge buffer by seeking to the desired offset to enlarge
	if (m_currentPos + amount > m_bufferSize)
	{
		ULONG	returnTo = m_currentPos;
		ULONG	growBy = MAX (OPTIMAL_BUFFERSIZE, amount);
		
		// Seek to enlarge buffer, then back to current position.
		Seek (m_currentPos + growBy);
		Seek (returnTo);
	}

	// Transfer data from out buffer until amount is exhausted
	ULONG	total  = 0;
	while (amount && Status() == PIPE_Ok)
	{
		ULONG increment = TransferData (what, &amount, &total, PIPE_Write); 
		
		// Update position, source pointer, and total if appropriate
		m_currentPos +=	increment;
		if (what) what += increment;
		if (m_currentPos > m_totalBuffered) m_totalBuffered = m_currentPos;
	}

	return total;
}

PipeResult SeekAdapterNode::Status()
{
	// File pointer should be at end of the input buffer to a real
	// end of stream to be signalled.  This deals with pre-fetch conditions.
	if (m_status == PIPE_Stream_End)
	{
		ASSERT (m_currentPos <=  m_total_in);
		return (m_currentPos == m_bufferSize) ? PIPE_Stream_End : PIPE_Ok;
	}
	else
		return m_status;
}

//============================================================================
// ExecNode
//============================================================================

ExecNode::ExecNode(String cmdLine, PipeRWMode openMode)
  : FileNode	(),
	m_cmdLine	(StripQuotes (cmdLine))
{
}

ExecNode::~ExecNode()
{
	// Close only if open was successful
	if (m_file && m_closeFile)
		pclose (m_file); // m_file->Close();
}


void ExecNode::Open()
{
	const char* openMode;
	
	if (m_openMode == PIPE_RWUnknown)
	{
		if (m_prev)
			m_openMode = PIPE_Write;
		else
			m_openMode = PIPE_Read;
	}
	
	// Determine the file/open mode
	switch (m_openMode)
	{
		case PIPE_Read:		openMode = "r"; break;
		case PIPE_Write:	openMode = "w"; break;
		default:			ASSERT(false);  break;
	}
	
	// Non-NULL pointer indicates a FILE* pointer was assigned via Attach() call
	if (m_file == NULL)
	{
		m_file = popen (m_cmdLine, openMode);
		if (m_file == NULL)
			m_status = PIPE_Stream_Error;
	}
}
	
//============================================================================
// HttpNode
//============================================================================

#define MAX_CONCURRENCY	1
	
int HttpNode::s_globalInitCount = 0;
	
static size_t writeCallback(char *d, size_t n, size_t l, void *p)
{
	HttpNode*	node	= CAST_(HttpNode, p);
	ULONG		amount  = n * l;
	
	return node->Write ((Byte*)d, amount);
}
	
HttpNode::HttpNode(String cmdLine)
  : MemoryNode	(),
	m_cmdLine	(cmdLine)
{
	ASSERT (s_globalInitCount >= 0);
	if (!s_globalInitCount++)
		curl_global_init(CURL_GLOBAL_ALL);
	
	m_cm = curl_multi_init();
	m_urls[0] = (Byte*)(LPCSTR)cmdLine;
	
	curl_multi_setopt(m_cm, CURLMOPT_MAXCONNECTS, MAX_CONCURRENCY);
	for (int i = 0; i < MAX_CONCURRENCY; i++)
	{
		CURL *e = curl_easy_init();
		
		curl_easy_setopt (e, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt (e, CURLOPT_WRITEDATA, this);
		curl_easy_setopt (e, CURLOPT_HEADER, 0L);
		curl_easy_setopt (e, CURLOPT_URL, m_urls[i]);
		curl_easy_setopt (e, CURLOPT_PRIVATE, m_urls[i]);
		curl_easy_setopt (e, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt (e, CURLOPT_FOLLOWLOCATION, 1);
				
		curl_multi_add_handle(m_cm, e);
	}
}

HttpNode::~HttpNode()
{
	curl_multi_cleanup(m_cm);
	
	ASSERT(s_globalInitCount >= 1);
	if (!--s_globalInitCount)
		curl_global_cleanup();
}


ULONG HttpNode::Read (Byte* where, ULONG amount)
{
	int		U = -1;

	while (m_status == PIPE_Ok && U)
	{
		curl_multi_perform(m_cm, &U);
		
		if (U)
		{
			fd_set	R, W, E;
			int		M;
			long	L;
			
			FD_ZERO(&R);
			FD_ZERO(&W);
			FD_ZERO(&E);
			
			if(curl_multi_fdset(m_cm, &R, &W, &E, &M)) {
				fprintf(stderr, "ERROR: curl_multi_fdset\n");
				m_status = PIPE_Buf_Error;
			}
			
			if(curl_multi_timeout(m_cm, &L)) {
				fprintf(stderr, "ERROR: curl_multi_timeout\n");
				m_status = PIPE_Timeout_Error;
			}
			if(L == -1)
				L = 100;
			
			if(M == -1) {
#ifdef WIN32
				Sleep(L);
#else
				sleep((unsigned int)L / 1000);
#endif
			}
			else
			{
				struct timeval	T;
				
				T.tv_sec = L/1000;
				T.tv_usec = (L%1000)*1000;
				
				if(0 > select(M+1, &R, &W, &E, &T)) {
					fprintf(stderr, "ERROR: select(%i,,,,%li): %i: %s\n",
							M+1, L, errno, strerror(errno));
					m_status = PIPE_Buf_Error;
				}
			}
		}
		
		int Q;
		
		while((m_msg = curl_multi_info_read(m_cm, &Q))) {
			if(m_msg->msg == CURLMSG_DONE) {
				char *url;
				char *location;
				long response_code;
				CURL *e = m_msg->easy_handle;
				
				curl_easy_getinfo(e, CURLINFO_PRIVATE, &url);
				if (m_msg->data.result != 0)
					fprintf(stderr, "RESULT: %d - %s <%s>\nfor more information, try 'curl -v -L %s' or 'wget %s' at the command prompt\n",
						m_msg->data.result, curl_easy_strerror(m_msg->data.result), url, url, url);

				curl_multi_remove_handle(m_cm, e);
				curl_easy_cleanup(e);
				
				m_avail_out = m_total_in;
				m_next_out  = m_bufferStart;
				m_status	= PIPE_Stream_End;
			}
			else {
				fprintf(stderr, "ERROR: CURLMsg (%d)\n", m_msg->msg);
				m_status = PIPE_Buf_Error;

			}
			
			//
			//			if(C < CNT) {
			//				init(m_cm, C++);
			//				U++; /* just to prevent it from remaining at 0 if there are more
			//					  URLs to get */
			//			}
		}
	}
	
	ULONG		written = 0;
	ULONG		counter = 0;
	
	// Assume that the PIPE_Stream_End has been reached, and now it is only a matter of moving data to output buffer
	if (m_avail_out)
		written = TransferData(where, &amount, &counter, PIPE_Read);
	
	return written;
}

	
//============================================================================
// NamedPipeNode
//============================================================================

NamedPipeNode::NamedPipeNode(String pipeName, PipeRWMode openMode)
  : FileNode	(pipeName, openMode),
	m_pipeHandle(-1)
{
	m_closeFile = true;
}

NamedPipeNode::~NamedPipeNode()
{
	// Close only if open was successful
	if (m_pipeHandle && m_closeFile)
	{
		close (m_pipeHandle); // m_file->Close();
		m_pipeHandle = NULL;
	}
}


void NamedPipeNode::Open()
{
	int openMode;
	
	if (m_openMode == PIPE_RWUnknown)
	{
		if (m_prev)
			m_openMode = PIPE_Write;
		else
			m_openMode = PIPE_Read;
	}
	
	// Determine the file/open mode
	switch (m_openMode)
	{
		case PIPE_Read:		openMode = O_RDONLY; break;
		case PIPE_Write:	openMode = O_WRONLY; break;
		default:			ASSERT(false);  break;
	}
	
	// Non-NULL pointer indicates a FILE* pointer was assigned via Attach() call
	if (m_pipeHandle == -1)
	{
		// does the named pipe exist?
		if (access (m_fileName, F_OK) == -1)
		{
			int result = mkfifo (m_fileName, 0777);
			if (result != 0)
			{
				VX_PRINTF (("Could not create named pipe FIFO %s\n", (LPCSTR)m_fileName));
				m_status = PIPE_Stream_Error;
			}
		}
		
		printf ("Synchronizing on named pipe:%s, %s mode\n\n", (LPCSTR)m_fileName, openMode == O_RDONLY ? "O_RDONLY" : "O_WRONLY");
		m_pipeHandle = open (m_fileName, openMode);
		
		if (m_pipeHandle < 0)
		{
			m_status = PIPE_Stream_Error;
			return;
		}
	}
}

ULONG NamedPipeNode::Read  (Byte* where, ULONG amount)
{
	if (m_pipeHandle == -1)
		Open();

	LONG result = 0;
	if (m_prev)
	{
		Byte		buffer[amount];
		
		// Do the Read() and cascade status from node
		result 		= m_prev->Read (where ? where : buffer, amount);
		m_status	= m_prev->Status();
		
		if (result)
			Write (where ? where : buffer, result);
	}
	else if (m_pipeHandle != -1)
		result = read (m_pipeHandle, where, amount);
	else
		result = 0;
	
	if (m_status == PIPE_Ok && !result)
		m_status = PIPE_Stream_Error;

#ifdef _DEBUG
	m_total_in += result;
	m_reads++;
#endif

	return result;
}


ULONG NamedPipeNode::Write (Byte* what,  ULONG amount)
{
	if (m_pipeHandle == -1)
		Open();

	ULONG result = 0;
	if (m_pipeHandle != -1)
		result = write (m_pipeHandle, what, amount);
	else
		m_status = PIPE_Stream_Error;
	
#ifdef _DEBUG
	// update total and internal status, don't overwrite PIPE_Stream_End
	m_total_out += result;
	m_writes++;
#endif

	return result;
}
	
//============================================================================
// FileNode 
//============================================================================

FileNode::FileNode()
  : PipeNode		(),
	m_file			(NULL),
	m_fileName		(),
	m_openMode		(PIPE_RWUnknown),
	m_closeFile		(false)
{
}

FileNode::FileNode(FILE* file, PipeRWMode mode)
  : PipeNode		(),
	m_file			(NULL),
	m_fileName		(),
	m_openMode		(PIPE_RWUnknown),
	m_closeFile		(false)
{
	Attach (file, mode);
}

  
FileNode::FileNode(String fileName, PipeRWMode openMode)
  : PipeNode		(),
	m_file			(NULL),
	m_fileName		(StripQuotes (fileName)),
	m_openMode		(openMode),
	m_closeFile		(true)
{
}

FileNode::~FileNode()
{
	// Close only if open was successful
	if (m_file && m_closeFile)
        fclose (m_file); // m_file->Close();
}


void FileNode::Open()
{
	const char* openMode;

	// Determine the file/open mode
	switch (m_openMode)
	{
		case PIPE_Read:		openMode = "r"; break;
		case PIPE_Write:	openMode = "w"; break;
        default:			ASSERT(false);  break;
	}

	// Non-NULL pointer indicates a FILE* pointer was assigned via Attach() call
	if (m_file == NULL)
	{
        m_file = fopen (m_fileName, openMode); // m_file->Open(m_fileName, openMode);
		if (m_file == NULL)
		{
			m_status = PIPE_Stream_Error;
			return;
		}
	}
}


FILE* FileNode::Attach (FILE* file, PipeRWMode openMode, bool closeOnExit /*= false*/)
{
	// Save for return value
	FILE*	prevFile = m_file;

	// Save file handle
	m_file = file;
	m_openMode = openMode;
	m_closeFile = closeOnExit;

	return prevFile;
}

FILE* FileNode::Detach()
{
	FILE*	prevFile = m_file;

	// Flush the file to ensure everything is written to destination 
	// if there is data in the buffer.  
    fflush (m_file); // m_file->Flush();

	// Zero out the file handle
	m_file = NULL;
	m_openMode = PIPE_RWUnknown;
	m_closeFile = false;

	return prevFile;
}


ULONG FileNode::Read (unsigned char* where, ULONG amount)
{
	// Need to open the file and init?
	if (m_file == NULL)
		Open();

	if (m_prev)
	{
		Byte		buffer[amount];
		
		// Do the Read() and cascade status from node
		amount		= m_prev->Read (where ? where : buffer, amount);
		m_status	= m_prev->Status();
		
		if (amount > 0)
			amount = Write (where ? where : buffer, amount);
	}
	else
	{
		// Handle EOF/errors before calling this routine.
		ASSERT (amount != 0);
		if (amount)
		{
			Byte	buffer[amount];
			
			TRY
			{
				amount = fread (where ? where : buffer, 1, amount, m_file); // m_file->Read (what, amount);
			}
			CATCH (CException, e)
			{
				THROW_LAST();
			}
			END_CATCH
			
#ifdef _DEBUG
			m_total_out += amount;
			m_reads++;
#endif
		}
	}
	
	// If at end of input, don't transfer any more - just return
	if (amount == 0)
		m_status = PIPE_Stream_End;

	return amount;
}


ULONG FileNode::Write (unsigned char* what,  ULONG amount)
{
	// Need to open the file and init?
	if (m_file == NULL)
		Open();

	// Handle EOF/errors before calling this routine.
	ASSERT (amount != 0);
	if (amount)
	{
		TRY
		{
			amount = fwrite (what, 1, amount, m_file); // m_file->Write (what, amount);
		}
		CATCH (CException, e)
		{
			THROW_LAST();
		}
		END_CATCH

#ifdef _DEBUG
		// update total and internal status, don't overwrite PIPE_Stream_End
		m_total_in += amount;
		m_writes++;
#endif
	}
	
	return amount;
}

ULONG FileNode::Seek (ULONG offset)
{
	// Shouldn't TRY anything when in error mode, but we can be at Stream_End.
	// Seek operations are currently only allowed during write operations.
	ASSERT (StatusOK());
	ASSERT (m_mode == PIPE_Write);
	
    fseek (m_file, (LONG)offset, 0); // m_file->Seek ((LONG)offset, CFile::begin);
	
	return offset;
}
	
ULONG FileNode::Size()
{
	struct	stat buffer;
	int		rc = stat (m_fileName, &buffer);
	return (rc == 0 ? buffer.st_size : -1);
}

void FileNode::Flush()
{
	// Flush our internal contents, to ensure everyting is written
	if (m_mode == PIPE_Write)
        fflush (m_file); // m_file->Flush();

	// Flush the next node in the pipeline
	if (m_next)
		m_next->Flush();
}


//============================================================================
// BitBucketNode
//============================================================================

ULONG BitBucketNode::Read (unsigned char* where, ULONG amount)
{
	// Update total and cascade pipe status
	m_count += amount = m_prev->Read (where, amount);
	m_status = m_prev->Status();

#ifdef _DEBUG
	m_total_in = m_count;
	m_reads++;
#endif

	return amount;
}


ULONG BitBucketNode::Write (unsigned char* what,  ULONG amount)
{
	// Update total and cascade pipe status
	m_count += amount = m_next->Write (what, amount);
	m_status = m_next->Status();

#ifdef _DEBUG
	m_total_out = m_count;
	m_reads++;
#endif

	return amount;
}
 
//============================================================================
// ZLibCompressNode
//============================================================================

ZLibCompressNode::ZLibCompressNode (What2Do what2do, ZlibLevel level, ULONG inSize , ULONG outSize)
:   PipeNode        (),
    m_stream		(NULL),
    m_inBuffer		(NULL),
    m_outBuffer		(NULL),
    m_inSize		(inSize),
    m_outSize		(outSize),
    m_what2do		(what2do),
    m_compressLevel (level),
    m_flushMode		(Z_PARTIAL_FLUSH),
    m_init			(true)
{
    // Take into account that LARGE => DEFlate => small and
    // small => infLATE => LARGE.  Reset in/outSize if the buffer
    // sizes are improperly set - default constructor assumes Read().
    if ((m_what2do == COMPRESS   && m_inSize < m_outSize) ||
        (m_what2do == DECOMPRESS && m_outSize < m_inSize))
    {
        // Swap in and out size for optimal throughput.
        ULONG tempLong  = m_inSize;
        m_inSize  = m_outSize;
        m_outSize = tempLong;
    }
    
    m_optimal_in = m_inSize;
    m_optimal_out = m_outSize;
}

ZLibCompressNode::~ZLibCompressNode()
{
    // Do cleanup only if library was inited
    if (!m_init)
    {
        // Is there any pending buffer stuff to be written?
        if (m_mode == PIPE_Write)
        {
            // Set flush mode for compressor, and continue until in/out exhausted
            m_flushMode = Z_FINISH;
            while (m_status == PIPE_Ok)
                Write (NULL, m_outSize);
        }
        
        int	libResult;
        
        // Un-initialize the compression library
        if (m_what2do == COMPRESS)
            libResult = deflateEnd (m_stream);
        
        else // if (m_what2do == DECOMPRESS)
            libResult = inflateEnd (m_stream);
        
        // Check shutdown status
        assert (!StatusOK() || (m_status == PIPE_Stream_End && libResult == Z_OK));
    }
    
    // free the stream structure and buffers
    delete m_stream;
    delete [] m_inBuffer;
    delete [] m_outBuffer;
}


void ZLibCompressNode::Init()
{
    // Make sure this is really needed
    assert (m_init);
    
    TRY
    {
        // allocate buffers
        m_stream	= new z_stream;
        m_inBuffer	= new Byte[m_inSize];
        m_outBuffer = new Byte[m_outSize];
        
        // set optimal in/out buffer sizes.  Note that these were also
        // set in the constructor, so optimal read/write may not happen
        // if the value is cached by the callee.
        m_optimal_in = m_inSize;
        m_optimal_out = m_outSize;
        
        // Set up data stream type and memory callbacks to NULL
        m_stream->data_type = Z_BINARY;
        m_stream->zalloc    = Z_NULL; // (alloc_func)ZLibCompressNode::Alloc;
        m_stream->zfree		= Z_NULL; // (free_func) ZLibCompressNode::Free;
        
        // Initialize the compression library
        int	libResult;
        if (m_what2do == COMPRESS)
            libResult = deflateInit (m_stream, m_compressLevel);
        
        else if (m_what2do == DECOMPRESS)
            libResult = inflateInit (m_stream);
        
        else
            ASSERT(false);	// Something is really wrong...
        
        // set initialized flag only now to allow for proper destructor cleanup
        m_init = false;
        
        // set our internal status based on return.  Whine if something is wrong.
        m_status = ZLib2PipeResult (libResult);
        assert (m_status == PIPE_Ok);
	}
    CATCH_ALL
    {
        // set error state, cleanup buffers in destructor, don't rethrow
        m_status = PIPE_Mem_Error;
        
        // KAK: Untested code - check flow of control from here...
        ASSERT(false);
    }
    END_CATCH
}


ULONG ZLibCompressNode::Filter()
{
    // Should always have somewhere to write into
    assert (m_avail_out != 0);
    
    // Set stream pointers for deflate call
    m_stream->avail_in	= m_avail_in;
    m_stream->next_in	= m_next_in;
    m_stream->avail_out = m_avail_out;
    m_stream->next_out  = m_next_out;
    
    // compress and update status if not in error state
    int libResult;
    if (m_what2do == COMPRESS)
        libResult = deflate (m_stream, m_flushMode);
    else
        libResult = inflate (m_stream, m_flushMode);
    
    // How much is now available in output buffer
    ULONG increment = m_outSize - m_stream->avail_out;
//    assert (increment != 0);
    
    // PIPE_Stream_End is set in the Process() routine once
    // all input is completely exhausted.  Setting EOS here
    // can result in some data being left unflushed.
    PipeResult status = ZLib2PipeResult (libResult);
    if (StatusOK())
        m_status  = status;
    
    // Reset internal pointers; m_avail_out assigned return
    // value from this method, m_next_out set in TransferData()
    m_avail_in  = m_stream->avail_in;
    m_next_in   = m_stream->next_in;
    
    // return the amount that was processed.
    return increment;
}


ULONG ZLibCompressNode::Read (Byte* where, ULONG amount)
{
    // First-time initialization of ZLib library required?
    if (m_init)	Init ();
    
    // Maintain running total
    ULONG	total = 0;
    while (amount && Status() == PIPE_Ok)
    {
        // Is the output buffer exhausted?
        if (m_avail_out == 0)
        {
            // 'Replenish' buffer by resetting count, pointer
            m_avail_out	= m_outSize;
            m_next_out 	= m_outBuffer;
            
            // Is the source buffer exhausted, but not at stream end?
            if (m_avail_in == 0)
            {
                // Reset pointers to beginning of input buffer
                m_avail_in = m_inSize;
                m_next_in  = m_inBuffer;
                
                // Do the Read() and cascade status from node
                m_avail_in = m_prev->Read (m_next_in, m_avail_in);
                m_status   = m_prev->Status();
                assert (StatusOK());
#ifdef _DEBUG
                m_total_in += m_avail_in;
#endif
                
                // Read indicates nothing available, but we will let the return
                // status from the compressor set PIPE_Stream_End when the output
                // buffer is completely exhausted.
                if (m_status == PIPE_Stream_End)
                {
                    m_flushMode = Z_FINISH;
                    m_status = PIPE_Ok;
                }
                else if (m_avail_in == 0)
                {
                    assert (!StatusOK() || m_status == PIPE_Stream_End);
                    break;
                }
            }
            
            // in => deflate/inflate => out, amount handled returned
            m_avail_out = Filter();
            if (m_avail_out == 0 && m_status == PIPE_Stream_End)
                break;
        }
        
        // Move data from out buffer into destination, update amount/total
        ULONG increment = TransferData (where, &amount, &total, PIPE_Read);
        if (where) where += increment;
    }
    
    return total;
}

ULONG ZLibCompressNode::Write (Byte* what,  ULONG amount)
{
    // First-time initialization of ZLib library required?
    if (m_init)	Init();
    
    // Maintain running total
    ULONG	total = 0;
    bool	clientBuffer = false;
    
    while (amount && m_status == PIPE_Ok)
    {
        // Avoid resetting input if client buffer is being exhausted.
        if (!clientBuffer)
        {
            // Is the input buffer exhausted?
            if (m_avail_in == 0)
            {
                // Special case handling during flush: don't transfer to
                // input buffer, use buffer passed on from callee.
                // DOES THIS HELP FOR ONETIME-MEMORY CONVERSIONS?
                if (m_flushMode == Z_FINISH)
                {
                    m_avail_in   = amount;
                    m_next_in    = what;
                    clientBuffer = true;
                }
                else
                {
                    // 'Replenish' buffer by resetting count, pointer
                    m_avail_in = m_inSize;
                    m_next_in  = m_inBuffer;
                }
            }
            else if (m_flushMode == Z_FINISH)
            {
                // If not flushing client's buffer, look to see if there is
                // some data in the in buffer.  Readjust the actual available
                // in amount in preparation for the Filter() call.
                m_avail_in  = m_inSize - m_avail_in;
                m_next_in   = m_inBuffer;
            }
        }
        
        if (m_flushMode != Z_FINISH)
        {
            // Otherwise, move data to in buffer
            ULONG	increment = TransferData (what, &amount, &total, PIPE_Write);
            if (what) what += increment;
        }
        
        if (m_avail_in == 0 || m_flushMode == Z_FINISH)
        {
            // Set up buffers for input.
            m_avail_out = m_outSize;
            m_next_out  = m_outBuffer;
            
            // in => deflate/inflate => out, amount handled returned
            ULONG increment = Filter();
            
            // Write into next node in pipeline	propogate status from node
            m_next->Write (m_outBuffer, increment);
            
            // Avoid overwritting PIPE_Stream_End or error condition
            if (m_status == PIPE_Ok)
                m_status = m_next->Status();
        }
    }
    
    return total;
}

void ZLibCompressNode::Flush()
{
    // Set flush mode
    m_flushMode = Z_FINISH;
    
    m_next->Flush ();
}

    
    
#ifdef THISDIDNTWORK
// compression library callbacks
voidpf _cdecl ZLibCompressNode::Alloc (voidpf opaque, uInt items, uInt size)
{
    // make compiler happy
    if (opaque) items += size - size;	
    
    return calloc(items, size);
    // Un-comment following line to get idle-time deletion via ZMidnightKiller
    // return MemSys_Alloc (items * size);
}


// compression library callbacks
void _cdecl ZLibCompressNode::Free (voidpf opaque, voidpf address)
{
    free (address);
    // Un-comment following line to get idle-time deletion via ZMidnightKiller
    // MemSys_Kill (&address);
    
    // make compiler happy    
    if (opaque) return;
}
#endif


// error code conversion
PipeResult ZLibCompressNode::ZLib2PipeResult (int ZLibReturn)
{
    switch (ZLibReturn)
    {
        case Z_OK:				return PIPE_Ok;
        case Z_STREAM_END:		return PIPE_Stream_End;
        case Z_NEED_DICT:		return PIPE_Data_Error;
        case Z_ERRNO:			return PIPE_Errno;
        case Z_STREAM_ERROR:	return PIPE_Stream_Error;
        case Z_DATA_ERROR:		return PIPE_Data_Error;
        case Z_MEM_ERROR:		return PIPE_Mem_Error;
        case Z_BUF_ERROR:		return PIPE_Buf_Error;
        case Z_VERSION_ERROR:	return PIPE_Version_Error;
    }
    
    return PIPE_Errno;
}


ULONG ZLibCompressNode::Size()
{
    return m_stream->total_out;
}

PipeResult ZLibCompressNode::Status()
{
    // Special case handling when stream end is encountered
    if (m_status == PIPE_Stream_End)
        // Make sure output buffer is completely cleared for stream end
        return m_avail_out == 0 ? PIPE_Stream_End : PIPE_Ok;
    else
        // All other situations, just return current status
        return m_status;
}
    
    
} // namespace Core
} // namespace Vixen

