#pragma once

namespace Vixen {

class Event;

/*!
 * @file vxmess.h
 * @brief Classes for intepreting Vixen binary protocol and dispatching events.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxstream.h
 * @see <A HREF=oview/binprot.htm> Vixen binary protocol</A>
 */

#define	MESS_MaxLogs	4
#define	MESS_MaxHosts	24
#define	MESS_CurrentVersion	8		// current version supported

#ifndef MESS_MaxBufSize
#define	MESS_MaxBufSize	8192		// maximum buffer size in bytes
#endif


#ifdef _DEBUG
/*!
 * @struct VXOpcode
 * @brief VXOpcode defines the format of an binary file opcode.
 *
 * In the DEBUG version, we generate the names of the opcodes
 * for readable printouts
 *
 * OP(classid, opcode) constructs an opcode
 *
 * @ingroup vixen
 * @see Messenger
 */
struct VXOpcode
{
	VXOpcode(uint16 c, uint16 o, const TCHAR* name)
		{ ClassID = uint16(c); Opcode = o; Name = name; }

	uint16		ClassID;
	uint16		Opcode;
	const TCHAR* Name;
};

#define	OP(classid, opcode)	VXOpcode(classid, opcode, TEXT(#opcode))

#else
struct VXOpcode
{
	VXOpcode(uint16 c, uint16 o)
		{ ClassID = uint16(c); Opcode = o; }

	uint16	ClassID;
	uint16	Opcode;
};

#define	OP(classid, opcode)	VXOpcode(classid, opcode)
#endif

/*!
 * @class Messenger
 * @brief Messengers are used to communicate changes to the currently displaying scene. 
 *
 * These changes may come from a file, as when an application loads its initial 
 * content. Scene updates may come from remote processors when operating 
 * in a distributed environment.
 *
 * All messengers use the same underlying binary protocol to express changes made to 
 * objects owned by that messengers. The <A HREF=oview/binprot.htm> Vixen binary protocol</A>
 * describes the Vixen API as an sequence of binary tokens representing each call 
 * that can affect the state of the scene. These tokens are interpreted by the 
 * messengers. Each Vixen object knows how interpret its own part of the protocol.
 *
 * All messengers have an internal dictionary which allows you to
 * retrieve objects based on string names. You can also iterate over
 * all the named objects in the stream. (Not all Vixen objects are assigned
 * names so you cannot retrieve every object in the scene this way.)
 * This capability allows programs to examine 3D content at run time.
 *
 * <B>Distributed Processing</B>
 *
 * When an object is distributed across multiple machines, each machine
 * maintains a local copy of the object. All updates made to the object
 * on each local machine are logged to the messengers. Events
 * generated on that machine are also logged.
 *
 * At the end of each frame, all the transactions accumulated by the
 * messengers are sent to the remote processors. Before
 * display starts for the next frame, transactions sent to the local
 * machine from the remote processors are applied to the objects and
 * remote events are handled.
 * @image html messenger.jpg
 *
 * Objects are identified to the messenger by their  handle. This is
 * an unique identifier for the object to the messenger. Handles
 * are not unique across files. The object's messenger handle is also
 * stored within the object (see SharedObj::GetID)
 *
 * <B>Event Handling</B>
 *
 * Messengers are also used to log events and propagate them. Each messenger
 * maintains a list of observers that can react to specific events and
 * channel them to individual objects. For example, you could direct
 * all mouse events to a specific engine in the simulation tree designed
 * to process them.
 *
 * A messenger has a set of public variables that determine its logging
 * behavior with respect to both updates and events. Whether a transaction
 * is logged, sent to a remote client or processed is independently
 * controllable for updates and events.
 *
 * <B>Implementation</B>
 *
 * The messenger internally maintains a mapping table to map object
 * handles from the binary file to object pointers in memory. It also
 * maintains a dictionary which maps names to objects for quick
 * lookup (it uses a hash table). Named objects attached to the messenger
 * reference the name in the dictionary rather than having an
 * attached string.
 *
 * @ingroup vixen
 * @see SharedObj::IsGlobal SharedObj::GetID Event Core::Stream
 */
class Messenger : public SharedObj
{
public:
	VX_DECLARE_CLASS(Messenger);

	/*!
	 * Stream open options.
	 * @see Stream::Open
	 */
		enum
		{
			OPEN_READ = Core::Stream::OPEN_READ,	//! open for read
			OPEN_WRITE = Core::Stream::OPEN_WRITE,	//! open for write
			OPEN_RW = Core::Stream::OPEN_RW,		//! open for read and write
		};
	/*!
	 * @brief messenger opcode types.
	 *
	 * A scene manager binary file is a series of opcodes and arguments
	 * describing scene manager API calls. The opcodes are broken up based
	 * on classes with each scene manager class having its own set of opcodes.
	 *
	 * All opcodes are composed of an 8 bit class identifier followed
	 * by an 8 bit opcode for that class.
	 *	SerialID enumerates all the class identifiers that are built in
	 *	XXXOps enumerates all the opcodes for class XXX
	 * An opcode is written out as 32 bits (but only 16 bits are used right now).
	 *
	 */
	enum MessageType
	{
		VIXEN_DoNothing =	0,				//!< skip zeros if in doubt
		VIXEN_Version =		0x011111111,	//!< Establish protocol version
		VIXEN_Connect =		0x022222222,	//!< Connect to remote object
		VIXEN_SetStreamID =	0x033333333,	//!< Set stream ID
		VIXEN_Exit =		0x044444444,	//!< Connection or application exit
		VIXEN_Begin =		0x055555555,	//!< Begin binary packet
		VIXEN_End	 =		0x066666666,	//!< End binary packet
		VIXEN_Sync =		0x077777777,	//!< Frame synchronization
		VIXEN_Event =		0x008888888,	//!< Event logged
		VIXEN_Remap =		0x099999999,	//!< Event handle remapped
		VIXEN_VecSize =		0x0AAAAAAAA,	//!< Size of position & normal
	};

	/*!
	 * @brief Messenger save options
	 * @see SharedObj::Save
	 */
	enum SaveOpts
	{
		MESS_Distribute =	1,
		MESS_Attach,
		MESS_Detach,
		MESS_ClearGlobal
	};

	/*!
	 * @brief message log type, designates priority and type of message.
	 * @see Event::Log
	 */
	enum LogType
	{
		MESS_FastLog =	0,		//!< high priority log, ignored locally
		MESS_UpdateLog,			//!< update log, ignored locally
		MESS_EventLog,			//!< event log, both local & remote
		MESS_LocalLog			//!< local log, both local & remote
	};

	/*!
	 * @brief Internal class to encapsulate object mapping for the messenger.
	 *
	 * Each object attached to the messenger has an entry in this array.
	 * @ingroup vixenint
	 * @internal
	 */
	class ObjMap : public ObjArray
	{
	public:
		ObjMap() : ObjArray() { MaxHandle = m_freeHandle = 0; }
		~ObjMap();

	//!	Attach this object, create a new handle if necessary.
		int			AttachObj(const SharedObj* o, int handle = 0);
	//!	Detach this object, free the handle.
		bool		DeleteObj(int handle);
	//! Return the object pointer for this handle.
		SharedObj*	GetObj(int handle) const;
	//! Clear the handle table, detach all objects.
		void		Empty();
	//! Replace this map with the handle table from the source.
		bool		Copy(const SharedObj* srcobj);
	//! Merge the source handle table with this one.
		bool		Merge(const ObjMap* src);
	//! Get the handle for this object.
		int			GetHandle(const SharedObj* o) const;

		int32		MaxHandle;	//!< Maximum handle stored so far
	protected:
	//! Generate new handle if input handle already used.
		int			NewHandle(int handle = -1);
		int32		m_freeHandle;
	};

	/*!
	 * @brief Internal property class used to maintain observation
	 * relationships for a stream.
	 *
	 * Each time an event is observed,
	 * an Observer property is linked into that event's list of observers.
	 * When an event occurs, its observer list is scanned and the
	 * OnEvent method is called for the observers of that event
	 * @ingroup vixenint
	 * @internal
	 */
	class Observer : public Core::List
	{
		friend class Messenger;
		VX_DECLARE_CLASS(Messenger::Observer);
	public:
		Observer() : Core::List(), Code(0) { }
		Observer(const SharedObj* target, uint32 code, const SharedObj* sender) : Core::List()
			{ Target = target; Code = code; Sender = sender; }

		void	Dispatch(Event*);
		int		Remove(const SharedObj* target, const SharedObj* sender);
		bool	operator==(const Observer&);

		ObjRef	Target;				// object that is observing
		ObjRef	Sender;				// object that is observed, if any
		uint32	Code;				// event code being observed
	};

	Messenger(Core::Stream* stream = NULL, bool dolock = false);
	virtual ~Messenger();

//! @name Name and Handle Mapping
//@{
//!	Read and interpret Vixen binary protocol from input stream.
	virtual bool			Load();
//! Construct full pathname given a possibly relative URL or filename.
	virtual	const TCHAR*	GetPath(const TCHAR* name, TCHAR* buf = NULL, int buflen = 0);
//! Find an object based on its name.
	virtual	SharedObj*		Find(const TCHAR*) const;
//! Find all objects whose names match a given string.
	virtual	Vixen::ObjArray* FindAll(const TCHAR* name) const;
//! Attach named object to messenger.
	virtual	bool			Define(const TCHAR*, const SharedObj*);
//! Class factory which associates new empty object with a handle.
	virtual	SharedObj*		Create(uint32 classid, int handle = 0);
//! Attach this object to the messenger.
	virtual	int				Attach(const SharedObj*, int flags = 0);
//! Attach this object and all objects it references to the messenger.
	virtual	void			AttachAll(const SharedObj* obj);
//! Detach this object from the messenger.
	virtual void			Detach(const SharedObj*);
//! Detatch all objects whose names match a given string.
	virtual void			DetachAll(const TCHAR* name, const SharedObj* obj = NULL);
//! Return the object associated with the input handle.
	SharedObj*				GetObj(int handle) const;
//! Return maximum handle used by this messenger.
	int						GetMaxHandle() const;
//! Return name dictionary of this messenger.
	NameTable*				GetNameDict() const;
//@}

//! @name Observation and Distribution
//@{
//! Return connection identifier for this messenger.
	int					GetConnectID() const;
//! Establish connection identifier for this messenger.
	virtual	void		SetConnectID(int id);
//! Distribute this object to remote clients.
	virtual void		Distribute(const SharedObj*, int opts = 0);
//! Connect a local proxy object with a remote object by name.
	virtual	void		Connect(const TCHAR* remotename, SharedObj* proxy = NULL, int opts = SharedObj::GLOBAL);
//! Observe events based on code and sender.
	virtual	bool		Observe(const SharedObj* target, int code, const SharedObj* sender);
//! Ignore events based on code and sender.
	virtual	bool		Ignore(const SharedObj* target, int code, const SharedObj* sender);
//! Set output stream for sending logged transactions to remote processors.
	virtual void		SetInStream(Core::Stream*);				
//! Set input stream for receiving transactions from remote processors.
	virtual void		SetOutStream(Core::Stream*);				
//! Return input stream for this messenger.
	Core::Stream*		GetInStream() const;
//! Return output stream for this messenger.
	Core::Stream*		GetOutStream() const;
//@}

//! @name I/O Operators
//@{
//! Read a 32 bit integer from input stream.
	Messenger&	operator>>(int32&);
//! Read a 16 bit integer from input stream.
	Messenger&	operator>>(int16&);
//! Read a 32 bit float from input stream.
	Messenger&	operator>>(float&);
//! Write a null-terminated string.
	Messenger&	operator>>(TCHAR*);
//! Read a 64 bit integer from stream.
	Messenger&	operator>>(int64&);
//! Read object pointer from handle.
	Messenger&	operator>>(SharedObj*&);

//! Write a 32 bit integer to output stream.
	Messenger&	operator<<(int32);
//! Write a 16 bit integer to output stream.
	Messenger&	operator<<(int16);
//! Write a 64 bit integer to output stream.
	Messenger&	operator<<(int64);
//! Write a 32 bit float output stream.
	Messenger&	operator<<(float);
//! Write a null-terminated string.
	Messenger&	operator<<(const TCHAR*);
//! Write handle from object pointer.
	Messenger&	operator<<(const SharedObj*);
//! Write opcode to output stream.
	Messenger&	operator<<(const VXOpcode&);
//! Write event to output stream.
	Messenger&	operator<<(const Event&);
//@}

//
// Internal stuff
//
	bool				Copy(const SharedObj*);
	DebugOut&			Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	virtual	bool		BeginOp(int logtype = MESS_UpdateLog, const SharedObj* obj = NULL) { return false; };
	virtual	void		EndOp() { };
	virtual int			CanSave(const SharedObj*, int opts);	
	virtual	Messenger&	OutObj(const SharedObj*);
	virtual	Messenger&	InObj(SharedObj*&);
	virtual	Messenger&	OutOp(const VXOpcode&);
//
// Overrides to use input and output streams
//
	virtual bool	Close();					// Close input and output stream.
	virtual void	Flush();					// Flush output stream.
	virtual bool	IsEmpty() const;			// Determine if input stream has input.
	virtual size_t	Read(char*, int);			// Read bytes from input stream.
	virtual size_t	Write(const char*, int);	// write bytes to output stream.
	virtual bool	Open(const TCHAR* name, int mode = Messenger::OPEN_RW);
	virtual bool	Input(int64*, int);			// Inputs block of 64 bit integers.
	virtual bool	Input(int32*, int);			// Inputs block of 32 bit integers.
	virtual bool	Input(int16*, int);			// Inputs block of 16 bit integers.
	virtual bool	Input(float*, int);			// Inputs block of 32 bit floats.
	virtual bool	Output(const int32*, int);	// Outputs block of 32 bit integers.
	virtual bool	Output(const int16*, int);	// Outputs block of 16 bit integers.
	virtual bool	Output(const float*, int);	// Outputs block of 32 bit floats.
	virtual bool	Output(const int64*, int);	// Outputs block of 64 bit integers.

//
// public data members
//
	int				LogType;		//!< current transaction log
	int				Version;		//!< current version of protocol
	int				FileVecSize;	//!< number of floats in position, normals for currently loading file
	bool			DoSync;			//!< true to frame-synchronize with remote processors, default is false
	bool			SendEvents;		//!< true to send events to remote clients, default is false
	bool			SendUpdates;	//!< true to send updates to remote clients, default is false
	static int		SysVecSize;		//!< number of floats in position, normals for current renderer
	THREAD_LOCAL bool	t_NoLog;	//!< true to disable transaction logging for current thread

protected:
	virtual	void	DoEvent();
	virtual	int		DoCommand(int command);

protected:
	Ref<NameTable>		m_Names;
	Ref<ObjMap>			m_Objs;
	Ref<PtrArray>		m_Observers;
	Ref<Core::Stream>	m_InStream;
	Ref<Core::Stream>	m_OutStream;
	int					m_ConnectID;
	int					m_OpenMode;
};

/*!
 * @class BufMessenger
 *
 * @brief Load or save Vixen binary protocol to a pool of fixed size memory buffers.
 *
 * Most of the Vixen messagers are built on top of this basic class.
 * The buffered messenger maintains its own memory buffers and does not
 * require input or output streams to work. Currently, it ignores any
 * streams which are assigned to it.
 *
 * The buffered messenger is a singleton - you can only instantiate one of them.
 * The architecture supports multiple writers and a single reader.
 *
 * @ingroup vixen
 * @see Messenger Synchronizer ThreadQueue
 */
class BufMessenger : public Messenger
{
	friend class World;
public:
	BufMessenger(Core::ThreadQueue* bufpool = NULL);
	~BufMessenger();

//	Overrides
	bool			Load();
	bool			IsEmpty() const;
	size_t			Read(char*, int);
	size_t			Write(const char*, int);
	bool			Open(const TCHAR* name, int mode = Messenger::OPEN_RW);
	bool			Close();
	void			Flush();
	bool			BeginOp(int logtype =  MESS_UpdateLog, const SharedObj* obj = NULL);
	void			EndOp();
	void			DoEvent();
	Messenger&		OutOp(const VXOpcode&);

//	New Methods
	static BufMessenger*	Get();
	int				GetBuffer(char** buffer);
	static int32	MaxBufSize;

protected:
	Core::Buffer*	SwitchBuffers(Core::Buffer* oldbuf);
	void			SetReadBuf(Core::Buffer* b);
	void			SetWriteBuf(Core::Buffer* b);
	Core::Buffer*	GetReadBuf() const;
	Core::Buffer*	GetWriteBuf();
	void			SelectReadLog(int logtype = MESS_UpdateLog);
	void			SelectWriteLog(int logtype = MESS_UpdateLog);
	int				GetReadLog() const;
	int				GetWriteLog() const;
	virtual bool	OnSend(Core::Buffer*)	{ return true; }

	static	BufMessenger*	s_OnlyOne;
	Core::Buffer*			m_ReadBuf;		// current read buffer
	char*					m_ReadPtr;		// current read pointer
	Ref<Core::ThreadQueue>	m_BufPool;		// pool of available buffers
	THREAD_LOCAL char*		t_LastOp;		// last opcode this thread wrote
	THREAD_LOCAL int		t_ReadLogType;	// the log this thread is reading from
	THREAD_LOCAL int		t_WriteLogType;	// the log this thread is writing to
	THREAD_LOCAL Core::Buffer*	t_WriteBuf[MESS_MaxLogs];
};

/*!
 * @class Synchronizer
 *
 * @brief Synchronizes a single scene across a network of processors at
 * either the object or the frame level.
 *
 * The synchronizer can have up to 32 simultaneous connections for frame level sychronization.
 * In this case, messages are passed between a master sychronizer and a set of remote slave connections.
 * If a fast local network is used, multiple machines with their own displays
 * can be made to present a single interactive scene.
 *
 * A synchronizer can also be used to distribute a 3D scene at the object level,
 * allowing a set of remote connections to simultaneously update a single
 * 3D world (as in a multiplayer game). It can be used to arbitrate input
 * events from remote sources such as speech and vision based tracking devices.
 *
 * This messenger passes all the events and updates from each connection to
 * the local 3D scene. All local events and updates directed at the
 * synchronizer are sent to all the connections.
 * The connections need not be of the same type and are processed
 * serially in the order they are added.
 * @image html synch.jpg
 *
 * @ingroup vixen
 * @see Messenger BufMessenger
 */
class Synchronizer : public BufMessenger
{
public:
	VX_DECLARE_CLASS(Synchronizer);
	Synchronizer(int maxbuf = 2 * MESS_MaxBufSize, Core::ThreadQueue* bufpool = NULL);
	~Synchronizer();

	virtual bool	ConnectObjs(uint32 flags);
	virtual	int32	WhoIsConnected() const;
	virtual	int32	GetSyncFlag(int i) const;

	bool			Open(const TCHAR* name, int mode = Messenger::OPEN_RW);
	bool			Close();
	SharedObj*		Create(uint32 classid, int handle = 0);
	bool			IsEmpty() const;
	bool			Load();
	void			Flush();
	size_t			Read(char*, int);
	bool			OnSend(Core::Buffer*);
	int				DoCommand(int cmd);
	Messenger&		InObj(SharedObj*&);

protected:
	virtual bool	SendAll(uint32 sendflags);
	bool			SendToArbitrator(int* connections, int nconn, uint32 sendflags);
	void			ChangeHandle(int oldhandle, int newhandle, uint32 sendmask);
	void			DoRemap(int oldhandle, int newhandle, uint32 sendmask);
	int				LookupHandle(int handle, uint32 sendmask);

	static	uint32	s_SyncTable[MESS_MaxHosts];
	mutable bool	m_IsOpen;
	bool			m_WasSent;		// indicates buffer was sent
	bool			m_IsFull;		// true if input buffer full
	bool			m_ForceEmpty;	// true to force empty input
	bool			m_ReadSocket;	// read from the socket as opposed to the buffers
	int32			m_MaxBufSize;	// size of send buffer
	int32			m_BytesSent;	// number of bytes sent
	mutable int32	m_CurConn;		// number of current connection	
	int32			m_MaxPrevID;	// maximum stream ID from previous frame
	int32			m_CurBase;
	char*			m_SendBuf;		// socket send buffer
	int32			m_SyncFlags;	// bit flags for clients the synced
	int32			m_SyncAll;		// bit flags for clients to sync
	int32			m_SendAll;		// bit flags for clients to send to
	int32			m_AllClients;	// bit flags for all clients
	int32			m_SendAgain;	// bit flags for sents that failed
	ObjRef			m_InMap;		// incoming ID remapping table
};


} // end Vixen