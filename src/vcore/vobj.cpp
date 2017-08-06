#include "vcore/vcore.h"
#include "vcore/vlock.h"

namespace Vixen {
namespace Core {

#ifdef VX_NOREFCOUNT
VX_IMPLEMENT_CLASS(LockObj, BaseObj);
#else
VX_IMPLEMENT_CLASS(LockObj, RefObj);
#endif

//===========================================================================
// Class implementation
//===========================================================================

bool	Class::s_isLinked = false;
Class*	Class::s_classFixup[4000];
Class*	Class::s_ifaceFixup[600];
long	Class::s_classCount = 0;
long	Class::s_ifaceCount = 0;



void Class::LinkClassHierarchy()
{
	// TODO: Protect with critical section

	if (s_isLinked)
		return;

	// Link next/child class pointers using derived, base class pointers
	long   classCount = s_classCount;
	while (classCount > 0)
	{
		Class*	derivedClass = s_classFixup[--classCount];
		Class*	baseClass    = s_classFixup[--classCount];
		VX_ASSERT (baseClass && derivedClass);

		// Link this node into head of chain
		derivedClass->m_pNextClass = baseClass->m_pFirstChild;
		baseClass->m_pFirstChild = derivedClass;
	}
	VX_ASSERT (classCount == 0);
#ifdef VIXEN_INTERFACE
	// Link next/child interface pointers using interface, base class pointers
	long   ifaceCount = s_ifaceCount;
	while (ifaceCount > 0)
	{
		const TCHAR*	baseName = (const TCHAR*)s_ifaceFixup[--ifaceCount];
		VInterface*		iface = s_ifaceFixup[--ifaceCount];
		Class*			baseClass  = s_ifaceFixup[--ifaceCount];
		VX_ASSERT (baseClass && iface);

		// Chain into single linked list
		iface->m_pNextInterface = baseClass->m_pNextInterface;
		baseClass->m_pNextInterface = iface;

		// Is this a derived interface implementation?
		classCount = s_classCount;
		for (classCount = s_classCount - 1; classCount > 0; classCount -= 2)
		{
			// Search linkage list linearly until match found
			if (STRCMP(s_classFixup[classCount]->GetName(), baseName) == 0)
			{
				// Set interface base class pointer offset
				iface->m_pBaseInterface = s_classFixup[classCount];
				break;
			}
		}
	}
	VX_ASSERT (ifaceCount == 0);
#endif
	// Class hierarchy linked - don't do this again
	s_isLinked = true;
}

/*!
 * @fn Class* Class::GetClass(const TCHAR* name)
 * @param name string name of class to retrieve.
 *
 * @return pointer to the class information for the named class.
 */
Class* Class::GetClass (const TCHAR* iName)
{
	VX_ASSERT (iName != NULL);

	// Set index pointer to derivedClass pointer
	long   classCount = s_classCount - 1;

	// Loop through all entries, examining the derivedClass information.
	for (; classCount > 0; classCount -= 2)
	{
		// Check if names match
		if (STRCMP(s_classFixup[classCount]->GetName(), iName) == 0)
			return s_classFixup[classCount];
	}

	// If the entire list is exhausted, return NULL to indicate failure
	return NULL;
}

	
/*!
 * @fn Class* Class::GetClass(uint32 classid)
 * @param classid	serial ID of the class to find.
 *
 * @return pointer to the class information for the designated class.
 *
 * @see BaseObj::GetClass Class::GetSerialID
 */
Class* Class::GetClass (uint32 serialid)
{
	long   classCount = s_classCount - 1;

	// Loop through all entries, examining the derivedClass information.
	for (; classCount > 0; classCount -= 2)
	{
		// Check if names match
		if (s_classFixup[classCount]->m_serialID == serialid)
			return s_classFixup[classCount];
	}

	// If the entire list is exhausted, return NULL to indicate failure
	return NULL;
}


void Class::Print(DebugOut& dbg)
{
	Class* base = GetBaseClass();
	endl(dbg << "<class name='" << GetName() << "' id='" << (int32) GetSerialID() << "'>");
	if (base)
	{
		endl(dbg << "<base>");
		base->Print(dbg);
		endl(dbg << "</base>");
	}
	endl(dbg << "</class>");
}

//===========================================================================
// BaseObj implementation
//===========================================================================
using namespace Core;

// special runtime-class structure for BaseObj (no base class)
Class BaseObj::ClassInfo =
{ 
	TEXT("BaseObj"), sizeof(BaseObj), 0/*id*/, NULL, 	// classname, size, serialID, CreateMethod
	NULL, NULL, NULL, 			// baseclass, nextclass, firstchild
	NULL, NULL					// AllocMethod, allocator
};

// add this here for the correct class hierarchy
VX_IMPLEMENT_CLASS (RefObj, BaseObj);

Allocator*	BaseObj::t_lastAllocAt;
int			BaseObj::Debug = 0;

// Because Unix must have at least one virtual function in a class
// in a cpp file in order to know where to put the vtbl
Class* BaseObj::GetClass() const
{
	return &ClassInfo;
}

/*!
 * @fn bool BaseObj::IsKindOf(const Class* class) const
 * @param class Class to check against.
 *
 * @returns \b true if this object is descended from the given class, else \b false.
 */
bool BaseObj::IsKindOf(const Class* pClass) const
{
	VX_ASSERT(this != NULL);

	// simple SI case
	register Class* pClassThis = GetClass();
	VX_ASSERT(pClass != NULL);
	VX_ASSERT(pClassThis != NULL);

	// Continue walking the inheritance chain until NULL (root) encountered.
	while (pClassThis != NULL)
	{
		if (pClassThis == pClass)
			return true;
		else
			pClassThis = pClassThis->GetBaseClass();
	}

	// walked to the top, no match
	return false;     
}

/*!
 * @fn bool BaseObj::IsClass(uint32 classid) const
 * @param classid Class identifier of class to check.
 *
 * @returns \b true if this object is descended from the given class, else \b false.
 */
bool BaseObj::IsClass(uint32 classID) const
{
	VX_ASSERT(this != NULL);

	// simple SI case
	register Class* pClassThis = GetClass();
	VX_ASSERT(pClassThis != NULL);

	// Continue walking the inheritance chain until NULL (root) encountered.
	while (pClassThis != NULL)
	{
		if (pClassThis->GetSerialID() == classID)
			return true;
		else
			pClassThis = pClassThis->GetBaseClass();
	}

	// walked to the top, no match
	return false;     
}


/*!
 * @fn Allocator* BaseObj::GetClassAllocator() const
 *
 * The class allocator for this class can be set by calling Class::SetAllocator.
 * If no class allocator has been explicitly set, the class allocator of
 * the parent class is returned. The default allocator for BaseObj, the common
 * ancestor, is the global allocator.
 *
 * @returns allocator used to create objects of this class.
 *
 * @see Class::SetAllocator Allocator BaseObj::GetAllocator GlobalAllocator
 */
Allocator* BaseObj::GetClassAllocator() const
{
	Class* pClassThis = GetClass();

	while (pClassThis)
	{
		if (pClassThis->GetAllocator())
			return pClassThis->GetAllocator();
		else
			pClassThis = pClassThis->GetBaseClass();
	}
	return GlobalAllocator::Get();
}

Class* Class::LinkClass (Class* derivedClass, Class* baseClass)
{
	VX_ASSERT (derivedClass != NULL);
	VX_ASSERT (baseClass != NULL);

	// Save base and derived classes in global fixup table
	s_classFixup[s_classCount++] = baseClass;
	s_classFixup[s_classCount++] = derivedClass;

	// Watch for overrun, return NULL pointer - will be set later
	VX_ASSERT (s_classCount < sizeof (s_classFixup)/sizeof(Class*));
	return NULL;
}

#ifdef VIXEN_INTERFACE
VInterface* Class::ComClassFactory()
{
	// Iterate from this class to root over all interfaces
	VInterfaceIter		iter (this);

	// Look for overridden implementation of VIClassFactory
	for (VInterface* check = iter.Next(); check; check = iter.Next())
		if (check->IsComInterface() && STRCMP(check->GetName(), TEXT("VIClassFactory")) == 0)
			return check;

	return NULL;
}

VInterface* Class::NextComInterface()
{
	// Walk interface chain until IsComInterface returns true
	for (VInterface* iface = this->GetNextInterface(); iface; iface = iface->GetNextInterface())
		if (iface->IsComInterface())
			return iface;

	return NULL;
}


VInterface* Class::LinkInterface (VInterface* interfaceInfo, Class* baseClass, const TCHAR* baseInterfaceName)
{
	VX_ASSERT (interfaceInfo != NULL);
	VX_ASSERT (baseClass != NULL);
	VX_ASSERT (baseInterfaceName != NULL);

	// Save base class and interface pointer in global fixup table
	s_ifaceFixup[s_ifaceCount++] = baseClass;
	s_ifaceFixup[s_ifaceCount++] = interfaceInfo;
	s_ifaceFixup[s_ifaceCount++] = (VInterface*)baseInterfaceName;

	// Watch for overrun, return NULL pointer - will be set later
	VX_ASSERT (s_ifaceCount < sizeof (s_ifaceFixup)/sizeof(Class*));
	return NULL;
}

void* BaseObj::GetInterface (const TCHAR* interfaceName)	const
{
	VInterfaceIter	iter(this);
	VInterface*		iface;

	// Check interface string representations
	while ((iface = iter.Next()) != NULL)
		if (STRCMP(interfaceName, iface->GetName()) == 0)
			return ((TCHAR*)this) + iface->GetOffset();

// 	// Check for possible internal interfaces
// 	if ((iface = (VInterface*)InternalGetInterface (interfaceName)) != NULL)
// 		return iface;
// 
	return NULL;
}


//===========================================================================
// VInterfaceIter
//===========================================================================

VInterfaceIter::VInterfaceIter (const BaseObj* pObj, Sequence sequence)
  : m_sequence			(sequence),
	m_pCurrClass (pObj->GetClass())
{
	// We need a valid BaseObj vtable in order to traverserse base classes
	VX_ASSERT (pObj->IsKindOf(CLASS_(BaseObj)));
	VX_ASSERT (sequence == ITER_JustClassInterfaces	||
			sequence == ITER_JustClassComInterfaces );

	// Set up the first interface pointer in the chain - NULL is an OK value
	if (sequence == ITER_JustClassInterfaces)
		m_pCurrInterface = m_pCurrClass->GetNextInterface();
	else
		m_pCurrInterface = m_pCurrClass->NextComInterface();
}

VInterfaceIter::VInterfaceIter (Class* pClass, Sequence sequence)
  : m_sequence			(sequence),
	m_pCurrClass (pClass)
{
	// If this fails, m_pCurrInterface and m_pCurrClass are invalid
	VX_ASSERT (pClass->IsClass());
	VX_ASSERT (sequence == ITER_JustClassInterfaces	||
			sequence == ITER_JustClassComInterfaces );

	// Set up the first interface pointer in the chain - NULL is an OK value
	if (sequence == ITER_JustClassInterfaces)
		m_pCurrInterface = m_pCurrClass->GetNextInterface();
	else
		m_pCurrInterface = m_pCurrClass->NextComInterface();
}

VInterfaceIter::VInterfaceIter (Sequence sequence)
  : m_sequence			(sequence),
	m_pCurrClass (CLASS_(BaseObj)),
	m_pCurrInterface	(NULL)
{
	VX_ASSERT (sequence == ITER_AllClasses		|| 
			sequence == ITER_AllComInterfaces	|| 
			sequence == ITER_AllComFactories);
}

VInterface* VInterfaceIter::Next()
{
	// trivial reject - attempt to call past end of iteration
	if (m_pCurrClass == NULL)
		return NULL;

	// Iterating an interface chain?  Get next element from linked list.
	if (m_pCurrInterface != NULL)
	{
		VInterface*		currInterface = m_pCurrInterface;

		if (currInterface != NULL)
		{
			switch (m_sequence)
			{
				case ITER_AllComInterfaces:
				case ITER_JustClassInterfaces:
					m_pCurrInterface = m_pCurrInterface->GetNextInterface();
					break;

				case ITER_JustClassComInterfaces:
					m_pCurrInterface = m_pCurrInterface->NextComInterface();
					break;

				default:
					// Will cause havoc with iteration - currInterface should be NULL...
					break;
			}

			// Ouch - circular reference isn't very good...
			VX_ASSERT (currInterface != m_pCurrInterface);

			// Return the next interface in the chain
			return currInterface;
		}
	}
	
	// Continue walking the inheritance chain until NULL (root) encountered.
	for(;;)
	{
		switch (m_sequence)
		{	
			// Unknown enumeration constant
			default:
				// assertFalse;
				break;
			
			// Object class to root - follow parent pointers
			case ITER_JustClassInterfaces:	
			case ITER_JustClassComInterfaces:
				m_pCurrClass = m_pCurrClass->GetBaseClass(); 
				break;

			// Inorder tree traversal 
			case ITER_AllClasses:
			case ITER_AllComInterfaces:
			case ITER_AllComFactories:
				// Check children first
				if (m_pCurrClass->GetFirstChildClass())
					m_pCurrClass = m_pCurrClass->GetFirstChildClass();

				// If no children, traverse siblings next
				else if (m_pCurrClass->GetNextClass())
					m_pCurrClass = m_pCurrClass->GetNextClass();
				
				// If no siblings, traverse parents last
				else
				{
					// Check parent's sibling for next sibling/any interfaces to iterate.
					// If either is NULL, continue up the hierarchy using parent pointers
					for(;;)
					{
						// Go up to parent class
						m_pCurrClass = m_pCurrClass->GetBaseClass();

						// End of iteration - last child of BaseObj has been processed
						if (m_pCurrClass == NULL)
							break;

						// Is there a parent sibling? - uncle/aunt
						if (m_pCurrClass->GetNextClass() != NULL)
						{
							m_pCurrClass = m_pCurrClass->GetNextClass();
							break;
						}
					}
				}
				break;
		}

		// End of iteration - last child of BaseObj has been processed
		if (m_pCurrClass == NULL)
			return NULL;

		// Check for exit condition
		if ((m_sequence == ITER_AllClasses) ||
			(m_sequence == ITER_AllComInterfaces	   && m_pCurrClass->HasComInterfaces())	 ||
			(m_sequence == ITER_AllComFactories		   && m_pCurrClass->HasComClassFactory()) ||
			(m_sequence == ITER_JustClassInterfaces    && m_pCurrClass->GetNextInterface())	 ||
			(m_sequence == ITER_JustClassComInterfaces && m_pCurrClass->HasComInterfaces()))
			break;
	} // for(;;)

	// Determine return value
	Class*	retVal = NULL;
	switch (m_sequence)
	{
		// Next interface pointer
		case ITER_AllComInterfaces:
		case ITER_JustClassInterfaces:
			m_pCurrInterface = m_pCurrClass->GetNextInterface();
			retVal = m_pCurrInterface;
			break;

		// Next COM/OLE interface pointer
		case ITER_JustClassComInterfaces:
			m_pCurrInterface = m_pCurrClass->GetNextInterface();
			if (!m_pCurrInterface->IsComInterface())
				m_pCurrInterface = m_pCurrInterface->NextComInterface();
			retVal = m_pCurrInterface;
			VX_ASSERT (m_pCurrInterface);
			break;

		// Next runtime class
		case ITER_AllClasses:
		case ITER_AllComFactories:
			retVal = m_pCurrClass;
			break;

		default:
			// assertFalse;
			retVal = NULL;
			break;
	}

	// return above result 
	return retVal;
}
#endif


}	// end Core
}	// end Vixen