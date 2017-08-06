
#include "vcore/vcore.h"
#include <stdarg.h>

#define strinc(s)		s++

#ifndef min
	#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
	#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace Vixen {
namespace Core {

String::String(char ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		AllocBuffer(nLength);
		memset(m_pchData, ch, nLength);
	}
}


void	String::AsMultiByte(char* buffer, size_t len) const
{
	strncpy(buffer, m_pchData, len);
}
/////////////////////////////////////////////////////////////////////////////
// String formatting

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

void String::FormatV(LPCTSTR lpszFormat, va_list argList)
{
	va_list argListSave;

	va_copy(argListSave, argList);
	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (const TCHAR* lpsz = lpszFormat; *lpsz != '\0'; lpsz = strinc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = strinc(lpsz)) == '%')
		{
			nMaxLen += strlen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = strinc(lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
					*lpsz == ' ')
			;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz = strinc(lpsz))
			;
		}
		VX_ASSERT (nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = strinc(lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = strinc(lpsz);
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz = strinc(lpsz))
				;
			}
			VX_ASSERT (nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		{
			switch (*lpsz)
			{
			// modifiers that affect size
				case 'h':
					nModifier = FORCE_ANSI;
					lpsz = strinc(lpsz);
					break;
				case 'l':
					lpsz = strinc(lpsz);
					break;

			// modifiers that do not affect size
				case 'F':
				case 'N':
				case 'L':
					lpsz = strinc(lpsz);
					break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
			case 'c':
			case 'C':
				nItemLen = 2;
				va_arg(argList, int);
				break;
			case 'c'|FORCE_ANSI:
			case 'C'|FORCE_ANSI:
				nItemLen = 2;
				va_arg(argList, int);
				break;

		// strings
			case 's':
				{
					const TCHAR* pstrNextArg = va_arg(argList, const TCHAR*);
					if (pstrNextArg == NULL)
						nItemLen = 6;  // "(null)"
					else
					{
						nItemLen = lstrlen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
				}
				break;

			case 'S':
				{
					const char* pstrNextArg = va_arg(argList, const TCHAR*);
					if (pstrNextArg == NULL)
						nItemLen = 6; // "(null)"
					else
					{
						nItemLen = strlen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
				}
				break;

			case 's'|FORCE_ANSI:
			case 'S'|FORCE_ANSI:
				{
					const char* pstrNextArg = va_arg(argList, const char*);
					if (pstrNextArg == NULL)
						nItemLen = 6; // "(null)"
					else
					{
						nItemLen = strlen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
				}
				break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
				case 'd':
				case 'i':
				case 'u':
				case 'x':
				case 'X':
				case 'o':
					va_arg(argList, int);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth+nPrecision);
					break;

				case 'e':
				case 'g':
				case 'G':
					va_arg(argList, double);
					nItemLen = 128;
					nItemLen = max(nItemLen, nWidth+nPrecision);
					break;

				case 'f':
					va_arg(argList, double);
					nItemLen = 128; // width isn't truncated
				// 312 == strlen("-1+(309 zeroes).")
				// 309 zeroes == max precision of a double
					nItemLen = max(nItemLen, 312+nPrecision);
					break;

				case 'p':
					va_arg(argList, void*);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth+nPrecision);
					break;

			// no output
				case 'n':
					va_arg(argList, int*);
					break;

				default:
					VX_ASSERT (false);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	GetBuffer(nMaxLen);
	vsprintf(m_pchData, lpszFormat, argListSave); // verify: <= GetData()->nAllocLength;
	ReleaseBuffer();

	va_end(argListSave);
}

}	// end Core
}	// end Vixen
