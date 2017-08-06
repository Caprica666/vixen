
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(PtrArray, SharedObj, VX_Array);
VX_IMPLEMENT_CLASSID(IntArray, PtrArray, VX_IntArray);
VX_IMPLEMENT_CLASSID(FloatArray, PtrArray, VX_FloatArray);
VX_IMPLEMENT_CLASSID(ObjArray, Array, VX_ObjArray);
VX_IMPLEMENT_CLASSID(BaseDict, SharedObj, VX_Dict);

static const TCHAR* opnames[] =
	{ TEXT("SetMatrix"), TEXT("SetAt"), TEXT("RemoveAt"), TEXT("Append"), TEXT("SetElemSize"), TEXT("SetSize") };

const TCHAR** ObjArray::DoNames = opnames;

ObjArray::ObjArray(const ObjArray& src) : Array<ObjRef>(src.GetSize())
{
	Array<ObjRef>::Copy((SharedObj*) &src);
}

bool ObjArray::SetAt(intptr i, const SharedObj* obj)
{
	if (obj && IsGlobal())
		GetMessenger()->Distribute(obj, 0);	
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ObjArray, ARRAY_SetAt) << this << int32(i) << obj;
	VX_STREAM_END(  );

	ObjRef ref = obj;
	return Array<ObjRef>::SetAt(i, ref);
}


int ObjArray::Save(Messenger& s, int opts) const
{
	intptr	maxelems = (BufMessenger::MaxBufSize - 3 * sizeof(int32)) / sizeof(RefObj);
	int32	h = SharedObj::Save(s, opts);
	intptr	i, n = GetSize();
	const SharedObj*	obj;

	if ((h < 0) || (n <= 0) || (opts == Messenger::MESS_ClearGlobal))
		return h;
	for (i = 0; i < n; ++i)
	{
		obj = GetAt(i);
		if (obj)
			obj->Save(s, opts);
	}
	if (h <= 0)
		return h;
	while (n > 0)
	{
		int32 start = 0;
		if (n < maxelems)
			maxelems = n;
		s << OP(VX_ObjArray, ARRAY_Append) << h << int32(maxelems);
		for (i = 0; i < maxelems; ++i)
		{
			obj = GetAt(start++);
			s << obj;
		}
		n -= maxelems;
	}
	return h;
}

bool ObjArray::Do(Messenger& s, int op)
{
	int32	i, n;
	SharedObj*	obj;

	switch (op)
	{
		case ARRAY_SetSize:
		s >> n;
		SetSize(n);
		break;

		case ARRAY_SetData:
		Empty();				// replace current data

		case ARRAY_Append:
		s >> n;					// number of object pointers
		for (i = 0; i < n; ++i)
		{
			s >> obj;
			Append(obj);
		}
		break;

		case ARRAY_SetAt:
		s >> n >> obj;
		SetAt(n, obj);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << ObjArray::DoNames[op - ARRAY_SetData]
					   << " " << this);
#endif
	return true;
}

DebugOut& ObjArray::Print(DebugOut& dbg, int opts) const
{
	if ((opts & (PRINT_Data | PRINT_Children)) == 0)
		return SharedObj::Print(dbg);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	for (intptr i = 0; i < m_Size; ++i)
	{
		const SharedObj* obj = GetAt(i);
		if (obj)
			obj->Print(dbg, opts | PRINT_Trailer);
		else
			endl(dbg << "<null/>");
	}
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}

DebugOut&	IntArray::Print(DebugOut& dbg, int opts) const
{
	if ((opts & (PRINT_Data | PRINT_Children)) == 0)
		return SharedObj::Print(dbg);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	if (opts & PRINT_Data)
	{
		int chunk = 8;
		for (intptr i = 0; i < m_Size; ++i)
		{
			int32 j = GetAt(i);
			dbg << j << " " ;
			if (--chunk == 0)
			{
				chunk = 8;
				endl(dbg);
			}
		}
	}
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}

DebugOut&	FloatArray::Print(DebugOut& dbg, int opts) const
{
	if ((opts & (PRINT_Data | PRINT_Children)) == 0)
		return SharedObj::Print(dbg);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	if (opts & PRINT_Data)
	{
		int chunk = 8;
		for (intptr i = 0; i < m_Size; ++i)
		{
			float j = GetAt(i);
			dbg << j << " " ;
			if (--chunk == 0)
			{
				chunk = 8;
				endl(dbg);
			}
		}
	}
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}
}	// end Vixen