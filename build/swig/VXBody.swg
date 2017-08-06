/*
 * SWIG interface file for animation engines
 */

%{
#include "body/vxbodytrack.h"
#include "body/vxkinecttrack.h"
%}

namespace Vixen
{
	%ignore BodyTracker::GetUsers;
	%apply	SharedObj*	{ BodyTracker* };
	%apply	SharedObj*	{ KinectTracker* };

}

ATTR_READONLY(Vixen::BodyTracker, Vixen::Vec3, UserPosition);
ATTR_READONLY(Vixen::BodyTracker, Vixen::Vec2, ImageSize);
ATTR_READONLY(Vixen::BodyTracker, Vixen::Skeleton, Skeleton);
ATTR_READONLY(Vixen::BodyTracker, Vixen::PoseMapper, PoseMapper);
ATTR_READONLY(Vixen::BodyTracker, float, FOV);
ATTR_READONLY(Vixen::BodyTracker, int, UserID);

ATTR_DEFINE(Vixen::BodyTracker, int, Options);
ATTR_DEFINE(Vixen::BodyTracker, Vixen::Texture, ColorTexture);
ATTR_DEFINE(Vixen::BodyTracker, Vixen::Texture, DepthTexture);
ATTR_DEFINE(Vixen::BodyTracker, Vixen::Vec3, WaistOffset);

%include "body/vxbodytrack.h"
%include "body/vxkinecttrack.h"


