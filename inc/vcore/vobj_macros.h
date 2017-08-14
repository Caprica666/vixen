#pragma once

/*
 * @def CLASS_REGISTER(classname)
 * Forces a class to be accessed so the linker will pull in its implementation.
 * Usually this occurs when you read a file which accesses classes which
 * your program does not explictly call.
 */

/*!
 * @def CLASS_(classname) 
 * Returns a pointer to class structure containing runtime information.
 * Can be used as a parameter to IsKindOf(), type compares, and for comparison 
 * purposes when walking the inheritance chain.  
 */
#define CLASS_(localClass) (&localClass::ClassInfo)

/*!
 * @def VX_DECLARE_CLASS(classname)
 * Declares members necessary for run-time type checking.
 * Place inside class declaration at the top.
 */
#define VX_DECLARE_CLASS(localClass)							\
public:															\
	void* operator new(size_t, void*);							\
	void* operator new(size_t, Core::Allocator* = NULL);		\
	virtual Core::Class* GetClass() const;						\
	static	Core::Class ClassInfo;								\
	static const TCHAR** DoNames;								\
 	static	Core::BaseObj* CreateObject(Core::Allocator* = NULL)

/*!
 * @def VX_IMPLEMENT_CLASS(classname, parentname)
 * Initializes static class information structure.
 * Defines compile-time information for the class.
 * This should be included once in the file which contains the implementation
 * of the class. Mutually exclusive with IMPLEMENT_CLASSID.
 */
#define VX_IMPLEMENT_CLASS(localClass, baseClass)							\
	void* localClass::operator new(size_t n, void* p) { return p; }			\
	void* localClass::operator new(size_t n, Core::Allocator* a)			\
		{  if (a == NULL) a = ClassInfo.GetAllocator();						\
		   VX_TRACE(Core::BaseObj::Debug, ("%s(%d)\n", ClassInfo.GetName(), n));\
		   return Core::BaseObj::operator new(n, a); }						\
 	Core::BaseObj* localClass::CreateObject(Core::Allocator* a)				\
 		{ return new (a) localClass(); }									\
	Core::Class* localClass::GetClass() const 								\
		{ return &localClass::ClassInfo; }									\
	Core::Class localClass::ClassInfo = {										\
		TEXT(#localClass), sizeof(localClass), 0, localClass::CreateObject, &baseClass::ClassInfo, \
		Core::Class::LinkClass(&localClass::ClassInfo, &baseClass::ClassInfo), NULL, NULL }

/*!
 * @def VX_IMPLEMENT_CLASSID(classname, parentname, serialid)
 * Initializes static class information structure, including serial ID for the class.
 * This should be included once in the file which contains the implementation
 * of the class. Mutually exclusive with IMPLEMENT_CLASS.
 */
#define VX_IMPLEMENT_CLASSID(localClass, baseClass, clsid)					\
	void* localClass::operator new(size_t n, void* p) { return p; }			\
	void* localClass::operator new(size_t n, Core::Allocator* a)			\
		{  if (a == NULL) a = ClassInfo.GetAllocator();						\
		   VX_TRACE(Core::BaseObj::Debug, ("%s(%d)\n", ClassInfo.GetName(), n));\
		   return Core::BaseObj::operator new(n, a); }						\
 	Core::BaseObj* localClass::CreateObject(Core::Allocator* a)				\
 		{ return new (a) localClass(); }									\
	Core::Class* localClass::GetClass() const 								\
		{ return &localClass::ClassInfo; }									\
	Core::Class localClass::ClassInfo = {										\
		TEXT(#localClass), sizeof(localClass), clsid, localClass::CreateObject, &baseClass::ClassInfo, \
		Core::Class::LinkClass(&localClass::ClassInfo, &baseClass::ClassInfo), NULL, NULL }
