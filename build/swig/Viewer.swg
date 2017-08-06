
namespace Vixen
{
	%ignore Viewer::ParseArgs(int, TCHAR**);

	%apply	SharedObj*	{ SharedWorld* };

	#ifdef SWIGCSHARP
		%typemap(cscode) SharedWorld %{
			public static Scene MainScene { get { return VixenLib.GetMainScene(); } }			
		%}
	#endif

	#ifdef SWIGJAVA
		%typemap(javacode) SharedWorld %{
			public static Scene MainScene { get { return VixenLib.GetMainScene(); } }			
		%}
	#endif

}

%include "sharedworld.h"
%include "util/vxviewerapp.h"

namespace Vixen
{
	%template(Viewer3D) Viewer<SharedWorld>;
}

%{
	#include "util/vxviewerapp.h"
	#include "sharedworld.h"
	typedef Viewer<SharedWorld>	Viewer3D;
%}

%init %{
	static bool initialize_vixen()
	{
		return World::Startup();
	}

	extern bool Vixen_Initialized = initialize_vixen();
%}







