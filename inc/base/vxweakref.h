#pragma once

/*!
 * @file vxweakref.h
 * @brief Weak references to objects (may reference deleted objects)
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxobj.h vref.h
 */

namespace Vixen {

/*!
 * @class WeakRef
 *
 * @brief Template class that implements a weakly-referenced pointer for a given class.
 *
 * A WeakRef acts like a smart pointer which can reference objects that may
 * have been deleted. For the most part, you can use a reference class just like a
 * pointer to an object. It automatically converts to
 * and from a real pointer by using operator overloading.
 * If the object it references has been deleted, the WeakRef returns a NULL pointer.
 *
 * When a pointer to a \b T is assigned into a WeakRef of type \b T, it
 * does not add a reference to the object. A NULL pointer is legal input and decouples the
 * WeakRef from its current object.
 *
 *
 * @ingroup vixen
 * @see RefObj::IncUse
 * @see RefObj Ref
 */
template <class T> class WeakRef
{
public:
//! @name Constructors
//@{
	WeakRef(const WeakRef<T>& src);			//!< Reference same object as source reference.
	WeakRef(const T* p = (const T*) NULL);	//!< Reference given object.
//@}

/*!
 * @name Dereference
 * A reference is automatically converted to a pointer to the object it references.
 */
//@{
	operator T*() const;					//!< Return a pointer to the object referenced.
	T*	operator->();						//!< Return a pointer to the object referenced.
	T*	operator*();						//!< Return a pointer to the object referenced.
	operator const T*() const;				//!< Return a pointer to the object referenced.
	const T* operator->() const;			//!< Return a pointer to the object referenced.
//@}

/*!
 * @name Assignment
 * When a reference or pointer is copied, the old object (if any)
 * is dereferenced and freed if it is no longer used. The reference count
 * on the new object is incremented.
 */
//@{
	WeakRef<T>&	operator=(const WeakRef<T>&);	//!< Reference same object as source reference.
	WeakRef<T>&	operator=(const T*);			//!< Reference the given object.
//@}

/*!
 * @name Comparison
 * Two references are considered equal if they both reference the same object.
 * Comparing a reference against zero or NULL is valid and succeeds if
 * no object is referenced.
 */
//@{
//	bool	operator==(const WeakRef<T>&) const;	//!< Return \b true if two references are equal.
//	bool	operator!=(const WeakRef<T>&) const;	//!< Return \b true if two references are not equal.
	bool	operator==(const T*) const;				//!< Return \b true if the given object is referenced.
	bool	operator!=(const T*) const;				//!< Return \b true if the given object is not referenced.
//	bool	operator!() const;						//!< Return \b true if no object is referenced.
	bool	IsNull() const;
//@}

protected:
	int		MakeRef(const SharedObj* obj);
	T*		GetPtr() const;

	int32		m_Handle;
};

}	// end Vixen