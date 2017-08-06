#include "vixen.h"

namespace Vixen
{
using namespace Core;

intptr		vixen_cls;

static const TCHAR* donames[] = 
{
	TEXT("Create"), TEXT("SetName"), TEXT("Copy"), TEXT("Delete"), TEXT("Print"),
	TEXT("Connect"), TEXT("RemapID"), TEXT("SetNameKey")
};

const TCHAR** SharedObj::DoNames = donames;

VX_IMPLEMENT_CLASSID(SharedObj, RefObj, VX_Obj);
VX_IMPLEMENT_CLASS(Property, BaseObj);
VX_IMPLEMENT_CLASS(NameProp, Property);

Property::~Property() { if (Owner) ((SharedObj*) Owner)->RemoveProp(this); }

/*!
 * @fn Property* SharedObj::GetProp(uint32 tag) const
 * @param tag	16-bit tag for the property we want
 *
 * Returns the first property with the given tag
 * that is attached to this object. Properties are
 * known only to the local machine and cannot be
 * distributed or saved in files.
 *
 * Like object flags, properties may be used to communicate
 * information between two cooperating threads. Property
 * list access is thread-safe.
 *
 * @returns property found or NULL if no such property
 *
 * @see SharedObj::AddProp SharedObj::GetName SharedObj::DelProp SharedObj::RemoveProp
 */
Property* SharedObj::GetProp(intptr tag) const
{
	ObjectLock lock(this);
	Property* p = m_Prop;
 
	while (p)
		if (p->IsA(tag))
			break;
		else p = (Property*) p->Next;
	return p;
}

/*!
 * @fn Property* SharedObj::GetProp(intptr tag, uint32 key) const
 * @param tag	tag for the property we want
 * @param key	key for the property we want
 *
 * Returns the first property with the given tag
 * that is attached to this object. Properties are
 * known only to the local machine and cannot be
 * distributed or saved in files.
 *
 * Like object flags, properties may be used to communicate
 * information between two cooperating threads. Property
 * list access is thread-safe.
 *
 * @returns property found or NULL if no such property
 *
 * @see SharedObj::AddProp SharedObj::GetName SharedObj::DelProp
 */
Property* SharedObj::GetProp(intptr tag, uint32 key) const
{
	ObjectLock lock(this);
	Property* p = m_Prop;
 
	while (p)
		if (p->IsA(tag) && (key == p->Key))
			break;
		else p = (Property*) p->Next;
	return p;
}

/*!
 * @fn void SharedObj::DelProp(intptr tag, uint32 key)
 * @param tag	tag for the property we want
 * @param key	key for the property we want
 *
 * Removes the property with the given tag and key which belongs
 * to this object. It is garbage collected (using \b delete) if
 * Property::NoFree is \b not set. Otherwise, it is assumed this
 * property is memory managed by another class and this object
 * will leave it alone.
 *
 * @see SharedObj::GetProp SharedObj::AddProp SharedObj::RemoveProp
 */
void SharedObj::DelProp(intptr tag, uint32 key)
{
	Property* p = RemoveProp(tag, key);
	if (p && !p->NoFree)			/* deletable by us? */
		delete p;					/* delete this one */
}

/*!
 * @fn Property* SharedObj::RemoveProp(Property* prop)
 * @param prop	property to detach
 *
 * Removes this property from the object's property list.
 * It is not garbage collected, just detached.
 *
 * @see SharedObj::GetProp SharedObj::AddProp SharedObj::DelProp
 */
Property* SharedObj::RemoveProp(Property* prop)
{
	ObjectLock lock(this);
	Property*	head = m_Prop;

	prop->Owner = NULL;
	if (head == NULL)
		return NULL;
	if (head == prop)
	{
		m_Prop = (Property*) prop->Next;
		return prop;
	}
	return (Property*) head->Remove(prop);
}

/*!
 * @fn void SharedObj::RemoveProp(intptr tag, uint32 key)
 * @param tag	tag for the property we want
 * @param key	key for the property we want
 *
 * Removes the property with the given tag and key which belongs
 * to this object. It is not garbage collected, just detached.
 *
 * @see SharedObj::GetProp SharedObj::AddProp SharedObj::DelProp
 */
Property* SharedObj::RemoveProp(intptr tag, uint32 key)
{
	Property head;
	Property* pred = &head;
	Property* p;
	ObjectLock lock(this);

	head.Next = (Property*) m_Prop;
	while (p = (Property*) pred->Next)		/* find one with our tag */
	{
		if (p->IsA(tag) &&(p->Key == key))	/* found the one to remove? */
		{
			pred->Next = p->Next;
			p->Next = NULL;
			p->Owner = NULL;
			break;
		}
		pred = p;							/* save this as predecessor */
	}
	m_Prop =(Property*) head.Next;
	head.Next = NULL;
	return p;
}

/*!
 * @fn void SharedObj::AddProp(Property* prop)
 * @param prop	Property to add
 *
 * Only one property with a particular tag and key may be added.
 * If the object already has a property
 * with the same tag and key as the input property, the original
 * property is replaced by the new one (and the old one is freed).
 *
 * Adding properties to an object is a thread-safe operation.
 *
 * @see SharedObj::DelProp SharedObj::GetProp SharedObj::RemoveProp
 */
void SharedObj::AddProp(Property* prop)
{
	Property head;
	Property* pred = &head;
	Property* p;

	Lock();
	head.Next = m_Prop;
	prop->Next = NULL;
	while (p = (Property*) pred->Next)	/* check the rest for our tag */
	{
		if (p == prop)
			return;
		if (p->IsA(prop) && (p->Key == prop->Key))	/* found one to replace? */
		{
			prop->Next = p->Next;	/* link into list */
			p->Next = NULL;
			break;
		   }
		pred = p;					/* save this as predecessor */
	}
	pred->Next = prop;				/* link into list */
	prop->Owner = this;				/* mark as owned by this object */
	m_Prop = (Property*) head.Next;
	Unlock();						/* unlock around delete */
	if (p)
	{
		p->Owner = NULL;			/* detach from this object */
		if (!p->NoFree)				/* deletable by us? */
			delete p;				/* delete old one */
	}
	head.Next = NULL;
	return;
}

/*!
 * @fn SharedObj* SharedObj::Create(uint32 id)
 * @param id	class ID of class to instantiate
 *
 * Class factory for Vixen objects. Messengers use this constructor to create
 * objects serialized from files or remote clients.
 *
 * @see VXClassID BaseObj::Delete Messenger::Create
 */
SharedObj* SharedObj::Create(uint32 id)
{
	Class*	vcls = Class::GetClass(id);
	if (vcls == NULL)
		return NULL;
	SharedObj* obj = (SharedObj*) vcls->CreateObject();
	return obj;
}

/*!
 * @fn SharedObj::SharedObj()
 *
 * Initially, an object's reference count is zero indicating it has
 * no outstanding references. A call to BaseObj::Delete will free the
 * object in this state. If an object is used once, its reference count
 * is set to one. SharedObj::Delete will also free such an object. If an
 * object is used more than once, calling delete will decrement the
 * reference count but not free the object.
 *
 * @see BaseObj::Delete SharedObj::Create Messenger::Create
 */
SharedObj::SharedObj() : LockObj(),
	m_Prop(NULL),
	m_ID(0)
{
	m_Flags = CHANGED;
	VX_TRACE2(Debug, ("%s::Create @ %p\n", ClassName(), this)); 
}

SharedObj::SharedObj(const SharedObj& src):	LockObj(),
	m_Prop(NULL),
	m_ID(0)
{
	m_Flags = CHANGED;
	VX_TRACE2(Debug, ("%s::Create @ %p %s\n", ClassName(), this, src.GetName())); 
}

SharedObj::~SharedObj()
{
	VX_TRACE2(Debug, ("%s::Delete @ %p %s\n", ClassName(), this, GetName()));
}

/*!
 * @fn bool SharedObj::Copy(const SharedObj* src)
 *
 * Copies the contents of one SharedObj into another.
 * All subclasses of Vixen objects should override
 * this function to copy their own data and call
 * the base implementation. The base implementation
 * should always be called if overridden.
 *
 * @returns \b true if successful copy, else \b false
 *
 * @see SharedObj::Clone SharedObj::operator=
 */
bool SharedObj::Copy(const SharedObj* src)
{
	VX_STREAM_BEGIN(s);
		*s << OP(ClassID(), OBJ_Copy) << this << src;
	VX_STREAM_END( );

	SetFlags(CHANGED | (src->m_Flags & ~(NOFREE | GLOBAL | SHARED | SAVED)));
	const TCHAR* name = src->GetName();
	if (name != GetName())
		SetName(name);
	return true;
}

/*!
 * @fn SharedObj* SharedObj::Clone() const
 *
 * Usually, clones share resources rather than being completely distinct copies.
 * For example, a cloned object array shares elements with the array it came from.
 *
 * @see SharedObj::Copy SharedObj::SharedObj
 */
SharedObj* SharedObj::Clone() const
{
	SharedObj* dst = Create(ClassID());

    if (!dst)							/* can't create new object? */
		VX_ERROR(("SharedObj.Clone: out of memory\n"), NULL);
	Lock();
    if (!dst->Copy(this))				/* can't copy into it? */
	{
		Unlock();
		dst->Delete();
		return NULL;
	}
	Unlock();
	VX_TRACE2(Debug, ("%s::Clone @ %p\n", ClassName(), this));
    return dst;
}

/*!
 * @fn bool SharedObj::Do(Messenger& s, int op)
 *
 * Performs the designated operation on this object,
 * getting necessary arguments from the messenger.
 * This function is used for processing updates from
 * remote clients or for loading objects transmitted in
 * Vixen binary protocol.
 *
 * Each subclass of SharedObj can have its own set of
 * opcodes. Typically, these will match the C++ API
 * for that object. Derived classes that add functionality
 * they want to expose in a distributed way should
 * override this function.
 *
 * The basic opcodes all objects have are:
 * - OBJ_Create	create empty object
 * - OBJ_Delete	delete object
 * - OBJ_Print	print as ASCII
 * - OBJ_Copy	copy object
 *
 * Object creation is not handled here but by the messenger.
 *
 * @par Example:
 * @code
 * // Sample Do function for ModelSwitch with one opcode SWITCH_SetIndex
 *    bool ModelSwitch::Do(Messenger& s, int op)
 *    { int32 v;
 *      switch (op)
 *      {
 *         case SWITCH_SetIndex:
 *         s >> v;
 *         SetIndex(v);
 *         break;
 *
 *         default:
 *         return Model::Do(s, op);
 *      }
 *		return true;
 *  }
 * @endcode
 *
 * @see Messenger Messenger::Load SharedObj::Save
 * @see <A HREF=../oview/binprot.htm>Vixen Binary Protocol</A>
 */
bool SharedObj::Do(Messenger& s, int op)
{
	SharedObj*	obj;
	TCHAR		name[VX_MaxString];
	int32		key, flag;
	Opcode		o = Opcode(op);		// for debugging

	switch (op)
	{
		case OBJ_SetName:
		s >> name;
		VX_ASSERT(STRLEN(name) <= VX_MaxName);
		s.Define(String(name), this);
		break;

		case OBJ_SetNameKey:
		s >> key >> name;
		SetName(String(name), key);
		break;

		case OBJ_SetActive:
		s >> flag;
		if (flag)
			SetActive(true);
		else SetActive(false);
		break;

		case OBJ_SetFlags:
		s >> flag;
		SetFlags(flag);
		break;

		case OBJ_Delete:
		s >> obj;
		Delete();
		break;

		case OBJ_Print:
		Print();
		break;

		case OBJ_Copy:
		s >> obj;
		Copy(obj);
		break;

		default:
		return false;
	}
#ifdef _DEBUG
	if (s.Debug)
	{
		const TCHAR* oname = GetName();
		if (oname == NULL)
			oname = TEXT(" ");
		endl(vixen_debug << ClassName() << "::"
			<< SharedObj::DoNames[op - OBJ_Create] << " " << oname << " " << this);
	}
#endif
	return true;
}

/*!
 * @fn int SharedObj::Save(Messenger& stream, int opts) const
 *	@param stream	archive to use for serializing
 *	@param opts		serialization options
 *
 *	Saves the entire state of the object in Vixen binary protocol
 *	Subclasses should override this function and save
 *	any additional data members of the derived class. Implementing
 *	this function allows your objects to be saved in Vixen binary files.
 *
 *	You should always call the Save function of your parent class
 *	and avoid saving if the handle returned is zero. This handle 
 *	is a messenger ID which uniquely identifies the object
 *	to the messenger. A zero return value indicates
 *	the object has already been saved to the stream. An object's
 *	save status (the SAVED flag) is reset to zero when the
 *	stream is closed.
 *
 * @par Example:
 * @code
 *  // Sample Save function for ModelSwitch with single opcode SWITCH_SetIndex
 *	   int ModelSwitch::Save(Messenger& s, int opts)
 *     {
 *		   int = Model::Save(s, opts);
 *         if (h > 0)
 *			   s << OP(VX_Switcher, SWITCH_SetIndex) << h << GetIndex();
 *         return h;
 *      }
 * @endcode
 *
 * @return stream handle of the object saved, <=0 if not saved
 *
 * @see ModelSwitch Core::Stream
 */
int SharedObj::Save(Messenger& s, int opts) const
{
	int32 h = s.CanSave(this, opts);		// attach object to stream
	if (h <= 0)								// already attached and created?
		return h;							// no save needed
	int32 cid = ClassID();
	VX_ASSERT(cid != 0);
	VX_TRACE(SharedObj::Debug || s.Debug, ("%s::Save %d %s\n", ClassName(), h, GetName()));
//
// Scan all the name properties and save them
// Key 0 is saved with a different opcode because it is the name
// used to identify the object in a distributed scene
//
	NameProp* np = (NameProp*) GetProp(PROP_NameTag);
	ObjectLock lock(this);
	while (np)
	{
		if (np->IsA(PROP_NameTag))
			if (np->Key == 0)
				s << OP(cid, OBJ_SetName) << h << (const TCHAR*) *np;
			else			
				s << OP(cid, OBJ_SetNameKey) << h << int32(np->Key) << (const TCHAR*) *np;
		np = (NameProp*) np->Next;
	}
	int32 flags = GetFlags() & (INACTIVE | SharedObj::DOEVENTS);
	if (IsShared(ClassID()) & INACTIVE)
		flags |= INACTIVE;
	if (flags)
		s << OP(VX_Obj, OBJ_SetFlags) << h << flags;
	return h;
}

/*!
 * @fn DebugOut& SharedObj::Print(DebugOut& dbg, int opts) const
 *
 * This function is useful during debugging to dump the
 * contents of an object in a readable form.
 * If running under the debugger these printouts will go to
 * the debugger output window. Otherwise, they are lost.
 *
 * Well-behaved Vixen extensions will override this function and produce
 * printed output for their additional data members.
 *
 * @see vdebug.h
 */
DebugOut& SharedObj::Print(DebugOut& dbg, int opts) const
{
	NameProp* np = (NameProp*) GetProp(PROP_NameTag);
	int	id = GetID();

	if (opts == 0)
		return dbg;
	if (opts & PRINT_Header)
	{
		dbg << "<" << ClassName();
		if (id != 0)
			dbg << " id='" << id << "'";
	}
	if (opts & (PRINT_Header | PRINT_Attributes))	// print the object namne
	{
		while (np)
		{
			if (np->IsA(PROP_NameTag))
				if (np->Key == 0)
				{
					dbg << " name='" << (const TCHAR*) *np << "'";
					break;
				}
			np = (NameProp*) np->Next;
		}
	}
	if ((opts & PRINT_Summary) == PRINT_Summary)
		endl(dbg << " />");
	else if (opts & PRINT_Trailer)
		endl(dbg << "</" << ClassName() << ">");
	else endl(dbg << ">");
	return dbg; 
}


/*!
 * @fn const TCHAR* SharedObj::GetName(int32 key) const
 * @param key	key for name property
 *
 * Object names are represented internally by name properties.
 * The \b key argument must match the \b key used when
 * setting the name or it will not be returned. The
 * default is to return the primary name, \b key zero.
 *
 * @returns string name of object or NULL if not named
 *
 * @see SharedObj::SetName Messenger::Define NameProp SharedObj::AddProp
 */
const TCHAR* SharedObj::GetName(int32 key) const
{
	NameProp* p = (NameProp*) GetProp(PROP_NameTag, key);
	if (p == NULL)
		return NULL;
	const TCHAR* name = *p;
	if ((name == NULL) || (*name == '\0'))
		return NULL;
	return name;
}

/*!
 * @fn bool SharedObj::SetName(const TCHAR* name, int32 key)
 * @param name	string name to give to object. If NULL,
 *				any name associated with the object is removed.
 * @param key	name property key for this name
 *
 * Attaches a string name to this object. If the
 * object already has a name, this name is replaced by
 * the new one and the call succeeds.
 *
 * Multiple strings may be attached to the object. They
 * can be distinguished because they have different <keys>.
 * The \b key is used both to access and update a string
 * name. The default \b key is zero and it is used to
 * indicate the primary object name. For objects exported
 * from modelers, the primary name is the name used by
 * the artist for that object.
 *
 * Object string names are maintained internally as a property
 * of class NameProp attached to the object via SharedObj::AddProp.
 * The property tag for names is PROP_NameTag. This function
 * always removes the existing name property. It creates a new
 * name property with the input string and key and attaches
 * this one to the object.
 *
 * @see SharedObj::AddProp NameProp Messenger::Define
 */
bool SharedObj::SetName(const TCHAR* name, int32 key)
{
	VX_STREAM_BEGIN(s);
		*s << OP(ClassID(), OBJ_SetName) << this << name;
	VX_STREAM_END(  );

	if (name == NULL)				// removing the name?
	{
		DelProp(PROP_NameTag, key);
		return true;
	}
	if (name)
	{
		NameProp* p = new NameProp(name);	// make a new name
		p->Key = uchar(key);
		AddProp(p);					// attach to the object
	}
	return true;
}

bool SharedObj::SetName(const TCHAR* name)
{ return SetName(name, 0);	}

/*!
 * @fn bool SharedObj::SetName(NameProp& prop)
 * @param prop	name property to attach to this object
 *
 * Attaches a name to this object. If the object already
 * has a name property, this one is removed and replaced by
 * the input property. The \b key used for the name properties
 * is zero so only the primary name tag is affected.
 *
 * Object string names are maintained internally as a property
 * of class NameProp attached to the object via SharedObj::AddProp.
 * The property tag for names is PROP_NameTag. 
 *
 * Any number of name tags may be attached to an object as long
 * as they have different keys. The Key for the primary name tag
 * used to identify the object is zero.
 *
 * This function is primarily used by classes who want to attach
 * their own name properties to the object because they are
 * controlling the memory management of the properties. One such
 * class is NameTable, a dictionary which provides quick lookup
 * of objects by name. When an object is associated with a dictionary,
 * its name property is actually part of the dictionary and is detached
 * when the dictionary no longer references the object.
 *
 * @see SharedObj::GetName SharedObj::AddProp NameProp NameTable Messenger::Define
 */
bool SharedObj::SetName(NameProp& np)
{
	if (np.Owner == NULL)			// nobody owns this property?
		AddProp(&np);				// add input property as new name
	else
	{
		NameProp* oldnp = (NameProp*) GetProp(PROP_NameTag, 0);
		if (oldnp)					// object has name property?
			*oldnp = (const TCHAR*) np;	// copy input property into it
		else
		{
			oldnp = new NameProp(np);
			AddProp(oldnp);			// add new name property to object
		}
	}
	return true;
}

/*!
 * @name int32 SharedObj::FindClass(const TCHAR* name)
 * @param name	name of class to find, not including the "VX" prefix
 *
 * Scans the list of existing Vixen classes for the application and returns
 * the class ID for the named class. If no such class exists, 0 is returned.
 *
 * @returns class ID or 0 if not found
 *
 * @see Class SharedObj::IsShared SharedObj::ShareClass BaseObj::ClassID BaseObj::ClassName
 */
int32 SharedObj::FindClass(const TCHAR* name)
{
	Class* vcls = Class::GetClass(name);
	if (vcls == NULL)
		return 0;
//	vcls = vcls->GetBaseClass();
//	if (vcls == NULL)
//		return 0;
	return vcls->GetSerialID();
}

/*!
 * @fn void SharedObj::ShareClass(uint32 classid, int32 v)
 * @param classid	class ID of class to set sharing options for
 * @param shareopts	Sharing options - a combination of:
 *	- 0 indicates the class shoul not be distributed at all.
 *	- SHARED specifies all objects of this class should be shared.
 *	- GLOBAL designates the class as global. 
 *	- INACTIVE disables objects of this class when received by clients.
 *
 * Distributed objects may be \b shared, \b global or not distributed.
 * Shared objects are transferred to remote clients if they request the entire scene.
 * Global objects are shared and all updates made to them after a
 * client connects are streamed to that clienht.
 *
 * @see SharedObj::FindClass Messenger::Distribute SharedObj::IsShared SharedObj::IsGlobal
 *
 */
void SharedObj::ShareClass(uint32 classid, int32 v)
{
	Class* vcls = Class::GetClass(classid);
	if (vcls == NULL)
		return;
// TODO: add this to Class
//	if (v)
//		vcls->IsGlobal = v;
//	else
//		vcls->IsGlobal = -1;
}

/*!
 * @fn int32 SharedObj::IsShared(uint32 class_id)
 * @param classid	class ID of class to check sharing options for
 *
 * If no sharing options were explicitly set for the class, it inherits
 * them from its closest ancestor which has been explicitly shared.
 * The sharing options are a combination of the following:
 * - 0 indicates the class shoul not be distributed at all.
 * - SHARED specifies all objects of this class should be shared.
 * - GLOBAL designates the class as global. 
 * - INACTIVE disables objects of this class when received by clients.
 *
 * A distributed object exists on multiple machines and is
 * kept synchronized by propagating updates across
 * the network using Vixen streams. An object which is only \b shared
 * will be transmitted to remote clients but subsequent updates
 * to the object are not propagated
 *
 * All updates to a \b global object are made immediately to the local
 * version of the object and are also logged by the messenger.
 * At the end of each frame, the transactions for all global objects
 * are sent to remote processors. Updates made to these objects remotely are
 * read by the scene's input stream and applied locally before the next frame begins.
 *
 * @see SharedObj::ShareClass Messenger::Distribute SharedObj::IsGlobal
 */
int32 SharedObj::IsShared(uint32 class_id)
{
	Class* vcls = Class::GetClass(class_id);
	int	v = 0;

//  while (vcls && ((v = vcls->IsGlobal) >= 0))
    while (vcls)
    {
		if (v > 0)					// definitely global?
			return v;
		vcls = vcls->GetBaseClass();
    }
    return 0;						// got a definitely not global parent
}

/*!
 * @fn void SharedObj::SetActive(bool active)
 * @param active	\b true make the object active, \b false deactivates it
 *
 * By default, all objects are active. They can be marked as  inactive
 * by this routine, which sets or clears the INACTIVE flag.
 * For many objects, this flag is ignored. There are some key exceptions. 
 * If a group is set to be inactive, it remains in the hierarchy but will not be visited.
 * If a scene is set inactive, it remains in the display list but will not
 * traverse its scene graph or render anything to the display. 
 *
 * @see Engine::SetActive SharedObj::SetFlags
 */
void SharedObj::SetActive(bool active)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Obj, OBJ_SetActive) << this << int32(active);
	VX_STREAM_END( OBJ_NotLocked )

    if (active)
		ClearFlags(INACTIVE);	/* make active */
    else
		SetFlags(INACTIVE);		/* make inactive */
}

} // end Vixen