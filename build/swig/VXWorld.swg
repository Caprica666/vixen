/*
 * SWIG interface file for World, World3D
 */
namespace Vixen
{
	%ignore FileLoader;
	%ignore SceneLoader;
	%ignore World::GetLoader;
	%ignore World::GetMessenger;
	%ignore World::GetEventAlloc;
	%ignore World::ParseArgs(int, TCHAR**);
	%ignore World::OnEvent;
	%ignore World::MakeEvent;
	%ignore World::DebugOutput;
	%ignore World::OnThreadExit;

	%ignore World3D::AddScene;
	%ignore World3D::KillScene;
	%ignore World3D::GetLoader;
	%ignore World3D::RawInput;

	%apply	SharedObj*	{ World3D* }
}

ATTR_DEFINE(Vixen::World, const char*, FileName);

%include "base/vxworld.h"
%include "scene/vxworld3d.h"


