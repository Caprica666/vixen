#pragma once

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
 * @class Ref
 *
 * @brief Template class that implements a reference-counted pointer for a given class.
 *
 * Each subclass of  \b RefObj can have a  reference class associated with it
 * that acts as a smart pointer which can reference count objects of
 * that class. For the most part, you can use a reference class just like a
 * pointer to an object. It automatically converts to
 * and from a real pointer by using operator overloading.
 *
 * When a pointer to a \b T is assigned into a VRef of type \b T, it
 * automatically dereferences the old object and garbage collects
 * it if necessary. The reference count on the new object is
 * then incremented. A NULL pointer is legal input and causes the
 * current value to be dereferenced without assigning a new value.
 *
 * The BaseObj constructor takes care of initializing the pointer to
 * NULL and the destructor takes care of dereferencing the
 * last value pointed to.
 *
 * @ingroup vcore
 * @see RefObj::IncUse
 * @see RefObj
 */
template <class T> class Ref : public Core::RefPtr
{
	friend class Core::RefObj;
public:
//! @name Constructors
//@{
	Ref(Core::RefObj* p = NULL) : RefPtr(p) { }	//!< Reference nothing.
	Ref(const Ref<T>& src) : RefPtr(src) { }	//!< Reference same object as source reference.
	Ref(const T* p) : RefPtr(p) { }				//!< Reference given object.
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
 * When a reference or pointer is copied, the old object referenced (if any)
 * is dereferenced and freed if it is no longer used. The reference count
 * on the new object is incremented.
 */
//@{
	Ref<T>&	operator=(const Ref<T>&);	//!< Reference same object as source reference.
	Ref<T>&	operator=(const T*);		//!< Reference the given object.
//@}

/*!
 * @name Comparison
 * Two references are considered equal if they both reference the same object.
 * Comparing a reference against zero or NULL is valid and succeeds if
 * no object is referenced.
 */
//@{
	bool	operator==(const Ref<T>&) const;	//!< Return \b true if two references are equal.
	bool	operator!=(const Ref<T>&) const;	//!< Return \b true if two references are not equal.
	bool	operator==(const T*) const;			//!< Return \b true if the given object is referenced.
	bool	operator!=(const T*) const;			//!< Return \b true if the given object is not referenced.
	bool	operator!() const;					//!< Return \b true if no object is referenced.
//@}

	static const Ref<T>	Null;	//!< a NULL reference.
};


