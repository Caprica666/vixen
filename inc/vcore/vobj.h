#pragma once

namespace Core {

/*!
 * @file vobj.h
 * @brief Base objects that support introspection of class hierarchy,
 * run-time type checking and reference-counted memory management.
 *
 * @ingroup vcore
 *
 * @see valloc.h
 */
//extern double GetTime();

// forward declarations
class BaseObj;
class RefObj;
class Class;
class Allocator;

#ifdef VIXEN_INTERFACE
//============================================================================
// module #defines and types
//============================================================================

// An interface mimics a Class structure, as do class/interface iterators
#define Interface		Class

/*!
 * @typedef InterfaceID
 * Number giving unique id used for serialization.
 * COM interfaces may used GUID here.
 * @see Class
 */
typedef void*			InterfaceID;
#endif

// Client-implemented routine to struct an object in a context.  This is used
// for classes that use DECLARE_CLASS in the header.
typedef BaseObj*	(*CreateMethod)(Allocator*);
typedef Allocator*	(*AllocMethod)(void);


#pragma warning(disable:4291)
/*!
 * @function CoreInit
 */
//============================================================================
extern bool CoreInit();
//============================================================================
	

/*!
 * @class BaseObj
 *
 * @brief Base object class with run-time type checking and allocation control.
 *
 * All descendants of the base class have run-time type checking and class-specific
 * control over how and when memory is allocated and freed. Most objects created within
 * the framework should be a subclass of BaseObj or one of its children. The class hierarchy
 * layout for derived classes and interfaces is maintained as a publicly
 * introspectable hierarchy of Class objects.
 * 
 * @ingroup vcore
 * @see Vixen::Core::Class Vixen::Core::RefObj
 */
//============================================================================
class BaseObj
//============================================================================
{
public:
	BaseObj();			//! constructor.
	virtual	~BaseObj();	//! virtual destructor.

//! @name Run Time Type Checking
//!@{
//! Returns class ID of this object.
	uint32				ClassID() const;
//! Returns name of this object's class.
	const TCHAR*		ClassName () const;
//! Determines whether object is derived from the given class.
	bool				IsClass(uint32 classId) const;
//! Returns class for this object.
	virtual Class*		GetClass() const;
//! Determines whether object is derived from the given class.
	bool				IsKindOf (const Class* pClass) const;
#ifdef VIXEN_INTERFACE	
//! Returns non-null pointer to named interface if supported by this object.
	void*				GetInterface(const TCHAR* iName) const;
// Returns pointer to interface list for this class
//	VInterface*	GetInterfaceList() const;
#endif
//!@}

//! @name Memory Allocation
//!@{
//! Allocates using the given allocator.
	inline void*        operator new (size_t size, Allocator* a = NULL);
//! Applies constructor to given memory.
	inline void*        operator new (size_t size, void* ptr);
//! Manages allocator protocol in destructor.
	inline void         operator delete (void* ptr);
//! Returns \b true if allocated on a heap.
	bool                IsHeapAlloc() const;
//! Returns allocator for this object.
	Allocator*          GetAllocator() const;
//! Returns default allocator for this class.
	Allocator*          GetClassAllocator() const;
//!@}

	static Class        ClassInfo;          // root class structure
	static int          Debug;              // hack: for debugging

protected:
//!	Sets allocator for this object - for use by allocators only
	void                SetAllocator(Allocator*);

	Allocator*          m_alloc;			// allocator
	THREAD_LOCAL Allocator* t_lastAllocAt;	// last allocation from operator new()
};

/*!
 * @class Class
 *
 *@brief Class descriptor for BaseObj subclasses.
 *
 * Each class derived from BaseObj has a class structure that contains the string
 * name of the class and information about the class hierarchy and interfaces.
 * This feature is used for run-time type checking and for object serialization.
 *
 * @ingroup vcore
 * @see BaseObj
 */
//============================================================================
class Class
//============================================================================
{
public:
// class attributes
//! Returns string name of class.
	const TCHAR*			GetName()							{ return m_pClassName; }
//! Returns byte size of objects in class.
	long					GetObjectSize()						{ return m_objectSize; }
//! Returns class ID for serialization.
	uint32					GetSerialID()						{ return m_serialID; }

// class factory and linkages - for typesafety (base/parent) and interface discovery (child)
//! Returns function to create instances.
	CreateMethod			GetClassFactory()					{ return m_pCreateMethod; }
//! Returns class this one was derived from.
	Class*					GetBaseClass()						{ return m_pBaseClass; }
//! Returns next class.
	Class*					GetNextClass()						{ return m_pNextClass; }
//! Returns first child class derived from this one.
	Class*					GetFirstChildClass()				{ return m_pFirstChild; }

#ifdef VIXEN_INTERFACE
// interfaces rely upon offsets within the object classes in which they are declared
//! Returns interface ID for serialization.
	InterfaceID				GetInterfaceID()					{ return m_interfaceID; }
//! Returns interface offset.
	long					GetOffset()							{ return m_interfaceOffset >= 0 ? m_interfaceOffset : -m_interfaceOffset; }
//! Returns first interface supported by this class.
	Interface*				GetBaseInterface() 					{ return m_pBaseInterface; }
//! Returns next interface.
	Interface*				GetNextInterface() 					{ return m_pNextInterface; }

// types of interfaces - all use same linking mechanism
//! Returns \b true if \em this is a class pointer.
	bool					IsClass()							{ return m_interfaceOffset == 0; }
//! Returns \b true if \em this is an interface pointer.
	bool					IsInterface()						{ return m_interfaceOffset != 0; }
//! Returns \b true if \em this is a COM interface pointer.
	bool					IsComInterface()					{ return m_interfaceOffset <  0; }

// interfaces
//! Returns \b true if this class has interfaces.
	bool					HasInterfaces()						{ return m_pNextInterface != NULL; }
//! Returns \b true if this class has a class factory.
	bool					HasClassFactory()					{ return m_pCreateMethod || (IsInterface() && m_pBaseClass->m_pCreateMethod); }
//! Returns \b true if this class supports any COM interfaces.
	bool					HasComInterfaces()					{ VX_ASSERT (IsClass()); return NextComInterface() != NULL; }
	bool					HasComClassFactory()				{ VX_ASSERT (IsClass()); return ComClassFactory() != NULL; }
//! Returns \b true if class supports interface with given serial ID.
	bool					HasInterface (InterfaceID id);
//! Returns \b true if class supports COM interface with given ID.
	bool					HasComInterface (InterfaceID id);
//! Returns next COM interface
	Interface*				NextComInterface ();
//! Returns COM class factory`
	Interface*				ComClassFactory ();

	static Interface*		LinkInterface (Interface* interfaceInfo, Class* baseClass, const TCHAR * baseInterfaceName);
#endif

// create an object, get class allocator
//! Creates empty instance of object of this class.
	BaseObj*				CreateObject(Allocator* a = NULL)	{ return (*m_pCreateMethod)(a); }
//! Returns allocator for this class.
	Allocator*				GetAllocator() const;
//!	Provides a function that returns the allocator to use for this class.
	void					SetAllocator(AllocMethod func);
//!	Establishes the allocator to use for this class.
	void					SetAllocator(Allocator* alloc);
//! Print XML description of class
	void					Print(DebugOut& vixen_debug);

// IID => Class mapping for dynamic creation
//! Returns class descriptor based on string name.
	static Class*			GetClass (const TCHAR * iName);
//! Returns class descriptor based on serial ID.
	static Class*			GetClass (uint32 serialid);

// Class hierarchy linkage routines - used by INTERFACE macros
	static Class*			LinkClass(Class* derivedClass, Class* baseClass);
	static void				LinkClassHierarchy();

//protected:
// for nothing else than compatability, we will preserve the order of these data elements
// to mirror the MFC equivalents.  This allows the base class to "overlay" and extend
// the base CObject without disturbing the overall hierarchy and client code (hopefully).

// begin MFC CRuntimeClass data members - should overlay base implementation
	const TCHAR*			m_pClassName;			// Class name as a string
	int						m_objectSize;			// Object size
	uint32					m_serialID;				// Serial ID of class for stream operations
	CreateMethod			m_pCreateMethod;		// Instantiates empty object
	Class*					m_pBaseClass;			// Base class for type-safe CastPtr()
	Class*					m_pNextClass;			// Next sibling
// end MFC members.  

// Added to allow interfaces and class traversal / discovery - see VInterfaceIter below
	Class*					m_pFirstChild;			// First derived class
// class allocator
	Allocator*				m_Alloc;				// allocator for class objects.
	AllocMethod				m_pGetAlloc;			// Pointer to allocator function

#ifdef VIXEN_INTERFACE
	InterfaceID				m_interfaceID;			// Interface key - Char*, COM/OLE: GUUID*
	long					m_interfaceOffset;		// interface offset: 'this' pointer offset in BaseObj
	Interface*				m_pBaseInterface;		// Base interface for interface aggregation
	Interface*				m_pNextInterface;		// Establishes interface chain for iteration and discovery
#endif

// Linkage static data members to fixup the hierarchy after static construction - private
	static bool				s_isLinked;
	static Class*			s_classFixup[];
	static Class*			s_ifaceFixup[];
	static long				s_classCount;
	static long				s_ifaceCount;
};

#ifdef VIXEN_INTERFACE
/*!
 * @class InterfaceIter
 * @brief Examines all the interfaces supported by a given object.
 *
 * @see Class
 * @ingroup vcore
 */

//============================================================================
class InterfaceIter
//============================================================================
{
public:
/*!
 * Special iterators for class hierarchy traversal and COM/OLE support.
 *
 * @code
 *	ITER_AllClasses				walk entire class hierarchy
 *	ITER_JustClassInterfaces	walk class interfaces to root
 *	ITER_JustClassComInterfaces	walk class COM/OLE interfaces to root
 *	ITER_AllComInterfaces		walk entire hierarchy, COM/OLE interfaces only
 *	ITER_AllComFactories		walk entire hierarchy, COM/OLE class factories only
 * @endcode
 */

	enum Sequence		
	{ 
		ITER_AllClasses,			// entire class hierarchy
		ITER_JustClassInterfaces,	// walk class interfaces to root
		ITER_JustClassComInterfaces,// walk class COM/OLE interfaces to root
		ITER_AllComInterfaces,		// entire hierarchy, COM/OLE interfaces only
		ITER_AllComFactories		// entire hierarchy, COM/OLE class factories only
	};


//! Constructors choose what and how to iterate.
	InterfaceIter (const BaseObj* pObj, Sequence = ITER_JustClassInterfaces);	// interfaces in pObj or base class
	InterfaceIter (Class* pClass, Sequence = ITER_JustClassInterfaces);			// interfaces implemented in+below pClass
	InterfaceIter (Sequence sequence);											// entire class hierarchy

//! Returns next item in sequence, \em NULL when finished.
	Interface*				Next();

protected:
	Sequence				m_sequence;
	Interface*				m_pCurrInterface;
	Class*					m_pCurrClass;
};
#endif

} // end Core
