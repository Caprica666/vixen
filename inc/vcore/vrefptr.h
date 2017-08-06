#pragma once

namespace Core {

/*!
 * @file vref.h
 * @brief Smart pointers for reference-counted objects.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 * @see vobj.h
 */


/*!
 * @class RefObj
 *
 * @brief Reference counted object.
 *
 * Descendants of this class are reference-counted so that memory and resources
 * used by the object are not reclaimed until the object is no longer used.
 * Objects that are reference counted exist as long as another object uses them.
 * Once the last reference to a reference counted object is removed,
 * the object will spontaneously destruct.
 * @ingroup vcore
 * @see BaseObj RefPtr VRef
 */
class RefObj : public BaseObj
{
public:
	VX_DECLARE_CLASS(RefObj);
//! constructor
	RefObj() : BaseObj() { m_refCount = 0; }
//! Increments reference count.
	void	IncUse() const;
//! Returns reference count.
	long 	GetUse() const;
//! Dereferences object and frees if unused.
	bool	Delete();
//! Adds a reference.
	long	AddRef();
//! Removes reference and releases resources if unused.
	long	Release();
//! Returns reference count.
	long	RefCount() const;

protected:
//! Reference count value, 0 indicates not referenced.
	mutable vint32	m_refCount;
	friend class RefPtr;		// for access to m_refCount
protected:
#ifndef VIXEN_EMSCRIPTEN
//! Do not use delete, use RefObj::Delete
	void operator delete(void*);
#endif
};

/*!
 * @class RefPtr
 *
 * @brief Smart RefObj pointer which automatically does reference counting.
 *
 * When you assign a non-NULL RefObj pointer to the RefPtr,
 * it increments the use count of the new object.
 * If the RefPtr already references a different object, that object's reference count is
 * decremented (and it is freed if necessary).  Assigning NULL to
 * a RefPtr is equivalent to removing a reference to the object pointer.
 *
 * @note This class is mostly protected - use VRef<T> template below
 * for a typesafe implementation of smart pointer to class T.
 *
 * @ingroup vcore
 * @see RefObj VRef
 */
class RefPtr
{
public:
//! @name Constructors
//@{
	RefPtr(const RefObj* ptr); 	//!< Reference given object.
	RefPtr(const RefPtr& src);	//!< Reference same object as source reference.
	RefPtr(RefObj* ptr = NULL);	//!< Constructor to reference nothing.
	~RefPtr();					//!< Destructor is \b never virtual.
//@}

//! @name Dereference
//@{
	operator RefObj*() const; 	//!< Return a pointer to the object referenced.
	RefObj*	operator*() const; 	//!< Return a pointer to the object referenced.
	RefObj*	operator->() const; //!< Return a pointer to the object referenced.
	RefObj*	operator*(); 		//!< Return a pointer to the object referenced.
//@}

//! @name Assignment
//@{
	RefPtr&	operator=(RefObj*);					//!< Reference the given object.
	RefPtr&	operator=(const RefPtr& src);	//!< Reference same object as source reference.
//@}

//! @name Comparison
//@{
	bool		operator==(const RefPtr& p) const;
	bool		operator==(const RefObj* p) const;	//!< Return \b true if given object is referenced.
	bool		operator!=(const RefObj* p) const;	//!< Return \b true if given object is not referenced.
	bool		operator!=(const RefPtr& p) const;
	bool		IsNull() const;						//!< Return \b true if nothing referenced.
//@}

protected:
	void		Delete();	//!< Dereference current object.
	RefObj*		m_objPtr;	//! Pointer to object referenced.
};

} // end Core