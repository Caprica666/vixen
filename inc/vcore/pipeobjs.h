///*************************************************************************
//
//		File:	Pipeobjs.h - Data pipeline objects
//
///*************************************************************************

#ifndef pipeobjs_h
#define pipeobjs_h

///***************************************************************************

#include "pipeline.h"	// for PipeNode, Dataline base class declarations

#include <curl/multi.h>

namespace Vixen {
namespace Core {

// class CFile;
	
//============================================================================
class MemoryNode : public PipeNode
//============================================================================
// Can be used to be either a source or sink depending on the constructor.
{
	DECLARE_DYNAMIC (MemoryNode);
public:
	// constructors/destructor
	MemoryNode ();						// sink - extract results via Buffer()
	MemoryNode (Byte* b, ULONG max);	// source - buffer and size
	virtual ~MemoryNode();

	// overridden PipeNode methods to act as either a sink or source
    virtual ULONG		Read  (Byte* where, ULONG amount);
    virtual ULONG		Write (Byte* what,  ULONG amount);
	virtual ULONG		Size();
	virtual void		Flush();
	virtual const char* Signature()		{ return "memo"; }

	// extract buffer contents once stream is complete
	Byte*               Attach (Byte* b, ULONG max);
	Byte*               Detach();

	// return writable allocation increment value
	ULONG&				AllocDelta()	{ return m_allocDelta; }
	bool&				KillBuffer()	{ return m_killBuffer; }
	String&				FileName()		{ return m_fileName; }

protected:
	void				CommonInit (Byte* buffer, ULONG maxCount);

	bool				m_canGrow;		// True iff buffer can be grown
	bool				m_killBuffer;	// Call 'delete m_bufferStart' in destructor?

    ULONG				m_total_in;		// total # of input chars read so far 
    ULONG				m_total_out;	// total # of chars output so far 

	Byte*               m_bufferStart;	// buffer pointer
	ULONG				m_bufferSize;	// current buffer size
	ULONG				m_allocDelta;	// allocation increment
	
	String				m_fileName;		// shared memory location
};


//============================================================================
class SeekAdapterNode : public MemoryNode
//============================================================================
// Adapter between QStringeamable nodes (FileNode, ZCompressNodes) and 
// non-QStringeamable nodes that require seek operations.
{
	DECLARE_DYNAMIC (SeekAdapterNode);
public:
	// constructor/destructor
	SeekAdapterNode();

	// overridden read/write functions to handle seek buffering
    virtual ULONG		Read  (Byte* where, ULONG amount);
    virtual ULONG		Write (Byte* what,  ULONG amount);
	virtual ULONG		Seek  (ULONG offset);

	// Status overridden to handle QStringeam_End, with currentPos < bufferSize
	virtual PipeResult Status();

protected:
	ULONG				m_currentPos;
	ULONG				m_totalBuffered;
};										
										


//============================================================================
class FileNode : public PipeNode
//============================================================================
// Can be used to be either a source or sink depending on the constructor.
{
	DECLARE_DYNAMIC (FileNode);
public:
	// constructors
	FileNode ();
	FileNode (FILE* file, PipeRWMode mode);
	FileNode (String fileName, PipeRWMode openMode = PIPE_Read);
	virtual ~FileNode();

	// overridden PipeNode methods to act as either a sink or source
    virtual ULONG		Read  (Byte* where, ULONG amount);
    virtual ULONG		Write (Byte* what,  ULONG amount);
	virtual ULONG		Seek  (ULONG offset);
	virtual void		Flush ();
    virtual const char* Signature()				{ return "file"; }

	// attach/detach file handle
	FILE*				Attach (FILE* file, PipeRWMode openMode, bool closeOnExit = false);
	FILE*				Detach ();
	bool&				CloseOnExit ()			{ return m_closeFile; }
    
    // attributes
    String&             FileName()              { return m_fileName; }
	ULONG				Size();

protected:
	// Open() alongside of the destructor account for much of the variation in opening FILE*
	virtual void		Open();

	FILE*				m_file;				// file pointer from fopen()
	String				m_fileName;			// QString name
	PipeRWMode			m_openMode;			// file/open mode
	bool				m_closeFile;		// True = close in destructor
};

	
//============================================================================
class ExecNode : public FileNode
//============================================================================
{
	DECLARE_DYNAMIC (ExecNode);
public:
	ExecNode (String cmdLine = "", PipeRWMode openMode = PIPE_RWUnknown);
	virtual ~ExecNode();
	
	// overridden FileNode methods
	virtual ULONG		Seek  (ULONG offset)	{ ASSERT (false); }
	virtual const char* Signature()				{ return "exec"; }

protected:
	virtual void		Open();
	
	String				m_cmdLine;
};
	
//============================================================================
class HttpNode : public MemoryNode
//============================================================================
{
	DECLARE_DYNAMIC (HttpNode);
public:
	HttpNode (String cmdLine = "");
	virtual ~HttpNode();
	
	// overridden  methods
	virtual ULONG		Read (Byte* where, ULONG amount);
	virtual const char* Signature()				{ return m_cmdLine; }
	
protected:
	CURLM*				m_cm;
	CURLMsg*			m_msg;
	Byte*				m_urls[10];
	String				m_cmdLine;
	
	static int			s_globalInitCount;
};
	
//============================================================================
class NamedPipeNode : public FileNode
//============================================================================
{
	DECLARE_DYNAMIC (NamedPipeNode);
public:
	NamedPipeNode (String cmdLine = "", PipeRWMode openMode = PIPE_Read);
	virtual ~NamedPipeNode();
	
	// overridden FileNode methods
	virtual ULONG		Read  (Byte* where, ULONG amount);
	virtual ULONG		Write (Byte* what,  ULONG amount);
	virtual ULONG		Seek  (ULONG offset)	{ ASSERT (false); }
	virtual const char* Signature()				{ return "pipe"; }
	
protected:
	virtual void		Open();
	
	int					m_pipeHandle;
};


//============================================================================
class BitBucketNode : public PipeNode
//============================================================================
// The proverbial bit-bucket; counts number of chars sent into etho-sphere.
// Can be used to pre-flight compression algorithms to determine best approach.
{
	DECLARE_DYNAMIC (BitBucketNode);
public:
	// Constructor, amount read
	BitBucketNode() : m_count(0)		{}

	// overrides
    virtual ULONG		Read  (Byte* where, ULONG amount);
    virtual ULONG		Write (Byte* what,  ULONG amount);
	virtual ULONG		Size()					{ return m_count; }
    virtual const char* Signature()				{ return "null"; }

protected:
	ULONG				m_count;	// Count number of chars in space
};

    
    
// global enumeration to avoid ZLibCompressNode:: prefix for class-scoped enum
enum What2Do { COMPRESS, DECOMPRESS };
    
// compression levels - from zlib.h header file
// follows classic size/time tradeoff with internal buffer (size)
// and how deep searches go into buffer looking for patterns (time)
enum ZlibLevel	  { NONE, BEST_SPEED, DEFAULT, BEST_SIZE };
	
#define COMPRESS_Sig		"żłìb"
#define COMPRESS_SigLen		sizeof(COMPRESS_Sig)

#include "zlib.h"				// for zLib structures and constants
    
//============================================================================
class ZLibCompressNode : public PipeNode
//============================================================================
// Implements zLib compression available at http://quest.jpl.nasa.gov/zlib/
//
// Compatible with java.util.zip.* compression package - used for tranmission
// of Java classes in packaged bundles.  Also used by PNG (replacing patented GIF).
//
// Credits to Mark Alder and Jean-Loup Gailly.
//
// Excellent documentation on the library can be found at:
// http://web2.airmail.net/markn/articles/zlibtool/zlibtool.htm
// http://www.cdrom.com/pub/infozip/zlib/rfc-zlib.html and .../rfc-deflate.html
{
    DECLARE_DYNAMIC (ZLibCompressNode);
public:
    // constructor/destructor
    ZLibCompressNode (What2Do what2do = COMPRESS, ZlibLevel level = DEFAULT, ULONG inSize = OPTIMAL_BUFFERSIZE, ULONG outSize = OPTIMAL_BUFFERSIZE);
    virtual ~ZLibCompressNode();
    
    // convert from zlib-delivered error codes into a core code result
    PipeResult          ZLib2PipeResult (int ZLibReturn);
    
    // overridden filter method to deflate/inflate
    virtual ULONG		Read  (Byte* where, ULONG amount);
    virtual ULONG		Write (Byte* what,  ULONG amount);
    virtual void		Flush ();
    virtual ULONG		Size();
    virtual PipeResult  Status();
    virtual const char* Signature()					{ return m_what2do == COMPRESS ? "deflate" : "inflate"; }
    
protected:
    // init library (allocate buffers), deflate/inflate in => out, flush before close
    void				Init();
    ULONG				Filter();
    
    struct z_stream_s*	m_stream;
    bool				m_init;
    What2Do             m_what2do;
    
    Byte*				m_inBuffer;
    ULONG				m_inSize;
    Byte*				m_outBuffer;
    ULONG				m_outSize;
    
    ZlibLevel			m_compressLevel;
    int					m_flushMode;
};
    
    
}   // namespace Core
}   // namespace Vixen

#endif // pipeobjs_h
