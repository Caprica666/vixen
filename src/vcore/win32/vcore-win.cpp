#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

/*
 * All strings of length zero reference the same internal data area _vEmptyStr.
 * String garbage collection relies on _vEmptyStr (the pointer to the empty data area)
 * being the actual data area inside _vThreadData. Do not mess with this.
 *
 * Core::CoreInit sets these up explicitly. You cannot rely on the static constructors in
 * vstring.cpp if your code is part of a DLL.
 */
extern	StringPool*	_vStringPool;
extern	StringData	_vInitThreadData[];
extern	LPCTSTR		_vEmptyStr;
TLSData				TLSData::t_TLSData;

void std_invalid_param(const wchar_t *expr, const wchar_t *func, const wchar_t *file, unsigned int line, uintptr_t unused)
{
	OutputDebugStringW(expr);
}

/*!
 * @fn bool CoreInit()
 *
 * Initializes the core features. After this call, allocators and run-time typing
 * will be available. It should be called before instantiating anything derived
 * from BaseObj.
 *
 * @relates BaseObj
 */
bool _cdecl CoreInit()
{
#ifndef VX_NOTHREAD
	Class* csec = CLASS_(CritSec);
	csec->SetAllocator(TLSData::Get()->GetLockPool());
#endif
	Class::LinkClassHierarchy();
	_set_invalid_parameter_handler(std_invalid_param);
	return true;
}

void _cdecl CoreExit()
{
	Core::NetStream::Shutdown();			// shut down internet session
}


} // end Core
} // end Vixen