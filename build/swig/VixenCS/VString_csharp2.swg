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

namespace Vixen {
namespace Core {
	%typemap(csin) String, String& "$csinput"
	%typemap(cstype) String, String&	"string"
	%typemap(imtype) String, String&	"string"
	%typemap(out) String&, String %{ if ($1) $result = SWIG_csharp_string_callback((const char*) *$1); %}

	%typemap(in, canthrow=1) String, String& %{
		if (!$input)
		{
			SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
			return $null;
		}
		$*1_ltype $1_str((const char*) $input);
		$1 = &$1_str;
	%}

	%typemap(csout, excode=SWIGEXCODE) String, String& %{
    {
		string ret = $imcall;$excode
		return ret;
	}
	%}

	%typemap(csvarout, excode=SWIGEXCODE2) String, String& %{
		get {
		  string ret = $imcall;$excode
		  return ret;
		}
	%}

	%typemap(csvarin, excode=SWIGEXCODE2) String, String & %{
    set {
      $imcall;$excode
    } %}

	%typemap(csclassmodifiers) String "internal class"
	
	%typemap(cscode) String %{
		public override bool Equals(Object obj)
		{
			if (obj == null)
				return IsEmpty();
			if (obj.GetType() == typeof(String))
			{
				String s = obj as String;
				return Compare(s.GetBuffer(s.GetLength())) == 0;
			}
			else if (obj.GetType() == typeof(string))
			{
				string s = obj as string;
				return Compare(s) == 0;
			}
			return false;
		}

		public static bool operator ==(String arg1, String arg2) { return arg1.Equals(arg2); }
		public static bool operator !=(String arg1, String arg2) { return !arg1.Equals(arg2); }
	%}
}
}
