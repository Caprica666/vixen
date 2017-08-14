///*************************************************************************
//
//		File:	Pipeline.h - Data transformation operations
//
///*************************************************************************

#pragma once

namespace Vixen::Core {

// PipeNode use the | operator (pipe) to concatenate nodes using 
// a UNIX-like syntax.  It proves flexibility in how individual pipe
// components are assembled and whether the pipeline results in a 
// stream (no seek operations) or is built with adapters to allow 
// non-streamable (seek() allowed) access to data.
//
// Example composes a pipeline consisting of a file reader, compressor,
// with results written into a memory block.  User calls are directed
// at the pipeline object to handle data flow control amongst the pipe nodes.
//
//	bool FileCompressToMemory (Str fileName, char* destMemoryBlock)
//	{
//		FileNode			readFile(fileName, PIPE_Read);
//		ZLibCompressNode	compress();
//		BufferedNode		intoMemory(destMemoryBlock);
//		Pipeline            pipe = readFile | compress | intoMemory;
//
//		// Since we have source/sink, just put pipeline into motion
//		pipe.Run();
//
//		// return True if read completed to end, False for any other error
//		return (pipe.Status() == PIPE_Stream_End);
//	}

///***************************************************************************

// forward declarations
class Pipeline;

enum PipeResult   { PIPE_Ok				=   0,	// normal operation 
					PIPE_Stream_End		=   1,	// end of data encountered
					PIPE_Errno			=  -1,	// generic error
					PIPE_Stream_Error	=  -2,	// read/write error
					PIPE_Data_Error		=  -3,	// invalid data (CRC check)
					PIPE_Mem_Error		=  -4,	// memory allocation fail
					PIPE_Buf_Error		=  -5,	// buffer size of zero
					PIPE_Version_Error	=  -6,	// version check fail
					PIPE_Seek_Error		= -10,	// seek opertion not supported
					PIPE_User_Abort		= -11,	// user abort
					PIPE_Invalid_Pipe   = -12,	// invalid pipeline setup or Null
					PIPE_Null_Pipe      = -13,	// Null or unitialized pipeline
					PIPE_Timeout_Error	= -14};	// Timeout during i/o

enum PipeRWMode	  { PIPE_RWUnknown, PIPE_Read, PIPE_Write };	// Data flow: via Read()/Write()
	
#ifndef Byte
typedef unsigned char Byte;
#endif
	
#define OPTIMAL_BUFFERSIZE		16384
	
//============================================================================
class PipeNode : public CObject
//============================================================================
{
    DECLARE_DYNAMIC (PipeNode);
public:
	// constructor, virtual destructor from CObject (_DEBUG has debug checks)
	PipeNode();
	#if defined (_DEBUG)
	virtual ~PipeNode();
	#endif

	// overridables
    virtual ULONG		Read  (Byte* where, ULONG amount)   { ASSERT(false); return (ULONG)-1; }
    virtual ULONG		Write (Byte* what,  ULONG amount)   { ASSERT(false); return (ULONG)-1; }
	virtual ULONG		Seek  (ULONG offset)                { ASSERT(false); return (ULONG)-1; }
	virtual void		Flush ()							{ if (m_next) m_next->Flush(); }
    virtual const char* Signature()							{ return ""; }

	// Status information
	virtual PipeResult	Status()							{ return m_status; }
	virtual void		AdjustNextPrev()					{ }
	bool				StatusOK()							{ return Status() >= PIPE_Ok; }
	Pipeline*           Pipeline()							{ return m_pipeline; }
	PipeRWMode			Mode()								{ return m_mode; }

	// concatenation operator for pipe nodes
	PipeNode&			operator|(PipeNode& insertNext);
	PipeNode*			m_next;				// linkage handled by operator| and checked
	PipeNode*			m_prev;				// by Pipeline for integrity
    
#ifdef _DEBUG
    void                Stats (ULONG& in, ULONG& out, ULONG& rd, ULONG& wr)
							{ in = m_total_in; out = m_total_out; rd = m_reads; wr = m_writes; }
#endif

	// Transfer data to callee's buffer, updates m_next and m_avail
	ULONG				TransferData (Byte* wherep, ULONG* amountp, ULONG* counterp, PipeRWMode readWrite);

protected:
    Byte*               m_next_in;			// next input char
    ULONG				m_avail_in;			// number of chars available at next_in 
    Byte*               m_next_out;			// next output char should be put there
    ULONG				m_avail_out;		// remaining free space at next_out
	
	ULONG				m_optimal_in;		// optimal read value, 0 = N/A
	ULONG				m_optimal_out;		// optimal write amount, 0 = N/A

	PipeResult			m_status;			// result of last Read/Write/Seek request
	PipeRWMode			m_mode;				// Read/Write operations being performed
    
	class Pipeline*     m_pipeline;			// pointer set by Pipeline
	
#ifdef _DEBUG
    ULONG               m_total_in;         // garbage in
    ULONG               m_total_out;        // garbage out
    ULONG               m_reads;			// read count
    ULONG               m_writes;			// buffer write count via TransferData()
#endif
    
private:
	friend class Pipeline;

	// Optimal buffer throughput - called by Pipeline::Run()
	ULONG				OptimalBufferSize();

	// prevent copy and assignment
	PipeNode (const PipeNode&);
	PipeNode& operator= (const PipeNode&);
};


//===========================================================================
class Pipeline : public CObject	    
//===========================================================================
{
    DECLARE_DYNAMIC (Pipeline);
public:
	// constructor/destructor
	Pipeline ();
	Pipeline (PipeNode& pipeSource);
	virtual ~Pipeline();

	// status of last requested operation
	inline PipeResult	Status()				{ return m_status; }

	// public interface
    ULONG				Read  (Byte* inBuffer,  ULONG amount);
    ULONG				Write (Byte* outBuffer, ULONG amount);
	ULONG				Seek  (ULONG offset);
	void				Flush ();

	// Run can be called with source/sink combination.
	// Size queries head/tail; works only if ZSeekAdapter/ZBuffereredNode present.
	ULONG				Run();

	// Assign this a value of True for heap-allocated nodes.  Pipeline deletes in dtor.
	bool&				DeleteNodesOnExit()		{ return m_deleteNodes; }
	PipeNode*			Head()					{ return m_pipeBegin; }
	PipeNode*			Tail()					{ return m_pipeEnd; }

	// assignment of pipe node start
	Pipeline&			operator=(PipeNode& pipeSource);
	
	// append a node to the end of the existing chain, or remove node
	void				Append (PipeNode* node);
	void				Remove (PipeNode* node);

protected:
	PipeNode*			m_pipeBegin;
	PipeNode*			m_pipeEnd;
	PipeResult			m_status;
	PipeRWMode			m_mode;
	bool				m_deleteNodes;
	bool				m_adjusted;

	// clear pointers to existing pipe nodes
	virtual void		Reset();

private:
	// prevent copy and assignment
	Pipeline (const Pipeline& other);
	Pipeline& operator=(Pipeline& other);
};

        
}   // namespace Core

