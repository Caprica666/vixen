/*!
 * @file varray.h
 * @brief Dynamically growing array of homogeneous elements.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 * @see vdict.h
 */

#pragma once

namespace Core {


#define	ARRAY_UserAlloc	((Allocator*) -1L)

/*!
 * @class BaseArray
 * @param ELEM	class for array elements
 * @param BASE	class to inherit from, must be subclass of VBaseArray
 *
 * @brief Array of arbitrary fixed-size elements that grows dynamically.
 *
 * You can let the system manage an arrays data area or you can manage
 * it yourself. If you explicitly provide a data area for an array,
 * the system will not try to enlarge past your specified limit,
 * nor will it garbage collect your data area.
 **
 * The template also designates the class of the array elements.
 * To be an array element, a class must have:
 *	@li an empty constructor that is public
 *	@li an implementation of operator==
 *	@li an implementation of operator=
 *
 * You can let the system manage an arrays data area or you can manage
 * it yourself. If you explicitly provide a data area for an array,
 * the system will not try to enlarge past your specified limit,
 * nor will it garbage collect your data area.
 *
 * Normally, array elements are allocated using the same allocator as
 * the parent class. To change this, you can specify the element allocator
 * for any array to allow complete control over data area allocation and
 * enlargement.
 *
 * @ingroup vcore
 * @see ObjArray ByteArray Allocator
 */
template <class ELEM, class BASE> class BaseArray : public BASE
{
public:
	//! Construct an array given number of elements and element size.
	BaseArray(intptr size = 0);
	BaseArray(const BaseArray<ELEM,BASE>&);	//! Clone the given array.
	virtual ~BaseArray();					//! Destroy array and all elements.

	intptr		GetSize() const;			//!< Return current number of elements in array.
	intptr		GetMaxSize() const;			//!< Return maximum possible elements for current data area.
	bool		SetSize(intptr size);		//!< Set current number of array elements.
	bool		SetMaxSize(intptr size);	//!< Set maximum element size of array data area.
	bool		SetAt(intptr index, const ELEM&);	//!< Replace \b ith element in array.
	ELEM&		GetAt(intptr index) const;	//!< Get \b ith element in array.
	ELEM*		First();					//!< Get first element in array.
	const ELEM*	First() const;
	ELEM*		Last();						//!< Get last element in array.
	const ELEM*	Last() const;
	ELEM&		operator[](intptr index);	//!< Get \b ith element in array.
	const ELEM&	operator[](intptr index) const;
	bool		IsEmpty() const;			//!< Returns \b true if array has no elements.
	const ELEM*	GetData() const;			//!< Return pointer to array element data area.
	ELEM*		GetData();					//!< Return pointer to array element data area.
	void		SetData(ELEM*);				//!< Set data area for array elements.
	Allocator*	GetElemAllocator() const;	//!< Get allocator used for array elements.
	void		SetElemAllocator(Allocator*);	//!< Establish allocator to use for array elements.

	void		Empty();					//!< Empties the array.
	void		RemoveAt(intptr index, intptr size = 1);	//!< Remove specified array elements.
	virtual intptr Append(const ELEM&);		//!< Add another element at end of array.
	virtual intptr Find(const ELEM&) const;	//!< Find index of given element.
	virtual intptr Merge(const BaseArray<ELEM, BASE>&);	//!< Merge two arrays.

protected:
//	Internal functions for class-specific element manipulation
	virtual bool Grow(intptr size);		//!< Enlarge array data area.

	Allocator*	m_ElemAlloc;	// element data area allocator
	int			m_UserData;		// is user allocated?
	intptr		m_Size;			// number of elements used by array
	intptr		m_MaxSize;		// maximum number of elements currently available
	ELEM*		m_Data;
};


} // end Core

