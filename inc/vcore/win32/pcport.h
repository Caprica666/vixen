/****
 *
 * Includes platform specific headers needed by the public headers
 * This is the PC specific version.  Much of the content of this
 * file must be consistent with whay is in linuxport.h for Linux/UNIX
 *
 *	_DEBUG		set to build DEBUG version
 *	VX_NOTHREAD	disabled threading support
 *
 ****/
#pragma once

#ifndef STRICT
#define	STRICT
#endif


/*
 * Microsoft Windows defines GetProp, RemoveProp and LoadImage
 * as macros. We undefine them here because they
 * are used by this module in the public API
 */
#ifdef GetProp
#undef GetProp
#endif

#ifdef RemoveProp
#undef RemoveProp
#endif

#ifdef LoadImage
#undef LoadImage
#endif

#define VX_MAX_PATH _MAX_PATH
#define VX_MaxPath _MAX_PATH

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows XP
#define _WIN32_WINNT 0x0500     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef ALIGN16
#define ALIGN16 __declspec(align(16))
#endif

#include <cassert>
#include <cmath>
#include <cfloat>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <strstream>
#include <new>
#include <errno.h>
#include <xmmintrin.h>

#undef THREAD_LOCAL
#ifndef VX_NOTHREAD
#ifdef VX_PTHREAD
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#endif
#define	THREAD_LOCAL	static
#else
#define	THREAD_LOCAL	static __declspec(thread)
#endif

typedef unsigned __int32 uint32;    /* 32 bit unsigned */
typedef __int32			int32;      /* 32 bit signed */
typedef unsigned char   uchar;      /* 8 bit unsigned */
typedef unsigned __int16  uint16;	/* 16 bit unsigned */
typedef __int16			int16;      /* 16 bit signed */
typedef unsigned        bits;       /* bit field */
typedef unsigned __int8	uint8;
typedef char			int8;
typedef __int64			int64;		/* 64 bit integer */
typedef volatile int	vint32;		/* volatile 32 bit */
typedef volatile __int64 vint64;	/* volatile 64 bit */
typedef void*			voidptr;	/* void pointer */
typedef int				socklen_t;
typedef volatile unsigned int	vuint32;	/* volatile unsigned 32 bit */
typedef volatile unsigned __int64	vuint64;/* volatile unsigned 64 bit */

#if defined(_M_X64) || defined(_Wp64)
typedef	__int64 intptr;	/* unsigned int interchangeable with pointer */
typedef __int64	offset_t;
#define	FSEEK	_fseeki64
#define	FTELL	_ftelli64
#else
typedef	int		intptr;		/* unsigned int interchangeable with pointer */
typedef long	offset_t;
#define	FSEEK	fseek
#define	FTELL	ftell
#endif

#ifdef _UNICODE
#define STRCPY wcscpy
#define STRCAT wcscat
#define STRNCPY wcsncpy
#define STRLEN wcslen
#define STRNCAT wcsncat
#define STRCMP wcscmp
#define STRNCMP wcsncmp
#define STRSTR wcsstr
#define STRCHR wcschr
#define STRRCHR wcsrchr
#define SPRINTF wsprintf
#define SSCANF swscanf
#define STRTOK wcstok
#define STRSPN wcsspn
#define STRCSPN wcscspn
#define	STRCASECMP	_wcsicmp
#define	STRNCASECMP	_wcsnicmp
#define	strlwr		_wcslwr
#define	strupr		_wcsupr

#define FOPEN		_wfopen
#define FPUTS		_wfputs
#define ATOF		_wtof
#define ATOL		_wtol
#define STRTOD		wcstod

#else
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
#define	STRCASECMP	_strcmpi
#define	STRNCASECMP	_strnicmp
#define	strlwr		_strlwr
#define	strupr		_strupr

#define FOPEN		fopen
#define ATOF		atof
#define ATOL		atol
#define STRTOD		strtod
#endif
#define VX_NUM_THREADS	4
#define VX_CACHELINE	64

#ifdef VX_USE_CILK
#include <cilk/cilk.h>

#else
#define cilk_for	for
#define	cilk_sync
#define	cilk_spawn

#ifdef _OPENMP
#include <omp.h>

#define PARALLEL_FOR(_N) parallel for num_threads(VX_NUM_THREADS) if (_N >= 64)
#endif
#endif

#if defined(_DEBUG) || defined(_SAFE)
#define	FORCE_INLINE	inline
#else
#define FORCE_INLINE __forceinline
#endif



