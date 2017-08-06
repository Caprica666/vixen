#if defined(_AFXDLL)
#define USE_MFC
#endif

#ifdef USE_MFC
	#include "vxmfc.h"
	#define	WORLD	MFC::Demo

#else
	#include "vixen.h"
	#include "vxutil.h"
	#include "util/vxviewerapp.h"

	#ifdef VIXEN_GLFW
		#include "ogl/vxglfwworld.h"
	#elif defined(_WIN32)
		#include "win32/vxwinworld.h"
	#else
		#include "linux/winworld.h"
	#endif
	#define	WORLD	Viewer< Vixen::WinWorld >
#endif

using namespace Vixen;

#define TEST_TEXTURE 1

class GeoTestApp : public WORLD
{
public:
	GeoTestApp();
protected:
	Scene*	MakeScene(const TCHAR* filename = NULL);
	Shape*	MakeShape(const TCHAR* name, const Col4& diffuse, Texture* = NULL);
};

GeoTestApp::GeoTestApp() : WORLD()
{
	WinRect.min.x = 100.0f;
	WinRect.min.y = 100.0f;
	WinRect.max.x = 900.0f;
	WinRect.max.y = 700.0f;

	Scene::DeviceDepth = 32;
	Bitmap::RGBADepth = 32;
	Scene::Debug = 1;
	FileLoader::Debug = 1;
	DoAsyncLoad = false;
}

Shape*	GeoTestApp::MakeShape(const TCHAR* name, const Col4& diffuse, Texture* texture)
{
	Shape*			shape = new Shape();
	Appearance*		app = new Appearance();
	TriMesh*		mesh;
	PhongMaterial*	mtl = new PhongMaterial(diffuse);

	mtl->SetDiffuse(Col4(diffuse.r / 2.0f, diffuse.g / 2.0f, diffuse.b / 2.0f));
	app->SetMaterial(mtl);
#ifdef TEST_TEXTURE
	if (texture)
	{
		Sampler* sampler = new Sampler(texture);
		sampler->SetName(TEXT("DiffuseMap"));
		app->SetSampler(0, sampler);
		sampler->Set(Sampler::TEXTUREOP, Sampler::DIFFUSE);
		mesh = new TriMesh(VertexPool::NORMALS | VertexPool::TEXCOORDS);
	}
	else
#endif
	mesh = new TriMesh(VertexPool::NORMALS);
	app->Set(Appearance::LIGHTING, true);
	shape->SetAppearance(app);
	shape->SetName(name);
	shape->SetGeometry(mesh);
	return shape;
}

Scene*	GeoTestApp::MakeScene(const TCHAR* filename)
{
	Scene*	scene = new Scene();
	Model*	root = new Model();
	Shape*	shape;
	Core::String name(TEXT("testgeo"));
	AmbientLight*	ambient = new AmbientLight(Col4(0.5f, 0.5f, 0.5f));
	Shader*	shader = new Shader(Shader::PIXEL);
	float	S = 1.0f;

#ifdef TEST_TEXTURE
	Texture*	image = new Texture(TEXT("brick.jpg"));
#else
	Texture*	image = NULL;
#endif
	shader->SetName(TEXT("ColorShader"));
	shader->SetSource("precision mediump float;\nvoid main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }\n");
	shader->SetInputDesc(TEXT("float4 position, float4 normal"));
	shader->SetOutputDesc(TEXT("float3 color"));
	ambient->SetName(TEXT("viewer.ambientlight"));
//	scene->SetAmbient(ambient);
	scene->SetModels(root);
	scene->SetName(name + TEXT(".scene"));
	root->SetName(name + TEXT(".root"));
#if 1
	shape = MakeShape(name + TEXT(".block"), Col4(0.0f, 0.0f, 1.0f), image);
	GeoUtil::Block((TriMesh*) shape->GetGeometry(), Vec3(S * 2, S, S));
	root->Append(shape);
	shape->Translate(-S / 2, 0.0f, 0.0f);
#endif
	shape = MakeShape(name + TEXT(".sphere"), Col4(0.5f, 1.0f, 0.5f), image);
	GeoUtil::Ellipsoid((TriMesh*) shape->GetGeometry(), Vec3(S, S, S), 9);
	root->Append(shape);
	shape = MakeShape(name + TEXT(".torus"), Col4(1.0f, 0.5f, 1.0f), image);
	GeoUtil::Torus((TriMesh*) shape->GetGeometry(), S / 2, S, 15);
	shape->Translate(3 * S, 2 * S, 0.0f);
	shape->Turn(Model::XAXIS, PI / 2);
	root->Append(shape);
	shape = MakeShape(name + TEXT(".icosahedron"), Col4(0.5f, 1.0f, 1.0f), NULL);
	GeoUtil::IcosaSphere((TriMesh*) shape->GetGeometry(), S, 2, true);
	shape->Translate(-3 * S, 2 * S, 0.0f);
	root->Append(shape);
	shape = MakeShape(name + TEXT(".cylinder"), Col4(0.5f, 0.5f, 1.0f), image);
	GeoUtil::Cylinder((TriMesh*) shape->GetGeometry(), GeoUtil::CYL_SIDE, S, S, 2 * S, 9);
	shape->Turn(Model::ZAXIS, -PI / 6);
	shape->Translate(4 * S, 0.0f, 0.0f);
	root->Append(shape);
	scene->ShowAll();
	DoAsyncLoad = false;
	return scene;
}


VIXEN_MAKEWORLD(GeoTestApp);

#ifdef USE_MFC
MFC::App  theMFCApp;
#endif


