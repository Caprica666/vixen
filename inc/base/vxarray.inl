
#pragma once
#include "scene/vxtypes.h"

namespace Vixen {

template <class ELEM> bool Vixen::Array<ELEM>::Copy(const SharedObj* src_obj)
{
	const Array*	src = (const Array*) src_obj;

	if (!SharedObj::Copy(src_obj))
		return false;
	if (src->IsClass(VX_Array))
	{
		ObjectLock dlock(this);
		ObjectLock slock(src_obj);
		if (!Vixen::Array<ELEM>::SetSize(src->m_Size))
			return false;
		for (intptr i = 0; i < src->m_Size; ++i)
			Vixen::Array<ELEM>::SetAt(i, src->GetAt(i));
	}
	return true;
}

template <class ELEM> int Vixen::Array<ELEM>::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	intptr n = Array::m_Size;

	if ((h <= 0) || (n <= 0))
		return h;
	if (sizeof(ELEM) != sizeof(int32))
		s << OP(VX_Array, ARRAY_SetElemSize) << h << int32(sizeof(ELEM));
	intptr	maxelems = (BufMessenger::MaxBufSize - 3 * sizeof(int32)) / sizeof(ELEM);
	ELEM*	data = (ELEM*) Array::m_Data;
	while (n > 0)
	{
		if (n < maxelems)
			maxelems = n;
		s << OP(VX_Array, ARRAY_Append) << h << int32(maxelems);
		if (sizeof(ELEM) & 3)
			s.Write((char*) data, (int) (maxelems * sizeof(ELEM)));
		else
			s.Output((int32*) data, (int) (maxelems * sizeof(ELEM) / sizeof(int32)));
		n -= maxelems;
		data += maxelems;
	}
	return h;
}

template <class ELEM> bool Vixen::Array<ELEM>::Do(Messenger& s, int op)
{
	int32		n;
	intptr		m;
	ELEM*		data;
	Opcode		o = Opcode(op);	// for debugging

	switch (op)
	{
		case ARRAY_SetElemSize:
		s >> m_ElemSize;
		break;

		case ARRAY_SetData:
		s >> n;							// number of elements
		Array::SetSize(n);						// enlarge array to new size
		if (sizeof(ELEM) & 3)
			s.Read((char*) Array::m_Data, n * m_ElemSize);
		else
			s.Input((int32*) Array::m_Data, n * m_ElemSize / sizeof(int32));
		break;

		case ARRAY_Append:
		s >> n;
		n *= m_ElemSize / sizeof(ELEM);
		m = Array::GetSize();					// number of elements before append
		Array::SetSize(n + m);					// enlarge array to new size
		data = Array::m_Data + m;				// -> after last array element
		if (sizeof(ELEM) & 3)
			s.Read((char*) data, n * sizeof(ELEM));
		else
			s.Input((int32*) data, n * sizeof(ELEM) / sizeof(int32));
		break;

		default:
		return SharedObj::Do(s, op);
	}
	if (s.Debug)
		endl(vixen_debug << Vixen::Array<ELEM>::ClassName() << "::"
					   << ObjArray::DoNames[op - ARRAY_SetData]
					   << " " << this);
    return true;
}


inline intptr ObjArray::Find(const SharedObj* obj) const
{
	ObjRef ref(obj); return Array<ObjRef>::Find(ref);
}

inline intptr ObjArray::Append(const SharedObj* obj)
{
	ObjRef ref(obj); return Array<ObjRef>::Append(ref);
}

inline const SharedObj* ObjArray::GetAt(intptr i) const
{
	if (i < 0 || i >= GetSize())
		return NULL;
	return (const SharedObj*)  Array<ObjRef>::GetAt(i);
}

inline SharedObj* ObjArray::GetAt(intptr i)
{
	if (i < 0 || i >= GetSize())
		return NULL;
	return (SharedObj*) Array<ObjRef>::GetAt(i);
}

} // end Vixen
