

#pragma once

namespace Vixen {

/*!
 * @class Array
 * @brief Homogeneous array of objects that grows dynamically.
 *
 * This class is templatized to allow the array to derive from any
 * subclass of BaseArray. This lets you add array functionality to
 * your own classes.
 * @param ELEM class for array elements
 *
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
 * There are several other useful subclasses of Array predefined.
 *	- IntArray	array of 32 bit integers
 *	- FloatArray	array of 32 bit floats
 *	- PtrArray	array of pointers void*
 *	- ObjArray	array of references to objects
 *
 * @par Examples:
 * @code
 *	Array<void*> foo;				// empty array of pointers
 *	Array<Vec3> pts(16);			// dynamic with room for 16 points
 *	Vec3 mydata[16];				// user data area
 *	Array<Vec3> mypts(16, mydata);	// for this point array
 * @endcode
 *
 * @ingroup vixen
 * @see ObjArray Array Allocator
 */
template <class ELEM> class Array : public Core::BaseArray<ELEM, SharedObj>
{
public:
	/*!
	 * @brief Thread-safe iterator for arrays.
	 *
	 * The array is locked and elements are accessed in increasing order
	 * with the index starting at zero. Array is unlocked when the iterator
	 * is destroyed.
	 */
	class ConstIter : protected ObjectLock
	{
	public:
		ConstIter(Array<ELEM>& a) : ObjectLock(&a), m_Index(-1) { }
		ConstIter(const Array<ELEM>& a) : ObjectLock(&a), m_Index(-1) { }

		//! Return array index of current element.
		intptr	GetIndex() const
			{ return m_Index; }

		//! Reset iterator to start at first element.
		void Reset(Array<ELEM>* a = NULL)
		{
			m_Index = -1;
			if (a == NULL)
				return;
			ObjPtr->Unlock();
			ObjPtr = a;
			a->Lock();
		}

		//! Retrieve next array element, NULL if no more left.
		const ELEM* Next()
		{
			Array<ELEM>* a = (Array<ELEM>*) ObjPtr;
			if (a == NULL)
				return NULL;
			if (++m_Index >= a->GetSize())
				return NULL;
			return (const ELEM*) &(a->GetAt(m_Index));
		}
	protected:
		intptr	m_Index;
	};

	class Iter : protected ObjectLock
	{
	public:
		Iter(Array<ELEM>& a) : ObjectLock(&a), m_Index(-1) { }
		Iter(const Array<ELEM>& a) : ObjectLock(&a), m_Index(-1) { }

		//! Return array index of current element.
		long	GetIndex() const
			{ return m_Index; }

		//! Reset iterator to start at first element.
		void Reset(Array<ELEM>* a = NULL)
		{
			m_Index = -1;
			if (a == NULL)
				return;
			ObjPtr->Unlock();
			ObjPtr = a;
			a->Lock();
		}

		//! Retrieve next array element, NULL if no more left.
		ELEM* Next()
		{
			Array<ELEM>* a = (Array<ELEM>*) ObjPtr;
			if (a == NULL)
				return NULL;
			if (++m_Index >= a->GetSize())
				return NULL;
			return (ELEM*) &(a->GetAt(m_Index));
		}
		//! Remove current array element.
		bool Remove()
		{
			Array<ELEM>* a = (Array<ELEM>*) ObjPtr;
			intptr	index = m_Index - 1;
			if (a == NULL)
				return false;
			if (index >= a->GetSize())
				return false;
			a->RemoveAt(index);
			m_Index = index;
			return true;
		}
	protected:
		intptr	m_Index;
	};

		
	/*!
	 * Constructor for array which allocates initial memory for elements.
	 * @param size	Number of elements data area should initially make room for.
	 *				If zero, little initial space is allocated but the
	 *				array will grow dynamically as space is needed.
	 *
	 * @param data	Pointer to memory for data array. If NULL, the array
	 *				will be managed dynamically. Otherwise, \b data is assumed
	 *				to point to a contiguous area of memory that can accomodate
	 *				\b size array elements and the array will not be enlarged
	 *				beyond this size.
	 *
	 * Creates and initializes an empty array (array with no elements).
	 *
	 * @see SharedObj ByteArray::SetMaxSize ByteArray::SetSize ByteArray::SetElemAllocator
	 */
	Array(intptr size = 0, ELEM* data = NULL)
	 :	Core::BaseArray<ELEM, SharedObj>(size)
	{
		m_ElemSize = sizeof(ELEM);
		if (data)
			Core::BaseArray<ELEM, SharedObj>::SetData(data);
		if (size)
			Core::BaseArray<ELEM, SharedObj>::SetMaxSize(size);
	}

	virtual	bool		Copy(const SharedObj*);
	virtual	bool		Do(Messenger& s, int opcode);
	virtual	int			Save(Messenger&, int) const;
	ELEM&		GetAt(intptr i) const { return Core::BaseArray<ELEM, SharedObj>::GetAt(i); }

#ifndef VX_NOTHREAD
	FORCE_INLINE bool	SetAt(intptr index, const ELEM& p)
		{ ObjectLock l(this); return Core::BaseArray<ELEM, SharedObj>::SetAt(index, p); }
	FORCE_INLINE void	RemoveAt(intptr index, intptr size = 1)
		{ ObjectLock l(this); Core::BaseArray<ELEM, SharedObj>::RemoveAt(index, size); }
	virtual intptr		Find(const ELEM& e) const
		{ ObjectLock l(this); return Core::BaseArray<ELEM, SharedObj>::Find(e); }
	virtual intptr		Merge(const Array<ELEM>& src)
		{ ObjectLock l(this); return Core::BaseArray<ELEM, SharedObj>::Merge(src); }
#endif
	/***
	 * Array::Do opcodes (and for binary file format)
	 ****/
	enum Opcode
	{
		ARRAY_SetData = SharedObj::OBJ_NextOp,
		ARRAY_SetAt,
		ARRAY_RemoveAt,
		ARRAY_Append,
		ARRAY_SetElemSize,
		ARRAY_SetSize,
		ARRAY_NextOp = SharedObj::OBJ_NextOp + 20,
	};
protected:
		int		m_ElemSize;
};

#ifdef SWIG
typedef Array<ObjRef>		SharedObjArray;
%template(SharedObjArray)	Array<ObjRef>;
#endif

/*!
 * @class ObjArray
 *
 * @brief Array of references to objects, elements which are reference counted.
 *
 * Updating an element in an object array causes the old element
 * to be dereferenced (and deleted if necessary) and replaced with
 * a new value. The new value automatically generates a reference.
 * When an object array is initially created, the object pointers for
 * the ObjRefs are NULL.
 *
 * @ingroup vixen
 * @see SharedObj RefObj ObjArray Array
 */
class ObjArray : public Array<ObjRef>
{
public:
	/*!
	 * @brief Thread-safe iterator for array of shared objects
	 * @see Array::Iter
	 */
	class Iter : public Array<ObjRef>::Iter
	{
	public:
		Iter(ObjArray& a) : Array<ObjRef>::Iter(a) { }
		Iter(const ObjArray& a) : Array<ObjRef>::Iter(a) { }

		void Reset(const ObjArray* a)
			{ Array<ObjRef>::Iter* t = this; t->Reset((Array<ObjRef>*) a); }

		void Reset(ObjArray* a = NULL)
			{ Array<ObjRef>::Iter* t = this; t->Reset((Array<ObjRef>*) a); }

		SharedObj* Next()
		{
			ObjArray* a = (ObjArray*) ObjPtr;
			if (a == NULL)
				return NULL;
			if (++m_Index >= a->GetSize())
				return NULL;
			return a->GetAt(m_Index);
		}
	};

	VX_DECLARE_CLASS(ObjArray);
	ObjArray(intptr size = 0)
		: Array<ObjRef>(size) { }
	ObjArray(const ObjArray& src);
	~ObjArray()	{ ObjArray::Empty(); }			// force it to call our version of Empty

	const SharedObj*	GetAt(intptr index) const;
	SharedObj*			GetAt(intptr index);
	bool				SetAt(intptr index, const SharedObj*);
	virtual int			Save(Messenger&, int) const;
	virtual bool		Do(Messenger&, int);
	virtual intptr		Find(const SharedObj* obj) const;
	virtual intptr		Append(const SharedObj* obj);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
};


/*!
 * @class RefArray
 * @param ELEM class of reference
 *
 * @brief Array of references to objects of a particular class.
 *
 * Updating an element in an object array causes the old element
 * to be dereferenced (and deleted if necessary) and replaced with
 * a new value. The new value automatically generates a reference.
 * When an object array is initially created, the object pointers for
 * the ObjRefs are NULL.
 *
 * @ingroup vixen
 * @see Core::RefPtr Core::RefObj ObjArray Array
 */
template <class ELEM> class RefArray : public ObjArray
{
public:
	RefArray(intptr size = 0) : ObjArray(size) { }
	RefArray(const ObjArray& src) : ObjArray(src) { }

	ELEM*			GetAt(intptr index)			{ return (ELEM*) ObjArray::GetAt(index); }
	const ELEM*		GetAt(intptr index) const		{ return (const ELEM*) ObjArray::GetAt(index); }
	intptr			Append(const ELEM* e)			{ return ObjArray::Append((SharedObj*) e); }
	intptr			Merge(const ObjArray& a)		{ return ObjArray::Merge(a); }
	intptr			Find(const ELEM* e) const		{ return ObjArray::Find((SharedObj*) e); }
	bool			SetAt(intptr index, const ELEM* e)	{ return ObjArray::SetAt(index, (SharedObj*) e); }
	Ref<ELEM>&		operator[](intptr index)			{ return (Ref<ELEM>&) ObjArray::operator[](index); }
	const Ref<ELEM>& operator[](intptr index) const		{ return (const Ref<ELEM>&) ObjArray::operator[](index); }
};


#ifdef SWIG
typedef Array<int32>	SharedIntArray;
typedef Array<float>	SharedFloatArray;
%template(SharedIntArray)	Array<int32>;
%template(SharedFloatArray)	Array<float>;
#endif

class IntArray : public Array< int32 >
{
public:
	IntArray(intptr size = 0) : Array< int32 >(size) { }
	VX_DECLARE_CLASS(IntArray);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
};

class FloatArray : public Array< float >
{
public:
	FloatArray(intptr size = 0) : Array< float >(size) { }
	VX_DECLARE_CLASS(FloatArray);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
};

class PtrArray : public Array< void* >
{
public:
	PtrArray(intptr size = 0) : Array<void*>(size) { }
	VX_DECLARE_CLASS(PtrArray);
};



} // end Vixen
