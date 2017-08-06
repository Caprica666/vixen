/*
 * @file winglue.h
 * Glue to make Windows-derived sources compile.
 * Defines common Windows types.
 */
#ifndef WINGLUEH
#define WINGLUEH

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WINDOWS_		// if _WINDOWS_ is not defined, freeimage.h defines BOOL and _WINDOWS_
typedef int				BOOL;
#endif

#ifndef _WINDEF_
#define WINAPI			__stdcall

   /* XXX This is from Win32's <windef.h> */
#  ifndef APIENTRY
#    define APIENTRY    __stdcall
#  endif

   /* XXX This is from Win32's <wingdi.h> and <winnt.h> */
#  ifndef WINGDIAPI
#   define WINGDIAPI __declspec(dllimport)
#  endif

typedef unsigned long	DWORD;
typedef unsigned short	WORD;
typedef void*			LPVOID;
typedef long*			LPLONG;
typedef void*			HANDLE;
#endif // _WINDEF_

#ifndef _WINNT_
typedef char			TCHAR;
typedef long			LONG;
typedef	const char*		LPCSTR;
typedef char*			LPSTR;
typedef	const TCHAR*	LPCTSTR;
typedef TCHAR*			LPTSTR;
typedef	const wchar_t*	LPCWSTR;
#define VOID			void
#endif // _WINNT_

#ifndef WINBASEAPI
#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI		DECLSPEC_IMPORT
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE      64
typedef intptr	SOCKET;
typedef int socklen_t;

typedef struct fd_set
{
	unsigned int	fd_count;				/* how many are SET? */
	SOCKET			fd_array[FD_SETSIZE];   /* an array of SOCKETs */
} fd_set;

#endif

#ifdef __cplusplus
};
#endif


#endif // WINGLUEH
