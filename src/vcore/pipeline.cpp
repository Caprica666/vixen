//***************************************************************************
//
//		File:	Pipeline.cpp - Data transformation operations
//
//***************************************************************************

#include "stdafx.h"
#include "pipeobjs.h"
#include <math.h>
#include <stdio.h>

namespace Vixen {
namespace Core {

//============================================================================
// PipeNode
//============================================================================

IMPLEMENT_DYNAMIC (PipeNode, CObject);

// Zero-initialize everything, since this object may
// be placed on the stack and not zero-initialized.
PipeNode::PipeNode ()
  : m_status		(PIPE_Null_Pipe),
	m_mode			(PIPE_RWUnknown),
	m_pipeline		(NULL),
	m_next			(NULL),
	m_prev			(NULL),
	m_next_in		(NULL),
	m_next_out		(NULL),
	m_avail_in		(0),
	m_avail_out		(0),
	m_optimal_in	(0),
	m_optimal_out	(0)
{
#ifdef _DEBUG
    m_total_in = m_total_out = 0;
    m_reads = m_writes = 0;
#endif
}

#if defined (_DEBUG)
PipeNode::~PipeNode()
{
	// Ensure this isn't part of some active Pipeline
	ASSERT (m_pipeline == NULL);
}
#endif

// concatenation operator for nodes
PipeNode&	PipeNode::operator|(PipeNode& insert)
{
	// Make sure the new node isn't part of an existing Pipeline
	ASSERT (!insert.m_next && !insert.m_prev && !insert.m_pipeline);
	
	// must proceed to the end of the pipe to get insertion pt
	PipeNode*	node = this;
	while (node->m_next)
		node = node->m_next;

	// Append to end
	insert.m_prev = node;
	node->m_next = &insert;

	// return reference to self to make pipe operator work "naturally"
	return *this;
}


ULONG PipeNode::TransferData (unsigned char* where, ULONG* amountp, ULONG* counterp, PipeRWMode readWrite)
{
	ASSERT (*amountp != 0);
	ULONG increment;

	if (readWrite == PIPE_Write)
	{
		increment = MIN (*amountp, m_avail_in);

		// Move data from input buffer to output buffer
		if (where) memcpy (m_next_in, where, increment);

		// update pointers and counts
		m_next_in  += increment;
		m_avail_in -= increment;
#ifdef _DEBUG
        m_total_in += increment;
        m_reads++;
#endif
	}
	else if (readWrite == PIPE_Read)
	{
		increment = MIN (*amountp, m_avail_out);

		// Move data from input buffer to output buffer
		if (where) memcpy (where, m_next_out, increment);

		// update pointers and counts
		m_next_out  += increment;
		m_avail_out -= increment;
#ifdef _DEBUG
        m_total_out += increment;
        m_writes++;
#endif
	}
	else
		ASSERT(false);

	// Most likely will lead to infinite loop in calling routine.
	ASSERT (increment != 0);

	// Update client's counts
	*amountp  -= increment;
	*counterp += increment;

	// Make sure we aren't changing read/write mode mid-stream.  Save mode
	// ASSERT (m_mode == PIPE_RWUnknown || m_mode == readWrite);	
	m_mode = readWrite;
    

	// return actual amount that was moved
	return increment;
}


ULONG PipeNode::OptimalBufferSize()
{
	// We start at either a source or sink and follow either
	// the next or prev pointers to determine an optimal size.
	// m_prev of NULL indicates a sink during write operation.
	bool		checkNext = (m_prev == NULL);
	ULONG		optimal = 0;
	PipeNode*	check = this;

	do
	{
		// Read optimal value before advancing
		if (checkNext)
		{
			optimal = check->m_optimal_in;
			check   = check->m_next;
		}
		else
		{
			optimal = check->m_optimal_out;
			check   = check->m_prev;
		}
	}
	// Continue down the chain until a non-zero value is found
	while (optimal == 0 && check);

	// If nobody in the Pipeline has an opinion, return default 2048
	if (optimal == 0)
		optimal = OPTIMAL_BUFFERSIZE / 2;

	return optimal;
}


//============================================================================
// Pipeline
//============================================================================

IMPLEMENT_DYNAMIC (Pipeline, CObject);

Pipeline::Pipeline()
  : m_pipeEnd		(NULL),
	m_pipeBegin		(NULL),
	m_status		(PIPE_Null_Pipe),
	m_mode			(PIPE_RWUnknown),
	m_deleteNodes	(false)
{
}


Pipeline::Pipeline(PipeNode& pipeSource)
  : m_pipeEnd		(NULL),
	m_pipeBegin		(NULL),
	m_status		(PIPE_Null_Pipe),
	m_mode			(PIPE_RWUnknown),
	m_deleteNodes	(false)
{
	// Use assignment operator to do the 'real' work.
	this->operator=(pipeSource);
}

Pipeline::~Pipeline()
{
#ifdef _DEBUG
    VX_PRINTF (("\n~Pipeline ( "));
    for (PipeNode* node = m_pipeBegin; node; node = node->m_next)
    {
        if (node->ISA_(FileNode))
			VX_PRINTF (("%s:%s", node->Signature(), (LPCSTR)CAST_(FileNode, node)->FileName()));
        else
            VX_PRINTF ((node->Signature()));
					   
        VX_PRINTF ((" %s", node->m_next ? " | ":""));
	}
    VX_PRINTF ((")\n"));
    for (PipeNode* node = m_pipeBegin; node; node = node->m_next)
    {
        ULONG   in, out, rd, wr;
        node->Stats (in, out, rd, wr);
        
        VX_PRINTF (("%ld -> %ld (r:%ld,w:%ld)%s", in, out, rd, wr, node->m_next ? " | ":""));
    }
    VX_PRINTF (("\n\n"));    
#endif
    
	// clear pipe node pointers
	Reset();
}


void Pipeline::Reset()
{
	// Are we already in an empty state?
	if (m_status == PIPE_Null_Pipe)
		return;

	// Walk the list forward until we get to the end of the linked list
	while (m_pipeBegin)
	{
		PipeNode*		release = m_pipeBegin;
	
		// Get next node pointer before resetting m_next.
		m_pipeBegin = m_pipeBegin->m_next;

		// Reset pointers
		release->m_pipeline = NULL;
		release->m_next = release->m_prev = NULL;

		// Delete nodes at client's request
		if (m_deleteNodes)
			delete release;
	}

	// set status to empty
	m_status = PIPE_Null_Pipe;
}


ULONG Pipeline::Read (unsigned char* inBuffer,  ULONG amount)
{
	m_mode = PIPE_Read;
	ULONG	total = 0;
//	while (amount > 0 /* && m_status == PIPE_Ok */)
	{
		// Request read operation and check status return 
		ULONG result = m_pipeEnd->Read (inBuffer, amount);

		// update count and pointer; watch for underflow values.
		total  += result;
		ASSERT (result <= amount);

		// Update our status based on the head node's
		m_status = m_pipeEnd->Status();
	}

	return total;
}

ULONG Pipeline::Write (unsigned char* outBuffer, ULONG amount)
{
	m_mode = PIPE_Write;
	ULONG	total = 0;
	while (amount && m_status == PIPE_Ok)
	{
		// Request read operation and check status return 
		ULONG delta = m_pipeBegin->Write (outBuffer, amount);
		
		// update count and pointer
		total  += delta;
		amount -= delta;
		if (outBuffer != NULL)
			outBuffer = outBuffer + delta;

		// Update our status based on the head node's
		m_status = m_pipeBegin->Status();
	}

	return total;
}

ULONG Pipeline::Seek (ULONG pos)
{
	PipeNode*	node = NULL;

	if (m_mode == PIPE_Write)
		node = m_pipeBegin;

	else if (m_mode == PIPE_Read)
		node = m_pipeEnd;

	else
		// TODO: Adaptive behavior: Insert seek adapter to service the request.
		ASSERT(false);

	// Perform seek and update status based on the node's
	if (node)
	{
		pos = node->Seek (pos);
		m_status = node->Status();
		return pos;
	}
	else
		// Seek failed
		return (ULONG) -1;
}

void Pipeline::Flush()
{
	// we should only be doing this following Write() operations
	// I don't know what the reprocussions are, but an ASSERT() would help...
	m_pipeBegin->Flush ();
}


ULONG Pipeline::Run()
{
	if (m_pipeBegin == NULL)
		return 0;
	
	// allow for individual nodes to adjust their "neighbors" - astc for example, will
	// collapse a input and output of FileNodes into command line parameters
	do
	{
		m_adjusted = false;
		for (PipeNode* node = Head(); node; node = node->m_next)
			node->AdjustNextPrev();
	}
	while (m_adjusted);

	ULONG	total = 0;
	ULONG	optimal = m_pipeEnd->OptimalBufferSize();
	long	delta;
	Byte	buffer[optimal];
	
	// continue reading stream until PIPE_Stream_End or error
	do
	{
		total += delta = Read (buffer, optimal);
		if (delta > 0 && !Tail()->ISA_(BitBucketNode))
			fwrite (buffer, 1, delta, stdout);
	}
	while (delta || m_status == PIPE_Ok);

	// return total amount of data processed
	return total;
}

Pipeline&	Pipeline::operator=(PipeNode& pipeSource)
{
	// Clear out any existing elements and set status
	Reset();
	m_status = PIPE_Ok;

	// Save off pipe start for Write() operations
	m_pipeBegin = &pipeSource;

	// Walk the list forward until we get to the end of the linked list.
	// m_pipeEnd is set to the pipe end for Read() operations.
	for (m_pipeEnd = &pipeSource; ; m_pipeEnd = m_pipeEnd->m_next)
	{
		// Set Pipeline pointer and before going to next node.
		m_pipeEnd->m_pipeline = this;
		m_pipeEnd->m_status = PIPE_Ok;

		// Reached the end of the linked list?
		if (m_pipeEnd->m_next == NULL)
			break;
	}

	// Return reference to this object
	return *this;
}

	
void Pipeline::Append (PipeNode* node)
{
	// Is this the first node to be added to the pipeline?
	if (m_pipeBegin == NULL)
	{
		ASSERT (m_pipeEnd == NULL);
		m_pipeBegin = m_pipeEnd = node;
	}
	// Append it to the end using the operator|()
	else if (m_pipeEnd)
	{
		m_pipeEnd->operator| (*node);
		m_pipeEnd = node;
	}
	
	// indicate that something was adjusted and set the status codes that we are ready to run()
	node->m_pipeline = this;
	node->m_status = PIPE_Ok;
	
	m_status = PIPE_Ok;
	m_adjusted = true;
}
	
void Pipeline::Remove (PipeNode* node)
{
	if (m_pipeBegin == node)
		m_pipeBegin =  node->m_next;
	
	if (m_pipeEnd == node)
		m_pipeEnd =  node->m_prev;
	
	// We must assume that at least one node remains, or that both
	// pipe begin and end now are NULL - the pipeline has been destroyed
	if (m_pipeEnd != m_pipeBegin)
	{
		if (node->m_prev)
			node->m_prev->m_next = node->m_next;
		
		if (node->m_next)
			node->m_next->m_prev = node->m_prev;
	}
	else if (m_pipeBegin == NULL)
		m_status = PIPE_Null_Pipe;
	
	// Clean up the node so that the destructor will not have any complaints
	node->m_pipeline = NULL;
	node->m_status = PIPE_Null_Pipe;
	if (m_deleteNodes)
		delete node;

	// signal that something changed
	m_adjusted = true;
}
	
    
}   // namespace Core
}   // namespace Vixen
