#pragma once


#ifndef VX_ERROR

namespace Core
{
	// Formatted debug print without newline
	extern void DebugPrint(const TCHAR *format, ...);

	// Formatted debug print with appended newline
	extern void ErrorPrint(const TCHAR *format, ...);

	// Make console window for output
	extern void MakeConsole();
}

#define	VX_ASSERT			assert

#if defined(_DEBUG)
#define	VX_PRINTF(x)		Core::DebugPrint x
#define VX_ERROR(S, E)		{ Core::ErrorPrint S; return E; }
#define VX_ERROR_RETURN(S)  { Core::ErrorPrint S; return; }
#define VX_WARNING(S)		{ Core::ErrorPrint S; }
#define	VX_TRACE(D, S)		{ if (D) Core::ErrorPrint S; }

	#if _TRACE > 1
	#define	VX_TRACE2(D, S)	{ if (D > 1) Core::ErrorPrint S; }
	#else
	#define	VX_TRACE2(D, S)
	#endif

#elif defined(_SAFE)
#define	VX_PRINTF(x)		Core::DebugPrint x
#define VX_ERROR(S, E)		{ Core::ErrorPrint S; return E; }
#define VX_ERROR_RETURN(S)  { Core::ErrorPrint S; return; }
#define	VX_WARNING(s)

#ifdef _TRACE
#define	VX_TRACE(D, S)			{ if (D) Core::ErrorPrint S; }
	#if _TRACE > 1
		#define	VX_TRACE2(D, S)	{ if (D > 1) Core::ErrorPrint S; }
	#else
		#define	VX_TRACE2(D, S)
	#endif
#else
	#define	VX_TRACE(D, S)
	#define	VX_TRACE2(D, S)
#endif

#else	// no debug

#define	VX_PRINTF(x)
#define	VX_WARNING(s)
#define	VX_TRACE(D, S)
#define	VX_TRACE2(D, S)
#define VX_ERROR(s, e)		{ return e; }
#define	VX_ERROR_RETURN(S)	{ return; }
#endif

class DebugOut
{
public:
	DebugOut() { }
	DebugOut& operator<<(const TCHAR* s);
	DebugOut& operator<<(TCHAR c);
	DebugOut& operator<<(int v);
	DebugOut& operator<<(float v);
	DebugOut& operator<<(void* p);
#if defined(_M_X64) || defined(_Wp64)
	DebugOut& operator<<(intptr v);
#endif
	void	precision(int) { }
};

extern DebugOut& endl(DebugOut& p);


#endif	// endif VX_ERROR