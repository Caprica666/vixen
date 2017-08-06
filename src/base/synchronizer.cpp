#include "vixen.h"

namespace Vixen {
using namespace Core;

VX_IMPLEMENT_CLASSID(Synchronizer, Messenger, VX_Synchronizer);

struct vxRemap
{
	vxRemap(int32 id = 0, uint32 mask = 0) { Handle = id; SendMask = mask; }

	int32	Handle;
	uint32	SendMask;
};

template <> inline uint32 Core::Dict<vxRemap, int32, Vixen::BaseDict>::HashKey(const vxRemap& k) const
{
	return HashInt(k.Handle);
}

template <> inline bool Core::Dict<vxRemap, int32, Vixen::BaseDict>::CompareKeys(const vxRemap& i, const vxRemap& j)
{
	return (i.Handle == j.Handle) && (i.SendMask == j.SendMask);
}

uint32	Synchronizer::s_SyncTable[MESS_MaxHosts] = { 0,
	0x000001, 0x000002, 0x000004, 0x000008, 0x000010, 0x000020, 0x000040, 0x000080,
	0x000100, 0x000200, 0x000400, 0x000800, 0x001000, 0x002000,	0x004000, 0x008000,
	0x010000, 0x020000, 0x040000, 0x080000, 0x100000, 0x200000, 0x400000,
};

/*!
 * @fn Synchronizer::Synchronizer(int maxbuf, VThreadQueue* bufpool)
 *
 * Constructs a synchronizer which uses the given buffer pool for
 * buffer management.
 */
Synchronizer::Synchronizer(int maxbuf, Core::ThreadQueue* bufpool)
: BufMessenger(bufpool)
{
	m_SendBuf = NULL;
	m_BytesSent = 0;
	m_MaxBufSize = maxbuf + 4 * sizeof(int32);
	m_WasSent = true;
	m_IsFull = false;
	m_CurConn = -1;
	m_ReadSocket = false;
	m_SyncFlags = m_SyncAll = m_SendAll = 0;
}

Synchronizer::~Synchronizer()
{
	Flush();
	Close();
}

/****
 *
 * Synchronizer override for Core::Stream::Open
 * Allocate buffers and initialize flags
 *
 ****/
bool Synchronizer::Open(const TCHAR* svrname, int mode)
{
	if (m_SendBuf == NULL)
	{
		m_SendBuf = new char[m_MaxBufSize];
		m_SendBuf += 2 * sizeof(int32);		// leave room for VX_Sync
		m_MaxBufSize -= 3 * sizeof(int32);	// and for VX_End
	}
	m_BytesSent = 0;
	m_WasSent = true;
	m_IsFull = false;
	m_MaxPrevID = m_CurBase = 0;
	m_SyncFlags = m_SyncAll = m_SendAll = m_AllClients = 0;
	m_CurConn = -1;
/*
 * If the name is NULL or begins with "master" assume we are
 * a server that can have multiple connections
 */
	if (svrname)
	{
		SetName(svrname);
		if (STRCASECMP(svrname, TEXT("master")) == 0)
			return true;
	}
	else
		svrname = GetName();
	m_SendAgain = 0;
/*
 * We are a client and talk to only one connection, our master.
 * If sending updates, emit an initial transaction to let the master
 * know we want to recieve updates.
 */
	if (GetConnectID() <= 0)
		SetConnectID(-1);
	if ((mode & Stream::OPEN_WRITE) &&
		(SendUpdates || SendEvents || DoSync))
	{
		int32 syncflag = GetSyncFlag(GetConnectID());
		if (DoSync)
			m_SyncAll = syncflag;
		m_SendAll = m_AllClients = syncflag;
		*this << int32(VIXEN_Begin) << 1L;
		*this << int32(VIXEN_DoNothing);
	}
	return true;
}

/****
 *
 * Synchronizer override for Core::Stream::Close
 * Garbage collect buffers and reset flags
 *
 ****/
bool Synchronizer::Close()
{
	if (SendUpdates || DoSync)
	{
		BeginOp(MESS_FastLog);
		*this << int32(VIXEN_Exit) << int32(GetConnectID());
		EndOp();
		Flush();
	}
	if (m_SendBuf)
	{
		m_SendBuf -= 2 * sizeof(int32);		// reclaim VX_Sync space
		delete m_SendBuf;
		m_SendBuf = NULL;
		m_MaxBufSize += 3 * sizeof(int32);	// reclaim VX_End space
		m_BytesSent = 0;
	}
	return BufMessenger::Close();
}

/****
 *
 * Synchronizer override for Core::Stream::Flush
 * Called at the end of every frame to send accumulated transations
 * to clients
 *
 ****/
void Synchronizer::Flush()
{
	BufMessenger::Flush();
	if (DoSync && (m_BytesSent == 0) && m_SendBuf)
	{
		*((int32*) m_SendBuf) = VIXEN_DoNothing;
		m_BytesSent += sizeof(int32);
		m_WasSent = false;
	}
	m_SyncFlags = 0;
	SendAll(m_SendAll);
}

/****
 *
 * Synchronizer override for VXmessenger::Load
 * Called at the start of every frame to read accumulated transations
 * from clients. Will not return until all clients have synchronized
 *
 ****/
bool Synchronizer::Load()
{
	m_ForceEmpty = true;
	BufMessenger::Load();		// load from memory buffers
	m_ForceEmpty = false;
	if (!m_InStream.IsNull() && !m_InStream->IsEmpty())
	{
		m_ReadSocket = true;
		do
			Messenger::Load();		// load from socket
		while (DoSync && m_SyncFlags && (m_SendAgain == 0));
		m_ReadSocket = false;
	}
	m_MaxPrevID = GetMaxHandle();
/*
 * Output a start of frame event if we are connected to anyone
 * requesting frame synchronization
 */
	if (!m_OutStream.IsNull() &&
		m_OutStream->IsOpen(Stream::OPEN_WRITE) &&
		DoSync && (GetConnectID() == 0) && m_SyncAll)
	{
		FrameEvent* e = new FrameEvent(this);

		e->Time = World::Get()->GetTime();
		e->Frame = Scene::GetTLS()->Frame;
		*this << *e;
		delete e;
	}
	return true;
}

size_t Synchronizer::Read(char* buffer, int n)
{
	if (m_ReadSocket)
		return Messenger::Read(buffer, n);
	else
		return BufMessenger::Read(buffer, n);
}

bool Synchronizer::IsEmpty() const
{
	if (!BufMessenger::IsEmpty())
		return false;
	if (m_ForceEmpty)
		return true;
	m_CurConn = -1;
	if (m_InStream.IsNull() || m_InStream->IsEmpty())
		return true;
	if (m_InStream->IsKindOf(CLASS_(Arbitrator)))
	{
		int connid = ((Core::Arbitrator*) (Core::Stream*) m_InStream)->GetConnection();
		m_CurConn = connid;
		if (m_SendAgain & GetSyncFlag(connid))
			return true;				// don't read if pending send
	}
	return false;
}


/*!
 * @fn int Synchronizer::DoCommand(int command)
 *
 * Dispatch these stream commands:
 * @code
 *	VIXEN_Begin		begin packet
 *	VIXEN_End			end packet
 *	VIXEN_Event		dispatch Event
 *	VIXEN_SetStreamID	atttach remote connection
 *	VIXEN_Exit		detach remote connection
 *	VIXEN_Sync		sychronize Ok message
 * @endcode
 */
int Synchronizer::DoCommand(int command)
{
	int32	n, m;
	int32	syncflag;
	TCHAR	namebuf[VX_MaxName];
	SharedObj*	obj;

	switch (command)
	{
		case VIXEN_SetStreamID:	// set stream ID
		*this >> n;				// read stream ID we should have
		VX_ASSERT((n >= 0) && (n < MESS_MaxHosts));
		VX_TRACE(Debug, ("VX_SetStreamID: %d", n));
		SetConnectID(n);
		m_SendAll = GetSyncFlag(n);
		return 0;

		case VIXEN_Begin:
		*this >> n;				// read source stream ID
		VX_TRACE(Debug, ("VX_Begin: %d", n));
		if ((n <= 0) && ((n = m_CurConn) < 0))
			return 0;			// begin from master is ignored
		syncflag = n;
		if (m_AllClients & syncflag)
			return 0;			// already connected
//		m_AllClients |= syncflag;
		return 0;

		case VIXEN_Exit:
		*this >> n;				// read connection ID
		VX_TRACE(Debug, ("VX_Exit: %d", n));
		if (n == 0)
		{
			m_SyncFlags = m_SyncAll = m_SendAll = 0;
			World::Get()->Stop();
		}
		else
		{
			if (m_OutStream->IsKindOf(CLASS_(Arbitrator)))
				((Arbitrator*) (Core::Stream*) m_OutStream)->RemoveConnection(n);
			uint32 syncflag = GetSyncFlag(n);
			if (syncflag)
			{
				m_SendAll &= ~syncflag;
				m_SyncAll &= ~syncflag;
				m_SyncFlags &= ~syncflag;
			}
		}
		return -1;

		case VIXEN_Sync:
		*this >> n;				// read new sync flag
		VX_TRACE(Debug, ("VX_Sync 0x%X", n));
		if (n == 0)
		{
			m_SyncFlags = 0;
			return 0;
		}
		m_AllClients |= n;
		m_SyncFlags &= ~n;
		m_SyncAll |= n;
		return 0;
/*
 * VX_Remap <oldid> <newid> <slaveflags>
 * Request to remap the local object <oldid> to <newid>. All the object
 * remapping requests are performed after everything is loaded
 */
		case VIXEN_Remap:			// remap object IDs
		*this >> n >> m >> syncflag;
		VX_TRACE(Debug, ("VX_Remap %d %d %p", n, m, syncflag));
		DoRemap(n, m, syncflag);
		return 0;

/*
 * Connect <handle> <name>
 * Connects a proxy on the remote machine to an object on this one.
 * The local object must be one defined in the stream dictionary.
 * The remote object handle is remapped to match the local handle if necessary
 */
		case VIXEN_Connect:
		*this >> n >> namebuf;
		syncflag = GetSyncFlag(m_CurConn);
		if (STRCASECMP(namebuf, TEXT("scene")) == 0)	// connecting to scene special
		{
			BeginOp(MESS_FastLog);
			*this << int32(VIXEN_SetStreamID) << int32(m_CurConn);
			EndOp();
			ConnectObjs(syncflag);			// send the scene to this connection
			return 0;
		}
		obj = Find(namebuf);				// find existing object
		if (obj == NULL)
		   { VX_ERROR(("VX_Connect: object %s not found", namebuf), 0) }
		VX_TRACE(Debug, ("VX_Connect %s %d %p\n",namebuf, n, obj));
// TODO: distribute the object ONLY to connected clients
//		if (m)								// wants updates from us?
//			Distribute(obj, m);				// distribute this object
		if (obj->GetID() != uint32(n))		// handles not the same?
			ChangeHandle(n, obj->GetID(), syncflag);
		return 0;
	   }
	n = BufMessenger::DoCommand(command);
	if (n <= 0)
		return n;
	if (m_CurConn > 0)
		syncflag = GetSyncFlag(m_CurConn);
	else
		syncflag = 0;
	return LookupHandle(n, syncflag);	// Remap handle if the input mapper says to
}

Messenger& Synchronizer::InObj(SharedObj*& obj)
{
	int32		handle;
	uint32		syncflag = 0;

	*this >> handle;
	if (m_CurConn > 0)
		syncflag = GetSyncFlag(m_CurConn);
	handle = LookupHandle(handle, syncflag);
	if (handle == 0)
		obj = (SharedObj*) NULL;
	else
		obj = m_Objs->GetObj(handle);
	return *this;
}

/****
 *
 * LookupHandle
 * Determines if an input handle needs to be remapped according to the
 * handle remapping dictionary. This allows input streams to supply
 * different handles for objects than the local machine uses.
 *
 ****/
int Synchronizer::LookupHandle(int handle, uint32 sendmask)
{
	if (m_InMap.IsNull() || (handle <= 0))
		return handle;
	Dictionary<vxRemap, int32>* mapper = (Dictionary<vxRemap, int32>*) (SharedObj*) m_InMap;
	vxRemap newid(handle, sendmask);
	int32* idptr = mapper->Find(newid);
	if (idptr)
		return *idptr;
	return handle;
}

/****
 *
 * DoRemap
 * Processes a handle remapping request. If the old handle is found in
 * our local remapping table, this request is from a remote client acknowledging
 * the remap. We delete the handle from the remapping table in this case.
 * If we are a remote client processing a remap request from the master,
 * change the local handle to match the new input value and relocate the
 * object in the local handle table.
 *
 ****/
void Synchronizer::DoRemap(int oldhandle, int newhandle, uint32 sendmask)
{
	Dictionary<vxRemap, int32>* mapper = (Dictionary<vxRemap, int32>*) (SharedObj*) m_InMap;
	vxRemap remap(oldhandle, sendmask);

	if (mapper)
	{
		int32* newid = mapper->Find(remap);
		if (newid)						// remove the mapping
		{
			VX_ASSERT(newhandle == *newid);
			mapper->Remove(remap);
			return;
		}
	}
	SharedObj* obj = m_Objs->GetObj(oldhandle);		// get object at old handle
	VX_ASSERT(obj->GetID() == uint32(oldhandle));	// object IDs match?
	if (m_Objs->AttachObj(obj, newhandle) != newhandle)
		VX_ERROR_RETURN(("Synchronizer::ProcessRemap remapping error"));
	obj->SetID(newhandle);							// clear object ID
	BeginOp(MESS_FastLog);						// echo the remap
	*this << int32(VIXEN_Remap) << int32(oldhandle) << int32(newhandle) << int32(sendmask);
	EndOp();
}

/****
 *
 * ChangeHandle
 * Requests a remote client to remap an object handle. VX_Remap request
 * is sent to the remote client and the mapping between the client
 * handle and the local handle is recorded in a remapping table.
 * Until the client acknowledges the remapping, we will allow
 * them to identify the object by this handle in messages and will
 * remap the handle appropriately.
 *
 ****/
void Synchronizer::ChangeHandle(int oldhandle, int newhandle, uint32 sendmask)
{
	if (SendUpdates && (m_SendAll & sendmask))
	{
		BeginOp(MESS_FastLog);
		*this << int32(VIXEN_Remap) << int32(oldhandle) << int32(newhandle) << int32(sendmask);
		EndOp();
	}
	VX_TRACE(Debug, ("Synchronizer::ChangeHandle VX_Remap %d %d %p", oldhandle, newhandle, sendmask));
	Dictionary<vxRemap, int32>* inmapper = (Dictionary<vxRemap, int32>*) (SharedObj*) m_InMap;
	if (inmapper == NULL)
		m_InMap = inmapper = new Dictionary<vxRemap, int32>;
	vxRemap remap(oldhandle, sendmask);
	inmapper->Set(remap, newhandle);
}

/****
 *
 * SyncStream_Create
 * Creates a local object and adds it the stream. If the handle provided
 * for the new object clashes with an existing object, we send a request
 * to remap the handle to the remote client which sent the create. The
 * local handle is kept in a remapping table until the client
 * acknowledges the change (since the client is still using the old
 * handle to identify the object in messages)
 *
 ****/
SharedObj* Synchronizer::Create(uint32 classid, int handle)
{
	SharedObj*	obj = NULL;
	int32	realhandle = handle;
	int32	sharing = IsShared(classid);

	if ((handle > 0) && (obj = m_Objs->GetObj(handle)))
		if (obj->IsClass(classid))					// class types match?
			return obj;								// is the same object
		else										// not the same object!
		{
			obj = NULL;								// force remap
			realhandle = -1;
		}
	if (obj == NULL)
	{
		obj = SharedObj::Create(classid);				// create new object
		if (obj == NULL)
			VX_ERROR(("Synchronizer::Create(%s) ERROR cannot create", Class::GetClass(classid)->GetName()), NULL);
	}
	if (sharing)
		obj->SetFlags(sharing | SAVED);			// distribute this one?
	else
	{
		obj->ClearFlags(GLOBAL | SHARED);	// not distributed
		obj->SetFlags(SAVED);					// don't try to distribute
	}
	realhandle = m_Objs->AttachObj(obj, realhandle);// attach to stream
	if (realhandle <= 0)
		VX_ERROR(("Synchronizer::Create(%s, %d) ERROR cannot map",  Class::GetClass(classid)->GetName(), handle), NULL);
	if ((handle > 0) && (realhandle != handle))		// mapping changed?
	{
		uint32 syncflag = (m_CurConn > 0) ? GetSyncFlag(m_CurConn) : 0;
		ChangeHandle(handle, realhandle, syncflag); // remap the handle
	}
	VX_TRACE(Debug, ("Synchronizer::Create(%s, %d) %p",  Class::GetClass(classid)->GetName(), realhandle, obj));
	if (obj->GetID() == 0)
		obj->SetID(realhandle);						// save handle in object
	return obj;
}

/****
 *
 * Synchronizer override for OnSend
 * Copy the contents of the given buffer into the send buffer.
 * If all of the data won't fit, reject the buffer
 *
 * Returns:
 *	<true> if buffer was copied, <false> if it can't be sent
 *
 ****/
bool Synchronizer::OnSend(Core::Buffer* buf)
{
	int32	len = buf->NumBytes;
	char*	data = (char*) buf;
	char*	sendptr = m_SendBuf;

	buf->State = Core::BufState( int(buf->State) & ~BUF_Pending );
	if (m_SendAll == 0)
		return true;
	if (m_IsFull || (m_BytesSent + buf->NumBytes >= m_MaxBufSize))
	{
		VX_TRACE(Debug > 2, ("Synchronizer::OnSend FULL %d:%d %d bytes", buf->Queue, buf->ID, buf->NumBytes));
		m_IsFull = true;
		buf->State = Core::BufState( int(buf->State) | BUF_Pending );
		return false;
	}
	VX_TRACE(Debug > 1, ("Synchronizer::OnSend SUCCEED %d:%d %d bytes", buf->Queue, buf->ID, buf->NumBytes));
	memcpy(sendptr + m_BytesSent, buf->GetData(), buf->NumBytes);
	m_BytesSent += buf->NumBytes;
	m_WasSent = false;
	m_IsFull = false;
	return true;
}

int32 Synchronizer::GetSyncFlag(int i) const
{
	if (GetConnectID() < 0)
		return 1;
	if (i == 0)
		return s_SyncTable[GetConnectID()];
	return s_SyncTable[i];
}

/*!
 * @fn bool Synchronizer::SendAll(uint sendflags)
 *
 * Sends the accumulated output to all clients designated
 * by \b sendflags. If any of the sends fail, the send will
 * be repeated.
 *
 * @return \b true if send successful on all clients, else \b false
 *
 * @see Synchronizer::OnSend
 */
bool Synchronizer::SendAll(uint32 sendflags)
{
	if (m_OutStream.IsNull() || !m_OutStream->IsKindOf(CLASS_(Arbitrator)))
		return false;

	Core::Arbitrator*	stream = (Core::Arbitrator*) (Core::Stream*) m_OutStream;
	int				connections[MESS_MaxHosts];
	int				ntotal = stream->GetSize();
	int				nconn = 0;

	for (int i = 0; i < ntotal; ++i)		// who needs to be sent something?
		if (GetSyncFlag(i + 1))
			connections[nconn++] = i;
	if (nconn <= 0)							// nothing needs sending
		return true;
	do										// try to send to all clients
	{
		if (stream->Select(connections, nconn) > 0)
			SendToArbitrator(connections, nconn, sendflags);
		sendflags = m_SendAgain;			// need to resent these
	}
	while (sendflags);
	return true;
}


bool Synchronizer::SendToArbitrator(int* connections, int nconn, uint32 sendflags)
{
	Arbitrator* stream = (Arbitrator*) (Core::Stream*) m_OutStream;

	VX_ASSERT(stream->IsKindOf(CLASS_(Arbitrator)));
	VX_ASSERT((intptr(m_SendBuf + m_BytesSent) & 3) == 0);
	*((int32*) (m_SendBuf + m_BytesSent)) = VIXEN_End;
	for (int i = 0; i < nconn; ++i)
	{
		int nbytes = m_BytesSent + 3 * sizeof(int32);
		int nread = 0;
		int connid = connections[i];
		intptr	curconn;
		int32* idata = (int32*) (m_SendBuf - 2 * sizeof(int32));
		char* sendptr = (char*) idata;
		uint32 syncflag = s_SyncTable[connid];

		if (connid < 0)
			continue;
		curconn = stream->GetAt(connid);
		if (curconn < 0)
			continue;
		if ((syncflag == 0) && (nconn == 1))
			syncflag = 1;
		if (!(sendflags & syncflag))		// send to this one?
			continue;						// no, just read from it
		if (m_SyncAll & syncflag)			// synchronize this one?
		{
			idata[0] = VIXEN_Sync;				// VX_Sync to synchronize this packet
			idata[1] = GetSyncFlag(GetConnectID());
		}
		else if (SendUpdates)					// sending update packets?
		{
			idata[0] = VIXEN_Begin;				// VX_Begin to begin this packet
			idata[1] = GetSyncFlag(GetConnectID());
		}
		else									// no, just listening for events
		{
			idata[0] = VIXEN_DoNothing;			// VX_DoNothing for events only
			idata[1] = VIXEN_DoNothing;
		}
		if (m_SendAll & syncflag)				// should send this one?
		{
			m_SendAgain |= syncflag;			// mark as tried to send
			if (stream->SendConnection(curconn, (char*) idata, nbytes))
			{
				if (idata[0] == VIXEN_Sync)		// sent a sync?
					m_SyncFlags |= syncflag;	// mark as needing sync back
				m_SendAgain &= ~syncflag;		// mark as sent successfully
				connections[i] = 0;				// remove connection
			}
		}
	}
	m_WasSent = (m_SendAgain == 0);
	if (m_WasSent)
	{
		m_IsFull = false;
		m_BytesSent = 0;
	}
	return m_WasSent;
}


/*!
 * @fn bool Synchronizer::ConnectObjs(uint32 flags)
 * @param flags	Bit flags indicating which clients to send objects to.
 *
 * Connects the objects of the local scene to the given connection.
 * The contents of the scene objects are sent to the connection
 * within a single VX_Begin / VX_End block (this may be a large
 * amount of data and may require multiple frames to transmit.
 * After the scene objects are sent, the connection is assigned a
 * connection id and told to update its local scene.
 * @code
 *	VX_Begin
 *	<send all the scene objects>
 *	VX_SetStreamID <n>
 *	VX_Event Event::ATTACH_SCENE
 *	VX_End
 * @endcode
 *
 * @see Messenger::Define Messenger::Attach
 */
int32 Synchronizer::WhoIsConnected() const
{
	return m_SyncAll;
}

bool Synchronizer::ConnectObjs(uint32 flags)
{
	if (!m_OutStream->IsOpen(Stream::OPEN_WRITE))
		return false;

	Scene* scene = GetMainScene();
	SharedObj*	obj;
	int32	n = GetMaxHandle();
	int32	handle;
	int32	debug = Debug;

	if ((flags & m_AllClients) != flags)
		return false;
	if (flags == 0)
		return false;
	if (m_AllClients == m_SendAll)	// already all connected?
		return false;
	m_SendAll |= (flags & m_AllClients);

	for (handle = 0; handle <= n; ++handle)
	{
		obj = GetObj(handle);			// clear all "saved" flags
		if (obj)
			obj->ClearFlags(SAVED);
	}
	SendUpdates = true;
	SelectWriteLog(MESS_FastLog);	// hi priority log
	BeginOp(MESS_FastLog);			// establish scene first
	if (Debug <= 1)
		Debug = 0;						// disable debug printout
	Group* dyn = scene->GetModels();
	if (dyn && dyn->IsActive())
		dyn->SetActive(false);
	else
		dyn = NULL;
	World3D::Get()->Save(*this, MESS_Distribute);
	scene->Save(*this, MESS_Distribute);
	Debug = debug;
	Event *e = new Event(Event::ATTACH_SCENE, 0, scene);
	e->Save(*this);
	EndOp();

	BeginOp(MESS_UpdateLog);
	if (dyn)
	{
		*this << OP(VX_Group, Group::GROUP_SetActive) << dyn << 1L;
		dyn->SetActive(true);
	}
	EndOp();
	return true;
}

}// end Vixen