
#include "vixen.h"

namespace Vixen {
using namespace Core;


VX_IMPLEMENT_CLASSID(Messenger, Core::Stream, VX_Messenger);
VX_IMPLEMENT_CLASS(Messenger::Observer, List);

bool	Messenger::t_NoLog;
int		Messenger::SysVecSize = VX_VEC3_SIZE;

Messenger::Messenger(Core::Stream* stream, bool dolock) : SharedObj()
{
	m_ConnectID = 0;
	Version = MESS_CurrentVersion;
	FileVecSize = 3;
	DoSync = SendUpdates = SendEvents = false;
	m_Observers = new PtrArray;
	m_Names = new NameTable;
	m_Objs = new ObjMap;
	if (dolock)
	{
		m_Names->MakeLock();
		m_Objs->MakeLock();
		m_Observers->MakeLock();
	}
	SetInStream(stream);
}

bool Messenger::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!SharedObj::Copy(srcobj))
		return false;

	Messenger*	src = (Messenger*) srcobj;

	m_Objs = src->m_Objs;
	m_Names = src->m_Names;
	m_Observers = src->m_Observers;
	m_Names->MakeLock();
	m_Objs->MakeLock();
	m_Observers->MakeLock();
	return true;
}


Messenger::~Messenger()
{
	m_Observers = (PtrArray*) NULL;
	m_Objs = (ObjMap*) NULL;
	m_Names = (NameTable*) NULL;
}

/*!
 * @fn void Messenger::Distribute(const SharedObj* obj, int opts)
 * @param obj		object to distribute.
 * @param shareopts	distribution options.
 *
 * Distributes an object to remote clients based on the input sharing options
 * and the sharing options specified for objects of this class. The input
 * options are used in addition to the class distribution options.
 * If the input options are -1, it indicates the object should <not> be
 * shared or distributed. The object and all objects it references are affected.
 *	- SHARED specifies all objects of this class should be shared.
 *	- GLOBAL send updates to this object to remote clients 
 *	- INACTIVE disables objects of this class when received by clients.
 */
void Messenger::Distribute(const SharedObj* obj, int opts)
{
	int32	flags;
	SharedObj*	cheat = (SharedObj*) obj;	// saving to archive does not really change object

	if (opts != -1)					// -1 means to undistribute
	{
		flags = obj->GetFlags() & (SHARED | GLOBAL | SAVED);
		if (flags)
			return;
		flags = IsShared(obj->ClassID()) | opts;
		if (!SendUpdates)
		{
			obj->SetFlags(flags);
			return;
		}
		if (flags & GLOBAL)
		{
			BeginOp(MESS_FastLog);
			cheat->Save(*this, MESS_Distribute);
			EndOp();
		}
		obj->SetFlags(flags);
		return;
	}
	if (!obj->IsShared())
		return;
	obj->ClearFlags(GLOBAL | SHARED);
	if (!SendUpdates)
		return;
	BeginOp(MESS_FastLog);
	cheat->Save(*this, MESS_ClearGlobal);
	EndOp();
}

/*!
 * @fn int Messenger::CanSave(const SharedObj* obj, int savemode)
 * @param obj		object to save
 * @param savemode	controls the behavior of the SharedObj::Save function:
 *
 * Called internally to determine whether an object needs to be saved or distributed.
 * Depending on the return value from this function, SharedObj::Save will save the object and
 * all associated information to the stream (return value > 0), save only the objects it
 * uses (return value 0) or save nothing (return value -1).
 *
 * @code
 *	0					save to file if not already saved
 *	MESS_Detach			detach the object and all objects it uses from this stream
 *	MESS_ClearGlobal	mark the object and all objects it uses as not distributed
 *	MESSH_Distribute	mark the object and all objects it uses as shared based on
 *						the sharing options for its class
 * @endcode
 *
 * @see SharedObj::ShareClass SharedObj::IsShared SharedObj::Save
 */
int	Messenger::CanSave(const SharedObj* obj, int savemode)
{
	const TCHAR* name = obj->GetName();
	int32		h;
	int32		flags = SAVED;

	switch (savemode)
	{
		case MESS_Detach:
		Detach(obj);						// remove object from stream
		return 0;							// indicate no-save 

		case MESS_ClearGlobal:
		obj->ClearFlags(GLOBAL | SHARED);	// mark as not global
		return 0;							// indicate no-save 

		case MESS_Distribute:
		if (obj->IsSet(SAVED))
			return 0;						// don't need to save twice
		flags = obj->IsShared() & ~INACTIVE;
		if (flags == 0)						// is object explicitly shared?
		{
			flags = IsShared(obj->ClassID());
			if (flags == 0)					// class not shared?	
				return -1;					// do not save this object
		}
		flags |= SAVED;
		break;

		default:		
		if (obj->IsSet(SAVED))
			return 0;						// don't need to save twice
	}
	h = Attach(obj, flags);					// attach object to stream
	if (name)								// and to name dictionary
		Define(name, obj);	
	if (!SendUpdates || (savemode > MESS_Distribute))
		return 0;
	if (h)
		*this << OP(uint16(obj->ClassID()), SharedObj::OBJ_Create) << h;
#ifdef _DEBUG
	SharedObj* oldobj = GetObj(h);
	VX_ASSERT(oldobj == obj); 
#endif
	return h;
}	

/*!
 * @fn SharedObj* Messenger::Find(const TCHAR* name) const
 * @param name	String name of object to retrieve.
 *
 * Finds the object with the given name in the messenger's dictionary.
 * Only named objects from which have been loaded by this messenger are accessible.
 *
 * If the input objects came from the Maya exporter, the name
 * of each object is prefixed with the file name and a dot.
 * The root of the scene graph is \b file.root and the root
 * of the simulation tree is \b file.simroot where \b file.vix
 * is the input file.
 *
 * @return pointer to object found, NULL if object not found
 *
 * @par Example:
 * @code
 *	Core::FileStream file("file.vix");
 *	Messenger input(file, true);
 *	Model	*mod, *root;
 *	input.Load();
 *	mod = (Model*) input.Find("file.root");
 *	mod->Remove();		// take out of file hierarchy
 *	root->Append(mod);	// to put into another
 * @endcode
 *
 * @see Messenger::Define SharedObj::SetName Messenger::FindAll
 */
SharedObj* Messenger::Find(const TCHAR* name) const
{
	NameProp	np(name);
	ObjRef*	ref;

	if (STRCHR(name, TEXT('*')) == 0)		// not a wildcard search?
	{
		if (ref = m_Names->Find(np))
			return (SharedObj*) *ref;
		else
			return NULL;
	}
	if (ref = m_Names->FindWild(name))
		return (SharedObj*) *ref;
	return NULL;
}

/*!
 * @fn ObjArray* Messenger::FindAll(const TCHAR* name) const
 * @param name	String name of objects to retrieve. If the name is NULL
 *				or blank, all named objects in the stream are returned.
 *				Otherwise, only the objects whose names match the input
 *				name (using "*" as a wildcard) are returned.
 *
 * Finds all of the objects with names matching the input name.
 * Asterisk is used as a special character which will match
 * anything. For example, "*ABC" will match any name that
 * ends in "ABC" and "ABC*" matches any name beginning with "ABC".
 * To match any name containing ABC, including at the beginning
 * 	or end, use "*ABC*".
 *
 * @return array of objects found or NULL if no matches
 *
 * @par Example:
 * @code
 *	// Find all the path controllers exported by MAX
 *	Core::FileStream file("file.vix");
 *	Messenger input(file, true);
 *	input.Load();
 *	ObjArray* paths = input.FindAll("*.path");
 * @endcode
 *
 * @see Messenger::Define SharedObj::SetName FileLoader::FindAll
 */
Vixen::ObjArray* Messenger::FindAll(const TCHAR* name) const
{
	return m_Names->FindAll(name);
}

void Messenger::DetachAll(const TCHAR* name, const SharedObj* obj)
{
	ObjRef*		ref;
	SharedObj*			cheat = (SharedObj*) obj;	// saving does not change object
	NameIter<ObjRef> iter(m_Names, name);

	while (ref = iter.Next())
		Detach(*ref);
	if (obj == NULL)
		return;
	obj->Save(*this, MESS_Detach);
}

void Messenger::AttachAll(const SharedObj* obj)
{
	obj->Save(*this, MESS_Attach);
}

/*!
 * @fn bool Messenger::Define(const TCHAR* name, const SharedObj* obj)
 * @param name	Name the object should have.
 * @param obj	Pointer to object to define.
 *
 * Associates a name with an object on this messenger.
 * If the name is already defined for the messenger,
 * the old object it referenced is removed and the
 * name is associated with the new object.
 *
 * @see Messenger::Find Messenger::Connect Messenger::Attach
 */
bool Messenger::Define(const TCHAR* name, const SharedObj* obj)
{
	if (name == NULL)
		return false;

	SharedObj*	cheat = (SharedObj*) obj;
	NameProp	np(name);
	NameTable&	dict = *((NameTable*) m_Names);
	ObjectLock	lock((SharedObj*) m_Names);

	dict[np] = obj;
	if (obj)
		cheat->SetName(name);
	return true;
}

/*!
 * @fn int Messenger::Attach(const SharedObj* obj, int flags)
 * @param obj	pointer to local object to attach to stream
 * @param flags	flags to set after attaching, GLOBAL causes
 *				updates to be saved to stream
 *
 * Attaches an object to this messenger. The object is assigned
 * a unique handle	that identifies it globally thruout the
 * system to remote processors.
 *
 * All objects loaded from a file messenger are automatically
 * attached to that messenger until it is closed.
 *
 * If the object is marked as global, by setting the GLOBAL
 * flag, any updates to the object are logged to the disparcher
 * and distributed to remote processors.
 *
 * @return object handle or 0 on error
 *
 * @see SharedObj::IsGlobal SharedObj::SetName Messenger::Detach
 */
int Messenger::Attach(const SharedObj* obj, int flags)
{
	int	handle;
	SharedObj*	cheat = (SharedObj*) obj;

	if (obj == NULL)
		return 0;
	handle = obj->GetID();
	if (flags)
		cheat->SetFlags(flags);
	if (handle > 0)
		return handle;
	handle = m_Objs->AttachObj(obj);
	cheat->SetID(handle);
	return handle;
}



/*!
 * @fn void Messenger::Connect(const TCHAR* remotename, SharedObj* proxy, int mode)
 * @param obj			Pointer to local object to connect.
 * @param remotename	Name of remote object to connect.
 *
 * Connects a local object on this machine with the named object
 * on a remote server. Any changes made to the object locally are
 * send to the server to update the remote object, too.
 * Changes made to the object on the remote machine are NOT
 * also made to the proxy object, however. Connection is a write-only
 * communication to the remote machine.
 *
 * @see SharedObj::IsGlobal SharedObj::SetName Messenger::Detach Messenger::Create
 */
void Messenger::Connect(const TCHAR* remotename, SharedObj* proxy, int opts)
{
	Attach(proxy, GLOBAL | SHARED);
	BeginOp(MESS_FastLog);
	*this << int32(VIXEN_Connect) << proxy << remotename;
	EndOp();	
}

/*!
 * @fn SharedObj* Messenger::Create(uint32 classid, int handle)
 * @param classid	Vixen class identifier telling which class to construct.
 * @param handle	messenger handle requested.
 *
 * Creates an empty object of the given class and attaches it to this
 * messenger. The messenger will be able to process updates and events
 * that apply to this object.
 *
 * If a messenger is associated with a display scene, it is used
 * to process events for that scene (generated by mouse motion,
 * file loads and other input sources). Changes to distributed objects
 * are encoded in Vixen binary protocol and sent to remote processors
 * using the messenger.
 *	
 * In Vixen protocol, objects are identified by a global \b handle
 * which is the same on all processors. Once an object has been
 * attached to a disparcher, the object pointer may be mapped to and
 * from a  handle, allowing the messenger to easily encode and
 * decode the protocol. The global handle of an object attached
 * to a scene messenger is saved in as the object id. File based
 * messengers do not affect the object id.
 *
 * Not all objects attached to a disparcher are distributed. Only those
 * marked with the GLOBAL flag will have transactions logged to
 * messenger.
 *
 * @return pointer to object created or NULL on failure
 *
 * @see SharedObj::Create SharedObj::ClassID 
 */
SharedObj* Messenger::Create(uint32 classid, int handle)
{
	SharedObj*	obj = m_Objs->GetObj(handle);		// see if object already there
	if (obj)
	{
		if (!obj->IsClass(classid))
			VX_ERROR(("Messenger::Create(%s, %d) object class is %s\n", Class::GetClass(classid)->GetName(), handle, obj->ClassName()), obj);
	}
	else
		obj = SharedObj::Create(classid);			// create new object
	handle = m_Objs->AttachObj(obj, handle);	// attach to stream
	if ((obj == NULL) || (handle <= 0))
		VX_ERROR(("Messenger::Create(%s) ERROR cannot create\n", Class::GetClass(classid)->GetName()), NULL);
#ifdef _DEBUG
	if (Messenger::Debug)
	{
		const TCHAR* name = NULL;
		if (obj)
			name = obj->GetName();
		if (name == NULL)
			name = TEXT("");
		VX_TRACE(Messenger::Debug, ("%s::Create %d %s 0X%X\n", Class::GetClass(classid)->GetName(), handle, name, obj));
	}
#endif
	if (obj->GetID() == 0)
		obj->SetID(handle);					// save handle in object
	return obj;
}

int	Messenger::GetMaxHandle() const
{
	return m_Objs->MaxHandle;
}

/*!
 * @fn void Messenger::Detach(const SharedObj* obj)
 *
 * Detaches an object from the messenger. If the object is local,
 * the messenger no longer references it. If the messenger was
 * the last reference to the object, the object is deleted
 * and its resources freed.
 *
 * @see Messenger::Attach Messenger::Create
 */ 
void Messenger::Detach(const SharedObj* obj)
{
	if (obj == NULL)
		return;

	int			handle = m_Objs->GetHandle(obj);
	const TCHAR* name = obj->GetName();
	if (name)
		Define(name, NULL);
	if (handle <= 0)
		return;
	m_Objs->DeleteObj(handle);
	if (obj->GetID() == uint32(handle))
		((SharedObj*) obj)->SetID(0);
	return;
}

/*!
 * @fn bool Messenger::Load()
 *
 * Loads Vixen data from available input, executing all the Vixen
 * opcodes and applying the changes to the scene graph and
 * simulation tree.
 *
 * @return \b true if all objects successfully loaded, else \b false
 *
 * @see Messenger::IsEmpty
 */
bool Messenger::Load()
{
	int32		opcode, handle;
	int32		classid;
	SharedObj*		obj;

/*
 * Read opcode and decode classid, handle, operation
 */
	while (!IsEmpty() && Input(&opcode, 1))
	{
		handle = DoCommand(opcode);		// process stream command
		if (handle < 0)					// end of data?
			return true;
		if (handle == 0)				// was a stream command?
			continue;					// continue processing
/*
 * Decode object opcode into classid, handle, operation
 * classID in high order 16 bits, opcode in low order 16 bits
 */
		classid = (uint32(opcode) >> 16) & 0xFF;
		opcode &= 0xFFFF;				// opcode in low order word, classid in hi
		switch (opcode)
		{
/*
 * Create <handle>
 * Creates an empty object of the specified class and associates it
 * with the given handle. If the handle already has an object, it
 * is replaced. 
 */
			case OBJ_Create:
			obj = Create(classid, handle);
			break;
/*
 * Delete <handle>
 * Detach the object referenced by this handle from the stream and free the
 * handle to be used again
 */
			case OBJ_Delete:
			obj = m_Objs->GetObj(handle);	// get object from handle
			if (obj && obj->GetID() == uint32(handle))
				Detach(obj);
			else
				VX_WARNING(("Cannot detach %s %d undefined\n", Class::GetClass(classid)->GetName(), handle));
			break;
/*
 * Execute an opcode on an existing local object. If the object's local ID
 * is different than the stream handle it means the object has a different
 * handle in the output stream. This happens when loading a file into an
 * already existing scene, which causes a lot of handle remapping.
 */
			default:
			obj = m_Objs->GetObj(handle);	// get object from handle
			if (obj)
			{
				VX_ASSERT(obj->IsClass(classid));
				if (!obj->Do(*this, opcode))	// do the operation
				   { VX_WARNING(("%s %d unknown opcode\n", Class::GetClass(classid)->GetName(), opcode)); }
			}
			else
				VX_WARNING(("%s %d undefined\n", Class::GetClass(classid)->GetName(), handle));
		}
	}
	return true;
}

/*!
 * @fn SharedObj* Messenger::GetObj(int handle) const
 * @param handle	0-based messenger handle
 *
 * The handle is the same as the object identifier returned by
 * SharedObj::GetID for that object.
 *
 * @see Messenger::Attach SharedObj::GetID
 */
SharedObj* Messenger::GetObj(int handle) const
{
	return m_Objs->GetObj(handle);
}

/*!
 * @fn void Messenger::SetConnectID(int id)
 *
 * Sets the messenger's connection identifier. Each messenger has a unique
 * identifier used when communicating across a network.
 */
void Messenger::SetConnectID(int id)
	{ m_ConnectID = id; }

/*!
 * @fn bool Messenger::Observe(const SharedObj* target, int code, const SharedObj* sender)
 * @param target	Observer object, target for events (cannot be NULL).
 * @param code		Event code to observe, if 0 all events for the sender are observed.
 * @param sender>	Object sending event, NULL observes all objects generating the event.
 *
 * Observe the event(s) generated by a particular object or	all objects.
 * When an event is generated by an object and logged to this messenger,
 * it will be dispatched to all objects set up to observe it.
 *
 * @return \b true if an observation was attached, else \b false
 *
 * @see Messenger::Ignore Event SharedObj::OnEvent
 */
bool Messenger::Observe(const SharedObj* target, int code, const SharedObj* sender)
{
	if (target == NULL)
		return false;
	Observer*	head = NULL;
	ObjectLock	lock((SharedObj*) m_Observers);

	if ((code < (int) m_Observers->GetSize()) &&
		(head = (Observer*) m_Observers->GetAt(code)))
	{
		Observer* cur = head;
		do									// scan for duplicates
		{
			if (cur->Target != target)		// same target
				continue;
			if (cur->Code && (cur->Code != code))
				continue;					// same code
			if ((cur->Sender != (const SharedObj*) NULL) && (cur->Sender != sender))
				continue;					// same sender
			return false;					// don't add
		}
		while (cur = (Observer*) cur->Next);
	}
	Observer*	obs = new Observer(target, code, sender);
	obs->Next = head;						// add observer to list
	m_Observers->SetAt(code, (void*) obs);
	VX_TRACE(Messenger::Debug, ("Messenger::Observe %s %d\n", Event::GetName(code), code));
	return true;
};

/*!
 * @fn bool Messenger::Observe(const SharedObj* target, int code, const SharedObj* sender)
 * @param target	Observer object, target for events (cannot be NULL).
 * @param code		Event code to ignore, if 0 all events for the sender are ignored.
 * @param sender>	Object sending event, NULL ignores all objects generating the event.
 *
 * Removes previously established observation for an event.
 * If all input arguments are NULL, all observations are removed.
 *
 * @return \b true if an observation was removed, else \b false
 *
 * @see Messenger::Observe Event SharedObj::OnEvent
 */
bool Messenger::Ignore(const SharedObj* target, int code, const SharedObj* sender)
{
	Observer*	obs;
	Observer*	next;
	int			rc = -1;
	ObjectLock	lock((SharedObj*) m_Observers);

	if (m_Observers.IsNull())
		return false;
	if (code && (code < (int) m_Observers->GetSize()))
	{
		if (obs = (Observer*) m_Observers->GetAt(code))
		{
			next = (Observer*) obs->Next;
			rc = obs->Remove(target, sender);
			if (rc >= 0) { VX_TRACE(Messenger::Debug, ("Messenger::Ignore %s %d\n", Event::GetName(code), code)); }
			if (rc > 0)
				m_Observers->SetAt(code, next);
		}
		return rc >= 0;
	}
	for (int i = 0; i < (int) m_Observers->GetSize(); ++i)
	{
		if (obs = (Observer*) m_Observers->GetAt(i))
		{
			do
			{
				next = (Observer*) obs->Next;
				rc = obs->Remove(target, sender);
				if (rc >= 0) { VX_TRACE(Messenger::Debug, ("Messenger::Ignore %s %d\n", Event::GetName(i), i)); }
			}
			while (obs = next);
			m_Observers->SetAt(i, NULL);
		}
	}
	return rc >= 0;
}

/****
 *
 * Name: Stream_DoCommand
 *
 * Description:
 *	Dispatch stream command
 *
 ****/
int Messenger::DoCommand(int command)
{
	int32	n;
	MessageType	c = MessageType(command);

	switch (c)
	{
		case VIXEN_Version:		// set protocol version
		*this >> n;
		Version = n;
		break;

		case VIXEN_VecSize:		// set protocol version
		*this >> n;
		FileVecSize = n;
		break;

		case VIXEN_DoNothing:	// skip this one
		break;

		case VIXEN_Begin:
		*this >> n;				// read stream ID
		VX_TRACE(Messenger::Debug > 1, ("VIXEN__Begin: %d\n", n));
		break;

		case VIXEN_End:
		VX_TRACE(Messenger::Debug > 1, ("VIXEN__End\n"));
		return -1;

		case VIXEN_Exit:
		*this >> n;				// read stream ID
		VX_TRACE(Debug > 1, ("VIXEN__Exit: %d\n", n));
		if (n == 0)
			World::Get()->Stop();
		return -1;

		case VIXEN_Event:		// event logged
		DoEvent();				// dispatch to observer
		break;

		default:				// not a stream command
		*this >> n;				// read next handle
		if (n == 0)
			{ VX_ERROR(("Messenger::Load NULL handle\n"), -1); }
		return n;
	}
	return 0;					// stream command, continue
}

/****
 *
 * Name: Stream_DoEvent
 *
 * Description:
 *	Dispatch event from a stream to the appropriate observer
 *
 ****/
void Messenger::DoEvent()
{
	int32		code;

	*this >> code;
	VX_ASSERT(code <= Event::MAX_CODE);
	Event*	e = World::Get()->MakeEvent(code);
	if (e == NULL)
	   { VX_ERROR_RETURN(("Messenger::DoEvent(%d) event cannot be created\n", code)); }
	e->Parse(*this);
#ifdef _DEBUG
	VX_TRACE((Event::Debug || (Messenger::Debug > 1)),
			 ("VX_Event %s Code = %d, Time = %f\n", Event::GetName(e->Code), e->Code, e->Time));
#endif
	if (code < (int) m_Observers->GetSize())
	{
		Observer*	obs = (Observer*) m_Observers->GetAt(code);
		if (obs)
			obs->Dispatch(e);
	}
	delete e;
}


/*
 *	Messenger& << int32
 *	Messenger& << int64
 *	Messenger& << int16
 *	Messenger& << float
 *	Messenger& << char*
 *
 * Writes a value of the given type to the output stream.
 * Numeric datatypes are copied directly. Strings include
 * a length. Object pointers are converted to handles
 * (an object's handle is its object identifier)
 *
 */ 
Messenger& Messenger::operator<<(int32 i)
{
	bool rc = Output(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator<<(int64 i)
{
	bool rc = Output(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator<<(int16 i)
{
	bool rc = Output(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator<<(float f)
{
	bool rc = Output(&f, 1);
	VX_ASSERT(rc);
	return *this;
}

/*!
 * @fn Messenger& Messenger::operator<<(const TCHAR* str)
 * @param str	null-terminated string to output
 *
 * Strings are output as a 32 bit integer size followed by
 * the string data padded to a 4 byte boundary.
 * The NULL string is output as zero (32 bit).
 * Strings may have a maximum length of VX_MaxString characters.
 * UNICODE output of strings is currently not supported.
 * All UNICODE strings are converted to multi-byte before output.
 *
 * @see Core::Stream::Write
 */
Messenger& Messenger::operator<<(const TCHAR* cp)
{
	int32	p, zero = 0, n = 0;
 
	if ((cp == NULL) || !(n = (int32) STRLEN(cp))) // NULL string
		Output(&n, 1);		// just output zero length
	else
	{
#if defined(_WIN32) && defined(_UNICODE)
		char* buf = (char*) _alloca(n * 2 + 1);
		String::WideToMultiByte(cp, buf, n * 2 + 1);
		size_t m = strlen(buf);
#else
		size_t m = n;
		const char* buf = cp;
#endif
		VX_ASSERT(m < VX_MaxString);
		p = (m + 3) & ~3;			// round up to next multiple of 4 bytes
		Output(&p, 1);				// include pad in length
		Write(buf, (int32) m);		// write string
		p -= (int32) m;				// calculate pad bytes needed
		if (p)
			Write((char*) &zero, p);	
	}
	return *this;
}

/*!
 * @fn Messenger& Messenger::operator<<(const Event& event)
 * @param event Event to log
 *
 * Logs the input event to the current messenger. The contents of the event
 * are saved and dispatched to any observers at the start of the
 * next frame. All events are sent to remote processors whether or
 * not they are observed there.
 *
 * @see Messenger::Observe Messenger::Ignore Event SharedObj::OnEvent
 */
Messenger& Messenger::operator<<(const Event& ev)
{
	VX_TRACE((Event::Debug || (Messenger::Debug > 1)),
			 ("Messenger::LogEvent %s Code = %d, Time = %f\n", Event::GetName(ev.Code), ev.Code, ev.Time));
	SharedObj* sender = ev.Sender;
	BeginOp(MESS_EventLog);
	ev.Save(*this);
	EndOp();
	return *this;
}

Messenger& Messenger::operator<<(const SharedObj* obj)
{
	return OutObj(obj);
}

Messenger& Messenger::operator<<(const VXOpcode& op)
{
	return OutOp(op);
}

Messenger& Messenger::OutOp(const VXOpcode& op)
{
	return *this << int32(uint32(uint16(op.ClassID) << 16) | op.Opcode);
}

Messenger& Messenger::OutObj(const SharedObj* obj)
{
	int32	handle = 0;
	if (obj)
	{
		handle = obj->GetID();
		if (handle == 0)
			handle = Attach(obj);
	}
	return *this << handle;
}

/*
 *	Messenger& >> int32&
 *	Messenger& >> int64&
 *	Messenger& >> int16&
 *	Messenger& >> float&
 *	Messenger& >> char*
 *
 * Reads a value of the given type from the input stream.
 * Object pointers are read as handles and mapped to pointers
 * by the stream. Strings assume the length comes before
 * the character data.
 *
 */ 
Messenger& Messenger::operator>>(int32& i)
{
	bool rc = Input(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator>>(int64& i)
{
	bool rc = Input(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator>>(int16& i)
{
	bool rc = Input(&i, 1);
	VX_ASSERT(rc);
	return *this;
}

Messenger& Messenger::operator>>(float& f)
{
	bool rc = Input(&f, 1);
	VX_ASSERT(rc);
	return *this;
}

/*!
 * @fn Messenger& Messenger::operator>>(TCHAR* str)
 * @param str	buffer to read string into
 *
 * Strings are output as a 32 bit integer size followed by
 * the string data padded to a 4 byte boundary.
 * The NULL string is output as zero (32 bit).
 * Strings may have a maximum length of 1024 characters
 * so the input buffer should be able to accomodate this size.
 *
 * @see Core::Stream::Write Messenger::operator<<(const TCHAR*)
 */
Messenger& Messenger::operator>>(TCHAR* cp)
{
	int32	n;

	*this >> n;
	VX_ASSERT(n < VX_MaxString);
#if defined(_WIN32) && defined(_UNICODE)
	char* buf =(char*)  _alloca(n + 1);
#else
	char* buf = cp;
#endif
	Read(buf, n);
	buf[n] = 0;

	return *this;
}

Messenger& Messenger::operator>>(SharedObj*& obj)
{
	return InObj(obj);
}

Messenger& Messenger::InObj(SharedObj*& obj)
{
	int32	handle;

	*this >> handle;
	if (handle > 0)
		obj = m_Objs->GetObj(handle);
	else
		obj = (SharedObj*) NULL;
	return *this;
}

/****
 *
 * Determine if there is input for us on the input stream.
 *
 ****/
bool	Messenger::IsEmpty() const
{
	return !m_InStream.IsNull() && m_InStream->IsEmpty();
}

/****
 *
 * Read from input stream.
 *
 ****/
size_t	Messenger::Read(char* buffer, int n)
{
	if (m_InStream.IsNull())
		return 0;
	return m_InStream->Read(buffer, n);
}

/****
 *
 * Write to output stream.
 *
 ****/
size_t	Messenger::Write(const char* buf, int n)
{
	if (m_OutStream.IsNull())
		return 0;
	return m_OutStream->Write(buf, n);
}

/*!
 * @fn bool Messenger::Output(const int32* buffer, int nwords)
 * @param buffer	buffer with data to output
 * @param nwords	number of integers to write
 *
 * The given number of 32 bit integers are copied from the
 * input buffer to this stream. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on output.
 *
 * @return \b true if all words were written, else \b false
 */
bool Messenger::Output(const int32* ip, int n)
{
	n *= sizeof(int32);
	return Write((const char*) ip, n) == n;
}

/*!
 * @fn bool Messenger::Output(const int64* buffer, int nwords)
 * @param buffer	buffer with data to output
 * @param nwords	number of integers to write
 *
 * The given number of 64 bit integers are copied from the
 * input buffer to this stream. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on output.
 *
 * @return \b true if all words were written, else \b false
 */
bool Messenger::Output(const int64* ip, int n)
{
	n *= sizeof(int64);
	return Write((const char*) ip, n) == n;
}
/*!
 * @fn bool Messenger::Output(const int16* buffer, int nwords)
 * @param buffer	buffer with data to output
 * @param nwords	number of integers to write
 *
 * The given number of 16 bit integers are copied from the
 * input buffer to this stream. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on output.
 *
 * @return \b true if all words were written, else \b false
 */
bool Messenger::Output(const int16* ip, int n)
{
	n *= sizeof(int16);
	return Write((const char*) ip, n) == n;
}

/*!
 * @fn bool Messenger::Output(const float* buffer, int nwords)
 * @param buffer	buffer with data to output
 * @param nwords	number of integers to write
 *
 * The given number of 32 bit single-precision floats are copied from the
 * input buffer to this stream. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on output.
 *
 * @return \b true if all words were written, else \b false
 */
bool Messenger::Output(const float* fp, int n)
{
	n *= sizeof(float);
	return Write((const char*) fp, n) == n;
}


/*!
 * @fn bool Messenger::Input(int32* buffer, int nwords)
 * @param buffer	input buffer
 * @param nwords	number of integers to read
 *
 * The given number of 32 bit integers are read from the stream
 * into the input buffer. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on input.
 *
 * @return \b true if all words were read, else \b false
 */
bool Messenger::Input(int32* ip, int n)
{
	n *= sizeof(int32);
	return Read((char*) ip, n) == n;
}

/*!
 * @fn bool Messenger::Input(int64* buffer, int nwords)
 * @param buffer	input buffer
 * @param nwords	number of integers to read
 *
 * The given number of 64 bit integers are read from the stream
 * into the input buffer. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on input.
 *
 * @return \b true if all words were read, else \b false
 */
bool Messenger::Input(int64* ip, int n)
{
	n *= sizeof(int64);
	return Read((char*) ip, n) == n;
}

/*!
 * @fn bool Messenger::Input(int16* buffer, int nwords)
 * @param buffer	input buffer
 * @param nwords	number of integers to read
 *
 * The given number of 16 bit integers are read from the stream
 * into the input buffer. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on input.
 *
 * @return \b true if all words were read, else \b false
 */
bool Messenger::Input(int16* ip, int n)
{
	n *= sizeof(int16);
	return Read((char*) ip, n) == n;
}

/*!
 * @fn bool Messenger::Input(float* buffer, int nwords)
 * @param buffer	input buffer
 * @param nwords	number of floats to read
 *
 * The given number of 32 bit floats are read from the stream
 * into the input buffer. If byte-swapping or reordering is
 * necessary, this routine may be overridden to perform this
 * behavior on input.
 *
 * @return \b true if all words were read, else \b false
 */
bool Messenger::Input(float* fp, int n)
{
	n *= sizeof(float);
	return Read((char*) fp, n) == n;
}

/****
 *
 * Flush output stream.
 *
 ****/
void	Messenger::Flush()
{
	if (!m_OutStream.IsNull())
		m_OutStream->Flush();
}

/*
 * Open named resource for input or output
 */
bool	Messenger::Open(const TCHAR* name, int mode)
{
	m_OpenMode = mode & Stream::OPEN_RW;
	switch (m_OpenMode)
	{
		case Stream::OPEN_READ:
		if (!m_OutStream.IsNull())
			return m_InStream->Open(name, mode);
		break;

		case Stream::OPEN_RW:
		if (m_OutStream != m_InStream)
			if (!m_InStream.IsNull())
			{
				m_InStream->Open(name, Stream::OPEN_READ);
				mode &= ~Stream::OPEN_READ;
			}
		// fall thru to write

		case Stream::OPEN_WRITE:
		if (!m_OutStream.IsNull())
			return m_OutStream->Open(name, mode);
	}
	m_OpenMode = 0;
	return false;
}

/*
 * Close both input and output streams
 */
bool	Messenger::Close()
{
	bool rc = true;

	if (!m_InStream.IsNull() && !m_InStream->Close())
		rc = false;
	if (!m_OutStream.IsNull() && !m_OutStream->Close())
		rc = false;
	m_Objs->Empty();
	m_OpenMode = 0;
	return rc;
}


/*
 * Print the dictionary of names and the objects they reference
 */
DebugOut& Messenger::Print(DebugOut& dbg, int opts) const
{
	const SharedObj*	obj;
	const TCHAR*		name = GetName();
	const ObjRef*		ref;
	DictIter<NameProp, ObjRef> iter(m_Names);

	if (opts == 0) opts = PRINT_Default;

	if (opts & PRINT_Attributes)
	{
		SharedObj::Print(dbg, opts & (PRINT_Header | PRINT_Attributes));
		while (ref = iter.Next())
		{
			obj = *ref;
			obj->Print(dbg, PRINT_Summary);
		}
		SharedObj::Print(dbg, opts & PRINT_Trailer);
	}
	else SharedObj::Print(dbg, opts);
	return dbg;
}

/****
 *
 * PROP_Observe is a property used internally to maintain observation
 * relationships for a stream. Each time an event is observed,
 * an Observer property is linked into that event's list of observers.
 * When an event occurs, its observer list is scanned and the
 * OnEvent method is called for the observers of that event
 *
 ****/
int Messenger::Observer::Remove(const SharedObj* target, const SharedObj* sender)
{
	Observer*	obs = NULL;
	Observer*	cur = this;
	int			rc = -1;

	do
	{
		if ((cur->Sender == sender) && sender)
			continue;
		if ((cur->Target != target) && target)
			continue;
		if (obs)
		{
			rc = 0;
			obs->Next = (Observer*) cur->Next;
		}
		else
			rc = 1;
		delete cur;
		return rc;
	}
	while ((obs == cur) && (cur = (Observer*) cur->Next));
	return -1;
}
	
void Messenger::Observer::Dispatch(Event* e)		// dispatch event to proper observer
{
	Observer*	obs = this;
	Observer*	next;

	do													// for all observers linked to us
	{
		next = (Observer*) obs->Next;
		if (((SharedObj*) obs->Sender == e->Sender) ||	// observing the sender?
			(obs->Sender == NULL))						// observing anybody sending the event?
			if (obs->Target != (const SharedObj*) NULL)
				obs->Target->OnEvent(e);
	}
	while (obs = next);
};

Messenger::ObjMap::~ObjMap()
{
	Empty();
}

SharedObj* Messenger::ObjMap::GetObj(int handle) const
{
	ObjectLock	lock(this);
	if (handle <= 0)
		return NULL;
	if (handle >= (int) GetSize())
		return NULL;
	return (SharedObj*) GetAt(handle);
}

/*!
 * @fn int Messenger::ObjMap::AttachObj(const SharedObj* obj, int handle)
 * @param handle	Handle of object to delete.
 *
 * Attaches the given object to the messenger and returns its handle.
 * If an object is attached to an output messenger, updates to it
 * are logged by the messenger. Objects attached to a messenger
 * have their state affected by transactions on that messenger.
 * An object must be marked as <global> in order to have transactions logged.
 * This is done by setting the GLOBAL flag.
 *
 * @return 0 if object was already attached, else handle of object attached
 *
 * @see Messenger::ObjMap::AttachObj Messenger::ObjMap::DeleteeObj
 */
int Messenger::ObjMap::AttachObj(const SharedObj* obj, int handle)
{
	ObjectLock	lock(this);
	if (handle && (handle < (int) GetSize()) &&
		(GetAt(handle) == obj))				// object already attached?
		return handle;
	int newhandle = NewHandle(handle);	// use object's handle
	if (newhandle <= 0)
		return newhandle;
	SetAt(newhandle, obj);
	if ((handle > 0) && (handle != newhandle))
	   { VX_WARNING(("Messenger::Attach requested handle %d mapped to %d\n", handle, newhandle)); }
	return newhandle;
}

/*!
 * @fn int Messenger::ObjMap::DeleteObj(int handle)
 * @param handle	Handle of object to delete.
 *
 * Dereferences the object with the given handle and detaches
 * it from the stream. The object itself is not deleted.
 *
 * @see Messenger::ObjMap::AttachObj
 */
bool Messenger::ObjMap::DeleteObj(int handle)
{
	if ((handle <= 0) ||				// out of range?
		(handle >= (int) GetSize()))
		return false;
	ObjectLock	lock(this);
	SharedObj*		obj = (SharedObj*) GetAt(handle);
	if (obj)
	{
		obj->ClearFlags(SAVED);		// mark as not saved
		SetAt(handle, (SharedObj*) NULL);
	}
	if ((m_freeHandle <= 0) || (m_freeHandle > handle))
		m_freeHandle = handle;			// make it the free one
	if ((MaxHandle == handle) &&		// deleted last one?
		(MaxHandle > 0))
		--MaxHandle;					// decrement handle table size
	return true;
}


/*!
 * @fn int Messenger::ObjMap::NewHandle(int newhandle)
 * @param newhandle	Preferred handle to use, -1 asks for new handle.
 *
 * Grabs the next free handle. If there are spaces
 * in the handle table, they will be used. If the
 * handle table becomes full, it is automatically extended.
 */
int Messenger::ObjMap::NewHandle(int newhandle)
{
	int n = (int) GetSize();
	if (newhandle <= 0)
	{
		if ((m_freeHandle > 0) && (m_freeHandle < n))
		{
			newhandle = m_freeHandle;
			while (++m_freeHandle < n)
				if (GetAt(m_freeHandle) == NULL)
					return newhandle;	// found another free one?
			m_freeHandle = 0;
		}
		else
		{
			newhandle = ++MaxHandle;
			m_freeHandle = 0;
		}
	}
	if (newhandle > MaxHandle)
		MaxHandle = newhandle;
	VX_ASSERT(newhandle > 0);	   
	return newhandle;
}


/*!
 * @fn int Messenger::ObjMap::GetHandle(const SharedObj* o) const
 * @param obj	Object whose handle we want.
 *
 * @return the handle that references the given object'
 *	0 if the object is not attached to this messenger. 
 */
int Messenger::ObjMap::GetHandle(const SharedObj* o) const
{
	if (o == NULL)
		return 0;
	return (int) Find(o);
}

bool Messenger::ObjMap::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	ObjMap*	src = (ObjMap*) srcobj;
	if (src)
	{
		if (!ObjArray::Copy(srcobj))
			return false;
		MaxHandle = src->MaxHandle;
		m_freeHandle = src->m_freeHandle;
	}
	return true;
}

void Messenger::ObjMap::Empty()
{
	ObjectLock	lock(this);
	if (GetSize() > 0)
	{
		for (int i = 0; i < (int) GetSize(); ++i)
			DeleteObj(i);
		ObjArray::Empty();
		SetAt(0, (SharedObj*) NULL);
	}
	MaxHandle = 1;
	m_freeHandle = 0;
}

}	// end Vixen