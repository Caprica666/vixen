#include "vxmfc.h"
#include "vxmedia.h"

using namespace Vixen;

//#define TEST_SOUND
#define TEST_VIDEO

class VideoTestApp : public MFC::Demo
{
public:
	VideoTestApp();
protected:
	Scene*	MakeScene(const TCHAR* filename = NULL);
	virtual	bool	OnEvent(Event*);
	void			ChangeVideoSize();

	Ref<MediaSource>	m_MediaSource;
	Ref<Shape>			m_VideoShape;
	Ref<Texture>		m_VideoTex;
};

VideoTestApp::VideoTestApp() : MFC::Demo()
{
	WinRect.min.x = 100.0f;
	WinRect.min.y = 100.0f;
	WinRect.max.x = 900.0f;
	WinRect.max.y = 700.0f;
	REGISTER_CLASS(MediaSource);
	DoAsyncLoad = false;
}


Scene*	VideoTestApp::MakeScene(const TCHAR* filename)
{
	Scene*			scene = new Scene();
	Shape*			shape = new Shape();
	Appearance*		app = new Appearance();
	TriMesh*		mesh = new TriMesh(VertexPool::TEXCOORDS);
	MediaSource*	video = new MediaSource();
	SoundPlayer*	sound = NULL;
	Texture*		tex = new Texture();
	Sampler*		sampler = new Sampler();
	Camera*			cam;
	Core::String	videofile;
	Core::String	soundfile;
	
	m_VideoTex = tex;
	if (!filename)
		filename = m_FileName;
	if ((filename == NULL) || (*filename == NULL))
		filename = TEXT("realvideo.mpg");
#ifdef TEST_SOUND
	sound = new SoundPlayer();
	sound->SetFileName(TEXT("Realaudio.MP3"));
#endif
	tex->SetName(TEXT("videotest.videotexture"));
//	tex->SetFormat(Texture::RENDERTARGET);
#ifdef TEST_VIDEO
	GetMessenger()->Observe(this, Event::START, video);
	m_MediaSource = video;
	video->SetName(TEXT("videotest.mediasource"));
	video->SetControl(Engine::CYCLE);
	video->SetFlags(SharedObj::DOEVENTS);
	video->SetTarget(tex);
	video->SetFileName(filename);
#else
	tex->Load(videofile);
#endif
	if (sound)
		video->Append(sound);
	sampler->SetName("Diffuse");
	sampler->Set(Sampler::TEXTUREOP, Sampler::EMISSION);
	sampler->Set(Sampler::TEXCOORD, 0);
	sampler->Set(Sampler::MIPMAP, false);
	sampler->SetTexture(tex);
	app->Set(Appearance::CULLING, false);
	app->Set(Appearance::LIGHTING, false);
	app->SetSampler(0, sampler);
	shape->SetAppearance(app);
	shape->SetName(TEXT("videotest.root"));
	shape->SetGeometry(mesh);
	scene->SetModels(shape);
	m_VideoShape = shape;
	mesh->MakeLock();
#ifdef TEST_VIDEO
	scene->SetEngines(video);
#elif defined(TEST_SOUND)
	scene->SetEngines(sound);
#endif
	cam = scene->GetCamera();
	cam->SetTranslation(Vec3(0.0f, 0.0f, 2.0f));
	cam->SetHither(0.1f);
	cam->SetName(TEXT("videotest.camera"));
	//scene->ShowAll();
	return scene;
}

bool VideoTestApp::OnEvent(Event* ev)
{
	SharedObj*	sender = ev->Sender;
	MediaSource*	msrc = (MediaSource*) sender;

	if ((ev->Code == Event::START) &&
		(msrc == (MediaSource*) m_MediaSource))
		ChangeVideoSize();
	return MFC::Demo::OnEvent(ev);
}

void VideoTestApp::ChangeVideoSize()
{
	TriMesh* mesh = (TriMesh*) m_VideoShape->GetGeometry();

	if ((mesh != NULL) && (mesh->GetNumVtx() == 0))
	{
		Vec3	videosize = m_MediaSource->GetVideoSize();
		Vec2	texsize(512,512);	// pixel dimensions of texture (powers of 2)

		if (videosize.x > 0)
		{
			texsize = m_MediaSource->GetTexSize();
			m_VideoTex->SetWidth(texsize.x);
			m_VideoTex->SetHeight(texsize.y);
			m_VideoTex->SetDepth(24);
		}
		ObjectLock	lock(mesh);
		GeoUtil::Rect(mesh, videosize.x / videosize.y, 1.0f, videosize.x / texsize.x, videosize.y / texsize.y);
//		GeoUtil::Rect(mesh, videosize.x / videosize.y, 1.0f);
	}
}

VIXEN_MAKEWORLD(VideoTestApp);

MFC::App	theMFCApp;
