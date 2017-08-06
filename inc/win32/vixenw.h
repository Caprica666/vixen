#pragma once

namespace Vixen {

/*!
 * Windows specific device information attach to the scene
 * @internal
 * @ingroup vixenint
 */
class DeviceInfo : public Property
{
public:
	VX_DECLARE_CLASS(DeviceInfo);

	DeviceInfo() : Property() { }
	HWND	WinHandle;	// handle to display window
  	HDC		DC;			// handle to GDI DC
  	void*	Device;		// handle to OpenGL rendering context
};

#define PROP_DevInfo	uint32(CLASS_(DeviceInfo))

}
