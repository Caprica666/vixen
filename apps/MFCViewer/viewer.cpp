#include "vxmfc.h"
#include "util/vxraypicker.h"

//#define TEST_PICK
#define TEST_LIGHT_SHADER 1

using namespace Vixen;

class CViewApp : public MFC::Demo
{
public:
	CViewApp();

protected:
	virtual void	SetScene(Scene*, Window = NULL);
	virtual	bool	OnInit();

#ifdef TEST_LIGHT_SHADER
	Light*	MakeAmbientLight(const TCHAR* render_options);
#endif

#ifdef TEST_PICK
	Engine*	MakeEngines();

public:
	Ref<RayPicker>	m_Picker;
#endif
};

CViewApp::CViewApp() : MFC::Demo()
{
	WinRect.min.x = 100.0f;
	WinRect.min.y = 100.0f;
	WinRect.max.x = 900.0f;
	WinRect.max.y = 700.0f;
	Appearance::DoLighting = false;
	Pose::Debug = 2;
	Skeleton::Debug = 2;
}

bool CViewApp::OnInit()
{
	if (MFC::Demo::OnInit())
	{
		FileLoader* loader = GetLoader();

		loader->SetFileFunc(TEXT("bvh"), &BVHLoader::ReadAnim, Event::LOAD_SCENE);
		return true;
	}
	return false;
}

#ifdef TEST_PICK
Engine* CViewApp::MakeEngines()
{
	Engine* naveng = Demo::MakeEngines();
	Appearance* app;
	PhongMaterial* mtl = new PhongMaterial(new Col4(1.0f, 0.2f, 1.0f));

	if (naveng == NULL)
		return NULL;
	if (m_Picker.IsNull())
		m_Picker = new RayPicker;
	m_Picker->SetName("RayPicker");
	app = new Appearance();
	app->SetMaterial(mtl);
	m_Picker->SetHilite(app);
	m_Picker->SetOptions(Picker::MESH | Picker::HILITE_SELECTED);
	m_Picker->SetButtons(MouseEvent::SHIFT);
	AddEngine(m_Picker);
	return naveng;
}
#endif


void CViewApp::SetScene(Scene* scene, Window win)
{
	Model*	root = scene->GetModels();

#ifdef TEST_LIGHT_SHADER
	scene->SetAmbient(MakeAmbientLight(scene->RenderOptions));
#endif

#ifdef TEST_PICK
	if (m_Picker.IsNull())
		m_Picker = new RayPicker;
	GetMessenger()->Observe(m_Picker, Event::MOUSE, NULL);
	GetMessenger()->Observe(this, Event::PICK, m_Picker);
	GetMessenger()->Observe(this, Event::TRI_HIT, m_Picker);
	m_Picker->SetTarget(root);
#endif
#ifdef _DEBUG
	if (World::Debug > 1)
		scene->Print(vixen_debug, SharedObj::PRINT_All);
	else if (World::Debug)
		scene->Print(vixen_debug, SharedObj::PRINT_Default);
#endif
	MFC::Demo::SetScene(scene, win);
}

#ifdef TEST_LIGHT_SHADER
Light* CViewApp::MakeAmbientLight(const TCHAR* render_options)
{
	const TCHAR* shadertype = NULL;
	Core::String	shaderfile = TEXT("shaders\\sphericalharmoniclight.");

	if (render_options)
		if (STRNCASECMP(render_options, TEXT("hlsl"), 4) == 0)
			shadertype = TEXT("hlsl");
		else if (STRNCASECMP(render_options, TEXT("glsl"), 4) == 0)
			shadertype = TEXT("glsl");
	if (shadertype)
	{
		Light*	shlight = new Light(TEXT("float4 Color, float Decay"), TEXT("sphericalharmoniclight"));
		Shader* shlight_shader = new Shader(Shader::LIGHT);

		shaderfile += shadertype;
		shlight->SetName("viewer.sphericalharmoniclight");
		shlight->SetColor(Col4(0.3f, 0.3f, 0.3f));
		shlight_shader->SetName("sphericalharmoniclight");
		shlight_shader->SetOutputDesc(TEXT("flux float 4, direction float 4"));
		shlight_shader->SetFileName(shaderfile);
		return shlight;
	}
	return NULL;
}
#endif

VIXEN_MAKEWORLD(CViewApp);

MFC::App	theMFCApp;
