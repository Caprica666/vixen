using System;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Data;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Microsoft.Win32;
using Microsoft.VisualStudio.OLE.Interop;
using Vixen;
using MagicMirror;
 

namespace AvatarAX
{
	/*!
	 * 3D ActiveX control for animatable, dressable 3D avatar.
	 * Input properties specify initial configuration -
	 * background, avatar, clothing, animations.
	 * 
	 * Implements the Avatar and AvatarEvents interfaces.
	 * @see Avatar AvatarEvents AvatarScene
	 */
	[
		Guid("3b38a5dc-af68-4b2a-9b5b-8e48c60f12b5"),
		ProgId("AvatarAX.AvatarControl"),  
		ComVisible(true),
		Serializable,
		ClassInterface(ClassInterfaceType.None),
		ComSourceInterfaces(typeof(AvatarEvents))
	]

	public partial class AvatarControl : Form3D, Avatar, IObjectSafety
	{
		public dynamic ConfigOpts = null;
		protected AvatarScene scene = null;

		public AvatarControl()
		{
			SharedWorld.DoAsyncLoad = false;
			UsePhysics = true;
			InitializeComponent();
			scene = new AvatarScene();
		}

		#region AvatarInterface

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

		/*!
		 * Load application configuration information from a URL in JSON format.
		 * The garments in the closet are defined but their 3D content is not loaded.
		 * 
		 * @see LoadGarment
		 */
		public dynamic LoadConfig(String url)
		{
			ConfigOpts = scene.LoadConfig(url);
			return ConfigOpts;
		}
		#endregion

		#region AvatarEvents interface
		public event AvatarScene.OnLoadSceneHandler OnLoadScene;
		public event AvatarScene.OnSetSceneHandler OnSetScene;
		public event AvatarScene.OnStopHandler OnStop;
		public event AvatarScene.OnInitHandler OnInit;
		public event AvatarScene.OnLoadAvatarHandler OnLoadAvatar;
		public event AvatarScene.OnLoadAnimHandler OnLoadAnim;

		protected override void OnSceneLoad(Scene s, string url)
		{
			scene.OnSceneLoad(Viewer, s, url);
		}
		#endregion

		/*
		 * Transfer event delegates from this control to the
		 * AvatarScene object that raises and handles them.
		 */
		protected void TransferDelegates()
		{
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
			scene.Initialize();
			if (ContentFile != null)
				scene.LoadScene(GetMediaPath(ContentFile));
			//Viewer.SetDebugLevel(1);
			return scene.MainScene;
		}

		protected override void OnVixen(VixenEventArgs args)
		{
			Event ev = args.VixEvent;

			base.OnVixen(args);
			scene.OnVixen(ev, Viewer);
		}

		#region IObjectSafety implementation
		private ObjectSafetyOptions m_options =  
            ObjectSafetyOptions.INTERFACESAFE_FOR_UNTRUSTED_CALLER |   
            ObjectSafetyOptions.INTERFACESAFE_FOR_UNTRUSTED_DATA;  
   
        public long GetInterfaceSafetyOptions( ref Guid iid, out int pdwSupportedOptions, out int pdwEnabledOptions)  
        {  
            pdwSupportedOptions = (int)m_options;  
            pdwEnabledOptions = (int)m_options;  
            return 0;  
        }  
  

        public long SetInterfaceSafetyOptions(ref Guid iid, int dwOptionSetMask, int dwEnabledOptions)  
        {  
            return 0;  
        }  
        #endregion

		#region COM Registration
		[ComRegisterFunction()]
		private static void ComRegister(Type t)
		{
			string keyName = @"CLSID\" + t.GUID.ToString("B");
			using (RegistryKey key =

			Registry.ClassesRoot.OpenSubKey(keyName, true))
			{
				key.CreateSubKey("Control").Close();
				using (RegistryKey subkey = key.CreateSubKey("MiscStatus"))
				{
					// 131456 decimal == 0x20180.
					long val = (long)
						(OLEMISC.OLEMISC_INSIDEOUT
						| OLEMISC.OLEMISC_ACTIVATEWHENVISIBLE
						| OLEMISC.OLEMISC_SETCLIENTSITEFIRST);
					subkey.SetValue("", val);
				}
				using (RegistryKey subkey = key.CreateSubKey("TypeLib"))
				{
					Guid libid = Marshal.GetTypeLibGuidForAssembly(t.Assembly);
					subkey.SetValue("", libid.ToString("B"));
				}
				using (RegistryKey subkey = key.CreateSubKey("Version"))
				{
					Version ver = t.Assembly.GetName().Version;
					string version = string.Format("{0}.{1}", ver.Major, ver.Minor);
					subkey.SetValue("", version);
				}
			}
		}

		[ComUnregisterFunction()]
		public static void UnregisterClass(Type key)
		{
			// Delete the entire CLSID\{clsid} subtree for this component.
			string keyName = @"CLSID\" + key.GUID.ToString("B");
			Registry.ClassesRoot.DeleteSubKeyTree(keyName);
		}
		#endregion
	}

}
