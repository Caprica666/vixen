namespace Core {

/*!
 * @fn Array::Array(intptr maxelems)
 * @param maxelems number of array elements to initially make room for
 *
 * Creates and initializes an array with no elements.
 *
 * @see Array::SetMaxSize Array::SetSize
 */
template <class ELEM, class BASE> BaseArray<ELEM, BASE>::BaseArray(intptr maxelems)
{
	m_Size = 0;
	m_MaxSize = 0;				// remember the size
	m_Data = NULL;
	m_ElemAlloc = NULL;
	m_UserData = false;
	if (maxelems)
		SetMaxSize(maxelems);
}

template <class ELEM, class BASE> FORCE_INLINE BaseArray<ELEM, BASE>::BaseArray(const BaseArray<ELEM, BASE>& src)
	: BASE(src)
{
	intptr maxsize = src.GetMaxSize();

	m_Size = 0;
	m_MaxSize = 0;
	m_Data = NULL;
	m_ElemAlloc = src.GetElemAllocator();
	m_UserData = false;
	SetMaxSize(maxsize);
	SetSize(src.GetSize());
}

template <class ELEM, class BASE> FORCE_INLINE intptr BaseArray<ELEM, BASE>::GetMaxSize() const
{ return m_MaxSize; }

template <class ELEM, class BASE> FORCE_INLINE intptr BaseArray<ELEM, BASE>::GetSize() const
{ return m_Size; }

/*!
 * @fn void Allocator* BaseArray::GetElemAllocator() const
 * @return pointer to the element allocator, never NULL
 *
 * The default allocation method uses the same allocator as
 * the parent class \b BASE of this ByteArray.
 * Normally this is the global allocator but, if your application changes
 * the default alloctor for this parent class,
 * the default element allocator changes accordingly.
 *
 * @see Allocator BaseArray::GetElemAllocator BaseObj GlobalAllocator
 */
template <class ELEM, class BASE> FORCE_INLINE Allocator* BaseArray<ELEM, BASE>::GetElemAllocator() const
{
	if (m_ElemAlloc)
		return m_ElemAlloc;
	return BASE::GetClassAllocator();
}

/*!
 * @fn void Array::SetElemAllocator(Allocator* alloc)
 * @param alloc Allocator to use for array element data storage.
 *
 * The default allocation method uses the global allocator.
 *
 * @see Allocator Array::GetElemAllocator
 */
template <class ELEM, class BASE> FORCE_INLINE void BaseArray<ELEM, BASE>::SetElemAllocator(Allocator* alloc)
{
	if (alloc)
		{ VX_ASSERT(m_MaxSize == 0); }
	m_ElemAlloc = alloc;
}

template <class ELEM, class BASE>  FORCE_INLINE const ELEM* BaseArray<ELEM, BASE>::GetData() const
{
	return m_Data;
}

template <class ELEM, class BASE>  FORCE_INLINE ELEM* BaseArray<ELEM, BASE>::GetData()
{
	return m_Data;
}

/*!
 * @fn bool BaseArray::Grow(int growto)
 *	@param growto	Number of elements the array should be able to hold after it has grown.
 *
 * Makes the array big enough to accomodate \b growto elements.
 * If the array can already hold this many elements, nothing is done.
 * Otherwise, the data area of the array is enlarged (reallocating if necessary)
 * so that \b growto contiguous elements can be stored.
 *
 * @return \b true if array was successfully grown, else \b false
 *
 * @see ByteArray::SetData ByteArray::SetMaxSize
 */
template <class ELEM, class BASE> bool BaseArray<ELEM, BASE>::Grow(intptr growto)
{
	intptr		bytes = growto * sizeof(ELEM);
	ELEM*		new_data;
	Allocator*	elemalloc = GetElemAllocator();

	if (growto < 0)						// no growth needed?
		return true;
	if (m_UserData)						// user managing data area?
		VX_ERROR(("Array::Grow: user data area exhausted\n"), false);
	if (m_Data == NULL)					// make new data area?
	{
		VX_ASSERT(m_Size == 0);
		new_data = (ELEM*) elemalloc->Alloc(bytes);
	}
	else
	{
		if (growto < 64)
		{
			growto = 64;	// minimum growth
			bytes = growto * sizeof(ELEM);
		}
		new_data = (ELEM*) elemalloc->Grow((void*) m_Data, bytes);
	}
	if (new_data == NULL)				// could not allocate?
		VX_ERROR(("Array::Grow: out of memory\n"), false);
	m_Data = new_data;					// save data area pointer
	m_MaxSize = growto;					// remember new size
	for (intptr i = m_Size; i <= growto - 1; ++i)
	{
		ELEM* e = (ELEM*) new_data;
		void* p = (void*) (e + i);
		new (p) ELEM();
	}
	return true;
}

/*!
 * @fn bool BaseArray::SetMaxSize(intptr s)
 *	@param s Current maximum size of array (number of elements its data area can hold)
 *
 * If the array is user-managed, this function establishes the
 * maximum number of elements that can be stored. If the array
 * is dynamically managed by the system, setting the maximum size
 * enlarges its data area to accomodate the required number of elements.
 *
 * @return \b true if maximum size successfully changed, else \b false
 *
 * @see Array::SetData Array::SetSize Array::Grow
 */
template <class ELEM, class BASE> bool BaseArray<ELEM, BASE>::SetMaxSize(intptr s)
{
	if (m_UserData)						// user managed array
		m_MaxSize = s;
	else if (s > m_MaxSize)				// enlarge array
		return Grow(s);					// if we can
	if (m_Size <= s)					// current size in range?
		return true;
	m_Size = s;							// restrict size
	return false;
}


/*!
 * @fn bool BaseArray::SetSize(intptr s)
 *	@param 	s Current number of elements contained in array
 *
 * If the array is dynamically managed, it will be enlarged to accomodate the new size.
 * If not, the array size cannot be set beyond the current maximum size.
 *
 *	@return \b true if size successfully changed, else \b false
 *
 * @see Array::SetData Array::Grow Array::SetMaxSize
 *
 */
template <class ELEM, class BASE> bool BaseArray<ELEM, BASE>::SetSize(intptr s)
{
	VX_ASSERT(s >= 0);
	if (s > m_MaxSize)
	{
		if (!Grow(s))
			return false;
	}
	else if ((s < m_Size) && m_Data)
		for (intptr i = s; i < m_Size; ++i)
		{
			ELEM* e = &m_Data[i];
			e->~ELEM();
		}
//		DestructElems(s, m_Size - 1);
	m_Size = s;
	return true;
}


/*!
 *
 * @fn void BaseArray::Empty()
 * Removes the elements in the array but not the array data area.
 * An array is considered empty if it has no elements.
 *
 * @see BaseArray::SetSize BaseArray::IsEmpty
 */
template <class ELEM, class BASE> inline void BaseArray<ELEM, BASE>::Empty()
{
	SetSize(0);
}

/*!
 * @fn bool BaseArray::SetAt(intptr i, const void* p)
 *	@param i	index of new element (0 based)
 *	@param elem	new element to add
 *
 *	Sets the \b ith element of the array to the given value.
 *	Element replacement is done by byte copy - no constructors are called.
 *	The number of bytes copied is determined by the element size
 *	of the array specified at initialization time.
 *	If the array is not large enough, it is extended to become
 *	1 1/2 times its current size.
 *
 * @return index of element added or -1 if out of memory
 */
template <class ELEM, class BASE> FORCE_INLINE bool BaseArray<ELEM, BASE>::SetAt(intptr i, const ELEM& elem)
{
	intptr maxsize = GetMaxSize();

	if (i >= maxsize)				// need to extend array?
	{
		intptr n = maxsize;

		if (n == 0)
			n = 4;
		else
			n += n >> 2;			// grow to 1 1/2 times current size
		if (n <= i)
			n = i + 1;
		if (!Grow(n))				// extend failure
			return false;
	}
	m_Data[i] = elem;
	if (i >= m_Size)				// enlarge array size if at end
		SetSize(i + 1);
	return true;
}


/*!
 * @fn void BaseArray::RemoveAt(intptr i, intptr n)
 *	@param i index of first element to remove (0 based)
 *	@param n number of elements to remove
 *
 *	After array elements have been removed, the following
 *	elements are shuffled up to eliminate the unused space.
 *
 * @return \b true if element was successfully removed, else \b false
 *
 * @see Array::SetElem Array::SetSize
 *
 */
template <class ELEM, class BASE> void BaseArray<ELEM, BASE>::RemoveAt(intptr i, intptr n)
{
	intptr	shuffle;
	intptr	size = GetSize();
	
	if ((i < 0) || (i >= size))			// element out of range?
		return;
	if (n == 0)
		n = 1;							// default is one element
	for (intptr j = i; j < i + n - 1; ++j)
	{
		ELEM* e = &m_Data[j];
		e->~ELEM();
	}
//	DestructElems(i, i + n - 1);		// destroy the removed ones
	shuffle = size - (i + n);			// number to shuffle up
	if (shuffle)						// shuffle up entries after the removed one
	{
		ELEM* elem = m_Data + i;
		memcpy(elem, elem + n, shuffle);
		memset(elem + shuffle, 0, n);
	}
	SetSize(size - n);

}

/*!
 * @fn int BaseArray::Merge(const Array& src)
 * @param src	Source array containing elements to be appended.
 *
 * Concatenates the contents of the source array into the destination array.
 * The destination array is enlarged if possible. When an object array
 * is appended, the objects are multiply referenced (not duplicated).
 *
 * @return index of last element successfully added or -1 on error
 *
 * @see Array::Grow
 */
template <class ELEM, class BASE> intptr BaseArray<ELEM, BASE>::Merge(const BaseArray& src)
{
	intptr ssize = src.GetSize();
	intptr tsize = GetSize();
	intptr	n = tsize + ssize;

	if (!SetMaxSize(n))
		return -1;
	for (intptr i = 0; i < ssize; ++i)
		m_Data[tsize + i] = src[i];
	SetSize(n);
	return n - 1;
}

template <class ELEM, class BASE> bool BaseArray<ELEM, BASE>::IsEmpty() const
	{ return GetSize() == 0; }

/*!
 * @fn Array::~Array
 *	Destructor for array element allocator. Calls the destructor for
 *	all remaining array elements and then frees data area
 *	(if it is not user managed)
 */
template <class ELEM, class BASE> BaseArray<ELEM, BASE>::~BaseArray()
{
	Empty();
	SetData(NULL);
	m_MaxSize = 0;
}

/*!
 * @fn bool BaseArray::SetData(void* data)
 * @param data	Pointer to contiguous memory area to use for storage.
 *				NULL indicates the memory is managed dynamically.
 *
 * The array can use a user-supplied memory area or the system
 * can dynamically allocate storage for the contiguous area as needed.
 * Switching between these two memory management methods
 * causes the contents of the data area to be lost.
 *
 * If a data area is user supplied, the ByteArray::SetMaxSize function is used to
 * establish the byte size of the area. Attempts to enlarge the allocation area
 * beyond this size will fail.
 *
 * @par Examples:
 * @code
 *	Array<short, BaseObj> myarray(16, 2);// dynamic array of shorts
 *	short numbers[16];					// user data area for array
 *	myarray.SetData(numbers);			// now user-managed point array
 *	myarray.SetMaxSize(16);				// tell array its size
 * @endcode
 *
 * @see Array::SetMaxSize Array::Grow
 */
template <class ELEM, class BASE> void BaseArray<ELEM, BASE>::SetData(ELEM* data)
{
	if (!m_UserData && m_Data)
		GetElemAllocator()->Free((void*) m_Data);
	if (data)							// user gave us a pointer?
	{
		m_UserData = true;				// mark as static
		m_Data = data;					// save user ptr
 }
	else								// no pointer, switch to dynamic
	{
		m_UserData = false;				// mark as dynamic
		m_Data = NULL;
		m_Size = 0;
	}
}

template <class ELEM, class BASE> FORCE_INLINE ELEM& BaseArray<ELEM, BASE>::GetAt(intptr i) const
	{ return m_Data[i]; }

template <class ELEM, class BASE> FORCE_INLINE const ELEM& BaseArray<ELEM, BASE>::operator[](intptr i) const
	{ return m_Data[i]; }

template <class ELEM, class BASE> FORCE_INLINE ELEM& BaseArray<ELEM, BASE>::operator[](intptr i)
	{ return m_Data[i]; }

template <class ELEM, class BASE> FORCE_INLINE const ELEM* BaseArray<ELEM, BASE>::First() const
{
	if (m_Size == 0)
		return NULL;
	return &m_Data[0];
}

template <class ELEM, class BASE> FORCE_INLINE const ELEM* BaseArray<ELEM, BASE>::Last() const
{
	if (m_Size == 0)
		return NULL;
	return &m_Data[m_Size - 1];
}

template <class ELEM, class BASE> FORCE_INLINE ELEM* BaseArray<ELEM, BASE>::First()
{
	if (m_Size == 0)
		return NULL;
	return &m_Data[0];
}

template <class ELEM, class BASE> FORCE_INLINE ELEM* BaseArray<ELEM, BASE>::Last()
{
	if (m_Size == 0)
		return NULL;
	return &m_Data[m_Size - 1];
}

/*!
 * @fn int BaseArray::Find(const ELEM& elem) const
 *	@param elem	value of the element to match
 *
 * Compares each element of the array to the input element and
 * returns the index of the first one that matches. Comparison
 * of elements is done using operator== (which must be defined
 * for your element class if you want to use Find).
 *
 * @return index of matching array element or -1 if not found
 *
 * @par Examples:
 * @code
 *	Array<int, BaseObj> foo;	// define integer array
 *	foo.Append(5);			// first element is 5
 *	foo.Append(6);			// second element is 5
 *	int t = foo.Find(7);	// returns -1
 *	t = foo.Find(6);		// returns 1
 * @endcode
 *
 * @see Array::SetAt Array::GetAt
 */
template <class ELEM, class BASE> intptr BaseArray<ELEM, BASE>::Find(const ELEM& elem) const
{
	int index = -1;
	for (intptr i = 0; i < m_Size; ++i)	// look for matching element
		if (m_Data[i] == elem)
			return i;							// found it
	return index;
}

/*!
 * @fn int BaseArray::Append(const ELEM& v)
 * @param v value of the new element
 *
 * Appends one element onto the end of the array.
 * If the array is dynamically managed, it is enlarged
 * to accomodate another element if necessary.
 *
 * @return index of element added (within array) or -1 if out of memory
 *
 * @par Examples:
 * @code
 *	Array<float, BaseObj> vals;
 *	vals.Append(1.0f);      // first element
 *	vals.Append(2.0f);      // second element
 *	vals.SetAt(5, 6.0f);    // sixth element
 *	vals.Append(7.0f);      // seventh element
 * @endcode
 *
 * @see Array::SetAt Array::SetSize Array::RemoveAt
 */
template <class ELEM, class BASE> FORCE_INLINE intptr BaseArray<ELEM, BASE>::Append(const ELEM& v)
{
	return SetAt(m_Size, v);
}

} // end Core