using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

using Vixen;

namespace VideoTest
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
			: base()
		{
			SharedWorld.DoAsyncLoad = false;
			InitializeComponent();
		}

	}

	public class VideoCanvas : Canvas3D
	{
		public static readonly DependencyProperty AudioFileProp = DependencyProperty.Register("AudioFile", typeof(System.String), typeof(VideoCanvas));
		public System.String AudioFile
		{
			get { return base.GetValue(AudioFileProp) as System.String; }
			set { base.SetValue(AudioFileProp, value); }
		}

		public VideoCanvas()
			: base()
		{
		}

		public override Scene MakeScene()
		{
			Scene scene = new Scene();
			Shape shape = new Shape();
			Appearance app = new Appearance();
			TriMesh mesh = new TriMesh(VertexPool.TEXCOORDS);
			MediaSource video = new MediaSource();
			SoundPlayer sound = null;
			Texture tex = new Texture();
			Sampler sampler = new Sampler();
			Camera cam;
			Vec2 videosize = new Vec2(1, 1);		// pixel dimensions of video frame
			Vec2 texsize = new Vec2(512, 512);   // pixel dimensions of texture (powers of 2)	

			video.Name = "videotest.simroot";
			video.Control = Engine.CYCLE;
			video.Target = tex;
			video.FileName = GetMediaPath(ContentFile);
			videosize = video.VideoSize;
			if (videosize.x > 0)
				texsize = video.TexSize;
			if (AudioFile != "")
			{
				sound = new SoundPlayer();
				sound.FileName = GetMediaPath(AudioFile);
				video.Append(sound);
			}
			tex.Name = "videotest.videotexture";
			sampler.Set(Sampler.TEXTUREOP, Sampler.EMISSION);
			sampler.Set(Sampler.TEXCOORD, 0);
			sampler.Set(Sampler.MIPMAP, 0);
			sampler.Texture = tex;
			app.Set(Appearance.CULLING, 0);
			app.Set(Appearance.LIGHTING, 0);
			app.SetSampler(0, sampler);
			shape.Appearance = app;
			GeoUtil.Rect(mesh, videosize.x / videosize.y, 1.0f, videosize.x / texsize.x, videosize.y / texsize.y);
			shape.Geometry = mesh;
			shape.Name = "videotest.root";
			scene.Models = shape;
			scene.Engines = video;
			cam = scene.Camera;
			cam.Translation = new Vec3(0.0f, 0.0f, 2.0f);
			cam.Hither = 0.1f;
			cam.Name = "videotest.camera";
			return scene;
		}
	}

}
