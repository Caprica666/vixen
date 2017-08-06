#pragma once

/*!
 * @file dxref.h
 * @brief Smart pointers for DirectX objects.
 *
 * @author Nola Donato
 *
 * @see vref.h
 */
#include "ole2.h"
namespace Vixen {
/*!
 * @class DXRef
 *
 * @brief Smart pointer to DirectX objects which automatically handles
 * COM reference counting.
 *
 * When you assign a non-NULL pointer to the DXRef,
 * it adds a reference to the DirectX object.
 * If the DXRef already references a different object, that object's reference count is
 * decremented (and it is freed if necessary).  Assigning NULL to
 * a DXRef is equivalent to removing a reference to the object pointer.
 *
 * @ingroup psming
 * @see DXRef
 */
class DXRefPtr
{
	class DXResources : public Array<IUnknown*>
	{
	public:
		DXResources() : Array<IUnknown*>() {  }
		~DXResources();
		int		Add(IUnknown* ptr);
		int		Release(int index);
	};

public:
	DXRefPtr(IUnknown* ptr = NULL);	//!< Constructor to reference nothing.
	~DXRefPtr();					//!< Destructor is \b never virtual.

	int			Release();					//!< Release DX object referenced.
	IUnknown*	operator*();				//!< Return a pointer to the object referenced.
	DXRefPtr&	operator=(IUnknown*);		//!< Reference the given object.
	static void	FreeAll();

protected:
	int			FindIndex(IUnknown* ptr);
	const IUnknown*	GetPtr() const;

	int			m_index;		//! index in master DX pointer table
	static DXResources*	s_DXObjs;	//! table of pointers to DirectX resources to be freed
};

/*!
 * @class DXRef
 *
 * @brief Template class that implements a reference-counted pointer for a DirectX class.
 *
 * Each DirectX object can have a \e reference class associated with it
 * that acts as a smart pointer which can reference count objects of
 * that class. For the most part, you can use a reference class just like a
 * pointer to an object. It automatically converts to
 * and from a real pointer by using operator overloading.
 *
 * When a pointer to a \b T is assigned into a DXRef of type \b T, it
 * automatically dereferences the old object and garbage collects
 * it if necessary. The reference count on the new object is
 * then incremented. A NULL pointer is legal input and causes the
 * current value to be dereferenced without assigning a new value.
 *
 * @ingroup psmint
 * @see DXRefPtr
 */
template <class T> class DXRef : public DXRefPtr
{
public:
	DXRef(T* p = NULL) : DXRefPtr(p) { }		//!< Reference nothing.

	operator T*();							//!< Return a pointer to the object referenced.
	T* operator->();						//!< Return a pointer to the object referenced.
	T*	operator*();						//!< Return a pointer to the object referenced.
	operator const T*() const;				//!< Return a pointer to the object referenced.
	const T* operator->() const;			//!< Return a pointer to the object referenced.

	DXRef<T>&	operator=(const T*);		//!< Reference the given object.

	bool	operator==(const DXRef<T>&) const;	//!< Return \b true if two references are equal.
	bool	operator!=(const DXRef<T>&) const;	//!< Return \b true if two references are not equal.
	bool	operator==(const T*) const;			//!< Return \b true if the given object is referenced.
	bool	operator!=(const T*) const;			//!< Return \b true if the given object is not referenced.
	bool	operator !() const;					//!< Return \b true if no object is referenced.
};

#include "dx11/dxref.inl"

}; // end Vixen