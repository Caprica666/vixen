#include "vcore/vcore.h"
#include <io.h>
#include <fcntl.h>

namespace Vixen
{
#if defined(_M_X64) || defined(_Wp64)
// TODO: Correctly handles 64 bit values
DebugOut& DebugOut::operator<<(intptr v)		{ Core::String s((long) v); Core::DebugPrint(s); return *this; }
#endif

DebugOut& DebugOut::operator<<(const TCHAR* s)	{ Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(TCHAR c)			{ Core::String s(c); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(int v)			{ Core::String s(v); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(float v)			{ Core::String s(v); Core::DebugPrint(s); return *this; }
DebugOut& DebugOut::operator<<(void* p)			{ Core::String s((int) p); Core::DebugPrint(s); return *this; }
DebugOut& endl(DebugOut& out)					{ Core::DebugPrint(TEXT("\n"));  return out; }


static DebugOut error_out;
DebugOut& vixen_debug = error_out;

namespace Core
{
void ErrorPrint(const TCHAR *format, ...)
{
	va_list args;
	TCHAR buffer[VX_MaxLine];

	va_start(args, format);
	if (vsprintf_s(buffer, VX_MaxLine - 4, format, args) < 0)
		return;
	puts(buffer);
	OutputDebugString(buffer);
}

#if defined(_DEBUG) || defined(_SAFE) || defined(_TRACE)
/***
 *
 * Debug printout support
 * The class below extends the generic OSTREAM class
 * to output to the Visual Studio Debug window. By default,
 * all debug printouts will go to the debugger window.
 *
 ***/
void DebugPrint(const TCHAR *format, ...)
{
	va_list args;
	TCHAR	buffer[VX_MaxLine];
	
	va_start(args, format);
	if (vsprintf(buffer, format, args) < 0)
		return;
	puts(buffer);
	OutputDebugString(buffer);
}

#else
void DebugPrint(const TCHAR *format, ...)
{
}
#endif


/*
 * This function makes a console window and sets it up so that
 * anything printed to standard output will go here.
 */

void MakeConsole()
{
	// allocate a console for this app
	AllocConsole();
	// redirect unbuffered STDOUT to the console
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int fileDescriptor = _open_osfhandle((intptr_t) consoleHandle, _O_TEXT);
	FILE *fp = _fdopen(fileDescriptor, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
	fileDescriptor = _open_osfhandle((intptr_t) consoleHandle, _O_TEXT);
	fp = _fdopen(fileDescriptor, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);
}
}	// end Core

}	// end Vixen