#pragma once

namespace Vixen {


template <class T> bool WeakRef<T>::IsNull() const
{
	return (GetPtr() == NULL);
}

template <class T> int WeakRef<T>::MakeRef(const SharedObj* obj)
{
	int			handle = m_Handle;

	if (obj != NULL)
	{
		Messenger*	mess = GetMessenger();
		if (mess == NULL)
			return 0;
		m_Handle = mess->Attach(obj);
	}
	else
		m_Handle = NULL;
	return m_Handle;
}

template <class T> inline T* WeakRef<T>::GetPtr() const
{
	if (m_Handle)
	{
		Messenger*	mess = GetMessenger();
		SharedObj*	obj = mess->GetObj(m_Handle);
		return (T*) obj;
	}
	return NULL;
}

/*!
 * @fn WeakRef<T>& WeakRef<T>::operator=(const T* ptr)
 * @param ptr Pointer to object to reference.
 *
 * The WeakRef<T> object is a \b smart \b pointer which handles references
 * to class T, which must be derived from RefObj. It uses operator
 * overrides to model a type of pointer which automatically
 * detects if the referenced object has been deleted.
 *
 * When a pointer to a T* is assigned into a WeakRef<T>, the
 * Messenger creates a reference to the object. If NULL is assigned,
 * the WeakRef does not reference an object. The Messenger handles
 * garbage collection of the object when the program endds.
 *
 * @see RefObj Messenger::MakeWeakRef
 */
template <class T> inline WeakRef<T>& WeakRef<T>::operator=(const T* ptr)
{
	MakeRef(ptr);
	return *this;
}

template <class T> inline WeakRef<T>& WeakRef<T>::operator=(const WeakRef<T>& srcref)
{
	m_Handle = srcref.m_Handle;
	return *this;
}

template <class T> inline WeakRef<T>::WeakRef(const WeakRef<T>& src)
{
	m_Handle = src.m_Handle;
}

template <class T> inline WeakRef<T>::WeakRef(const T* ptr) : m_Handle(0)
{
	MakeRef(ptr);
}

template <class T> inline bool	WeakRef<T>::operator==(const T* ptr) const
{
	return (GetPtr() == ptr);
}

template <class T> inline bool	WeakRef<T>::operator!=(const T* ptr) const
{
	return (GetPtr() != ptr);
}

template <class T> inline WeakRef<T>::operator T*() const
{
	return GetPtr();
}

template <class T> inline T* WeakRef<T>::operator->()
{
	return GetPtr();
}

template <class T> inline T* WeakRef<T>::operator*()
{
	return GetPtr();
}

template <class T> inline WeakRef<T>::operator const T*() const
{
	return GetPtr();
}

template <class T> inline const T* WeakRef<T>::operator->() const
{
	return GetPtr();
}

}	// end Vixen