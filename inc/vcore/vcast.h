#pragma once

namespace Core {


//===========================================================================
// Type-safe casting operators
//===========================================================================
//
// The following macros provide "safe" casts to derived classes of a pointer
// or reference.  These macros only work for VObjs.  In retail code, they
// simply provide a cast, however, in debug builds they will ASSERT if the
// cast is an illegal one.  These macros should be used wherever potentially 
// unsafe casts are used for BaseObj-derived classes.
//
// NOTE: Calls to ISA_() will still result in a Class structure 
// traversal, even in the retail builds.  This is not a big performance hit,
// just that you should be aware it is not a "free" check, just as with 
// compiler-generated RTTI functions.
//
// The proper usage is as follows:
//
// class Foo : public BaseObj
// {
//		VX_DECLARE_CLASS (Foo);			// in .h file (Foo declaration)
//      ...
// };
//
// IMPLEMENT_CLASS (Foo, BaseObj);			// in .cpp file (Foo implementation)
//    
// void Foo::FooMethod (BaseObj* objPtr, BaseObj& objRef)
// {
//	  bool isFoo   = objPtr->ISA_(Foo);
//    Foo* fooPtr = CAST_(Foo, objPtr);
//    Foo& fooRef = CASTREF_(Foo, objRef);
// }

#if defined(_DEBUG)

// Walk the inheritance chain to check for class description match
inline const BaseObj* Cast (const Class* pClass, const BaseObj* ptr)
{
	// CastPtr() fails since the class type doesn't match the pointer type.  
	if (ptr != NULL)
	{
		// Fix by calling IsKindOf (CLASS_(yourClass)) *before*
		// CastPtr() to ensure type-safety, even in the retail build.
		// ASSERT (ptr->IsKindOf (pClass));
	}
	return (ptr);
}   

// Typesafe cast to handle object conversions
inline const BaseObj* Cast (const Class* pClass, const void* optr)
{ 
	return Cast (pClass, (const BaseObj*)optr); 
}

#define CastPtr(cls, ptr)   ((cls*)  (Cast (CLASS_(cls),  (ptr))))
#define CastRef(cls, ref)   ((cls&) *(Cast (CLASS_(cls), &(ref))))

// C++ Runtime type support - beware, since an attempt to cast to pointer
// yields NULL if the cast cannot be performed, and causes a exception if an 
// attempt is made to cast to a reference that fails.  Note that IsKindOf()
// currently doesn't work for RTTI - need to pass both class *and* pointer
// instead of dereferencing the pointer...
#elif defined (USE_RTTI)	

#define CastPtr(cls, ptr)	dynamic_cast<cls*>(ptr)	
#define CastRef(cls, ptr)	dynamic_cast<cls&>(ptr)

// Retail - just use simple cast operator
#else 

#define CastPtr(cls, ptr)   ((cls*) (ptr))
#define CastRef(cls, ref)   ((cls&) (ref))

#endif

//===========================================================================
// Public macro interface
//===========================================================================

#define ISA_(cls)				IsKindOf (CLASS_(cls))
#define CAST_(cls, ptr)			CastPtr (cls,ptr)
#define CASTREF_(cls, ref)		CastRef (cls,ref)

} // end Core

