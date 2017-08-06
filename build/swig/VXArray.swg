/*
 * SWIG interface file for VXArray
 */
#ifdef SWIGCSHARP
%include "VixenCS/VXArray_csharp.swg"
#endif

namespace Vixen {

	namespace Core
	{
		%ignore ARRAY_UserAlloc;
		%ignore BaseArray::operator=;
		%ignore BaseArray::Iter;
		%ignore PtrArray;
		%ignore BaseArray::operator[];
		%ignore BaseArray::GetData;
		%ignore BaseArray::SetData;
		%ignore BaseArray::GetElemAllocator;
		%ignore BaseArray::SetElemAllocator;
	}	// end Core


	%include "vcore/varray.h"

	typedef Core::BaseArray< ObjRef, SharedObj > CoreObjArray;
	typedef Core::BaseArray< int32, SharedObj > CoreIntArray;
	typedef Core::BaseArray< float, SharedObj > CoreFloatArray;

	%template(CoreObjArray)		Core::BaseArray< ObjRef, SharedObj >;
	%template(CoreFloatArray)	Core::BaseArray< float, SharedObj >;
	%template(CoreIntArray)		Core::BaseArray< int32, SharedObj >;

	%ignore PtrArray;
	%ignore ObjArray::Iter;
	%ignore ObjArray::GetAt(int) const;
	%ignore ObjArray::Print;
	%ignore ObjArray::Debug;
}	// end Vixen

%include "base/vxarray.h"

namespace Vixen
{
	typedef IntArray	IndexArray;
	REF_TYPEMAP(IntArrayRef, IntArray)
	REF_DEFINE(IntArrayRef)
	%template(IntArrayRef)		Ref<IntArray>;
}	// end Vixen
