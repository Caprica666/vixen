/* -----------------------------------------------------------------------------
 * VString.swg
 *
 * Typemaps for Core::String and const Core::String&
 * These are mapped to a C# String and are passed around by value.
 *
 * To use non-const VString references use the following %apply.  Note 
 * that they are passed by value.
 * %apply const VString & VString &};
 * ----------------------------------------------------------------------------- */
CSHARP_ARRAYS_FIXED(wchar_t, char)
//CSHARP_ARRAYS_FIXED(WCHAR, char)
//%apply WCHAR FIXED[]	{ WCHAR* wcharBuffer }
//%apply wchar_t FIXED[]	{ wchar_t* wcharBuffer }

namespace Vixen
{
namespace Core
{
%typemap(csclassmodifiers) String "internal class";
%csmethodmodifiers SetWide "public unsafe";
%csmethodmodifiers AsWide "public unsafe";
%csmethodmodifiers MultiByteToWide "public unsafe";
  
%typemap(cstype, out="System.String") String& "System.String"

%typemap(csin, 
		pre="	int len = $csinput.Length();"
			"	String temp_$csinput = new String((char) 0, len * 2 + 1);"
			"	temp_$csinput.SetWide($csinput, (uint) len);"
        ) String&	"$csclassname.getCPtr(temp_$csinput)"


%typemap(csvarout, excode=SWIGEXCODE2) String& %{
    get {
      IntPtr cPtr = $imcall;
	  if (cPtr == IntPtr.Zero)
		return null;
      String temp = new String(cPtr, $owner);
	  int	 mblen = (int) temp.GetLength() * 2 + 1;
	  System.String ret = new System.String((char) 0, mblen);
	  temp.AsWide(ret, (uint) mblen);$excode
      return ret;
    }
%}


%typemap(csvarin, excode=SWIGEXCODE2) String&  %{
	set
	{
		int	mblen = value.Length *2 + 1;
		String	temp_value = new String((char) 0, mblen);
		temp_value.SetWide(value, (uint) value.Length);
		$imcall;$excode
	}
%}


%extend String {
	void SetWide(const WCHAR* wcharBuffer, size_t len)
	{
		char* multibyte = (char*) alloca(len * 2 + 1);
		$self->WideToMultiByte(wcharBuffer, multibyte, len);
	}
}
}
}