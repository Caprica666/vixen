/*!
 * @file vxobj.h
 *
 * @brief Defines scene manager base object.
 * This class provides support for reference counting, debug printing,
 * run-time type checking and property sets.
 *
 * @author Nola Donato
 * @ingroup vixen
 * @see vobj.h vref.h
 */

#pragma once

namespace Vixen {

class Core::CritSec;
class SharedObj;
class Property;
class Messenger;
class Event;

#define PROP_NameTag	intptr(CLASS_(NameProp))

#ifdef VX_NOREFCOUNT
/*
 * Reference counting is disabled if VX_NOREFCOUNT is set.
 * All memory is garbage collected at the same time when the
 * transaction has finished. Destructors are not called.
 */
template <class T> class Ref
{
public:
	Ref(const Ref<T>& src)					{ m_objptr = src.m_objptr; }
	Ref(const T* p = NULL)					{ m_objptr = (T*) p; }
	Ref(T* p)								{ m_objptr = p; }
	bool	IsNull() const					{ return m_objptr == (T*) NULL; }
	operator T*() const						{ return m_objptr; }
	T* operator->()							{ return m_objptr; }
	T*	operator*()							{ return m_objptr; }
	operator const T*() const				{ return m_objptr; }
	const T* operator->() const				{ return m_objptr; }
	Ref<T>&	operator=(const Ref<T>& src)	{ return operator=((T*) src.m_objptr); }
	bool	operator==(const T* p) const	{ return p == m_objptr; }
	bool	operator!=(const T* p) const	{ return p != m_objptr; }
	bool	operator==(const Ref<T>& r) const	{ return r.m_objptr == m_objptr; }
	bool	operator!=(const Ref<T>& r) const	{ return r.m_objptr != m_objptr; }
	Ref<T>&	operator=(const T* p)
	{
		if (!p)
			delete m_objptr;
		m_objptr = (T*) p;
		return *this;
	}

protected:
	T*	m_objptr;
};
#endif

typedef Ref< SharedObj > ObjRef;

extern	intptr		vixen_cls;
#define	REGISTER_CLASS(CLS)	vixen_cls += intptr(&CLS::ClassInfo)

/*!
 * @class Property
 * @brief Base class for object property sets.
 *
 * Properties are small packets of information that can be attached
 * to individual objects. Vixen uses them internally to maintain
 * object-specific information for scene graph traversal, collision
 * detection and scripting. Applications can also use properties to
 * keep application-specific data associated with an object.
 *
 * Each property has a tag and a key.
 * Internally, Vixen maintains a next pointer for linkage.
 * One or more properties can be associated with any SharedObj.
 * Only one property with a given tag and key can be tied to any object.
 * Usually the tag is used to distinguish what type of property is
 * attached, i.e. which subclass of SharedObj.
 *
 * The key is a unique identifier for that property, permitting you to
 * keep multiple properties of the same type on a single object.
 * Vixen uses this feature internally for multi-threaded operation on
 * a shared-memory cluster of parallel processors. Properties shared by
 * a thread are \b versioned, where the key indicates the version number
 * to be used for that thread.
 *
 * Normally, when an object has no outstanding references and its resources
 * are freed, all properties attached to the object are deleted.
 * Properties are not reference counted. A property may not be shared
 * by multiple objects. If Vixen will be deleting your properties,
 * they should be allocated with \b new because Vixen destroys them with \b delete.
 * Properties may come from allocators, allowing you complete control
 * over their memory behavior. If a property is owned by an object,
 * the Property destructor detaches it before destroying the property.
 *
 * @ingroup vixen
 * @see SharedObj::GetProp SharedObj::DelProp SharedObj::AddProp Allocator
 */
class Property : public Core::List
{
public:
	VX_DECLARE_CLASS(Property);
	Property(intptr tag = 0) : List()	{ Key = 0; Owner = NULL; }
	virtual ~Property();
	Property&	operator=(const Property& p);
	bool	IsA(intptr t)	{ return intptr(GetClass()) == t; }
	bool	IsA(Property* p)	{ return p->GetClass() == GetClass(); }

	SharedObj*	Owner;				//!< owning object
	int32		Key;				//!< key identifying owning thread
	bits		NoFree : 1;			//!< if true, don't free this property
	bits		PropUnused1 : 1;	//!< free to be used by subclasses
	bits		PropUnused2 : 1;	//!< free to be used by subclasses
	bits		PropUnused3 : 1;	//!< free to be used by subclasses
	bits		PropUnused4 : 4;	//!< free to be used by subclasses
};

/*!
 * @class NameProp
 * @brief Used to attach string names to objects.
 *
 * Name properties are attached by messengers which keep
 * dictionaries of the named objects read or written from files.
 * Name properties are allocated by SharedObj::SetName
 * or by Messenger::Define.
 *
 * @ingroup vixen
 * @see Messenger SharedObj::SetName
 */
class NameProp : public Property
{
public:
	VX_DECLARE_CLASS(NameProp);
//! Construct name property from string
	NameProp(const TCHAR* name, intptr tag);
	NameProp(const TCHAR* name = NULL);
	//! Construct this name property from another
	NameProp(const NameProp& s);
//! Return pointer to string name
	operator const TCHAR*() const				{ return m_String; }
//! Return string reference
	operator Core::String&()					{ return m_String; }
//! \b true if the string name is empty
	bool		IsEmpty() const					{ return m_String.IsEmpty() != 0; }
//! compare input string with name string of this property
	NameProp& operator=(const TCHAR* str);
//! copy input name property into this one
	NameProp& operator=(const NameProp& s);
//! compare name property to string
	bool operator==(const TCHAR* s) const		{ return m_String == s; }
//! compare name property to string
	bool operator!=(const TCHAR* s) const		{ return m_String != s; }

protected:
	Core::String m_String;
};


inline NameProp::NameProp(const TCHAR* name, intptr tag)
 :	Property(tag), m_String(name) { }

inline NameProp::NameProp(const TCHAR* name)
 :	Property(PROP_NameTag), m_String(name) { }

inline NameProp::NameProp(const NameProp& s)
 :	Property(PROP_NameTag), m_String(s.m_String) { }

inline NameProp& NameProp::operator=(const NameProp& s)
{
	m_String = s.m_String;
	return *this;
}

inline NameProp& NameProp::operator=(const TCHAR* str)
{
	m_String = str;
	return *this;
}

#pragma warning(disable:4291)


/*!
 * @class SharedObj
 * @brief Base class to provide automation and distribution.
 *
 * It also provides support for run-time type checking, reference counting,
 * property sets and is the foundation upon which the Vixen object distribution
 * mechanism is built.
 *
 * <B>Reference Counting</B>
 *
 * Any class derived from SharedObj contains a use count indicating how many other
 * objects reference this one. SharedObj works together with Ref to make
 * reference counting mostly transparent to client code. Ref classes are made
 * for each derivative of SharedObj that act like smart pointers to hide the
 * implementation of reference counting.
 *
 * Although you can use new to allocate, you should never use the C++
 * delete operator to free a class you have derived from SharedObj.
 * Instead, use ::Delete(SharedObj*) or the SharedObj::Delete member function
 * to ensure that the object is only deleted if it is no longer referenced.
 *
 * <B>Run Time Type Checking</B>
 *
 * Each SharedObj contains a unique class identifier that indicates which class
 * the object belongs to in Vixen. This class ID is an index into a global class
 * table maintained by Vixen. The class table allows you to perform run-time queries
 * to get information about an object (whether it is a subclass of a particular class,
 * what it's string name is, etc.). This feature of Vixen may not work reliably
 * if you use multiple inheritance.
 *
 * <B>Property Sets</B>
 *
 * An SharedObj may have an arbitrary number of properties associated with it.
 * These properties must be derived from Property and are linked together in a list
 * attached to the object. Memory for property sets may be managed either by the scene manager
 * (which uses new and delete) or by the client code.
 *
 * Property sets are not sent to remote nodes when the scene graph is distributed
 * across a parallel network. Properties are intended for storing data in the local
 * scene graph for use with local engines - not for generalized network use.
 *
 * <B>Extending Vixen</B>
 *
 * Vixen extensions are well behaved subclasses of SharedObj. This means that they support
 * all SharedObj overrides for saving and loading to a stream, copy constructors,
 * destructors and printing.
 *
 * Vixen extensions must override most of the automation functions section to work
 * in a distributed environment. If your class does not have any additional data fields,
 * you do not have to implement your own copy constructor, loading or saving.
 * You still have to use the Vixen class macros, however.
 *
 * When a Vixen object is saved to a file or transferred to a remote machine,
 * its state is saved by outputting a set of Vixen opcodes to the output stream
 * associated with the main messenger. Similarly, when an object is loaded
 * from a file or updates to an object are read from a stream, the messenger
 * asks the Vixen object to apply its opcodes to update the object state. Usually,
 * there is a Vixen opcode for every API function that changes the state of an object.
 * Subclasses must also define opcodes for attributes that can be updated and for
 * functions that change the object state.
 *
 * Currently, the Vixen dispatch mechanism does not provide for reading back information
 * from a remote node. This is because it is assumed all scene graph nodes
 * are locally available and that necessary geometry, texture and engine data
 * is also on the local node.
 *
 * @note
 * You should not call delete on any subclass of SharedObj because
 * it will not handle reference counting properly. Call SharedObj::Delete or ::Delete instead.
 *
 * @ingroup vixen
 * @nosubgrouping
 * @see BaseObj RefObj Property VRef
 */
class SharedObj : public Core::LockObj
{
public:
	VX_DECLARE_CLASS(SharedObj);

//! @name Construction
//!@{
	SharedObj();	//!< Construct Empty object
	~SharedObj();
//! Construct one object from another
	SharedObj(const SharedObj&);
//! Copy from one object to another
	SharedObj&				operator=(const SharedObj&);
//! Copy from one object to another
	virtual	bool			Copy(const SharedObj*);
//! Clone this object, usually shallow copy
	virtual	SharedObj*		Clone() const;
//!@}

//! @name Type Checking
//!@{
			uint32		GetID() const;	//!< Get object identifier (messenger handle)
//! Set object identifier (internal)
			void		SetID(uint32 id);
//! Find the class ID of a class given its string name
	static	int32		FindClass(const TCHAR* name);
//! Instantiate a object given the class ID
	static	SharedObj*	Create(uint32 id = 0);
//!@}

/*! @name ThreadSafe State
 * These functions manage the object state flags and locks.
 * The state flags are accessed in a thread safe way and do not
 * require locking to be enabled. The locking routines manage
 * a heavier, operating system lock that allows object updates
 * to be thread-safe.
 */
//!@{
	bool			HasChanged() const;				//!< Check if changed flag is set
	void			SetChanged(bool) const;			//!< Set changed flag to true or false
	bool			IsActive() const;				//!< Check if inactive flag is set
	virtual void	SetActive(bool);				//!< Set activity state
	bool			IsSet(uint32 flags) const;		//!< Check if state flags are set
	int32			IsGlobal() const;				//!< Check if object is known globally
	int32			IsShared() const;				//!< Check if object is shared
	static int32	IsShared(uint32);				//!< Check if class is shared
	uint32			GetFlags() const;				//!< Get object state flags
	void			SetFlags(uint32 flags) const;	//!< Set object state flags
	void			ClearFlags(uint32 flags) const;	//!< Clear set of state flags
//!@}


//! @name Extension
//!@{
	virtual	bool		Do(Messenger& s, int opcode);	//!< Apply the next object transaction from messenger
//! Save object to messenger in Vixen protocol
	virtual	int			Save(Messenger&, int) const;
//! Respond to event observed by object
	virtual bool		OnEvent(Event*) { return false; }
//! Print members of class on debug output
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
//! Indicate distribution status for members of the given class
	static void			ShareClass(uint32 classid, int32 v = SHARED);
//!@}


/*! @name	Property Sets
 * Each object has a thread-safe list of properties which allow
 * application-specific data to be attached to individual objects.
 * The string name attached to an object is a name property.
 */
//!@{
	const TCHAR* GetName(int32 key = 0) const;			//!< Retrieve this object's name
	bool		SetName(const TCHAR*, int32 key);		//!< Change this objects name
	bool		SetName(const TCHAR*);
	bool		SetName(NameProp&);						//!< Add this name property to object
	void		AddProp(Property*);						//!< Add a property to this object
	Property*	GetProp(intptr tag) const;				//!< Retrieve a property based on its tag
	Property*	GetProp(intptr tag, uint32 key) const;	//!< Retrieve a property based on tag and key
	void		DelProp(intptr tag, uint32 key = 0);	//!< Delete a property based on tag and key
	Property*	RemoveProp(intptr tag, uint32 key = 0);	//!< Remove a property based on tag and key
	Property*	RemoveProp(Property*);					//!< Detach a property from this object
//!@}

	/*!
	 * Object state flags used internally by SharedObj.
	 * @see SharedObj::SetFlags SharedObj::GetFlags
	 */
	enum
	{
		NOFREE =	(1 << 15),	//!< object cannot be freed
		SAVED =		(1 << 14),	//!< object has been saved
		GLOBAL =	(1 << 13),	//!< object is distributed to all
		SHARED =	(1 << 12),	//!< object is shared
	// middle bits free for application use
		DOEVENTS =	4,			//!< object should throw events
		CHANGED	 =	1,			//!< object has changed
		INACTIVE =	2,			//!< object inactive or disabled
		OBJ_LASTUSED =	4,		//!< last used flag
	};

	/*!
	 * Opcodes for the binary file format
	 * The order of the strings in SharedObj::Opnames must be the same.
	 */
	enum Opcode
	{
		OBJ_Create = 1,
		OBJ_SetName,
		OBJ_Copy,
		OBJ_Delete,
		OBJ_Print,
		OBJ_SetNameKey,
		OBJ_SetActive,
		OBJ_SetFlags,
		OBJ_NextOp = 20,
	};

	/*!
	 * Options understood by SharedObj::Print
	 * @see SharedObj::Print
	 */
	enum PrintOpts
	{
		PRINT_Header = 1,
		PRINT_Attributes = 2,
		PRINT_Trailer = 4,
		PRINT_Children = 8,
		PRINT_Data = 16,
		PRINT_Summary = PRINT_Header | PRINT_Trailer,
		PRINT_Default = PRINT_Summary | PRINT_Attributes | PRINT_Children,
		PRINT_All = PRINT_Default | PRINT_Data,
	};


protected:
	int32				m_ID;			//!< object identifier (messenger handle)
	mutable Property*	m_Prop;			//!< property list head
	mutable vint32		m_Flags;		//!< object information flags
};

typedef Core::ObjLock ObjectLock;

} // end Vixen