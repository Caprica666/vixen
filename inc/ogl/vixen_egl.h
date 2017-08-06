#pragma once
#include "base/vxbase.h"
namespace Vixen {

#define	IMAGE_MaxTexUnits	8
typedef	IntArray		IndexArray;
typedef	int32			VertexIndex;
#define	VXGLIndex		GL_UNSIGNED_SHORT
};

#include "vxexport.h"
#include "ogl/vxgdev.h"

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
	EGLNativeWindowType	WinHandle;	// handle to window
  	EGLContext			Device;		// handle to OpenGL rendering context
	EGLDisplay			DC;
};

#define PROP_DevInfo	uint32(CLASS_(DeviceInfo))

}
