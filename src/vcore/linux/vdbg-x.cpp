
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

	DebugOut& vixen_debug = std::cerr;

namespace Core
{

	void MakeConsole()
	{
		#pragma message ("vdbg-x.cpp: MakeConsole: Implement me")
	}

void DebugPrint(const TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	fprintf(stderr, format, args);
	fputs("\n", stderr);
}

void ErrorPrint(const TCHAR *format, ...)
{
	va_list args;

	va_start(args,format);
	fprintf(stderr, format, args);
	fputs("\n", stderr);
}
}
}
