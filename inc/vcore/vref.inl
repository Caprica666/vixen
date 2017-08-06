/****
 *
 * Inlines for VRef class
 *
 ****/

template <class T> const Ref<T>	Ref<T>::Null;


/*!
 * @fn Ref<T>& Ref<T>::operator=(const T* ptr)
 * @param ptr Pointer to object to reference.
 *
 * The Ref<T> object is a \b smart \b pointer which handles references
 * to class T, which must be derived from RefObj. It uses operator
 * overrides to model a type of pointer which automatically
 * handles reference counting behind the scenes.
 *
 * When a pointer to a T* is assigned into a Ref<T>, it
 * automatically dereferences the old object and garbage collects
 * it if necessary. The reference count on the new object is
 * then incremented. A NULL pointer is legal input and causes the
 * current value to be dereferenced without assigning a new value.
 *
 * The RefObj constructor takes care of initializing the pointer to
 * NULL and the destructor takes care of dereferencing the
 * last value pointed to.
 *
 * @see RefObj::IncUse RefObj
 */
template <class T> Ref<T>& Ref<T>::operator=(const T* ptr)
{
	Core::RefPtr::operator=(ptr);
	return *this;
}

/*!
 * @fn Ref<T>& Ref<T>::operator=(const Ref<T>& src)
 * @param src Reference to copy.
 *
 * When one reference is assigned into another, both references
 * point to the same object. The assignment operation
 * automatically dereferences the old object and garbage collects
 * it if necessary.
 *
 * @see RefObj::IncUse RefObj
 */
template <class T> inline Ref<T>& Ref<T>::operator=(const Ref<T>& src)
	{ Core::RefPtr::operator=(src); return *this; }

template <class T> inline bool Ref<T>::operator==(const Ref<T>& src) const
	{ return src.m_objPtr == m_objPtr; }

template <class T> inline bool Ref<T>::operator!=(const Ref<T>& src) const
	{ return src.m_objPtr != m_objPtr; }

template <class T> inline bool Ref<T>::operator==(const T* p) const
	{ return (const T*) m_objPtr == p; }

template <class T> inline bool Ref<T>::operator!=(const T* p) const
	{ return (const T*) m_objPtr != p; }

template <class T> inline Ref<T>::operator T*() const
	{ return (T*) m_objPtr; }

template <class T> inline Ref<T>::operator const T*() const
	{ return (const T*) m_objPtr; }

template <class T> inline T* Ref<T>::operator->()
	{ return (T*) m_objPtr; }

template <class T> inline const T* Ref<T>::operator->() const
	{ return (const T*) m_objPtr; }

template <class T> inline T* Ref<T>::operator*()
	{ return (T*) m_objPtr; }

template <class T> inline bool Ref<T>::operator!() const
	{ return m_objPtr == NULL; }



