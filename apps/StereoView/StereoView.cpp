// StereoView.cpp : Defines the entry point for the application.
//
#include "vixen.h"
#include "vxutil.h"
#include "util/vxviewerapp.h"
#include "win32/vxwinworld.h"

#include "OVR.h"

using namespace Vixen;

class Oculus : public Engine
{
public:
	Oculus();
	~Oculus();

	bool			InitCamera(Camera*);
	virtual bool	OnStart();
	virtual bool	Eval(float);
	void			Shutdown();

protected:
	OVR::DeviceManager*		m_DeviceManager;
	OVR::HMDDevice*			m_Device;
	OVR::SensorDevice*		m_Sensor;
	OVR::SensorFusion		m_SensorFusion;
	OVR::HMDInfo			m_HMD;
};

class StereoApp : public Viewer<WinWorld>
{
public:
	StereoApp();
	virtual bool	OnEvent(Event*);
	virtual Engine*	MakeEngines();
	virtual void	OnExit();
	virtual Scene*	NewScene(Window);

protected:
	Ref<Oculus>			m_Oculus;
	Ref<DeviceBuffer>	m_FilterParams;
};

StereoApp::StereoApp() : Viewer<WinWorld>()
{
	m_IsFullScreen = true;
	m_StereoEyeSep = 6.2f;
	Transformer::Debug = 1;
}

VIXEN_MAKEWORLD(StereoApp);

bool StereoApp::OnEvent(Event* ev)
{
	if ((ev->Code == Event::SET_TIME) && !m_FilterParams.IsNull())
	{
		FrameEvent* fe = (FrameEvent*) ev;

		if (fe->Frame & 1)		// odd frame - right eye (red tint)
		{
			m_FilterParams->Set(TEXT("AddColor"), Vec4(0.3f, 0.0f, 0.0f, 0.0f));
		}
		else					// even frame - left eye (blue tint)
		{
			m_FilterParams->Set(TEXT("AddColor"), Vec4(0.0f, 0.0f, 0.3f, 0.0f));
		}
	}
	else if (ev->Code == Event::KEY)
	{
		KeyEvent* ke = (KeyEvent*) ev;
		if (ke->KeyCode == 'z')
		{
			GetMainScene()->ShowAll();
			return true;
		}
	}
	return Viewer<WinWorld>::OnEvent(ev);
}

Scene*	StereoApp::NewScene(Window win)
{
	Scene*			scene = Viewer<WinWorld>::NewScene(win);
	Appearance*		post = new Appearance();
	Shader*			postshader = new Shader(Shader::PIXEL);
	DeviceBuffer*	postdata = new DeviceBuffer("float4 AddColor", 4 * sizeof(float));

	m_FilterParams = postdata;
	postdata->Set(TEXT("AddColor"), Vec4(0, 0, 0.2f, 0));
	postshader->SetName(TEXT("PostProcessor"));
	postshader->SetInputDesc(TEXT("float4 AddColor"));
	postshader->SetSource(TEXT("float4 PostProcessor(float3 color) { return float4(AddColor.r + color.r, AddColor.g + color.g, AddColor.b + color.b, 1.0); }\n"));
	post->SetMaterial(postdata);
	post->SetPixelShader(postshader);
	scene->SetPostProcess(post);
	scene->SetFlags(SharedObj::DOEVENTS);
	GetMessenger()->Observe(this, Event::SET_TIME, scene);
	return scene;
}

Engine* StereoApp::MakeEngines()
{
	Transformer* xform = new Transformer;
	Navigator*	nav = new Navigator();

//	nav->SetSpeed(0.2f);
	xform->SetName(TEXT("viewer.camera.xform"));
	nav->SetName(TEXT("viewer.camera.navigator"));
	xform->Append(nav);
	m_Oculus = new Oculus();
	xform->Append(m_Oculus);
	SetNavigator(xform);
	return xform;
}

void StereoApp::OnExit()
{
	m_Oculus->Shutdown();
	Viewer<WinWorld>::OnExit();
}


Oculus::Oculus() : Engine()
{
	m_DeviceManager = NULL;
	m_Device = NULL;
	m_Sensor = NULL;
}

Oculus::~Oculus()
{
	Shutdown();
}

void Oculus::Shutdown()
{
	if (m_DeviceManager != NULL)
	{
		m_DeviceManager->Release();
		m_DeviceManager = NULL;
	}
	if (m_Device != NULL)
	{
		m_Device->Release();
		m_Device = NULL;
	}
	if (m_Sensor != NULL)
	{
		m_Sensor->Release();
		m_Sensor = NULL;
	}
//	OVR::System::Destroy();
}

bool Oculus::OnStart()
{
	if (m_DeviceManager == NULL)
	{
		OVR::System::Init();
		m_DeviceManager = OVR::DeviceManager::Create();
		if (m_DeviceManager)
			m_Device = m_DeviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
		if ((m_Device == NULL) ||
			!m_Device->GetDeviceInfo(&m_HMD))
			VX_ERROR(("Oculus::OnStart cannot create Oculus device"), true);
	}
	if (m_Sensor == NULL)
	{
		m_Sensor = m_Device->GetSensor();
		if (m_Sensor)
			m_SensorFusion.AttachToSensor(m_Sensor);
		else
			VX_ERROR(("Oculus::OnStart cannot attach Oculus sensor"), true);
	}
	Camera* cam = GetMainScene()->GetCamera();
	cam->MakeLock();
	InitCamera(cam);
	return true;
}

bool Oculus::Eval(float t)
{
	if ((m_Device == NULL) || (m_Sensor == NULL))
		return true;
	if (!m_SensorFusion.IsAttachedToSensor())
		return true;

	OVR::Quatf	orient = m_SensorFusion.GetOrientation();
	Camera*		cam = GetMainScene()->GetCamera();
	Quat		q(orient.x, orient.y, orient.z, -orient.w);

	if (cam)
		cam->SetRotation(q);
	return true;
}

bool Oculus::InitCamera(Camera* cam)
{
	float aspect, fov;

	if (m_Device == NULL)
		return false;
	aspect = m_HMD.HResolution / (2.0f * m_HMD.VResolution);
	fov = m_HMD.VScreenSize / (2.0f * m_HMD.EyeToScreenDistance);
	cam->SetAspect(aspect);
	cam->SetFOV(fov);
	return true;
}
