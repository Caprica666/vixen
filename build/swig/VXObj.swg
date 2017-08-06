/*
 * SWIG interface file for SharedObj
 */
#ifdef SWIGCSHARP
%include "VixenCS/VXObj_csharp.swg"
#endif

#ifdef SWIGJAVA
%include "VixenJava/VXObj_java.swg"
#endif

%define REF_DEFINE(REFTYPE)
	%naturalvar REFTYPE;	
	%ignore REFTYPE::operator-;
	%ignore REFTYPE::operator*;
	%ignore REFTYPE::REFTYPE(Core::RefObj*);
	%rename(IsEqual)	REFTYPE::operator==;
	%rename(NotEqual)	REFTYPE::operator!=;
	%rename(Assign)		REFTYPE::operator=;
%enddef

%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::GetClass$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::OnEvent$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::Print$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::ClassInfo$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::Do$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::Opcode$";
%rename("$ignore", regextarget=1, fullname=1, %$isfunction) "Vixen::.*::DoNames$";

namespace Vixen
{
	%extend SharedObj
	{
		void Dump()
		{
			$self->Print();
		}

		void Dump(const TCHAR* options)
		{
			int opts = SharedObj::PRINT_Default;
			if (STRSTR(options, TEXT("data")))
				opts |= SharedObj::PRINT_Data;
			$self->Print(vixen_debug, opts);
		}
	}
	%ignore Class;
	%ignore CLASS_;
	%ignore vixen_cls;
	%ignore	Property;
	%ignore ObjectLock;
	%ignore	REGISTER_CLASS;
		
	%ignore	SharedObj::ShareClass;
	%ignore	SharedObj::PrintLevel;
	%ignore	SharedObj::AddProp;
	%ignore	SharedObj::DelProp;
	%ignore	SharedObj::GetProp;
	%ignore	SharedObj::RemoveProp;
	%ignore SharedObj::PrintOpts;
	%ignore SharedObj::Print;
	%ignore SharedObj::Save;
	%ignore SharedObj::ObjFlags;
	%ignore SharedObj::operator=;
	%ignore SharedObj::SetName(NameProp&);
	%ignore SharedObj::SetName(const char*, int);

	%newobject	SharedObj::Clone;
	%newobject	SharedObj::Create;

	REF_TYPEMAP(ObjRef, SharedObj)
} // end Vixen

ATTR_DEFINE(Vixen::SharedObj, const char*, Name);
ATTR_DEFINE(Vixen::SharedObj, int, ID);
ATTR_DEFINE(Vixen::SharedObj, bool, Active, IsActive);
ATTR_DEFINE(Vixen::SharedObj, bool, Changed, HasChanged);

%include "base/vxobj.h"

namespace Vixen
{
	REF_DEFINE(ObjRef)
	%template(ObjRef)		Ref< SharedObj >;
}







