/*!
 * @file vsocket.h
 * @brief Socket communication and synchronization classes.
 *
 * @author Nola Donato
 * @ingroup vcore
 * @see vstream.h
 */

#pragma once

namespace Core {

#define	STREAM_DefaultPort	1234
#define	STREAM_MaxReads		10

/*!
 * @class Socket
 * @brief Communication streams that transmits and receives information using sockets.
 *
 * This low speed form of communication cannot be used for high performance
 * transactions such as synchronization. It is effective for communicating low
 * bandwidth information from some input devices.
 *
 * Sockets are useful for two way communication where a client application
 * provides information which is processed by a server. To make objects available for remote manipulation,
 * both client and server must open socket streams and publish the objects to them.
 * Sockets need dedicated IP addresses for both client and server which are provided as
 * names when the sockets are opened.
 * 
 * @ingroup vcore
 * @see VXSockArbitrator Messenger Core::FileStream Core::Stream
 */
class Socket : public Stream
{
public:
	Socket(int port = STREAM_DefaultPort);
	~Socket();
	virtual bool	Open(const char* name, int mode = Stream::OPEN_RW);
	virtual bool	Close();
	virtual bool	IsEmpty() const;
	virtual size_t	Read(char* buffer, size_t n);
	virtual size_t	Write(const char* buf, size_t nbytes);
	virtual	intptr	GetHandle() const;

	int				MaxReads;		// maximum reads before empty

protected:
	timeval			m_TimeOut;		// timeout for input wait
	int				m_Port;			// port to connect with
	mutable SOCKET	m_CurSock;		// current socket for read/write
	mutable int		m_NumReads;		// number of socket reads so far

};

/*!
 * @class SocketArbitrator
 * @brief Listens for input and broadcasts output to a set of remote clients.
 *
 * It opens a socket connection to listen and for each client that connects.
 * This socket stream can be used to maintain a single server that maintains
 * a scene graph across a set of multiple clients. It can also be used to
 * implement a distributed 3D world across a network.
 * 
 * @ingroup vcore
 * @see Arbitrator
 */
class SocketArbitrator : public Arbitrator
{
public:
	SocketArbitrator(int port = STREAM_DefaultPort);
	~SocketArbitrator();

	int		AddConnection(intptr connection);
	int		FindConnection(intptr connection) const;
	bool	SendConnection(intptr connid, const char* buf, int nbytes);
	int		Select(int* connections, int n);
	bool	Open(const TCHAR* name, int mode = Stream::OPEN_RW);
	bool	Close();
	bool	IsEmpty() const;
	size_t	Read(char* buffer, size_t nbytes);
	size_t	Write(const char* buf, size_t nbytes);
	intptr	GetAt(int i) const;
	int		GetSize() const;
	void	RemoveConnection(intptr connection);
	bool	SendAll(intptr sendflags);

	int				MaxReads;		// maximum reads before empty

protected:
	bool			DoConnect;		// send connected obj list
	mutable fd_set	m_Pending;		// FDs with pending input
	SOCKET			m_Listener;		// socket used to listen for input
	timeval			m_TimeOut;		// timeout for input wait
	int				m_Port;			// port to connect with
	mutable SOCKET	m_CurSock;		// current socket for read/write
	uint32			m_NumRecvd;		// number of clients who have responded
	uint32			m_NumClients;	// number of clients
	mutable int32	m_NumReads;		// number of socket reads so far
	mutable fd_set	m_Clients;		// FDs to listen for input on
};

} // end Core