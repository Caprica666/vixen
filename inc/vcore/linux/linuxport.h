/****
 *
 * Includes platform specific headers needed by the public headers
 * This is the LINUX specific version.
 *
 * Compilation defines:
 *	_DEBUG			set to build DEBUG version
 *	VX_NOTHREAD	disabled threading support
 *
 ****/
#pragma once

#define FREEIMAGE_LIB 1

#define AFX_API_EXPORT
#define WIN_UNIX_STDCALL
#define WIN_UNIX_CDECL
#define VX_MAX_PATH 128
#define VX_MaxPath 128

#include <cassert>
#include <cmath>
#include <cfloat>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <strstream>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket close

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


typedef unsigned int	uint32;     /* 32 bit unsigned */
typedef int				int32;      /* 32 bit signed */
typedef unsigned char   uchar;      /* 8 bit unsigned */
typedef unsigned short  uint16;     /* 16 bit unsigned */
typedef short           int16;      /* 16 bit signed */
typedef unsigned        bits;       /* bit field */
typedef unsigned char	uint8;
typedef unsigned char	ubyte;
typedef char			int8;
typedef long			int64;		/* 64 bit integer */
typedef unsigned long	uint64;		/* 64 bit integer */
typedef void* volatile	vptr;		/* volatile pointer */
typedef int volatile	vint32;		/* volatile 32 bit integer */
typedef long long volatile vint64;	/* volatile 64 bit integer */

#ifdef _LP64
typedef	unsigned __int64 intptr;	/* unsigned int interchangeable with pointer */
typedef __int64			offset_t;
#define	FSEEK	fseeko
#define	FTELL	ftello
#else
typedef	unsigned int	intptr;		/* unsigned int interchangeable with pointer */
typedef long			offset_t;
#define	FSEEK	fseek
#define	FTELL	ftell
#endif





#define	_cdecl	// No need cdecl on Linux
#define __cdecl
typedef int32_t			BOOL;	// FreeImage uses int32_t for this type
typedef unsigned char	BYTE;
typedef unsigned int	DWORD;
typedef unsigned short	WORD;
typedef void*			LPVOID;
typedef long*			LPLONG;
typedef void*			HANDLE;

typedef char			TCHAR;
typedef int32_t			LONG;	// FreeImage uses int32_t for this type
typedef	const char*		LPCSTR;
typedef char*			LPSTR;
typedef	const TCHAR*	LPCTSTR;
typedef TCHAR*			LPTSTR;


/*
 * Single precision math wrappers
 */
#define	cosf(v)		((float) cos(v))
#define	sinf(v)		((float) sin(v))
#define	tanf(v)		((float) tan(v))
#define	asinf(v)	((float) asin(v))
#define	acosf(v)	((float) acos(v))
#define	atanf(v)	((float) atan(v))
#define	sqrtf(v)	((float) sqrt(v))
#define	atan2f(a,b)	((float) atan2(a,b))

//#define	strcasecmp	_strcmpi
//#define	strncasecmp	_strnicmp
#define lstrlen strlen
#define _istspace isspace
#define wcslen strlen
#define _tclen strlen

#define STRLEN		strlen
#define STRCPY		strcpy
#define STRNCPY		strncpy
#define STRCAT		strcat
#define STRNCAT		strncat
#define STRCMP		strcmp
#define STRNCMP		strncmp
#define STRSTR		strstr
#define STRCHR		strchr
#define STRRCHR		strrchr
#define	SPRINTF		sprintf
#define SSCANF		sscanf
#define STRTOK		strtok
#define STRSPN		strspn
#define STRCSPN		strcspn
#define	STRCASECMP	strcasecmp
#define	STRNCASECMP	strncasecmp
#define	strlwr		_strlwr
#define	strupr		_strupr
#define FOPEN		fopen
#define ATOF		atof
#define ATOL		atol
#define STRTOD		strtod


#ifdef VX_USE_CILK
#define	PARALLEL_FOR	cilk_for
#define	PARALLEL_SYNC	cilk_sync
#define PARALLEL_SPAWN	cilk_spawn
#define	PARALLEL_NUMTHREADS	 __cilkrts_get_nworkers()

#else
#define	PARALLEL_FOR	for
#define	PARALLEL_SYNC
#define	PARALLEL_SPAWN
#define	PARALLEL_NUMTHREADS	1
#endif

#define TEXT(_XXX)	_XXX

inline void strupr(char* s)
{ char c; while (c = *s) *s++ = toupper(c); }

inline void strlwr(char* s)
{ char c; while(c = *s) *s++ = tolower(c); }

typedef std::ostream DebugOut;

// TODO: what should this be on Linux?
#define DLL_CALLCONV

#define FORCE_INLINE	inline

#define		VK_ESCAPE	0x1B
#define		VK_END		0x23
#define		VK_HOME		0x24
#define		VK_LEFT		0x25
#define		VK_UP		0x26
#define		VK_RIGHT	0x27
#define		VK_DOWN		0x28
#define		VK_INSERT	0x2D
#define		VK_DELETE	0x2E
#define		VK_F1		0x70
#define		VK_F2		0x71
#define		VK_F3		0x72
#define		VK_F4		0x73
#define		VK_F5		0x74
#define		VK_F6		0x75
#define		VK_F7		0x76
#define		VK_F8		0x77
#define		VK_F9		0x78
#define		VK_F10		0x79

// Check these two also
#define		VK_F11		0x7A
#define		VK_F12		0x7B

// Need to find the correct values for these
#define		VK_PRIOR	0x1
#define		VK_NEXT		0x1
#define		VK_ADD		0x1
#define		VK_SUBTRACT	0x1
#define		VK_CONTROL	0x1
