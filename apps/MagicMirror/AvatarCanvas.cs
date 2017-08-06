using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Interop;
using System.IO;
using System.Reflection;

using Vixen;

namespace MagicMirror
{
	/*!
	 * 3D WPF control for virtual dressing room viewer.
	 * Input properties specify initial configuration -
	 * background, avatar, clothing, animations.
	 */
	public class AvatarCanvas : Canvas3D, Avatar
	{
		/*!
		 * URL of the JSON file describing the closet.
		 * Indicates the name of each garment, the URL to the 3D content
		 * and the URL to the 2D thumbnail.
		 */
		public string ClosetFile
		{
			get { return base.GetValue(ClosetFileProp) as System.String; }
			set { base.SetValue(ClosetFileProp, value); }
		}

		/*!
		 * String with the URL or path to the hair 3D content file to load.
		 */
		public string ConfigFile
		{
			get { return base.GetValue(ConfigFileProp) as System.String; }
			set { base.SetValue(ConfigFileProp, value); }
		}

		public static readonly DependencyProperty ClosetFileProp = DependencyProperty.Register("ClosetFile", typeof(System.String), typeof(AvatarCanvas));
		public static readonly DependencyProperty ConfigFileProp = DependencyProperty.Register("ConfigFile", typeof(System.String), typeof(AvatarCanvas));
		public dynamic ConfigOpts = null;
		protected AvatarScene scene = null;

		public AvatarCanvas()
			: base()
		{
			Canvas3D.Debug = 0;
			World.DoAsyncLoad = false;
		}

		protected override void OnInitialized(System.EventArgs e)
		{
			SharedWorld world;
			string fname;

			if (scene == null)
				scene = new AvatarScene();
			base.OnInitialized(e);
			world = SharedWorld.Get();
			if (MediaDir == null)
				MediaDir = world.GetMediaDir();
			fname = world.FileName;
			if ((fname != null) && fname.ToLower().EndsWith(".json"))
				ConfigOpts = LoadConfig(fname);
			else if (ConfigFile != null)
				ConfigOpts = LoadConfig(ConfigFile);
			if (ConfigOpts.mediadir != null)
				MediaDir = ConfigOpts.mediadir;
			Scene.RenderOptions = ConfigOpts.render;
		}

		#region "Avatar Methods"
		/*!
		 * @param url	URL of 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 */
		public void LoadScene(string url)
		{
			scene.LoadScene(GetMediaPath(url));
		}

		/*!
		 * @param url	URL with 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 * This scene defines a hairstyle for the avatar.
		 */
		public void LoadHair(String url)
		{
			scene.LoadHair(GetMediaPath(url));
		}

		/*!
		 * Display the named avatar. This function will only display the avatar
		 * if it has already been loaded. If the name is null, the current avatar is shown.
		 * 
		 * @see LoadAvatar HideAvatar
		 */
		public void LoadAvatar(string name, string url)
		{
			scene.LoadAvatar(name, GetMediaPath(url));
		}

		/*!
		 * Hide the named avatar if it is currently displayed.
		 * If the name is null, the current avatar is hidden.
		 * 
		 * @see LoadAvatar ShowAvatar
		 */
		public void ShowAvatar(string name)
		{
			scene.ShowAvatar(name);
		}

		/*!
		 * Hide the named avatar if it is currently displayed.
		 * If the name is null, the current avatar is hidden.
		 * 
		 * @see LoadAvatar ShowAvatar
		 */
		public void HideAvatar()
		{
			scene.HideAvatar();
		}

		/*!
		 * @param url		URL to 3D animation file to load.
		 * @param target	name of avatar/garment to apply animation to
		 *					if null, the current avatar is assumed
		 * Load a 3D animation content file (BVH, Havok or Vixen format)
		 * and apply it to the named avatar or garment. The name of the
		 * animation is the base name of the file without the extension.
		 * 
		 * @see PlayAnimation PauseAnimation
		 */
		public void LoadAnimation(string url, string target)
		{
			scene.LoadAnimation(GetMediaPath(url), target);
		}

		/*!
		 * Plays the named animation on the current avatar.
		 * The animation will not be played if it has not already been loaded.
		 * @param name		name of the animation to play.
		 * @param blend		time to blend to this animation (0 = no blending).
		 * 
		 * @see LoadAnimation PauseAnimation RandomAnimation
		 */
		public void PlayAnimation(string name, float blend)
		{
			scene.PlayAnimation(name, blend);
		}

		/*!
		 * Pauses the currently playing animation.
		 * 
		 * @see LoadAnimation PlayAnimation
		 */
		public void PauseAnimation(string name)
		{
			scene.PauseAnimation(name);
		}

		/*
		 * Select and play a random animation from the named category.
		 * @param category	animation category
		 * @param blend		time to blend to this animation (0 = no blending)
		 * 
		 * The animation categories are defined in the JSON config
		 * file in the "animation" section.
		 */
		public string RandomAnimation(string category, float blend)
		{
			return scene.RandomAnimation(category, blend);
		}
			
		/*!
		 * @param name	name to assign to loaded garment.
		 * @param url	URL to 3D content file for garment.
		 * Load a 3D clothing content file (Havok format)
		 * and handle a LoadGarmentEvent when load is complete.
		 */
		public void LoadGarment(string name, string url)
		{
			scene.LoadGarment(name, url);
		}

		/*!
		 * Display the named garment on the avatar. This function will only display the clothing
		 * if it has already been loaded.
		 * 
		 * @see LoadGarment HideGarment
		 */
		public void ShowGarment(string name)
		{
			scene.ShowGarment(name);
		}

		/*!
		 * Hides the named garment if it is currently draped on the avatar.
		 * 
		 * @see ShowGarment LoadGarment
		 */
		public void HideGarment(string name)
		{
			scene.HideGarment(name);
		}

		/*!
		 * Load a closet description from a URL in JSON format.
		 * The garments in the closet are defined but their 3D content is not loaded.
		 * 
		 * @see LoadGarment
		 */
		public void LoadCloset(string name)
		{
			scene.LoadCloset(name);
		}

		public dynamic LoadConfig(string url)
		{
			return scene.LoadConfig(GetMediaPath(url));
		}

		#endregion

		#region AvatarEvents interface
		public event AvatarScene.OnLoadSceneHandler OnLoadScene;
		public event AvatarScene.OnSetSceneHandler OnSetScene;
		public event AvatarScene.OnStopHandler OnStop;
		public event AvatarScene.OnInitHandler OnInit;
		public event AvatarScene.OnLoadAvatarHandler OnLoadAvatar;
		public event AvatarScene.OnLoadAnimHandler OnLoadAnim;
		public event AvatarScene.OnSelectGarmentHandler OnSelectGarment;
		public event AvatarScene.OnLoadGarmentHandler OnLoadGarment;
		public event AvatarScene.OnLoadClosetHandler OnLoadCloset;

		protected override void OnSceneLoad(Scene s, string url)
		{
			scene.OnSceneLoad(Viewer, s, url);
		}
		#endregion

		/*
		 * Transfer event delegates from this control to the
		 * AvatarScene object that raises and handles them.
		 */
		public void TransferDelegates()
		{
			scene.SelectGarmentEvent += OnGarmentSelected;
			if (OnSelectGarment != null)
				scene.SelectGarmentEvent += OnSelectGarment;
			if (OnLoadGarment != null)
				scene.LoadGarmentEvent += OnLoadGarment;
			if (OnInit != null)
				scene.InitSceneEvent += OnInit;
			if (OnLoadScene != null)
				scene.LoadSceneEvent += OnLoadScene;
			if (OnSetScene != null)
				scene.SetSceneEvent += OnSetScene;
			if (OnLoadAvatar != null)
				scene.LoadAvatarEvent += OnLoadAvatar;
			if (OnStop != null)
				scene.StopEvent += OnStop;
			if (OnLoadAnim != null)
				scene.LoadAnimEvent += OnLoadAnim;
			if (OnLoadCloset != null)
				scene.LoadClosetEvent += OnLoadCloset;
		}

		public Garment FindGarment(string name)
		{
			return scene.FindGarment(name);
		}

		public Engine FindAnimation(string name)
		{
			return scene.FindAnimation(name);
		}

		public override Scene MakeScene()
		{
			if (MediaDir != null)
				Directory.SetCurrentDirectory(MediaDir);
			TransferDelegates();
			scene.Initialize();
			SharedWorld.DoAsyncLoad = true;
			//Viewer.SetDebugLevel(1);
			return scene.MainScene;
		}

		protected override void OnVixen(VixenEventArgs args)
		{
			if (scene != null)
				scene.OnVixen(args.VixEvent, Viewer);
		}

		virtual protected void OnGarmentSelected(String name)
		{
			scene.LoadGarment(name, null);
		}
	}
}

