/*
 * SWIG interface file VCore objects
 */
#define _cdecl

#ifdef SWIGCSHARP
%include "VixenCS/VCore_csharp.swg"
#endif

#ifdef SWIGJAVA
%include "VixenJava/VCore_java.swg"
#endif

%ignore VX_DECLARE_CLASS;
%ignore Opcode;

namespace Vixen
{

	namespace Core
	{
		typedef	BaseObj	List;
		typedef BaseObj	Allocator;
		typedef void	CritSec;
		typedef void*	DebugOut;

		%ignore	DebugOut;
		%ignore Allocator;
		%ignore TLS;
		%ignore	Class;
		%ignore Buffer;
		%ignore	CoreInit;
		%ignore CoreExit;
		%ignore ObjLock;
		%ignore	GetTime;

		%ignore BaseObj::GetInterface;
		%ignore BaseObj::operator new;
		%ignore BaseObj::operator delete;
		%ignore BaseObj::IsHeapAlloc;
		%ignore BaseObj::GetAllocator;
		%ignore BaseObj::GetClassAllocator;
		%ignore BaseObj::IsKindOf;
		%ignore BaseObj::CreateObject;
		%ignore BaseObj::DoNames;
		%ignore BaseObj::Debug;
		%ignore BaseObj::GetClass;
		%ignore BaseObj::ClassInfo;

		%ignore RefObj::IncUse;
		%ignore RefObj::GetUse;
		%ignore RefObj::Delete;
		%ignore LockObj::m_Lock;
		%ignore RefPtr::operator-;
		%ignore RefPtr::operator->;
		%ignore RefPtr::operator RefObj*;

		%rename(Equal)		RefPtr::operator==;
		%rename(NotEqual)	RefPtr::operator!=;
		%rename(Assign)		RefPtr::operator=;

		// Handle RefObj reference counting
		%feature("ref")		RefObj	"$self->AddRef();"
		%feature("unref")	RefObj	"$self->Release();"
		%feature("ref")		LockObj	"$self->AddRef(); $self->MakeLock();"		
	}
}

ATTR_READONLY(Vixen::Core::BaseObj, const char*, ClassName, ClassName);
ATTR_READONLY(Vixen::Core::BaseObj, int, ClassID, ClassID);

namespace Vixen
{
	%include "vcore/vobj.h"
	%include "vcore/vrefptr.h"
	%include "vcore/vlock.h"
	%include "vcore/vref.h"
}

