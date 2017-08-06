/*!
 * @file vstream.h
 *
 * @brief Defines basic communication classes for I/O.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 * @see vxmess.h vxfile.h
 */

#pragma once

namespace Core {

/*!
 * @class Stream
 * @brief Basic communication class used by Vixen to read and write data from external sources.
 *
 * The base class just provides the basic API and is intended for subclassing only.
 * Hooks are provided to permit byte-swapping streams for porting to other platforms.
 *
 * @see FileStream Socket Arbitrator
 */
class Stream : public LockObj
{
public:
	VX_DECLARE_CLASS(Stream);

//! @name Construction
//@{
	Stream(const TCHAR* name = NULL);	//!< Construct stream for named resource.
	Stream(const Stream&);			//!< Construct stream pipe from the given pipe.
	virtual ~Stream();				//!< Destroy stream and free resources.
//@}

//! @name Pathnames
//@{
//	Get the base directory for this pipe.
	virtual const TCHAR*	GetDirectory() const;
//! Set the base directory for this pipe.
	virtual void			SetDirectory(const TCHAR* name);
//! Construct full pathname given a possibly relative URL or filename.
	virtual	const TCHAR*	GetPath(const TCHAR* filename, TCHAR* buf = NULL, int maxlen = 0);
//! Construct full pathname using default directory if relative
	static String			MakePath(const TCHAR* filename, const TCHAR* dir = NULL);	
//! Parse full pathname and extract directory and file base.
	static void				ParseDirectory(const TCHAR* filename, TCHAR* filebase = NULL, TCHAR* destbuf = NULL);
//! Returns true if input file is an absolute path or URL, else returns false.
	static	bool			IsRelativePath(const TCHAR* filename);
//@}

//! @name I/O
//@{
	virtual	intptr	GetHandle() const;		//! Get device-specific stream handle
	virtual void	Flush()	{ };			//!	Flush accumulated output.
	virtual bool	Close();				//!	Close the pipe and free resources used.
	virtual bool	IsEmpty() const;		//!	Determine if pipe has input.
	virtual	size_t	Read(char*, size_t);	//!< Read bytes from pipe.
	virtual	size_t	Write(const char*, size_t); //!< write bytes to pipe.

//! Seek to a position in the stream.
	virtual long	Seek(long seekto, int from = Stream::SEEK_FROM_START);
//!	Called internally to connect the named resource to the pipe.
	virtual bool	Open(const TCHAR* name, int mode = Stream::OPEN_RW);
//! Determines if pipe has been opened with the given options.
	virtual	bool	IsOpen(int mode = Stream::OPEN_RW) const;
//@}

/*!
 * Stream open and seek options.
 * Seeking from the end of the stream is not supported for
 * all types of streams.
 * @see Stream::Open Stream::Seek
 */
	enum
	{
		OPEN_READ = 1,			//! open for read
		OPEN_WRITE = 2,			//! open for write
		OPEN_RW = 3,			//! open for read and write
		OPEN_NOBLOCK = 4,		//! open for non-blocking I/O
		OPEN_SMALL_PACKETS = 8,	//! optimize communication for small packets
		OPEN_SEEK = 16,			//! open with permission to seek
		SEEK_FROM_START = 0,	//! absolute seek from the start of the stream
		SEEK_FROM_HERE = 1,		//! relative seek from the current position
		SEEK_FROM_END = 2,		//! relative seek from the end of the stream
	};
protected:
	String			m_Directory;
	int				m_openmode;
};

/*!
 * @class Arbitrator
 * @brief Combines input from multiple connections into a single stream.
 *
 * If attached as the input stream of the messenger, the arbitrator
 * passes all the events and updates from each connection to the messenger.
 * If attached as the output stream, all events and updates directed
 * at the arbitrator are sent to all the connections.
 *
 * The connections are abstract types identified to the arbitrator
 * by their \b device \b handle. Connections are processed
 * serially in the order they are added. All connections
 * should be added before opening the arbitrator.
 *
 * In the default implementation, the connections are streams
 * and the device handle is a pointer to the Core::Stream object.
 *
 * @see SocketArbitrator
 */
class Arbitrator : public Stream
{
public:
	VX_DECLARE_CLASS(Arbitrator);
	Arbitrator(const TCHAR* name = NULL) : Stream(name) { m_CurConn = -1; };

//! Determine which connections can receive output.
	virtual int	Select(int* connectids, int n);
//! Adds a connection.
	virtual int		AddConnection(intptr devhandle);
//! Finds the connection ID for a connection given its device handle.
	virtual int		FindConnection(intptr devhandle) const;
//! Removes a connection.
	virtual void	RemoveConnection(int connectid);
//! Send data to this connection.
	virtual bool	SendConnection(intptr connectid, const char* buf, int n);
//! Finds the device handle for a connection given its connection ID.
	virtual intptr	GetAt(int connectid) const;
//! Get number of connections.
	virtual int		GetSize() const;
//! Get current connection being read from.
	virtual int		GetConnection() const;

	bool			IsEmpty() const;
	void			Flush();
	size_t			Read(char* buf, size_t n);
	size_t			Write(const char* buf, size_t n);
	bool			Close();

protected:
	mutable int		m_CurConn;
	BaseArray<RefPtr, BaseObj>	m_Connections;
};

/*!
 * @fn const TCHAR* Stream::GetDirectory() const
 *
 * The base directory for this stream is the directory
 * given to Core::Stream::SetDirectory. If no directory
 * has been explicitly specified, the base directory
 * is the directory containing the last file opened
 * with this stream.
 *
 * @return string with directory name, NULL if none
 *
 * @see Stream::SetDirectory Stream::Open Stream::ParseDirectory
 */
inline const TCHAR* Stream::GetDirectory() const
{ return m_Directory.IsEmpty() ? NULL : (const TCHAR*) m_Directory; }

} // end Core
