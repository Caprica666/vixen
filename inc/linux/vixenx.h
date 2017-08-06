#pragma once

#include "vixen.h"
#include <GL/glx.h>

namespace Vixen {

class DeviceInfo : public Property
{
	VX_DECLARE_CLASS(DeviceInfo);
public:
	Window		WinHandle;		// handle to X11 window
	Display*	DispHandle;		// handle to X11 Display
	GLXContext	Device;			// handle to OpenGL rendering context
	XVisualInfo*	VisHandle;
};

#define PROP_DevInfo	intptr(CLASS_(DeviceInfo))


}
