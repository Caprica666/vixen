/****
 *
 * Inlines for RefPtr class
 *
 ****/
namespace Core {

#ifndef VIXEN_EMSCRIPTEN
inline void  RefObj::operator delete (void* ptr)
{
	BaseObj::operator delete(ptr);
}
#endif

inline long  RefObj::AddRef()			
{ 
	// If negative, you've probaly overrun Ulong_Max count -or-
	// memory has been overwritten - negative values caught by Release().
	VX_ASSERT (m_refCount >= 0);
	return InterlockInc(&m_refCount); 
}

inline long  RefObj::Release()
{
	VX_ASSERT (m_refCount > 0);

	// Save local copy to avoid returning <badPtr>->m_refCount after delete
	long	refCount = InterlockDec(&m_refCount);

	// Can discard object.  BaseObj::operator delete() takes care of Allocator handling
	if (refCount == 0 && GetAllocator() != NULL) 
		delete this; 
	return refCount;
}

inline long RefObj::RefCount() const			
{ 
	return m_refCount; 
}

inline void  RefObj::IncUse() const
{ 
	// If negative, you've probaly overrun Ulong_Max count -or-
	// memory has been overwritten - negative values caught by Release().
	VX_ASSERT (m_refCount >= 0);
	InterlockInc(&m_refCount); 
}

inline bool RefObj::Delete()
{
	VX_ASSERT(m_refCount >= 0);
	int	refCount = InterlockDec(&m_refCount);

	if (GetAllocator() != NULL)
	{
#ifdef _DEBUG
		if (refCount < 0)
			VX_PRINTF(("RefObj::Delete %p reference count < 0\n", this));
#endif
		if (refCount <= 0) 
			delete this; 
	}
	return (refCount == 0);
}

inline long RefObj::GetUse() const			
{ 
	return m_refCount; 
}

inline RefPtr::RefPtr(const RefPtr& src)
{ 
	if (m_objPtr = src.m_objPtr)
		m_objPtr->IncUse();
}

inline RefPtr::RefPtr(const RefObj *ptr)
{
	if (m_objPtr = (RefObj*) ptr)
		m_objPtr->IncUse();
}

inline RefPtr::RefPtr(RefObj *ptr)
{
	if (m_objPtr = (RefObj*) ptr)
		m_objPtr->IncUse();
}


/*
inline RefPtr::RefPtr(void *ptr)
{
	if (m_objPtr = (RefObj*) ptr)
		m_objPtr->IncUse();
}
*/

inline RefPtr::~RefPtr()
	{ Delete(); }

inline void RefPtr::Delete()
{
	if (m_objPtr)
	   {
		m_objPtr->Delete();
		m_objPtr = NULL;
	   }
}

inline RefObj* RefPtr::operator->() const
	{ return m_objPtr; }

inline RefPtr::operator	RefObj*() const
	{ return m_objPtr; }

inline RefPtr& RefPtr::operator=(const RefPtr& src)
{
	return RefPtr::operator=(src.m_objPtr);
}

inline RefObj*	RefPtr::operator*()
{
	return m_objPtr;
}

inline RefObj*	RefPtr::operator*() const
{
	return m_objPtr;
}

inline RefPtr& RefPtr::operator=(RefObj* ptr)
{
	RefObj*	oldptr = m_objPtr;

	if (m_objPtr == ptr)			/* same as current? */
		return *this;
	if (ptr)						/* not assigning NULL? */
		(ptr)->IncUse();
	m_objPtr = ptr;				
	if (oldptr)
		oldptr->Delete();			/* delete current value */
	return *this;
}

FORCE_INLINE bool RefPtr::IsNull() const
	{ return m_objPtr == NULL; }

FORCE_INLINE bool	RefPtr::operator==(const RefObj* p) const
{ 
	return m_objPtr == p; 
}

FORCE_INLINE bool	RefPtr::operator==(const RefPtr& p) const
{ 
	return m_objPtr == p.m_objPtr;
}

FORCE_INLINE bool	RefPtr::operator!=(const RefObj* p) const
{ 
	return m_objPtr != p;
}

FORCE_INLINE bool	RefPtr::operator!=(const RefPtr& p) const
{ 
	return m_objPtr != p.m_objPtr;
}

} // end Core