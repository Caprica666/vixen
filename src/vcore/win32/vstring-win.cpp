#include "vcore/vcore.h"
#include <tchar.h>

#define strinc(s)		s++

namespace Vixen {
namespace Core {
	
String::String(TCHAR ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		AllocBuffer(nLength);
#ifdef _UNICODE
		for (int i = 0; i < nLength; i++)
			m_pchData[i] = ch;
#else
		memset(m_pchData, ch, nLength);
#endif
	}
}


/////////////////////////////////////////////////////////////////////////////
// Special conversion constructors

void	String::WideToMultiByte(const WCHAR* src, char* dst, int widelen)
{
	int mblen = widelen * 2 + 1;
	::WideCharToMultiByte(CP_ACP, 0, src, widelen, dst, mblen, NULL, NULL);
}

void	String::MultiByteToWide(const char* src, WCHAR* dst, int mblen)
{
	int widelen = mblen;
	::MultiByteToWideChar(CP_ACP, 0, src, mblen, dst, widelen);
}

#ifdef _UNICODE
String::String(LPCSTR lpsz, int nLength)
{
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength);
		int n = ::MultiByteToWideChar(CP_ACP, 0, lpsz, nLength, m_pchData, nLength+1);
		ReleaseBuffer(n >= 0 ? n : -1);
	}
}

String::String(LPCSTR lpsz)
{
	int nLength = lpsz ? strlen(lpsz) : 0;
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength);
		int n = ::MultiByteToWideChar(CP_ACP, 0, lpsz, nLength, m_pchData, nLength+1);
		ReleaseBuffer(n >= 0 ? n : -1);
	}
}

const String& String::operator=(const char* lpsz)
{
	int len = lpsz ? strlen(lpsz) : 0;
	AllocBuffer(len * 2);
	int n = ::MultiByteToWideChar(CP_ACP, 0, lpsz, len, m_pchData, len+1);
	ReleaseBuffer(n >= 0 ? n : -1);
	return *this;
}

void	String::AsMultiByte(char* buffer, size_t len) const
{
	::WideCharToMultiByte(CP_ACP, 0, m_pchData, len, buffer, len+1, NULL, NULL);
}

void	String::AsWide(WCHAR* buffer, size_t len) const
{
	if ((m_pchData == NULL) || (len <= 0))
		*buffer = 0;
	int n = GetLength();
	if (n >= len)
		n = len - 1;
	memcpy(buffer, m_pchData, n);
	buffer[n] = 0;
}

#else // not _UNICODE
String::String(const WCHAR* lpsz, size_t nLength)
{
	Init();
	if (nLength != 0)
	{
		int len = (int) nLength;
		AllocBuffer(nLength*2);
		long n = ::WideCharToMultiByte(CP_ACP, 0, lpsz, len, m_pchData,
									   (len * 2) + 1, NULL, NULL);
		ReleaseBuffer(n >= 0 ? n : -1);
	}
}

void	String::AsMultiByte(char* buffer, size_t len)const
{
	if ((m_pchData == NULL) || (len <= 0))
	{
		*buffer = 0;
		return;
	}
	size_t n = GetLength();
	if (n >= len)
		n = len - 1;
	memcpy(buffer, m_pchData, n);
	buffer[n] = 0;}

void	String::AsWide(WCHAR* buffer, size_t len) const
{
	if (m_pchData == NULL)
		*buffer = 0;
	VX_ASSERT(len < INT_MAX);
	::MultiByteToWideChar(CP_ACP, 0, m_pchData, (int) len, buffer, (int) len + 1);
}

#endif //!_UNICODE


/////////////////////////////////////////////////////////////////////////////
// String formatting

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

void String::FormatV(LPCTSTR lpszFormat, va_list argList)
{
//	VX_ASSERT (AfxIsValidString(lpszFormat));

	va_list argListSave = argList;

	AllocBuffer(VX_MaxPath);
	// make a guess at the maximum length of the resulting string
	size_t nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = strinc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = strinc(lpsz)) == '%')
		{
			nMaxLen += _tclen(lpsz);
			continue;
		}

		size_t nItemLen = 0;

		// handle '%' character with format
		size_t nWidth = 0;
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
			nWidth = _ttoi(lpsz);
			for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = strinc(lpsz))
			;
		}
		VX_ASSERT (nWidth >= 0);

		size_t nPrecision = 0;
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
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = strinc(lpsz))
				;
			}
			VX_ASSERT (nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if (STRNCMP(lpsz, _T("I64"), 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
#if !defined(_X86_) && !defined(_ALPHA_)
			// __int64 is only available on X86 and ALPHA platforms
			VX_ASSERT (FALSE);
#endif
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
				case 'h':
					nModifier = FORCE_ANSI;
					lpsz = strinc(lpsz);
					break;
				case 'l':
					nModifier = FORCE_UNICODE;
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
				va_arg(argList, TCHAR);
				break;
			case 'c'|FORCE_ANSI:
			case 'C'|FORCE_ANSI:
				nItemLen = 2;
				va_arg(argList, char);
				break;
			case 'c'|FORCE_UNICODE:
			case 'C'|FORCE_UNICODE:
				nItemLen = 2;
				va_arg(argList, WCHAR);
				break;

		// strings
			case 's':
				{
					LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
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
#ifndef _UNICODE
					LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
					if (pstrNextArg == NULL)
						nItemLen = 6;  // "(null)"
					else
					{
						nItemLen = wcslen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
#else
					LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
					if (pstrNextArg == NULL)
						nItemLen = 6; // "(null)"
					else
					{
						nItemLen = lstrlenA(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
#endif
				}
				break;

			case 's'|FORCE_ANSI:
			case 'S'|FORCE_ANSI:
				{
					LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
					if (pstrNextArg == NULL)
						nItemLen = 6; // "(null)"
					else
					{
						nItemLen = lstrlenA(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
				}
				break;

			case 's'|FORCE_UNICODE:
			case 'S'|FORCE_UNICODE:
				{
					LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
					if (pstrNextArg == NULL)
						nItemLen = 6; // "(null)"
					else
					{
						nItemLen = wcslen(pstrNextArg);
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
					if (nModifier & FORCE_INT64)
						va_arg(argList, __int64);
					else
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
					VX_ASSERT (FALSE);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	GetBuffer(nMaxLen);
#ifdef _UNICODE
	vswprintf_s(m_pchData, nMaxLen, lpszFormat, argListSave); // verify: <= GetData()->nAllocLength;
#else
	vsprintf_s(m_pchData, nMaxLen, lpszFormat, argListSave); // verify: <= GetData()->nAllocLength;
#endif
	ReleaseBuffer();

	va_end(argListSave);
}

}	// end Core
}	// end Vixen