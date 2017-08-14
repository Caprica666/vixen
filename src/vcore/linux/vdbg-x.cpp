
#include "vcore/vcore.h"
#include <stdarg.h>

/***
 *
 * Debug printout support
 * By default, all debug printouts will go to the debugger window.
 *
 ***/

namespace Vixen
{
    
DebugOut& DebugOut::operator<<(const TCHAR* s)	{ Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(TCHAR c)			{ Core::String s(c); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(int v)			{ Core::String s(v); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(float v)			{ Core::String s(v); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(void* p)			{ Core::String s((long) p); Core::DebugPrint(s); return *this; }
DebugOut& endl(DebugOut& out)					{ Core::DebugPrint(TEXT("\n"));  return out; }
    


//	DebugOut& vixen_debug = std::cerr;

namespace Core
{

	void MakeConsole()
	{
		#pragma message ("vdbg-x.cpp: MakeConsole: Implement me")
	}

void __cdecl DebugPrint(const TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	fprintf(stderr, format, args);
	fputs("\n", stderr);
}

void __cdecl ErrorPrint(const TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	fprintf(stderr, format, args);
	fputs("\n", stderr);
}
}
}
