/*
 * SWIG interface file for Scene, Fog, Engine
 */
namespace Vixen {
	%ignore SCENE_RootChanged;
	%ignore SCENE_WindowChanged;
	%ignore SCENE_CameraChanged;
	%ignore SCENE_EnvChanged;
	%ignore SCENE_FBChanged; 
	%ignore SCENE_KillMe;
	%ignore SceneThread;
	%ignore Fog;
	%ignore Scene::TLS;
	%ignore Scene::GetFog;
	%ignore Scene::SetFog;
	%ignore Scene::GetEngines() const;
	%ignore Scene::GetModels() const;
	%ignore Scene::GetCamera() const;
	%ignore Scene::GetViewport(Box2*) const;
	%ignore Scene::GetDevInfo;
	%ignore Scene::GetRenderer;
	%ignore Scene::GetLights;
	%ignore Scene::GetStats;
	%ignore Scene::SetTime;
	%ignore Scene::GetTLS;
	%ignore Scene::OnFrame;
	%ignore Scene::InitCamera;
	%ignore Scene::DoDisplay;
	%ignore Scene::Begin;
	%ignore Scene::End;
	%ignore Scene::Flip;
	%ignore Scene::DoRender;
	%ignore Scene::InitThread;
	%ignore Scene::EndThread;
	%ignore Scene::SetRenderMatrix;
	%ignore Scene::SetViewMatrix;
	%ignore Scene::GetFrameIndex;
	%ignore Scene::IsExit;
	%ignore Scene::GetChanged;
	%ignore Scene::Notify;
	%ignore Scene::IsChild;
	%ignore Scene::GetChild;
	%ignore Scene::RemoveChild;
	%ignore Scene::GetWorldMatrix;
	%ignore Scene::InitRender;
	%ignore Scene::GetRenderer;
	%ignore Scene::SetSorter;
	%ignore Scene::InitThreadGlobals;
	%ignore Scene::FrameFunc;
	%ignore Scene::SupportDDS;
	%apply	SharedObj*	{ Engine* };
	%apply	SharedObj*	{ Scene* };

	%extend Scene
	{
		void OnResize()
		{
			$self->SetViewport();
		}
	}
}
	
ATTR_DEFINE(Vixen::Scene, int, Options);
ATTR_DEFINE_NEW(Vixen::Scene, Vixen::Box2, Viewport)
ATTR_DEFINE(Vixen::Scene, bool, AutoAdjust, IsAutoAdjust);
ATTR_DEFINE_NEW(Vixen::Scene, Vixen::Col4, BackColor);
ATTR_DEFINE(Vixen::Scene, Vixen::Camera, Camera);
ATTR_DEFINE(Vixen::Scene, Vixen::Model, Models);
ATTR_DEFINE(Vixen::Scene, Vixen::Engine, Engines);
ATTR_DEFINE(Vixen::Scene, float, TimeInc);
ATTR_DEFINE(Vixen::Scene, Vixen::Light, Ambient);
ATTR_DEFINE(Vixen::Scene, intptr, Window);

%include "scene/vxscene.h"
%include "scene/vxscene.inl"



