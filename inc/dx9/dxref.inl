/****
 *
 * Inlines for DXRefPtr and DXRef classes
 *
 ****/

inline DXRefPtr::DXRefPtr(IUnknown* ptr)
{
// COM does AddRef when you get the pointer, don't need to do another here
	if (ptr)
		m_index = FindIndex(ptr);
	else
		m_index = 0;
}

inline DXRefPtr::~DXRefPtr()
{
	Release();
}


inline DXRefPtr& DXRefPtr::operator=(IUnknown* ptr)
{
	int index = m_index;		// save current index
	if (index > 0)
	{
		IUnknown* oldptr = s_DXObjs->GetAt(index);

		if (oldptr == ptr)
			return *this;
		s_DXObjs->Release(index);
		s_DXObjs->SetAt(index, ptr);
	}
	else
		m_index = FindIndex(ptr);
	return *this;
}

inline const IUnknown* DXRefPtr::GetPtr() const
	{ return (m_index > 0) ? (IUnknown*) s_DXObjs->GetAt(m_index) : (IUnknown*) NULL; }

inline IUnknown* DXRefPtr::operator*()
	{ return (IUnknown*) GetPtr(); }

/*!
 * @fn DXRef<T>& DXRef<T>::operator=(const T* ptr)
 * @param ptr Pointer to object to reference.
 *
 * The DXRef<T> object is a \b smart \b pointer which handles references
 * to class T, which must be derived from DXRefPtr. It uses operator
 * overrides to model a type of pointer which automatically
 * handles reference counting behind the scenes.
 * *
 * @see DXRefObj::IncUse DXRefObj
 */
template <class T> DXRef<T>& DXRef<T>::operator=(const T* ptr)
{
	DXRefPtr::operator=((IUnknown*) ptr);
	return *this;
}


template <class T> inline bool DXRef<T>::operator==(const T* p) const
	{ return GetPtr() == (const IUnknown*) p; }

template <class T> inline bool DXRef<T>::operator!=(const T* p) const
	{ return GetPtr() != (const IUnknown*) p; }

template <class T> inline DXRef<T>::operator T*()
	{ return (T*) GetPtr(); }

template <class T> inline DXRef<T>::operator const T*() const
	{ return (const T*) GetPtr(); }

template <class T> inline T* DXRef<T>::operator->()
	{ return (T*) GetPtr(); }

template <class T> inline const T* DXRef<T>::operator->() const
	{ return (const T*) GetPtr(); }

template <class T> inline T* DXRef<T>::operator*()
	{ return (T*) GetPtr(); }




