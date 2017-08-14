#include "vcore/vcore.h"

#pragma once

/*!
 * @file vstring.h
 * @brief String manipulation classes.
 *
 * Compatible with MFC CString API but provides custom thread- and
 * temporary-heap based allocation strategies.
 *
 * All shadowed methods are funneled into the String class to intercept
 * any changes to the underlying buffer involving reference counting or
 * copy-on-write behaviors.
 *
 * @ingroup vcore
 * @author Kim Kinzie
 * @author API compatible with CString from the Microsoft Foundation Classes C++ library.
 */
namespace Core {

class Allocator;

/*!
 * @brief Internal representation of Core::String
 * @ingroup vcore
 * @internal
 */
struct StringData
{
	Allocator*  pAlloc;
	vint32      nRefs;
	short       nDataLength;
	short       nAllocLength;

	TCHAR* data() { return (TCHAR*)(this+1); }
};


/*!
 * @class String
 * @brief MFC-style string class with optimized implementation.
 *
 * @ingroup vcore
 * @nosubgrouping
 */
class String
{
	friend class StringPool;
	friend String operator+(const String&, const String&);
	friend String operator+(const String&, LPCTSTR);

public:
//! @name Constructors
//@{
	String();							//!< constructs empty String
	String(const String& stringSrc);	//!< copy constructor
	String(TCHAR ch, int nRepeat = 1);	//!< from a single character
	String(const TCHAR* lpsz);			//!< from an ANSI string (converts to TCHAR)
	String(const TCHAR* lpch, size_t nLength);	//!< subset of characters from an ANSI string
	String(const unsigned char* psz);	//!< from unsigned characters
	~String();	
#if defined(_WIN32)
#ifdef _UNICODE
	String(const char* lpsz);			//!< from an ANSI string (converts to TCHAR)
	String(const char* lpsz, int nLength);
#else
	String(const wchar_t* wcharBuffer, size_t nLength);
#endif
	static void	WideToMultiByte(const wchar_t* wcharBuffer, char* charBuffer, int widelen);
	static void	MultiByteToWide(const char* charBuffer, wchar_t* wcharBuffer, int mblen);
#endif
	void	AsMultiByte(char* charBuffer, size_t maxlen) const;
	void	AsWide(wchar_t* wcharBuffer, size_t maxlen) const;
	// for constructing from user-allocated memory
	void*	operator new(size_t size, void* p)	{ return p; }
	void*	operator new(size_t size)				{ return new char[size]; }

//@}

//! @name Numeric and case conversions
//@{
    String(double aNum);						//!< Converts double to string
    String(float aNum);							//!< Converts float to string
    String(long aNum);							//!< Converts long to string
    String(int aNum);							//!< Converts int to string
    String(short aNum);							//!< Converts short to string
    String(unsigned long aNum);					//!< Converts ulong to string
    String(unsigned int aNum);					//!< Converts uint to string
    String(unsigned short aNum);				//!< Converts ushort to string
    double			toDouble() const;			//!< Converts string to double.
    float			toFloat() const;			//!< Converts string to float.
    long			toLong() const;				//!< Converts string to long.
    int				toInt() const;				//!< Converts string to int.
    short			toShort() const;			//!< Converts string to short.
    unsigned long	toULong() const;			//!< Converts string to ulong.
    unsigned int	toUInt() const;				//!< Converts string to uint.
    unsigned short	toUShort() const;			//!< Converts string to ushort.
	void MakeUpper();							//!< NLS aware conversion to uppercase
	void MakeLower();							//!< NLS aware conversion to lowercase
//@}

//! @name Accessors
//@{
	size_t	GetLength() const;					//!< Returns number of characters.
	bool	IsEmpty() const;					//!< Returns  true if zero length.
	void	Empty();							//!< Clear contents to empty.
 	TCHAR	GetAt(size_t nIndex) const;			//!< Returns single character at zero-based index.
	TCHAR	operator[](size_t nIndex) const;	//!< Returns single character at zero-based index.
	TCHAR&	operator[](size_t nIndex);			//!< Returns single character at zero-based index.
	void	SetAt(size_t nIndex, TCHAR ch);		//!< Set a single character at zero-based index.
	operator LPCTSTR() const;					//!< Returns pointer to const string.
//@}

//! @name Assignment
//@{
	const String& operator=(const String& stringSrc);	//!< Ref-counted copy from another String.
	const String& operator=(TCHAR ch);					//!< Set string content to single character.
	const String& operator=(LPCTSTR lpsz);				//!< Copy from ANSI string (converts to TCHAR).
	const String& operator=(const unsigned char* psz);	//!< Copy from unsigned chars.
#if defined(_WIN32) && defined(_UNICODE)
	const String& operator=(const char* lpsz);
#endif

    const String& operator=(double aNum);				//!< Convert string to double.
    const String& operator=(float aNum);				//!< Convert string to float.
    const String& operator=(long aNum);					//!< Convert string to long.
    const String& operator=(int aNum);					//!< Convert string to int.
    const String& operator=(short aNum);				//!< Convert string to short.
    const String& operator=(unsigned long aNum);		//!< Convert string to long.
    const String& operator=(unsigned int aNum);			//!< Convert string to uint.
    const String& operator=(unsigned short aNum);		//!< Convert string to ushort.
//@}

//! @name Comparison
//@{
 	int		Compare(LPCTSTR lpsz) const;				//!< Straight character comparison.
 	int		CompareNoCase(LPCTSTR lpsz) const;			//!< Compare ignoring case.
//@}

//! @name Substring
//@{
	String Mid(size_t nFirst, size_t nCount) const;	//!< Returns nCount characters starting at zero-based nFirst.
	String Mid(size_t nFirst) const;					//!< Returns all characters starting at zero-based nFirst.
	String Left(int nCount) const;						//!< Returns first nCount characters in string.
	String Right(int nCount) const;						//!< Returns nCount characters from end of string.
	String SpanIncluding(LPCTSTR lpszCharSet) const;	//!< characters from beginning that are also in passed string
	String SpanExcluding(LPCTSTR lpszCharSet) const;	//!< characters from beginning that are not also in passed string
//@}

//! @name Trimming
//@{
	void TrimRight();									//!< Remove whitespace starting from right edge.
	void TrimLeft();									//!< Remove whitespace starting from left side .
	void TrimRight(TCHAR chTarget);						//!< Remove continuous occurrences of chTarget starting from right.
	void TrimRight(LPCTSTR lpszTargets);				//!< Remove continuous occcurrences of characters in passed string,starting from right.
	void TrimLeft(TCHAR chTarget);						//!< Remove continuous occurrences of chTarget starting from left.
	void TrimLeft(LPCTSTR lpszTargets);					//!< Remove continuous occcurrences of characters in passed string, starting from left.
//@}

//! @name Concatenation
//@{
	const String& operator+=(const String& string);	//!< Concatenate from another String.
	const String& operator+=(TCHAR ch);				//!< Concatenate a single character.
	const String& operator+=(LPCTSTR lpsz);			//!< Concatenate a UNICODE character after converting it to TCHAR.
//! Append to string.
	friend String operator+(const String& string1, const String& string2);
	friend String operator+(const String& string, TCHAR ch);
	friend String operator+(TCHAR ch, const String& string);
	friend String operator+(const String& string, LPCTSTR lpsz);
	friend String operator+(LPCTSTR lpsz, const String& string);
//@}

//! @name Manipulation
//@{
	size_t Replace(TCHAR chOld, TCHAR chNew);			//!< Replace occurrences of chOld with chNew.
	size_t Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);	//!< Replace occurrences of substring lpszOld with lpszNew; empty lpszNew removes instances of lpszOld.
	size_t Remove(TCHAR chRemove);						//!< Remove occurrences of chRemove.
	size_t Insert(size_t nIndex, TCHAR ch);				//!< Insert character at zero-based index; concatenates if index is past end of string.
	size_t Insert(size_t nIndex, LPCTSTR pstr);			//!< Insert substring at zero-based index; concatenates if index is past end of string.
	size_t Delete(size_t nIndex, size_t nCount = 1);	//!< Relete nCount characters starting at zero-based index.
//@}

//! @name Buffer Access
//@{
	LPTSTR GetBuffer(size_t nMinBufLength);				//!< Get pointer to modifiable buffer at least as long as nMinBufLength.
	void   ReleaseBuffer(size_t nNewLength = -1);		//!< Release buffer, setting length to nNewLength (or to first nul if -1).
	LPTSTR GetBufferSetLength(size_t nNewLength);		//!< Get pointer to modifiable buffer exactly as long as nNewLength.

	// Use LockBuffer/UnlockBuffer to turn refcounting off
	LPTSTR LockBuffer();								// turn refcounting back on
	void   UnlockBuffer();								// turn refcounting off
//@}

//! @name Searching
//@{
	int	 Find(TCHAR c) const;							// return index of first occurence of a character
	int	 Find(LPCTSTR str) const;						// return index of first occurence of a string
	int	 FindNoCase(TCHAR c) const;						// return index of first occurence of a character (case-insensitive)
	int	 FindNoCase(LPCTSTR str) const;					// return index of first occurence of a string (case-insensitive)
	int	 ReverseFind(TCHAR c) const;					// return index of last occurence of a character
	void GetField (size_t n, String& dstStr, TCHAR delimiter = TEXT(','));	//!< CStr compatability - retrieve nth delimited field
	bool GetNextLine(const TCHAR* lpPtr, size_t *lpnStart, String& csStr, TCHAR chDelimeter, bool bGetNonBlankLine);
//@}

//! @name Formatting
//@{
	void Format(LPCTSTR lpszFormat, ...);		//!< Printf-like formatting using passed string.
	void FormatV(LPCTSTR lpszFormat, va_list argList);	//!< Printf-like formatting using variable arguments parameter.
//@}
    
    static void CoreInit();
    static void CoreFini();

protected:
	LPTSTR m_pchData;			//!< Pointer to ref counted string data.

	vint32&		RefCount() const;
	StringData*	GetData() const;
	Allocator*	GetPool() const;

	void Init();
	void AllocCopy(String& dest, size_t nCopyLen, size_t nCopyIndex, size_t nExtraLen) const;
	void AllocBuffer(size_t nLen);
	void AssignCopy(size_t nSrcLen, LPCTSTR lpszSrcData, Allocator* dataAloc);
	void ConcatCopy(size_t nSrc1Len, LPCTSTR lpszSrc1Data, size_t nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(size_t nSrcLen, LPCTSTR lpszSrcData);
	void CopyBeforeWrite();
	void CopyBeforeWriteImpl();
	void AllocBeforeWrite(size_t nLen, Allocator* pAlloc);
	void Release();
	static void FreeData(StringData* pData);
	static size_t SafeStrlen(LPCTSTR lpsz);
	static void Release(StringData* pData);
};

// Empty static string objects
const String&		EmptyString();

// inline methods

inline const String& String::operator=(const unsigned char* lpsz)	{ *this = (LPCTSTR)lpsz; return *this; }
inline String::String(const unsigned char* lpsz)				{ Init(); *this = (LPCTSTR)lpsz; }
inline long	  String::toLong() const							{ return ATOL (m_pchData); }
inline double String::toDouble() const							{ return ATOF (m_pchData); }
inline size_t String::SafeStrlen(LPCTSTR lpsz)					{ return (lpsz == NULL) ? 0 : STRLEN(lpsz); }

inline String::operator LPCTSTR() const							{ assert(m_pchData != NULL); return m_pchData; }
inline StringData* String::GetData() const						{ assert(m_pchData != NULL); return ((StringData*)m_pchData)-1; }
inline Allocator* String::GetPool() const						{ assert(m_pchData != NULL); return (((StringData*)m_pchData)-1)->pAlloc; }
inline String::String()											{ Init(); }
inline void  String::Init()										{ m_pchData  = EmptyString().m_pchData; }
inline vint32& String::RefCount() const							{ assert(m_pchData != NULL); return (((StringData*)m_pchData)-1)->nRefs; }
inline size_t  String::GetLength() const						{ return m_pchData ? (((StringData*)m_pchData)-1)->nDataLength : 0; }
inline bool  String::IsEmpty() const							{ return m_pchData ? GetData()->nDataLength == 0 : true; }
inline TCHAR String::GetAt(size_t nIndex) const					{ assert(nIndex >= 0 && nIndex < GetData()->nDataLength); return m_pchData[nIndex]; }
inline TCHAR String::operator[](size_t nIndex) const			{ assert(nIndex >= 0 && nIndex < GetData()->nDataLength); return m_pchData[nIndex]; }
inline TCHAR& String::operator[](size_t nIndex)					{ assert(nIndex >= 0 && nIndex < GetData()->nDataLength); CopyBeforeWrite(); return m_pchData[nIndex]; }
inline void  String::CopyBeforeWrite()							{ if (RefCount() > 1) CopyBeforeWriteImpl(); }
// numeric conversion
inline String::String(float aNum)								{ Init(); *this = aNum; }
inline String::String(long aNum)								{ Init(); *this = aNum; }
inline String::String(int aNum)									{ Init(); *this = aNum; }
inline String::String(short aNum)								{ Init(); *this = aNum; }
inline String::String(double aNum)								{ Init(); *this = aNum; }
inline String::String(unsigned int aNum)						{ Init(); *this = aNum; }
inline String::String(unsigned long aNum)						{ Init(); *this = aNum; }
inline String::String(unsigned short aNum)						{ Init(); *this = aNum; }
inline const  String& String::operator=(float aNum)				{ *this = (double) aNum; return *this; }
inline const  String& String::operator=(short aNum)				{ *this = (int) aNum; return *this; }
inline const  String& String::operator=(unsigned int aNum)		{ *this = (unsigned long) aNum; return *this; }
inline const  String& String::operator=(unsigned short aNum)	{ *this = (unsigned long) aNum; return *this; }
inline float  String::toFloat() const							{ return (float) toDouble (); }
inline unsigned long  String::toULong() const					{ return toLong(); }
inline int	  String::toInt() const								{ return toLong(); }
inline short  String::toShort() const							{ return (short) toLong(); }
inline unsigned int   String::toUInt() const					{ return toLong(); }
inline unsigned short String::toUShort() const					{ return (unsigned short) toLong(); }

// comparators
inline bool operator==(const String& s1, const String& s2)		{ return s1.Compare(s2) == 0; }
inline bool operator==(const String& s1, LPCTSTR s2)			{ return s1.Compare(s2) == 0; }
inline bool operator==(LPCTSTR s1, const String& s2)			{ return s2.Compare(s1) == 0; }
inline bool operator!=(const String& s1, const String& s2)		{ return s1.Compare(s2) != 0; }
inline bool operator!=(const String& s1, LPCTSTR s2)			{ return s1.Compare(s2) != 0; }
inline bool operator!=(LPCTSTR s1, const String& s2)			{ return s2.Compare(s1) != 0; }
inline bool operator<(const String& s1, const String& s2)		{ return s1.Compare(s2) < 0; }
inline bool operator<(const String& s1, LPCTSTR s2)				{ return s1.Compare(s2) < 0; }
inline bool operator<(LPCTSTR s1, const String& s2)				{ return s2.Compare(s1) > 0; }
inline bool operator>(const String& s1, const String& s2)		{ return s1.Compare(s2) > 0; }
inline bool operator>(const String& s1, LPCTSTR s2)				{ return s1.Compare(s2) > 0; }
inline bool operator>(LPCTSTR s1, const String& s2)				{ return s2.Compare(s1) < 0; }
inline bool operator<=(const String& s1, const String& s2)		{ return s1.Compare(s2) <= 0; }
inline bool operator<=(const String& s1, LPCTSTR s2)			{ return s1.Compare(s2) <= 0; }
inline bool operator<=(LPCTSTR s1, const String& s2)			{ return s2.Compare(s1) >= 0; }
inline bool operator>=(const String& s1, const String& s2)		{ return s1.Compare(s2) >= 0; }
inline bool operator>=(const String& s1, LPCTSTR s2)			{ return s1.Compare(s2) >= 0; }
inline bool operator>=(LPCTSTR s1, const String& s2)			{ return s2.Compare(s1) <= 0; }
} // end Core

//inline std::ostream& operator<<(std::ostream& strm, const Core::String& qstr)		{ return strm.operator<<((LPCTSTR) qstr); }
