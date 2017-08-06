#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

VX_IMPLEMENT_CLASS(Stream, LockObj);

VX_IMPLEMENT_CLASS(Arbitrator, LockObj);

/*!
 * @fn Stream::Stream(const TCHAR* name)
 * @param name	string name of connection
 *
 * Constructs a stream with the given name. Usually, the name
 * indicates what the stream connects to. It can be the name of
 * a local disk file, a URL or the name of a machine.
 * The constructor does not explicitly connect to the
 * named resource at construction time.
 *
 * @see Stream::Open Stream::SetDirectory
 */
Stream::Stream(const TCHAR* name) : LockObj()
{
	m_openmode = 0;
}

/*!
 * @fn Stream::~Stream()
 *
 * If the stream is currently connected, it is flushed and closed.
 *
 * @see Stream::Open Stream::Flush Stream::Close Stream::Stream
 */
Stream::~Stream()
{
	Flush();
	Close();
}

/*!
 * @fn intptr Stream::GetHandle() const
 * Depending on what kind of stream it is, this function
 * may return a pointer or an integer. For file streams,
 * the device handle is FILE*, for socket streams it
 * is a SOCKET. It is up to the caller to know what type
 * of stream they are dealing with and interpret accordingly.
 *
 * The base implementation returns 0.
 *
 * @return handle or 0 if stream is not open
 */
intptr Stream::GetHandle() const { return 0; }

/*!
 * @fn bool Stream::IsEmpty() const
 *
 * @return \b true if the stream has no more input available, else \b false
 *
 * @see Stream::Read
 */
bool Stream::IsEmpty() const
{
	return true;
}

/*!
 * @fn bool Stream::IsOpen(int mode) const
 * @param mode	Check for read, write or both by setting STREAM_Read and/or STREAM_Write.
 *				A zero argument will return \b true if the stream is open for both.
 *
 * @return \b true if the stream is currently open with the given mode, else \b false
 *
 * @see Stream::IsEmpty Stream::Open
 */
bool Stream::IsOpen(int mode) const
{
	if (mode == 0)
		mode = Stream::OPEN_RW;
	return (m_openmode & mode) != 0;
}

/*!
 * @fn void Stream::SetDirectory(const TCHAR* name)
 * @param name	string name of base directory or URL
 *
 * Sets the directory prepended to relative pathnames for
 * files or URLs referenced by this stream.
 *
 * @see Stream::Open Stream::ParseDirectory
 */
void Stream::SetDirectory(const TCHAR* name) 
{
	m_Directory = name;
	m_Directory.MakeLower();
}

/*!
 * @fn const TCHAR* Stream::ParseDirectory(const TCHAR* filename, TCHAR* filebase = NULL, TCHAR* destbuf = NULL)
 * @param filename	fully qualified pathname to parse
 * @param filebase	buffer to store base name of input file without directory or extension
 * @param destbuf	buffer to store resulting directory in
 *
 * Parses the directory associated with the filename and returns
 * a pointer to a string with the directory. If the filename has
 * no directory, NULL is returned. This routine also works for URLs.
 * If an output buffer to get the result is returned, this routine is
 * thread-safe. Otherwise, it returns a pointer to a static buffer.
 *
 * @see Stream::SetDirectory Stream::GetPath
 */
void Stream::ParseDirectory(const TCHAR* filename, TCHAR* filebase, TCHAR* buf)
{
	const TCHAR*	src = filename;
	const TCHAR*	result = NULL;
	const TCHAR*	delim = NULL;
	size_t			i;
	size_t			n;
	TCHAR			dirbuf[VX_MaxPath];
	bool			full_path = IsRelativePath(filename);

	if (buf == NULL)
		buf = dirbuf;
	*buf = 0;
	if (filename == NULL)
		return;
	while (*src != 0)
	{
		if ((*src == '/') || (*src == '\\'))
			delim = src;
		++src;
	}
	if (delim)
	{
		n = delim - filename;
		for (i = 0; i < n; ++i)
			buf[i] = filename[i];
		buf[n] = 0;
		src = delim + 1;
	}
	else
		src = filename;
	if (filebase == NULL)
		return;
	delim = STRRCHR(src, TEXT('.'));
	if (delim)
		n = delim - src;
	else
		n = STRLEN(src);
	for (i = 0; i < n; ++i)
		filebase[i] = src[i];
	filebase[n] = 0;	
}

/*!
 * @fn bool Stream::IsRelativePath(const TCHAR* filename)
 * @param filename	fully qualified pathname to parse
 *
 * Determines whether the input file path is relative or absolute.
 * @returns true if relative path, else false
 * @see Stream::SetParseDirectory
 */
bool Stream::IsRelativePath(const TCHAR* filename)
{
	if (filename == NULL)
		return false;
	if (*filename == TEXT('/') || *filename == TEXT('\\'))
		return false;
	if (STRCHR(filename, TEXT(':')))
		return false;
	return true;
}

/*!
 * @fn const char* Stream::GetPath(const TCHAR* fname, TCHAR* buf, int buflen)
 * @param fname		name of referenced file or URL
 * @param buf		buffer to get full pathname or NULL
 * @param buflen	length of buffer
 *
 * Returns the full path name of a filename or URL to open on
 * this stream. Each stream has a base directory which is prepended
 * to relative pathnames. This function performs that check and
 * returns a fully resolved filename.
 *
 * @note If no buffer is supplied, this function returns a pointer
 * to an internal static buffer. In this case, the function is
 * not re-entrant and not thread-safe.
 *
 * @return name of fully qualified pathname or NULL for deferred load
 */
const TCHAR* Stream::GetPath(const TCHAR* infile, TCHAR* buf, int buflen)
{
static	TCHAR			fname[VX_MaxPath];
		bool			full_path = false;
		const TCHAR*	dir;
		size_t			n;

	if (buf == NULL)
	{
		buf = fname;
		buflen = VX_MaxPath;
	}
	VX_ASSERT(buflen > 0);
	full_path = !IsRelativePath(infile);
	if (!full_path && (dir = GetDirectory()) && (n = STRLEN(dir)))
	{
		STRCPY(buf, dir);
		--n;
		if ((dir[n] != TEXT('/')) && (dir[n] != TEXT('\\')))
			STRCAT(buf, TEXT("/"));
		STRCAT(buf, infile);
	}
	else
		STRCPY(buf, infile);
	TCHAR* p = buf;
	while (*p)
	{
		*p = tolower(*p);
		if (buflen > 0)
		{
			++p;
			--buflen;
		}
	}
	return buf;
}

/*!
 * @fn String Stream::MakePath(const TCHAR* fname, const TCHAR* dir)
 * @param fname name of referenced file or URL
 * @param dir	pathname to default directory or NULL
 *
 * Returns the full path name of a filename or URL to open on
 * this stream. If provided, the base directory is prepended
 * to relative pathnames. This function returns a fully resolved filename.
 *
 * @return fully qualified pathname
 */
String Stream::MakePath(const TCHAR* infile, const TCHAR* dir)
{
	bool			full_path = IsRelativePath(infile);
	size_t			n;
	String			str;

	if (!full_path && dir && (n = STRLEN(dir)))
	{
		str = dir;
		--n;
		if ((dir[n] != TEXT('/')) && (dir[n] != TEXT('\\')))
			str += TEXT("/");
		str += infile;
	}
	else
		str = infile;
	return str;
}

/*!
 * @fn bool Stream::Open(const TCHAR* name, int mode)
 * @param name	name of resource to connect to (filename, URL, machine)
 * @param mode	open mode: STREAM_Read and/or STREAM_Write
 *
 * Opens a stream connected to a named resource. The interpretation of \b name
 * varies depending on what type of stream it is. For file and
 * net pipes, \b name is the name of the file to open. For socket
 * stream, it is the IP address or host name to connect to.
 *
 * @see Stream::Close Core::FileStream Core::NetStream Stream::SetDirectory
 */
bool Stream::Open(const TCHAR* name, int mode)
{
	m_openmode = mode;
	if (name == NULL)
		return true;
/*
 * Extract base file name and make it the name of this stream
 */
	TCHAR		 buf[VX_MaxPath];
	const TCHAR* s = name;
	const TCHAR* t = name;

	do
	{
		s = t;
		size_t p = STRCSPN(s, TEXT("/\\"));		// -> after next directory delimiter
		t += p;
	}
	while (*t++);						// -> start of file basename
	if (s != t)
	{
		size_t n = s - name;
		STRNCPY(buf, name, n);
		buf[n] = 0;	
		SetDirectory(buf);
	}
	return true;
}

/*!
 * @fn bool Stream::Close()
 *
 * Closes the stream and releases all of its resources.
 * For file or net streams, this will close the currently open file
 * or URL. For socket streams, it will disconnect and close the socket.
 *
 * @see Stream::Open
 */
bool Stream::Close()
{
	m_openmode = 0;
	return true;
}

/*!
 * @fn size_t Stream::Read(char* buffer, size_t nbytes)
 * @param buffer	input buffer to read into
 * @param nbytes	number of bytes to read
 *
 * Reads the given number of bytes from the stream and stores them
 * in the given input buffer. This function blocks until all
 * of the bytes are available or an error occurs.
 *
 * @return number of bytes successfully read, 0 on error
 *
 * @see Stream::IsEmpty Stream::Write
 */
size_t Stream::Read(char* buffer, size_t n)
{
	return 0;
}

/*!
 * @fn size_t Stream::Write(const char* buffer, size_t nbytes)
 * @param buffer	buffer to write from
 * @param nbytes	number of bytes to writes
 *
 * Writes the given number of bytes from the buffer to
 * the stream. This function blocks until all
 * of the bytes are written or an error occurs.
 *
 * @return number of bytes successfully written, 0 on error
 *
 * @see Stream::IsEmpty Stream::Read
 */
size_t Stream::Write(const char* buffer, size_t n)
{
	return 0;
}

/*!
 * @fn Stream::Seek(long offset, int from = SEEK_FROM_START)
 * @param offset	0-based offset to seek to
 * @param from		one of: SEEK_FROM_START, SEEK_FROM_END, SEEK_FROM_HERE
 *
 * Seek is not supported for all types of streams. For example,
 * file-based streams are seekable but streams which provide continuous input,
 * like sockets, are not.
 *
 * @return 0 if seek succeeded, else non-zero
 */
long Stream::Seek(long offset, int from)	{ return -1; }

bool Arbitrator::IsEmpty() const
{
	Stream*		s;

	if (m_CurConn > 0)
	{
		s = (Stream*) (RefObj*) m_Connections.GetAt(m_CurConn);
		if (s && !s->IsEmpty())
			return false;
	}
	for (int i = 0; i < (int) m_Connections.GetSize(); ++i)
	{
		Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(i);

		if (!s->IsEmpty())
		{
			m_CurConn = i;
			return false;
		}
	}
	m_CurConn = -1;
	return true;
}

size_t Arbitrator::Read(char* buf, size_t n)
{
	if (m_CurConn < 0)
		return 0;
	Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(m_CurConn);
	if (s == NULL)
		return 0;
	return s->Read(buf, n);
}

size_t Arbitrator::Write(const char* buf, size_t n)
{
	if (m_CurConn < 0)
		return 0;
	Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(m_CurConn);
	if (s == NULL)
		return 0;
	return s->Write(buf, n);
}

void Arbitrator::Flush()
{
	for (intptr i = 0; i < m_Connections.GetSize(); ++i)
	{
		Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(i);

		s->Flush();
	}
}

bool Arbitrator::Close()
{
	for (intptr i = 0; i < m_Connections.GetSize(); ++i)
	{
		Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(i);

		if (s == NULL)
			continue;
		s->Flush();
		s->Close();
	}
	return Stream::Close();
}

int Arbitrator::GetConnection() const
{
	return m_CurConn;
}

/*!
 *@fn int Arbitrator::FindConnection(intptr devhandle) const
 * @param devhandle	Device handle of connection to find.
 *
 * Finds a connection based on its device handle.
 * The device handles used to identify connections are passed as unsigned
 * integers and may have a different interpretation depending on the which
 * subclass of Arbitrator is used.
 *
 * @return index of connection or -1 if not found
 *
 * @see Arbitrator::AddConnection Arbitrator::GetSize
 */
int Arbitrator::FindConnection(intptr connection) const
{
	RefObj*	obj = (RefObj*) connection;

	return (int) m_Connections.Find(obj);
}

/*!
 * @fn void	Arbitrator::RemoveConnection(int connid)
 * @param connection	Identifier of connection to send to. This is the value returned
 *						by AddConnection, not the device handle.
 *
 * Removes the given connection from this arbitrator.
 *
 * @see Arbitrator::AddConnection Arbitrator::FindConnection
 */
void Arbitrator::RemoveConnection(int connid)
{
	m_Connections.RemoveAt(connid);
}

/*!
 * @fn int	Arbitrator::AddConnection(intptr devhandle)
 * @param devhandle	Device handle of connection to add.
 *
 * Adds the given connection to this arbitrator. Device
 * handles are passed as unsigned integers and may
 * have a different interpretation depending on the which
 * subclass of arbitrator is used. The device handles for
 * the base implementation are really pointers to streams.
 *
 * The value returned is the connection ID, a number which
 * identifies the connection to this arbitrator. The connection
 * IDs start at 1 and are assigned in the order connections are added.
 *
 * @return id of connection (0 - MESS_MaxHosts), or -1 on failure
 *
 * @see Arbitrator::RemoveConnection Arbitrator::FindConnection Arbitrator::ConnectObjs
 */
int	Arbitrator::AddConnection(intptr devhandle)
{
	Stream*	newconn = (Stream*) devhandle;

	if (newconn == NULL)
		return -1;
	for (int i = 0; i < (int) m_Connections.GetSize(); ++i)
	{
		Stream* s = (Stream*) (RefObj*) m_Connections.GetAt(i);
		if (s == newconn)
			return i;
	}
	return (int) m_Connections.Append(newconn);				
}

/*!
 * @fn bool	Arbitrator::SendConnection(intptr connid, const char* buf, int nbytes)
 * @param connection	Identifier of connection to send to. This is the value returned
 *						by AddConnection, not the device handle.
 *
 * Sends the given data to the specified connection.
 *
 * @return \b true if send was successful, else \b false
 *
 * @see Arbitrator::AddConnection Arbitrator::FindConnection Arbitrator::ConnectObjs
 */
bool Arbitrator::SendConnection(intptr connid, const char* buf, int nbytes)
{
	return true;
}

/*!
 *@fn uint Arbitrator::GetAt(int connid) const
 * @param connid	Connection ID of connection to get.
 * Gets a connection's device handle based on its connection ID.
 * The connection ID returned by Arbitrator::AddConnection
 * when the connection is associated with the arbitrator.
 *
 * @return device handle of connection or -1 if not found
 *
 * @see Arbitrator::AddConnection Arbitrator::GetSize
 */
intptr Arbitrator::GetAt(int connid) const
{
	if ((connid < 0) || (connid >= (int) m_Connections.GetSize()))
		return intptr(-1);
	return (intptr) (RefObj*) m_Connections.GetAt(connid);	
}

/*!
 *@fn int Arbitrator::GetSize() const
 *
 * Determines how many connections this arbitrator has.
 *
 * @return number of connections
 *
 * @see Arbitrator::AddConnection Arbitrator::GetAt
 */
int Arbitrator::GetSize() const
	{ return (int) m_Connections.GetSize(); }


/*!
 * @fn int Arbitrator::Select(int* connections, int n)
 * @param connections	Array of connection IDs for connections to check.
 * @param n				Number of IDs to check.
 *
 * Determines whether the given connections are ready or not.
 * If a connection is not ready, its identifier in the input
 * array is set to zero. The connection identifiers in the input
 * array are the return values from Arbitrator::AddConnection.
 * Please do not pass device handles here.
 *
 * It is a good idea to check which connections are ready before
 * sending data to them. Some implementations may not work
 * correctly and could lose data when sending to a connection
 * which is not ready.
 *
 * @see Arbitrator::AddConnection Arbitrator::SendConnection
 */
int Arbitrator::Select(int* connections, int n)
{
	int nready = 0;

	for (int i = 0; i < n; ++i)
	{
		int connid = connections[i];
		if ((connid <= 0) ||
			(connid > (int) m_Connections.GetSize()) ||
			(m_Connections.GetAt(connid) <= 0))
		{
			connections[i] = 0;			// indicate this one not ready
			continue;
		}
		++nready;
	}
	return nready;
}

}	// end Core
}	// end Vixen