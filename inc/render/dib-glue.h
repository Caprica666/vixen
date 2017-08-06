//
// This file contains definitions required to operate on DIBs
// without including the MS headers.
//
#ifndef _WINGDI_

typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;


// BITMAPFILEHEADER is a tricky structure, because it will not function
// perfectly if the compiler pads each field to a 4-byte boundary.
#if _WIN32
#include <pshpack2.h>
#endif

namespace Vixen {

typedef struct tagBITMAPFILEHEADER {
	word    bfType;
	dword   bfSize;
	word    bfReserved1;
	word    bfReserved2;
	dword   bfOffBits;
} BITMAPFILEHEADER;

#if _WIN32
#include <poppack.h>
#endif

/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#define RGB(r,g,b)          ((dword)(((byte)(r)|((word)((byte)(g))<<8))|(((dword)(byte)(b))<<16)))

#define GetRValue(rgb)      ((byte)(rgb))
#define GetGValue(rgb)      ((byte)(((word)(rgb)) >> 8))
#define GetBValue(rgb)      ((byte)((rgb)>>16))

#endif	// _WINGDI

#if !defined(_WINDOWS_) && !defined(FREEIMAGE_IO)

typedef struct tagBITMAPINFOHEADER{
	dword	biSize;
	long	biWidth;
	long	biHeight;
	word	biPlanes;
	word	biBitCount;
	dword	biCompression;
	dword	biSizeImage;
	long	biXPelsPerMeter;
	long	biYPelsPerMeter;
	dword	biClrUsed;
	dword	biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	byte	rgbBlue;
	byte	rgbGreen;
	byte	rgbRed;
	byte	rgbReserved;
} RGBQUAD;

} // end Vixen

#endif	// _WINDOWS
