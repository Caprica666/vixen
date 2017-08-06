/*
 * SWIG interface file for Model, Shape, Camera, Light
 */
namespace Vixen
{
	%ignore Model::Hit;
	%ignore Model::WasRendered;
	%ignore Model::SetRendered;
	%ignore Model::Cull;
	%ignore Model::Display;
	%ignore Model::CalcMatrix;
	%ignore Model::CalcStats;

	%ignore Camera::GetCullPlanes;
	%ignore Camera::NumClipPlanes;
	%ignore Camera::PushClipPlane;
	%ignore Camera::PopClipPlane;
	%ignore Camera::GetDeviceCoords;
	%ignore Camera::GetDeviceCoords;
	%ignore Camera::CalcViewPlanes;
	%ignore Camera::ProjPerspective;
	%ignore Camera::ProjOrtho;

	%apply	SharedObj*	{ Model* };
	%apply	SharedObj*	{ Camera* };
	%apply	SharedObj*	{ ExtModel* };
}

ATTR_DEFINE(Vixen::Model, int, Hints);
ATTR_DEFINE_NEW(Vixen::Model, Vixen::Vec3, Translation);
ATTR_DEFINE_NEW(Vixen::Model, Vixen::Quat, Rotation);
ATTR_READONLY_NEW(Vixen::Model, Vixen::Vec3, Center);
ATTR_DEFINE(Vixen::Model, bool, Culling, IsCulling);
ATTR_DEFINE(Vixen::Model, Vixen::Matrix, Transform);
ATTR_DEFINE(Vixen::ExtModel, const char*, FileName);
ATTR_DEFINE(Vixen::ExtModel, const char*, ExtName);
ATTR_DEFINE(Vixen::ExtModel, int, Options);

ATTR_DEFINE_NEW(Vixen::Camera, Vixen::Box3, ViewVol);
ATTR_DEFINE(Vixen::Camera, int, Type);
ATTR_DEFINE(Vixen::Camera, float, Aspect);
%attribute(Vixen::Camera, float, FieldOfView, GetFOV, SetFOV);
ATTR_DEFINE(Vixen::Camera, float, Hither);
ATTR_DEFINE(Vixen::Camera, float, Yon);
ATTR_DEFINE(Vixen::Camera, float, Height);


%include "scene/vxmodel.h"
%include "scene/vxcam.h"
%include "scene/vxextmodel.h"


