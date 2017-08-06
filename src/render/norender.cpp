/****
 *
 *	These are the generic implementations of rendering.
 *	They do not do anything!
 *
 ****/
#include "vixen.h"

namespace Vixen {
VX_IMPLEMENT_CLASS(DeviceInfo, SharedObj);

Renderer* Renderer::CreateRenderer(int options) { return NULL; }


};