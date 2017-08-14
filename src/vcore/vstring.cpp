// Taken from MFC to allow customized allocation strategies - thread and
// temporary heap allocations without synchronization calls like
// InterlockedIncrement()/InterlockedDecrement().

#include "vcore/vcore.h"
#include <stdarg.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#define strinc(s)		s++

// StringData length = string length + zero-termination char + header structure
#define VSTRINGDATA(n)		((((n)+1)*sizeof(TCHAR))+sizeof(StringData))

// // included here to avoid cyclical
// UTIL_EXT_CLASS int UTIL_FormatDouble(double value, int prec, int bIsPercent,
// 									 char *szBuf, int bInsertCommas, char *szCommaDecimal = ",.");



/////////////////////////////////////////////////////////////////////////////
// static class data
namespace Vixen {
namespace Core {
	/*
	 * All strings of length zero reference the same internal data area _vEmptyStr.
	 * String garbage collection relies on _vEmptyStr (the pointer to the empty data area)
	 * being the actual data area inside _vThreadData. Do not mess with this.
	 */
	StringData	_vInitThreadData[2] = { NULL, -1, 0, 0, 0, 0, 0, 0 };
	StringData*	_vThreadData = &_vInitThreadData[0];
	LPCTSTR		_vEmptyStr = _vThreadData->data();

//    void String::CoreInit()
//    {
//        _vInitThreadData[0].pAlloc = new StringPool(GlobalAllocator::Get());
//    }
//
//    void String::CoreFini()
//    {
//        delete StringPool::Get();
//    }
    
	const String& EmptyString()	{ return *((String*) &_vEmptyStr); }

	String operator+(const String& string1, const String& string2)
	{
		String s;
		s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
			string2.GetData()->nDataLength, (const TCHAR*) string2);
		return s;
	}

	String operator+(const String& astring, LPCTSTR lpsz)
	{
	//	VX_ASSERT (lpsz == NULL || AfxIsValidString(lpsz));
		String s;
		s.ConcatCopy(astring.GetData()->nDataLength, (const TCHAR*) astring,
			String::SafeStrlen(lpsz), lpsz);
		return s;
	}

	String operator+(LPCTSTR lpsz, const String& astring)
	{
	//	VX_ASSERT (lpsz == NULL || AfxIsValidString(lpsz));
		String s;
		s.ConcatCopy(String::SafeStrlen(lpsz), lpsz, astring.GetData()->nDataLength,
			astring.m_pchData);
		return s;
	}

	String operator+(const String& string1, TCHAR ch)
	{
		String s;
		s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
		return s;
	}

	String operator+(TCHAR ch, const String& string1)
	{
		String s;
		s.ConcatCopy(1, &ch, string1.GetData()->nDataLength, string1.m_pchData);
		return s;
	}


// Construction/Destruction

String::String(const String& stringSrc)
{
	VX_ASSERT(stringSrc.RefCount() != 0);
	if (stringSrc.RefCount() >= 0)
	{
		m_pchData = stringSrc.m_pchData;
		InterlockInc(&stringSrc.RefCount());
	}
	else
	{
		Init();

		// Optimization: copy only if source is non-empty
		if (stringSrc.GetLength())
			*this = stringSrc.m_pchData;
	}
}


String::~String()
{
	if (GetLength())
	{
		vint32	count = RefCount();
		VX_ASSERT (RefCount() != 0);
		if (InterlockDec (&RefCount()) <= 0)
			FreeData(GetData());
	}
}

void String::Release()
{
	if (GetLength())
	{
		VX_ASSERT(RefCount() != 0);
		if (InterlockDec(&RefCount()) <= 0)
			FreeData(GetData());
		Init();
	}
}

void String::Release(StringData* pData)
{
	if (pData != _vThreadData)
	{
		VX_ASSERT (pData->nRefs != 0);
		if (--pData->nRefs <= 0)
			FreeData(pData);
	}
}

void String::Empty()
{
	if (GetData()->nDataLength == 0)
		return;

	if (RefCount() >= 0)
		Release();
	else
		Init();
	VX_ASSERT (GetData()->nDataLength == 0);
	VX_ASSERT (RefCount() < 0 || GetData()->nAllocLength == 0);
}

void String::CopyBeforeWriteImpl()
{
	// RefCount() checked by CopyBeforeWrite() inline method
	// if (RefCount() > 1)
	{
		StringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(TCHAR));
	}
	VX_ASSERT (RefCount() <= 1);
}

void String::AllocBeforeWrite(size_t nLen, Allocator* dataAlloc)
{
	if (m_pchData)
	{
		StringData* data = GetData();
		if (data)
			if ((data->nRefs > 1) || (nLen > data->nAllocLength) || (data->pAlloc != dataAlloc))
			{
				Release();
				AllocBuffer(nLen);
			}
	}
	else if (nLen > 0)
		AllocBuffer(nLen);
	VX_ASSERT(RefCount() <= 1);
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

void String::AllocCopy(String& dest, size_t nCopyLen, size_t nCopyIndex,
	size_t nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	size_t nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(TCHAR));
	}
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

String::String(LPCTSTR lpsz)
{
	Init();
	AssignCopy(SafeStrlen(lpsz), lpsz, NULL);
}


//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const String&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void String::AssignCopy(size_t nSrcLen, LPCTSTR lpszSrcData, Allocator* dataAlloc)
{
	// Optimization for empty string
	if (nSrcLen == 0)
		Empty();
	else
	{
		AllocBeforeWrite(nSrcLen, dataAlloc);
		GetData()->nDataLength = nSrcLen;
		memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
		m_pchData[nSrcLen] = '\0';		// in case new data is shorter than old
	}
}

const String& String::operator=(const String& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if (RefCount() < 0 ||
			stringSrc.RefCount() < 0 ||
			stringSrc.GetPool() != GetPool())
		{
			// actual copy necessary since one of the strings is locked
			size_t n = stringSrc.GetLength();

			if (n > 0)
				AssignCopy(n, stringSrc.m_pchData, stringSrc.GetPool());
			else
				Empty();
		}
		else
		{
			// can just copy references around
			Release();
			m_pchData = stringSrc.m_pchData;
			InterlockInc(&stringSrc.RefCount());
		}
	}
	return *this;
}

const String& String::operator=(LPCTSTR lpsz)
{
//	VX_ASSERT (lpsz == NULL || AfxIsValidString(lpsz));
	AssignCopy(SafeStrlen(lpsz), lpsz, StringPool::Get());
	return *this;
}



//////////////////////////////////////////////////////////////////////////////
// concatenation

// NOTE: "operator+" is done as friend functions for simplicity
//      There are three variants:
//          String + String
// and for ? = TCHAR, LPCTSTR
//          String + ?
//          ? + String

void String::ConcatCopy(size_t nSrc1Len, LPCTSTR lpszSrc1Data,
	size_t nSrc2Len, LPCTSTR lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new String object

	size_t nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
	}
}

//////////////////////////////////////////////////////////////////////////////
// concatenate in place

void String::ConcatInPlace(size_t nSrcLen, LPCTSTR lpszSrcData)
{
	StringData* pOldData = GetData();
	//  -- the main routine for += operators
	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)
		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (RefCount() > 1 || pOldData->nDataLength + nSrcLen > pOldData->nAllocLength)
	{
		// we have to grow the buffer, use the ConcatCopy routine
		ConcatCopy(pOldData->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		String::Release(pOldData);
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(m_pchData + pOldData->nDataLength, lpszSrcData, nSrcLen * sizeof(TCHAR));
		pOldData->nDataLength += nSrcLen;
		VX_ASSERT (pOldData->nDataLength <= pOldData->nAllocLength);
		m_pchData[pOldData->nDataLength] = '\0';	// needed when contents shift left
	}
}

const String& String::operator+=(LPCTSTR lpsz)
{
//	VX_ASSERT (lpsz == NULL || AfxIsValidString(lpsz));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const String& String::operator+=(TCHAR ch)
{
	if (RefCount() > 1 || GetData()->nDataLength >= GetData()->nAllocLength)
	{
		// lifted code from ConcatInPlace(1, &ch); to avoid duplicate checks.
		// we have to grow the buffer, use the ConcatCopy routine
		StringData* pOldData = GetData();
		ConcatCopy(pOldData->nDataLength, m_pchData, 1, &ch);
		VX_ASSERT (pOldData != NULL);
		String::Release(pOldData);
	}
	else
	{
		// fast concatenation when buffer big enough.  Use short pointer
		// to assign both character and zero-termination character
		unsigned short*	insertAt = (unsigned short*)(m_pchData + GetData()->nDataLength++);
		*insertAt = ch;
	}

	return *this;
}

const String& String::operator+=(const String& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

LPTSTR String::GetBuffer(size_t nMinBufLength)
{
	VX_ASSERT (nMinBufLength >= 0);

	if (RefCount() > 1 || nMinBufLength > GetData()->nAllocLength)
	{
#ifdef _DEBUG
//		// give a warning in case locked string becomes unlocked
//		if (IsAllocatedString() && RefCount() < 0)
//			TRACE0("Warning: GetBuffer on locked String creates unlocked String!\n");
#endif
		// we have to grow the buffer
		StringData* pOldData = GetData();
		size_t nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(TCHAR));
		GetData()->nDataLength = nOldLen;
		String::Release(pOldData);
	}
	VX_ASSERT (RefCount() <= 1);

	// return a pointer to the character storage for this string
	VX_ASSERT (m_pchData != NULL);
	return m_pchData;
}

void String::ReleaseBuffer(size_t nNewLength)
{
	CopyBeforeWrite();  // just in case GetBuffer was not called

	if (nNewLength == -1)
		nNewLength = STRLEN(m_pchData); // zero terminated
//	size_t oldLen = GetData()->nAllocLength;
	VX_ASSERT (nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR String::GetBufferSetLength(size_t nNewLength)
{
	VX_ASSERT (nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}


LPTSTR String::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	RefCount() = -1;
	return lpsz;
}

void String::UnlockBuffer()
{
	VX_ASSERT (RefCount() == -1);
	RefCount() = 1;
}

//---------------------------------------------------------------------------
// void CStr::GetField (unsigned int n, CStr& dstStr, TCHAR delimiter = ',')
//
// Copies the "nth" field from this string into the "dstStr" and then
// strips leading and trailing blanks.
//
// Parameters: n      - The field number to find (0 based).
//             dstStr - The returned string.
//---------------------------------------------------------------------------
void String::GetField (size_t n, String& dstStr, TCHAR delimiter)
{
	TCHAR	delim[2] = {delimiter, TEXT('\0')};
	TCHAR*	token;
	String	buffer = *this;


	token = STRTOK(buffer.GetBuffer(GetLength()), delim);
	while (token && n--)
		token = STRTOK(NULL, delim);

	if (token)
	{
		// Trim leading blanks and quotes
		while (*token == ' ' || *token == '"')
			token++;

		// Trim trailing blanks and quotes
		for (TCHAR* tokEnd = token + STRLEN(token); tokEnd > token && (*tokEnd == ' ' || *tokEnd == '"'); tokEnd--)
			*tokEnd = '\0';

		dstStr = token;
	}
	else
		dstStr.Empty();

	buffer.ReleaseBuffer();
}


//---------------------------------------------------------------------------
// BOOL CStr::GetNextLine ( LPTSTR lpPtr, int *lpnStart, CStr& csStr, char chDelimeter )
//
// Parameter:   lpPtr	   - Entire buffer
//				lpnStart   - Starting position in the buffer, return new position
//			    csStr	   - Result Line
//				chDelimeter - Character use to separate a line
//              bGetNonBlank  - Skip blank and get a NonBlankline
// Return:
//		FALSE	End of Line
//		TRUE	GetNextLine OK
//---------------------------------------------------------------------------
bool String::GetNextLine ( LPCTSTR lpIn,
	size_t *lpnStart,
	String& csStr,
	TCHAR chDelimeter,
	bool bGetNonBlank
)
{
	bool	bDone = false;
	size_t	nLocPos = 0;
	size_t inLen = STRLEN( lpIn );

	// end of buffer
	if( inLen <= *lpnStart )
	{
//      pEventMgr->PostEvent( EM_LOW_LEVEL_INFO, 0, __FILE__, __LINE__, "inLen %d <= *lpnStart %d", inLen, *lpnStart);
		return false;
	}

	while ( 1 )
	{
		LPCTSTR ptr = &lpIn[*lpnStart];
		nLocPos = 0;
		csStr.Empty();
		while ( ptr[nLocPos] != chDelimeter && ptr[nLocPos] != 0 )
		{
			csStr.Insert(nLocPos, ptr[nLocPos] );

			nLocPos++;
		}

		csStr.Insert(nLocPos, TEXT('\0') );
		if ( chDelimeter == 0x0D )
		{
			if ( ptr[nLocPos+1] == 0x0A )
			{
				*lpnStart += (nLocPos+2); // 0d, 0a
			}
		}
		else
			*lpnStart += (nLocPos+1); // 0d, 0a


		if ( 	ptr[nLocPos] == 0 )
			return false;

		if ( !bGetNonBlank ) break;

		if ( ptr[0] != chDelimeter ) break;

	}

	return true;
}


void String::MakeUpper()
{
	CopyBeforeWrite();
	strupr(m_pchData);

}

void String::MakeLower()
{
	CopyBeforeWrite();
	strlwr(m_pchData);
}

void String::SetAt(size_t nIndex, TCHAR ch)
{
	VX_ASSERT (nIndex >= 0);
	VX_ASSERT (nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

String::String(LPCTSTR lpch, size_t nLength)
{
	Init();
	AssignCopy(nLength, lpch, NULL);
}

//////////////////////////////////////////////////////////////////////////////
// Assignment operators

const String& String::operator=(TCHAR ch)
{
	AssignCopy(1, &ch, StringPool::Get());
	return *this;
}


const String& String::operator=(double aNum)
{
	size_t		len;
	LPTSTR	buffer;

	buffer = GetBuffer(32);
	len = SPRINTF(buffer, TEXT("%.2f"), aNum);
	ReleaseBuffer (len);
	return *this;
}

const String& String::operator=(long aNum)
{
	LPTSTR	buffer = GetBuffer(32);
	SPRINTF(buffer, TEXT("%dl"), aNum);
	ReleaseBuffer (-1);
	return *this;
}

const String& String::operator=(unsigned long aNum)
{
	LPTSTR	buffer = GetBuffer(32);
	SPRINTF(buffer, TEXT("%ul"), aNum);
	ReleaseBuffer (-1);
	return *this;
}

const String& String::operator=(int aNum)
{
	LPTSTR	buffer = GetBuffer(32);
	SPRINTF(buffer, TEXT("%d"), aNum);
	ReleaseBuffer (-1);
	return *this;
}


//////////////////////////////////////////////////////////////////////////////
// less common string expressions


//////////////////////////////////////////////////////////////////////////////
// Advanced manipulation

size_t String::Delete(size_t nIndex, size_t nCount /* = 1 */)
{
	size_t nNewLength = GetData()->nDataLength;

	// Optimization: erase entire string
	if (nCount == nNewLength && nIndex == 0)
	{
		CopyBeforeWrite();
		m_pchData[0] = 0;
		GetData()->nDataLength = 0;
	}
	else if (nCount > 0 && nIndex < nNewLength)
	{
		CopyBeforeWrite();
		size_t nBytesToCopy = nNewLength - (nIndex + nCount) + 1;

		memmove(m_pchData + nIndex,
			m_pchData + nIndex + nCount, nBytesToCopy * sizeof(TCHAR));
		GetData()->nDataLength = nNewLength - nCount;
	}

	return nNewLength;
}

size_t String::Insert(size_t nIndex, TCHAR ch)
{
	CopyBeforeWrite();

	size_t nNewLength = GetData()->nDataLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength++;

	if (GetData()->nAllocLength < nNewLength)
	{
		StringData* pOldData = GetData();
		LPTSTR pstr = m_pchData;
		AllocBuffer(nNewLength);
		memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(TCHAR));
		String::Release(pOldData);
	}

	// move existing bytes down
	memmove(m_pchData + nIndex + 1,
		m_pchData + nIndex, (nNewLength-nIndex)*sizeof(TCHAR));
	m_pchData[nIndex] = ch;
	GetData()->nDataLength = nNewLength;

	return nNewLength;
}

size_t String::Insert(size_t nIndex, LPCTSTR pstr)
{
	size_t nInsertLength = SafeStrlen(pstr);
	size_t nNewLength = GetData()->nDataLength;
	if (nInsertLength > 0)
	{
		CopyBeforeWrite();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength;

		if (GetData()->nAllocLength < nNewLength)
		{
			StringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(TCHAR));
			String::Release(pOldData);
		}

		// move existing bytes down
		memmove(m_pchData + nIndex + nInsertLength,
			m_pchData + nIndex,
			(nNewLength-nIndex-nInsertLength+1)*sizeof(TCHAR));
		memcpy(m_pchData + nIndex,
			pstr, nInsertLength*sizeof(TCHAR));
		GetData()->nDataLength = nNewLength;
	}

	return nNewLength;
}

size_t String::Replace(TCHAR chOld, TCHAR chNew)
{
	size_t nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew)
	{
		// otherwise modify each character that matches in the string
		CopyBeforeWrite();
		LPTSTR psz = m_pchData;
		LPTSTR pszEnd = psz + GetData()->nDataLength;
		while (psz < pszEnd)
		{
			// replace instances of the specified character only
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz = strinc(psz);
		}
	}
	return nCount;
}

size_t String::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
{
	// can't have empty or NULL lpszOld

	size_t nSourceLen = SafeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	size_t nReplacementLen = SafeStrlen(lpszNew);

	// loop once to figure out the size of the result string
	size_t nCount = 0;
	LPTSTR lpszStart = m_pchData;
	LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
	LPTSTR lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = STRSTR(lpszStart, lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}
		lpszStart += STRLEN(lpszStart) + 1;
	}

	// if any changes were made, make them
	if (nCount > 0)
	{
		CopyBeforeWrite();

		// if the buffer is too small, just
		//   allocate a new buffer (slow but sure)
		size_t nOldLength = GetData()->nDataLength;
		size_t nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
		if (GetData()->nAllocLength < nNewLength || RefCount() > 1)
		{
			StringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, pOldData->nDataLength*sizeof(TCHAR));
			String::Release(pOldData);
		}
		// else, we just do it in-place
		lpszStart = m_pchData;
		lpszEnd = m_pchData + GetData()->nDataLength;

		// loop again to actually do the work
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = STRSTR(lpszStart, lpszOld)) != NULL)
			{
				size_t nBalance = nOldLength - (lpszTarget - m_pchData + nSourceLen);
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
					nBalance * sizeof(TCHAR));
				memcpy(lpszTarget, lpszNew, nReplacementLen*sizeof(TCHAR));
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += STRLEN(lpszStart) + 1;
		}
		VX_ASSERT (m_pchData[nNewLength] == '\0');
		GetData()->nDataLength = nNewLength;
	}

	return nCount;
}


size_t String::Remove(TCHAR chRemove)
{
	CopyBeforeWrite();

	LPTSTR pstrSource = m_pchData;
	LPTSTR pstrDest = m_pchData;
	LPTSTR pstrEnd = m_pchData + GetData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest = strinc(pstrDest);
		}
		pstrSource = strinc(pstrSource);
	}
	*pstrDest = '\0';
	size_t nCount = pstrSource - pstrDest;
	GetData()->nDataLength -= nCount;

	return nCount;
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

String String::Mid(size_t nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

String String::Mid(size_t nFirst, size_t nCount) const
{
	// out-of-bounds requests return sensible things
	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;

	VX_ASSERT (nFirst >= 0);
	VX_ASSERT (nFirst + nCount <= GetData()->nDataLength);

	// optimize case of returning entire string
	if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
		return *this;

	String dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

String String::Right(int nCount) const
{
	String dest;
	size_t len = GetData()->nDataLength;
	if (nCount < 0)
	{
		nCount = -nCount;
		AllocCopy(dest, 0, -nCount, 0);
		return dest;
	}
	else if ((size_t) nCount >= len)
		return *this;
	AllocCopy(dest, nCount, len - nCount, 0);
	return dest;
}

String String::Left(int nCount) const
{
	String dest;
	size_t len = GetData()->nDataLength;
	if (nCount < 0)
	{
		nCount += (long) len;
		if (nCount < 0)
			nCount = 0;
	}
	else if ((size_t) nCount >= len)
		return *this;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

// strpos equivalent
int String::Find(TCHAR c) const
{
	const TCHAR* tmp = STRCHR(m_pchData, c);
	if (tmp)
	{
		size_t len = tmp - m_pchData;
		VX_ASSERT(len < INT_MAX);
		return (int) len;
	}
	return -1;
}

int String::Find(LPCTSTR str) const
{
	const TCHAR* tmp = STRSTR(m_pchData, str);
	if (tmp)
	{
		size_t len = tmp - m_pchData;
		VX_ASSERT(len < INT_MAX);
		return (int) len;
	}
	return -1;
}

int String::FindNoCase(TCHAR c) const
{
	const TCHAR* haystack = m_pchData;
	TCHAR needle = tolower(c);

	while (*haystack)
	{
		if (tolower(*haystack) == needle)
		{
			size_t len = haystack - m_pchData;
			VX_ASSERT(len < INT_MAX);
			return (int) len;
		}
		++haystack;
	}
	return (int) -1;
}

int String::FindNoCase(LPCTSTR str) const
{
	const TCHAR* haystack = m_pchData;
	const TCHAR* needle = str;
	size_t		n;

	if (haystack == NULL)
		return -1;
	n = STRLEN(needle);
	while (*haystack)
	{
		if (STRNCASECMP(haystack, needle, n) == 0)
		{
			size_t len = haystack - m_pchData;
			VX_ASSERT(len < INT_MAX);
			return (int) len;
		}
		++haystack;
	}
	return -1;
}

int String::ReverseFind(TCHAR c) const
{
	const TCHAR* tmp = STRRCHR(m_pchData, c);
	if (tmp)
	{
		size_t len = tmp - m_pchData;
		VX_ASSERT(len < INT_MAX);
		return (int) len;
	}
	return -1;
}

// strspn equivalent
String String::SpanIncluding(LPCTSTR lpszCharSet) const
{
	size_t s = STRSPN(m_pchData, lpszCharSet);
	VX_ASSERT(s < INT_MAX);
	return Left((int) s);
}

// strcspn equivalent
String String::SpanExcluding(LPCTSTR lpszCharSet) const
{
	size_t s = STRCSPN(m_pchData, lpszCharSet);
	VX_ASSERT(s < INT_MAX);
	return Left((int) s);
}

int String::Compare(LPCTSTR lpsz) const
{
	return STRCMP(m_pchData, lpsz);
}

int String::CompareNoCase(LPCTSTR lpsz) const
{
	return STRCASECMP(m_pchData, lpsz);
}


// formatting (using wsprintf style formatting)
void String::Format(LPCTSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void String::TrimRight(LPCTSTR lpszTargetList)
{
	// find beginning of trailing matches
	// by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != TEXT('\0'))
	{
		if (STRCHR(lpszTargetList, *lpsz) != NULL)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}

void String::TrimRight(TCHAR chTarget)
{
	// find beginning of trailing matches
	// by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (*lpsz == chTarget)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}

void String::TrimRight()
{
	// find beginning of trailing spaces by starting at beginning (DBCS aware)

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (isspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}

void String::TrimLeft(LPCTSTR lpszTargets)
{
	// if we're not trimming anything, we're not doing any work
	if (SafeStrlen(lpszTargets) == 0)
		return;

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;

	while (*lpsz != TEXT('\0'))
	{
		if (STRCHR(lpszTargets, *lpsz) == NULL)
			break;
		lpsz = strinc(lpsz);
	}

	if (lpsz != m_pchData)
	{
		// fix up data and length
		size_t nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}
}

void String::TrimLeft(TCHAR chTarget)
{
	// find first non-matching character

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;

	while (chTarget == *lpsz)
		lpsz = strinc(lpsz);

	if (lpsz != m_pchData)
	{
		// fix up data and length
		size_t nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}
}

void String::TrimLeft()
{
	// find first non-space character

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;

	while (isspace(*lpsz))
		lpsz = strinc(lpsz);

	if (lpsz != m_pchData)
	{
		// fix up data and length
		size_t nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}
}


}	// end Core
}	// end Vixen
