/*
 * SWIG interface file for utility classes
 */
namespace Vixen
{
	%ignore NavRecorder;
	%ignore Navigator::OnNavInput;
	%ignore Navigator::OnMouse;
	%ignore Navigator::OnKey;
	%ignore Navigator::LogEvent;
	%ignore Flyer::OnNavigate;
	%ignore Picker::OnPick;
	%ignore Picker::OnNoPick;

	%apply	SharedObj*	{ MouseEngine* }
	%apply	SharedObj*	{ Picker* }
	%apply	SharedObj*	{ Navigator* }
	%apply	SharedObj*	{ Flyer* }
	%apply	SharedObj*	{ RayPicker* }
	%apply	SharedObj*	{ NamePicker* }
}

ATTR_DEFINE(Vixen::Navigator, float, TurnSpeed);
ATTR_DEFINE(Vixen::Flyer, float, Scale);

#ifdef SWIGCSHARP
%attribute(Vixen::Flyer, int, DegreesOfFreedom, GetDOF, SetDOF);
#endif

%include "util/vxnavigator.h"
%include "util/vxflyer.h"
%include "util/vxraypicker.h"

ATTR_READONLY(Vixen::Picker, Vixen::ObjArray, Selected);
ATTR_READONLY(Vixen::Picker, Vixen::Model, Nearest);
ATTR_READONLY(Vixen::Picker, float, NearDist);
ATTR_DEFINE(Vixen::Picker, int32, Buttons);
ATTR_DEFINE(Vixen::Picker, int32, Options);
ATTR_DEFINE(Vixen::Picker, Vixen::Appearance, Hilite);
ATTR_DEFINE(Vixen::RayPicker, Vixen::Model, PickShape);
ATTR_DEFINE(Vixen::RayPicker, Vixen::Ray, Ray);
ATTR_DEFINE(Vixen::NamePicker, const char*, NameFilter);
