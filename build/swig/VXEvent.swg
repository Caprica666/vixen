/*
 * SWIG interface file for Event
 */
#ifdef SWIGCSHARP
#include "VixenCS/VXEvent_csharp.swg"
#endif

#ifdef SWIGJAVA
#include "VixenJava/VXEvent_java.swg"
#endif

namespace Vixen
{
	%ignore Event::Data;
	%ignore Event::Parse;
	%ignore Event::Save;
	%rename(Copy)	Event::operator=;
}
%include "base/vxevent.h"
ATTR_READONLY(Vixen::Event, const char*, Name);

namespace Vixen
{
	REF_TYPEMAP(SceneRef, Scene)
	REF_TYPEMAP(MeshRef, Mesh)
	REF_DEFINE(SceneRef)
	%template(SceneRef)		Ref< Scene >;
	REF_DEFINE(MeshRef)
	%template(MeshRef)		Ref< Mesh >;
}

%include "base/vxsysevents.h"
