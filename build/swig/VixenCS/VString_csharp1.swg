/* -----------------------------------------------------------------------------
 * VString.swg
 *
 * Typemaps for Core::String and const Core::String&
 * These are mapped to a C# String and are passed around by value.
 *
 * To use non-const String references use the following %apply.  Note 
 * that they are passed by value.
 * %apply const Core::String & Core::String &};
 * ----------------------------------------------------------------------------- */
namespace Vixen {
namespace Core {


%typemap(ctype) String "char *"
%typemap(imtype) String "String"
%typemap(cstype) String "String"


%typemap(in, canthrow=1) String  %{
	if (!$input)
	{
		SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
		return $null;
	}
	$1 = $input;
%}

%typemap(out) String
%{
	$result = SWIG_csharp_string_callback((const char*) $1);
%}

%typemap(csin) String "$csinput"

%typemap(csout, excode=SWIGEXCODE) String {
    Core::String ret = $imcall;$excode
    return ret;
  }

%typemap(typecheck) String = char *;

%typemap(throws, canthrow=1) String
%{
	SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1);
	return $null;
%}

// const Core::String &
%typemap(ctype) String "char *"
%typemap(imtype) String "String"
%typemap(cstype) String "String"


%typemap(in, canthrow=1) String
%{
	if (!$input)
	{
		SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
		return $null;
	}
	Core::String $1_str($input);
	$1 = &$1_str;
%}

%typemap(out) String
%{
	$result = SWIG_csharp_string_callback((const char*) $1);
%}

%typemap(csin) String "$csinput"

%typemap(csout, excode=SWIGEXCODE) String
{
    String ret = $imcall;$excode
    return ret;
}


%typemap(csvarin, excode=SWIGEXCODE2) String
%{
    set
	{
      $imcall;$excode
    }
%}


%typemap(csvarout, excode=SWIGEXCODE2) String
%{
    get
	{
      String ret = $imcall;$excode
      return ret;
    }
%}

%typemap(typecheck) const String & = char *;

%typemap(throws, canthrow=1) String
%{
	SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, (const char*) $1);
   return $null;
%}

}
}