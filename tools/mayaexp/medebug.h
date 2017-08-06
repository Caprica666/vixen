#pragma once

//#define	meDebugPrint	vDebugPrint

extern void meDebugPrint(char *format, ...);
extern void meLog(int level, char *format, ...);


#define ME_ERROR(S, E)		{ meDebugPrint S; return E; }
#define ME_WARNING(S)		{ meDebugPrint S; }
#define	ME_TRACE(D, S)		{ if (D) meDebugPrint S; }
#define ME_ERROR_RETURN(S)	{ meDebugPrint S; return; }




