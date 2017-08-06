#pragma once

namespace Vixen {
#define	VX_ASSERT			assert

#ifdef _DEBUG
#define	VX_PRINTF(x)		printf x
#define VX_ERROR(S, E)		{ printf S; return E; }
#define VX_ERROR_RETURN(S)  { printf S; return; }
#define VX_WARNING(S)		{ printf S; }
#define	VX_TRACE(D, S)		{ if (D) printf S; }

	#if _TRACE > 1
	#define	VX_TRACE2(D, S)	{ if (D > 1) printf S; }
	#else
	#define	VX_TRACE2(D, S)
	#endif

#elif defined(_SAFE)
#define	VX_PRINTF(x)		printf x
#define VX_ERROR(S, E)		{ printf S; return E; }
#define VX_ERROR_RETURN(S)  { printf S; return; }
#define	VX_WARNING(s)

#if _TRACE > 0
#define	VX_TRACE(D, S)			{ if (D) printf S; }
	#if _TRACE > 1
		#define	VX_TRACE2(D, S)	{ if (D > 1) printf S; }
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
	void	precision(int) { }
};

extern DebugOut& endl(DebugOut& p);

namespace Core
{
	inline void MakeConsole() { }
}
} // end Vixen
