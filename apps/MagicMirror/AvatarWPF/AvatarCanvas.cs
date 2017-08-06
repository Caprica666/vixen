using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Interop;
using System.IO;
using System.Reflection;

using Vixen;
using MagicMirror;

namespace MagicMirror.Viewer
{
	/*!
	 * 3D WPF control for virtual dressing room viewer.
	 * Input properties specify initial configuration -
	 * background, avatar, clothing, animations.
	 */
	public partial class AvatarCanvas : Canvas3D, Avatar
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
		 * String describing physics options.
		 * - "none"		indicates no physics is required.
		 *				physics will not be initialized and physics files
		 *				may fail to load.
		 * - "havok"	indicates Havok physics should be used.
		 *				physics will be initialized and you will
		 *				be able to load HKT files.
		 */
		public string PhysicsOptions
		{
			get { return base.GetValue(PhysicsOptionsProp) as System.String; }
			set { base.SetValue(PhysicsOptionsProp, value);  }
		}

		/*!
		 * String with the name of the avatar to load.
		 * The URL for the avatar is computed based on this name,
		 * the media path and the physics properties.
		 */
		public string AvatarName
		{
			get { return base.GetValue(AvatarNameProp) as System.String; }
			set { base.SetValue(AvatarNameProp, value); }
		}

		/*!
		 * String with the URL or path to the Vixen script file to run on startup.
		 * This file can preload animations to play on the avatar.
		 */
		public string ScriptFile
		{
			get { return base.GetValue(ScriptFileProp) as System.String; }
			set { base.SetValue(ScriptFileProp, value); }
		}

		public static readonly DependencyProperty PhysicsOptionsProp = DependencyProperty.Register("PhysicsOptions", typeof(System.String), typeof(AvatarCanvas));
		public static readonly DependencyProperty AvatarNameProp = DependencyProperty.Register("AvatarName", typeof(System.String), typeof(AvatarCanvas));
		public static readonly DependencyProperty ClosetFileProp = DependencyProperty.Register("ClosetFile", typeof(System.String), typeof(AvatarCanvas));
		public static readonly DependencyProperty ScriptFileProp = DependencyProperty.Register("ScriptFile", typeof(System.String), typeof(AvatarCanvas));

		private AvatarScene scene;

		public AvatarCanvas()
			: base()
		{
			Canvas3D.Debug = 0;
			World.DoAsyncLoad = false;
			scene = new AvatarScene();
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
		 * @param url	URL to 3D animation file to load.
		 * Load a 3D animation content file (BVH or Vixen format)
		 * and apply it to the current avatar.
		 * 
		 * @see PlayAnimation PauseAnimation
		 */
		public void LoadAnimation(string url)
		{
			scene.LoadAnimation(GetMediaPath(url));
		}

		/*!
		 * Plays the named animation on the current avatar.
		 * The animation will not be played if it has not already been loaded.
		 * 
		 * @see LoadAnimation PauseAnimation
		 */
		public void PlayAnimation(string name)
		{
			scene.PlayAnimation(name);
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
		#endregion

		#region AvatarEvents interface
		public event AvatarScene.OnLoadSceneHandler OnLoadScene;
		public event AvatarScene.OnSetSceneHandler OnSetScene;
		public event AvatarScene.OnStopHandler OnStop;
		public event AvatarScene.OnInitHandler OnInit;
		public event AvatarScene.OnLoadAvatarHandler OnLoadAvatar;
		public event AvatarScene.OnLoadAnimHandler OnLoadAnim;
		public event AvatarScene.OnSelectGarmentHandler OnSelectGarment;

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
				scene.SelectGarmentEvent += OnGarmentSelected;
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
		}

		public override Scene MakeScene()
		{
			if (MediaDir != null)
				Directory.SetCurrentDirectory(MediaDir);
			TransferDelegates();
			SharedWorld.DoAsyncLoad = true;
			World.DoAsyncLoad = true;
			scene.PhysicsOptions = PhysicsOptions;
			scene.ScriptFile = GetMediaPath(ScriptFile);
			scene.Initialize();
			if (ContentFile != null)
				scene.LoadScene(GetMediaPath(ContentFile));
			if (AvatarName != null)
				scene.LoadAvatar(AvatarName, GetAvatarFileName(AvatarName));
			//Viewer.SetDebugLevel(1);
			return scene.MainScene;
		}
	

		public System.String GetAvatarFileName(String name)
		{
			String fname;
			if (PhysicsOptions.ToLower().Contains("havok"))
				fname = name + "/" + name + "avatar.hkt";
			else
				fname = name + "/" + name + "avatar.vix";
			return GetMediaPath(fname);
		}

		protected override void OnVixen(VixenEventArgs args)
		{
			if (scene != null)
				scene.OnVixen(args.VixEvent, Viewer);
		}

		protected void OnGarmentSelected(String name)
		{
			scene.LoadGarment(name, null);
		}
	}
}

