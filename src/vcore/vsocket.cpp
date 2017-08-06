#include "vcore/vcore.h"
#include "vcore/vdebug.h"

#if defined(_DEBUG) && defined(_WIN32)
#define	SOCKMSG(s)		endl(vixen_debug << s << WSAGetLastError())
#define	SOCKERR(s, v)	{ endl(vixen_debug << s << WSAGetLastError()); return v; }
#else
#define	SOCKMSG(s)
#define	SOCKERR(s, v)	return v
#endif

#define	SOCK_MaxBufSize	64*STREAM_MaxBufSize	// socket buffer size
#define	SOCK_MinBufSize	4*sizeof(int32)

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")

inline SOCKET GETFD(uint32 i, fd_set* fdset)
{
	if (i >= fdset->fd_count)
		return SOCKET(SOCKET_ERROR);
	return fdset->fd_array[i];
}
#else
#define GETFD(i, fdset) i
#endif


namespace Vixen {
namespace Core {

/*!
 * @fn Socket::Socket(int port)
 *
 *	Constructs a socket stream for the given port.
 */
Socket::Socket(int port) : Stream()
{
#ifdef _WIN32
	WSADATA wsaData;
	if (SOCKET_ERROR == WSAStartup(0x202, &wsaData))
	{
		SOCKMSG("WSAStartup failed with error");
		WSACleanup();
	}
#endif
	m_TimeOut.tv_sec  = 0;
	m_TimeOut.tv_usec = 0;
	m_Port = port;
	m_CurSock = SOCKET(SOCKET_ERROR);
	m_NumReads = 0;
	MaxReads = STREAM_MaxReads;
}

/*!
 * intptr Socket::GetHandle() const
 * Provides access to the underlying SOCKET associated
 * with an open stream. If the stream has a socket open,
 * the return value will be a non-null SOCKET handle represented as an integer.
 *
 * @returns SOCKET handle or 0 if no socket open
 * @see Stream::GetHandle
 */
intptr Socket::GetHandle() const { return (intptr) m_CurSock; }

Socket::~Socket()
{
	Close();
}

/*!
 * @fn bool Socket::Open(const TCHAR* svrname, size_t mode)
 * @param name	String describing the socket IP address (four numbers
 *				separated by dots as in 10.15.16.1) or the name of
 *				the remote machine to connect with.
 * @param mode	How to open socket: STREAM_Read and/or STREAM_Write
 *
 * Opens a socket connection to the named machine or IP address for
 * read, write or both.
 *
 * @see Stream::Open Stream::Close
 */
bool Socket::Open(const char* svrname, int mode)
{
	intptr		addr;
	int			buf_size = SOCK_MinBufSize;
	int			opt_size = sizeof(int);
	SOCKET		msgsock;
	int			val = 1;
	struct hostent*	hp;
	sockaddr_in	remote;
	String		streamname(svrname);

	msgsock = socket(AF_INET,SOCK_STREAM,0); // Open a socket
	if (msgsock == SOCKET(SOCKET_ERROR))
		SOCKERR("Socket::Open socket open failed", false);
//	setsockopt(msgsock, SOL_SOCKET, SO_SNDBUF, (const char *) &buf_size, opt_size );
//	setsockopt(msgsock, SOL_SOCKET, SO_RCVBUF, (const char *) &buf_size, opt_size );
	setsockopt(msgsock, SOL_SOCKET, SO_REUSEADDR, (const char *) &val, sizeof(BOOL));
	if (mode & OPEN_SMALL_PACKETS)
	{
		setsockopt(msgsock, SOL_SOCKET, TCP_NODELAY, (const char *) &val, sizeof(BOOL));
		setsockopt(msgsock, SOL_SOCKET, SO_SNDBUF, (const char *) &buf_size, opt_size );
	}
	if (isalpha(svrname[0]))
		hp = gethostbyname(svrname); // server address is a name
	else
	{
		addr = inet_addr(svrname);  // Convert nnn.nnn address to a usable one
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
	}
	if (hp == NULL)
	{
		TCHAR buf[128];
		SPRINTF(buf, TEXT("Socket::Open cannot resolve address %s, Error "), svrname);
		SOCKERR(buf, false);
	}

	memset(&remote, 0, sizeof(remote));
	memcpy(&(remote.sin_addr), hp->h_addr, hp->h_length);
	remote.sin_family = hp->h_addrtype;
	remote.sin_port = htons(m_Port);

	if (SOCKET_ERROR == connect(msgsock, (struct sockaddr*) &remote, sizeof(remote)))
		SOCKERR("Socket::Open socket connect failed", false);
	m_CurSock = msgsock;
	return Stream::Open(streamname, mode);
}

bool Socket::Close()
{
	if (m_openmode)
	{
		Stream::Close();
		if (m_CurSock != SOCKET(SOCKET_ERROR))
		{
			shutdown(m_CurSock, 2);
			closesocket(m_CurSock);
		}
		m_CurSock = SOCKET(SOCKET_ERROR);
		return true;
	}
	return Stream::Close();
}

/****
 *
 * Socket override for Stream::Read
 * Try to recieve a message from the socket.
 * Check for error or closure and read data if available.
 *
 ****/
size_t Socket::Read(char* buffer, size_t len)
{
	long	nread = 0;
	long	ofs = 0;
	long	l = (long) len;

	if (!(m_openmode & OPEN_READ))
		return 0;
	if (m_CurSock == SOCKET(SOCKET_ERROR))	// can't read anything
		return 0;
	while ((l -= nread) > 0)
	{
		nread = recv(m_CurSock, buffer + ofs, l, 0);
		VX_TRACE(Debug, ("Socket::Read %d", nread));
		if ((nread == SOCKET_ERROR) ||				// socket read error?
				(nread <= 0))
		{ SOCKERR("Socket::Read socket recv failed ", false); }
		ofs += nread;								// count bytes read
	}
	return l;
}

bool Socket::IsEmpty() const
{
	int			rc;
	fd_set		input;

	if (!(m_openmode & OPEN_READ))
		return true;
	if (m_CurSock == SOCKET(SOCKET_ERROR))
		return true;
	FD_ZERO(&input);
	FD_SET(m_CurSock, &input);
	rc = select(1, &input, 0, 0, (timeval*) &m_TimeOut);
	if (rc <= 0)							// no input pending
		return true;
	VX_TRACE(Debug, ("Socket::IsEmpty NOT"));
	if (m_NumReads++ < MaxReads)	// be empty after MaxReads recvs
		return false;
	m_NumReads = 0;					// start recv counter again
	return true;							// no more input
}

size_t Socket::Write(const char* buf, size_t nbytes)
{
	long	nsent = 0;
	long	n = (long) nbytes;
	while ((nbytes -= nsent) > 0)
	{
		VX_TRACE(Debug, ("Socket::Write %p %d", m_CurSock, n));
		nsent = send(m_CurSock, buf, n, 0);
		if (nsent == SOCKET_ERROR)	// error, remove this client
			SOCKERR("Socket::Write socket send failed", false);
		buf += nsent;			// couldn't send it all
		n -= nsent;			// try to send the rest
	}
	return n;
}

/*!
 * @fn SocketArbitrator::SocketArbitrator(int port)
 *
 * Constructs a socket arbitrator for the given port.
 *
 * @see SocketArbitrator::Open Socket
 */
SocketArbitrator::SocketArbitrator(int port) : Arbitrator()
{
	m_NumClients = 0;
	m_Port = port;
	m_Listener = SOCKET(SOCKET_ERROR);
	FD_ZERO(&m_Pending);
	FD_ZERO(&m_Clients);
	MaxReads = STREAM_MaxReads;
#ifdef _WIN32
	WSADATA wsaData;
	if (SOCKET_ERROR == WSAStartup(0x202, &wsaData))
	{
		SOCKMSG("WSAStartup failed with error");
		WSACleanup();
	}
#endif
}

/*!
 * @fn bool SocketArbitrator::Open(const TCHAR* svrname, int mode)
 * @param name	Ignored, can connect with any remote client
 *				the remote machine to connect with.
 * @param mode	How to open socket: STREAM_Read and/or STREAM_Write
 *
 * Opens a socket to listen for communication from remote clients.
 * When a remote listener connects, all object updates and events
 * sent by that client will be applied to the local Vixen scene
 * if SocketArbitrator is the input stream for the synchronizer.
 * If used as the output stream for the synchronizer,
 * SocketArbitrator will send updates and events made by the local
 * application to the remote clients.
 *
 * Also: Stream::Open Socket::Open Synchronizer
 */
bool SocketArbitrator::Open(const TCHAR* svrname, int mode)
{
	int			buf_size = SOCK_MinBufSize;
	int			opt_size = sizeof(int);
	sockaddr_in	local;
	SOCKET		msgsock;
	int			val = 1;
	local.sin_family      = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY; 
	local.sin_port        = htons(m_Port);

	msgsock = socket(AF_INET,SOCK_STREAM,0); // Open a socket
	if (msgsock == SOCKET(SOCKET_ERROR))
		SOCKERR("SockArbitrator::Open socket open failed", false);
	setsockopt(msgsock, SOL_SOCKET, TCP_NODELAY, (const char *) &val, sizeof(BOOL));
//	setsockopt(msgsock, SOL_SOCKET, SO_SNDBUF, (const char *) &buf_size, opt_size );
//	setsockopt(msgsock, SOL_SOCKET, SO_RCVBUF, (const char *) &buf_size, opt_size );
	if (mode & OPEN_SMALL_PACKETS)
	{
		setsockopt(msgsock, SOL_SOCKET, TCP_NODELAY, (const char *) &val, sizeof(BOOL));
		setsockopt(msgsock, SOL_SOCKET, SO_SNDBUF, (const char *) &buf_size, opt_size );
	}
/*
 * This stream can accept input from as many as 32
 * client sockets. m_Listener is the socket which listens for input.
 */
	VX_ASSERT(m_NumClients == 0);
	FD_ZERO(&m_Clients);
//	ioctlsocket(m_listener, FIONBIO, &val);	// non blocking reads
	if (SOCKET_ERROR == bind(msgsock, (struct sockaddr*) &local, sizeof(local)))
		SOCKERR("Socket::Open socket bind failed", false);
	if (SOCKET_ERROR == listen(msgsock, 5))
		SOCKERR("Socket::Open socket listen failed", false);
	m_Listener = msgsock;
	m_NumClients = 0;
	return Arbitrator::Open(svrname, mode);
}


/*!
 * @fn int SocketArbitrator::Select(int* connections, int n)
 *
 * A socket arbitrator will return \b true indication
 * if all of the clients can be selected for output.
 */
int SocketArbitrator::Select(int* connections, int n)
{
	fd_set	output = m_Clients;
	int		nready = 0;

	if (select(m_NumClients, 0, &output, 0, &m_TimeOut) <= 0)
		return 0;
	for (int i = 0; i < n; ++i)
	{
		int connid = connections[i];
		SOCKET	fd = GETFD(connid, &output);
		if (fd == SOCKET(SOCKET_ERROR))
		{
			connections[i] = 0;			// indicate this one not ready
			continue;
		}
		++nready;
	}
	return nready;
}

/****
 *
 * SocketArbitrator override for Stream::Close
 * Close socket and cleanup
 *
 ****/
SocketArbitrator::~SocketArbitrator()
{
	Close();
}

bool SocketArbitrator::Close()
{
	if (m_openmode)
	{
		Arbitrator::Close();
		if (m_Listener != SOCKET(SOCKET_ERROR))
		{
			closesocket(m_Listener);
			m_Listener = SOCKET(SOCKET_ERROR);
		}
		return true;
	}
	return Arbitrator::Close();
}

bool SocketArbitrator::IsEmpty() const
{
	int					retval, n = m_NumClients + 1;
	SocketArbitrator*	cheat = (SocketArbitrator*) this;

	if (!(m_openmode & OPEN_READ))
		return true;
/*
 * If there is no input pending from last time, check to see if there
 * is any new input pending. If not, exit as empty
 */
	if (m_CurSock == SOCKET(SOCKET_ERROR))		// no input pending from last time?
	{
		m_NumReads = 0;					// start recv counter again
		m_Pending = m_Clients;
		FD_SET(m_Listener, &m_Pending);
		retval = select(n, &m_Pending, 0, 0, (timeval*) &m_TimeOut);
		if (retval < 0)
			SOCKERR("Socket::IsEmpty socket select failed", true);
		if (retval == 0)				// Timeout - no more input
			return true;
	}
/*
 * Examine all file descriptors with pending input.
 * Input from the listener is a new connection.
 * Input from another socket is data pending to be read.
 */
	for (int i = 0; i < n; ++i)
	{
		SOCKET fd = GETFD(i, &m_Pending);
		if (fd == SOCKET(SOCKET_ERROR))	// no input on this file descriptor?
			continue;					// no, look again
		if (fd == m_Listener)			// input from listener?
		{								// accept a new connection
			sockaddr_in remote;
			socklen_t fromlen = sizeof(remote);
			m_CurSock = accept(m_Listener, (struct sockaddr*) &remote, &fromlen);
			if (m_CurSock == INVALID_SOCKET)
				SOCKERR("Socket::IsEmpty socket accept failed", true);
			if (!FD_ISSET(m_CurSock, &m_Clients))
			{
				++n;
				cheat->AddConnection(m_CurSock);
				FD_SET(m_CurSock, &m_Pending);
			}
			FD_CLR(m_Listener, &m_Pending);
			--i;
			--n;
			continue;
		}
		m_CurSock = fd;					// new socket to read from
		if (m_NumReads++ < MaxReads)		// be empty after MaxReads recvs
			return false;
		m_NumReads = 0;			// start recv counter again
		return true;					// report empty status
	}
	cheat->m_NumReads = 0;				// start recv counter again
	m_CurSock = SOCKET(SOCKET_ERROR);
	return true;						// no more input
}

/****
 *
 * Socket override for Stream::Read
 * Try to recieve a message from the socket.
 * Check for error or closure and read data if available.
 *
 ****/
size_t SocketArbitrator::Read(char* buffer, size_t len)
{
	long	nread = 0;
	long	ofs = 0;
	long	l = (long) len;

	if (!(m_openmode & OPEN_READ))
		return 0;
	if (m_CurSock == SOCKET(SOCKET_ERROR))			// can't read anything
		return 0;
	while ((l -= nread) > 0)
	{
		nread = recv(m_CurSock, buffer + ofs, l, 0);
//		VX_TRACE(Debug, ("Socket::Read %d", nread));
		if ((nread == SOCKET_ERROR) || (nread <= 0))// socket read error?
		{
			RemoveConnection(m_CurSock);
			m_CurSock = SOCKET(SOCKET_ERROR);
			SOCKERR("Socket::Read socket recv failed ", false);
		}
		ofs += nread;								// count bytes read
	}
	return nread;
}

size_t SocketArbitrator::Write(const char* buf, size_t nbytes)
{
	long	nsent = 0;
	long	n = (long) nbytes;

	while ((nbytes -= nsent) > 0)
	{
		VX_TRACE(Debug, ("Socket::Write %p %d", m_CurSock, n));
		nsent = send(m_CurSock, buf, n, 0);
		if (nsent == SOCKET_ERROR)	// error, remove this client
			SOCKERR("Socket::Write socket send failed", false);
		buf += nsent;				// couldn't send it all
		n -= nsent;					// try to send the rest
	}
	return n;
}


void SocketArbitrator::RemoveConnection(intptr connid)
{
	intptr devhandle = FindConnection(connid);
	FD_CLR(devhandle, &m_Clients);
	--m_NumClients;
}

bool SocketArbitrator::SendConnection(intptr connid, const char* buf, int nbytes)
{
	SOCKET	socket = (SOCKET) connid;
	int32	nsent = 0;
	while ((nbytes -= nsent) > 0)
	{
		VX_TRACE(Debug, ("Socket::Write %p %d", connid, nbytes));
		nsent = send(socket, buf, nbytes, 0);
		if (nsent == SOCKET_ERROR)	// error, remove this client
			SOCKERR("Socket::Write socket send failed", false);
		buf += nsent;				// couldn't send it all
		nbytes -= nsent;			// try to send the rest
	}
	return true;
}


int	SocketArbitrator::AddConnection(intptr connection)
{
	VX_TRACE(Debug, ("Socket::AddConnection %p", connection));
	SOCKET	sock = (SOCKET) connection;
	FD_SET(sock, &m_Clients);
	return ++m_NumClients;
}

intptr SocketArbitrator::GetAt(int i) const
{
	if (i >= 0)
		return GETFD(i, &m_Clients);
	return -1L;
}

int SocketArbitrator::GetSize() const
{
	return m_NumClients;
}

int SocketArbitrator::FindConnection(intptr connection) const
{
	for (uint32 i = 0; i < m_NumClients; ++i)
	{
		SOCKET fd = GETFD(i, &m_Clients);
		VX_ASSERT(sizeof(SOCKET) <= sizeof(intptr));
		if (intptr(fd) == connection)
			return i;
	}
	return -1;
}


}	// end Core
}	// end Vixen
