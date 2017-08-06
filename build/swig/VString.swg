#define	_WIN32
typedef int		size_t;
typedef char	TCHAR;
typedef char*	LPCWSTR;

namespace Vixen
{
	namespace Core
	{
		%naturalvar String;
		%ignore _vInitThreadData;
		%ignore _vThreadData;
		%ignore _vEmptyStr;
		%ignore StringData;
		%ignore String::GetNextLine;
		%ignore String::String(const unsigned char*);
		%ignore String::String(const WCHAR[]);
		%ignore String::String(const WCHAR[], size_t);
		%ignore String::String(const String&);
		%ignore String::String(float, int);
		%ignore String::String(float);
		%ignore String::String(long);
		%ignore String::String(short);
		%ignore String::String(unsigned long);	
		%ignore String::String(unsigned int);

		%ignore String::operator[];
		%ignore String::operator=;
		%ignore String::operator+;
		%ignore	String::operator+=;
		%ignore	String::operator!=;
		%ignore	String::operator LPCTSTR;
		%ignore String::Format;
		%ignore	String::FormatV;
		%ignore String::GetField;
		%ignore String::ReleaseBuffer;
		%ignore String::GetBufferSetLength;
		%ignore String::FreeExtra();
		%ignore String::LockBuffer();
		%ignore String::UnlockBuffer();
		%ignore EmptyString();
		%ignore operator==(String& s1, String& s2);
		%ignore operator!=(String& s1, String& s2);
		%ignore operator<(String& s1, String& s2);
		%ignore operator>(String& s1, String& s2);
	} // end Core
} // end Vixen

#ifdef SWIGCSHARP
	%include "VixenCS/VString_csharp2.swg"
#endif

namespace Vixen
{
%include "vcore/vstring.h"

} // end Vixen