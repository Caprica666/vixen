//***************************************************************************
//
//		File:	VBase.h - Standard Declarations
//
//***************************************************************************

#pragma once


//===========================================================================
// SECTION: global #defines
//===========================================================================

/*
 * Compilation Options
 */
#ifndef TRUE
#define TRUE	true
#endif

#ifndef FALSE
#define FALSE	false
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef PI
#define PI 3.14159265358979f
#endif

#ifndef min
	#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
	#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// _declspec(export) #defines go here

//===========================================================================
// SECTION: Standard Types
//===========================================================================

// TODO: collapse with next section

typedef unsigned long		uint32;     /* 32 bit unsigned */
typedef long				int32;      /* 32 bit signed */
typedef unsigned char		uchar;      /* 8 bit unsigned */
typedef unsigned short		uint16;     /* 16 bit unsigned */
typedef short				int16;      /* 16 bit signed */
typedef unsigned			bits;       /* bit field */
typedef unsigned char		uint8;
typedef unsigned char		ubyte;
typedef char				int8;
typedef __int64				int64;		/* 64 bit integer */
#ifdef _M_IA64
typedef	unsigned __int64	intptr;		/* unsigned int interchangeable with pointer */
#else
typedef	unsigned int		ulong;		/* unsigned int interchangeable with pointer */
#endif

// Bytes: 
typedef char				Char;		// Single character
typedef unsigned char		Uchar;		// Single, unsigned character

// Shorts: 
typedef short				Short;
typedef unsigned short		Ushort;

// Longs: 
typedef long				Long;
typedef unsigned long		Ulong;

// Ints: 
typedef int                 Int;
typedef unsigned int		Uint;

// const-ness
typedef const char			ConstChar;
typedef const Str			ConstStr;

// Flags:
typedef Short 				Fs;
typedef Long 				Fl;
typedef Int 				Flags;		// Type Int to be like enums.

#define FS_None 			0
#define FL_None 			0

// size
typedef size_t				Size_t;


//===========================================================================
//	Global operator new() and delete()
//===========================================================================

// placement new operator
inline void* _cdecl operator new (size_t size, void* where);
void*		_cdecl operator new(size_t size);
void		_cdecl operator delete(void* p);

// For debugging - passes __FILE__ and __LINE__
// #if defined (_DEBUG)
// void*		_cdecl operator new (unsigned int size, int, const char *, int);
// #endif


