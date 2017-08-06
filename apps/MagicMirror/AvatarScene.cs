using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Net;
using System.Web.Helpers;
using Vixen;
using MayaIO;

namespace MagicMirror
{

	/*!
	 * @class AvatarScene 
	 * Implements an animatable 3D avatar that you can dress in different outfits.
	 * Content files for the background, avatar, clothing and animations can be separately loaded.
	 * Events are produced when content is loaded and when the current animation stops.
	 *
	 * @see KinectAvatarScene FashionScene Avatar
	 */
	public class AvatarScene : Avatar
	{
		public bool		SuppressNavigation = false;
		public string	NowPlaying
		{
			get
			{
				if (nowPlaying == null)
					return null;
				if (!nowPlaying.IsClass((uint)SerialID.VX_Animator))
					return null;
				return nowPlaying.Name;
			}
		}

		protected ArrayList		avatars = new ArrayList();
		protected dynamic		config = null;
		protected Puppet        currentAvatar = null;
		protected Garment		currentGarment = null;
		protected Scene         mainScene = null;
		protected Physics		physics = null;
		protected bool          isVisible = false;
		protected Flyer		    navigator = null;
		protected Engine        cameraController = null;
		protected Engine        simRoot = null;
		protected Model			sceneRoot = null;
		protected Model			garmentRoot = null;
		protected ExtModel		interiorRoot = null;
		protected Model         avatarRoot = null;
		protected Closet		closet = null;
		protected bool			sceneLoaded = false;
		protected Scriptor		scriptor = null;
		protected String		sceneFile = null;
		protected Vec4			groundPlane = new Vec4(0.0f, 1.0f, 0.0f, 0.0f);
		protected Engine		nowPlaying = null;
		private Random			_random = new Random();

		public AvatarScene()
		{
		}


		/*!
		 * Returns the Vixen Scene object which encapsulates the 3D environment,
		 * avatar, clothing and 3D UI.
		 */
		public Scene MainScene { get { return mainScene; } }

		#region AvatarEvents interface
		/*!
		 * @param name	base name of the scene loaded (without ".scene" suffix)
		 * @param url	full path to the scene content file loaded
		 * Event generated when a 3D content file has been loaded.
		 */
		public delegate void OnLoadSceneHandler(String name, String url);
		public event OnLoadSceneHandler LoadSceneEvent;

		/*!
		 * @param name	base name of the scene loaded (without ".scene" suffix)
		 * Event generated when the current 3D scene has started to display.
		 */
		public delegate void OnSetSceneHandler(String name);
		public event OnSetSceneHandler SetSceneEvent;

		/*!
		 * @param name	name of the Engine object which stopped
		 *				(the name of the animation)
		 * Event generated when a 3D animation has stopped.
		 */
		public delegate void OnStopHandler(String name);
		public event OnStopHandler StopEvent;

		/*!
		 * Event generated when the 3D window has been initialized and is ready to display.
		 */
		public delegate void OnInitHandler();
		public event OnInitHandler InitSceneEvent;

		/*!
		 * @param name	name of avatar loaded (same name as passed to LoadAvatar)
		 * Event generated when the a 3D avatar content file has finished loading.
		 */
		public delegate void OnLoadAvatarHandler(String name);
		public event OnLoadAvatarHandler LoadAvatarEvent;

		/*!
		 * @param name	name of garment loaded (same name as passed to LoadGarment)
		 * @param url	full path to the scene content file loaded
		 * Event generated when the a 3D clothing content file has finished loading.
		 */
		public delegate void OnLoadGarmentHandler(String name, String url);
		public event OnLoadGarmentHandler LoadGarmentEvent;

		/*!
		 * @param name name of garment selected (same name as passed to LoadGarment)
		 * Event generated when a 3D clothing item is selected.
		 */
		public delegate void OnSelectGarmentHandler(String name);
		public event OnSelectGarmentHandler SelectGarmentEvent;

		/*!
		 * @param name	name of animation loaded (same name as passed to LoadAnimation)
		 * @param url	path to animation file loaded
		 * Event generated when the a 3D avatar content file has finished loading.
		 */
		public delegate void OnLoadAnimHandler(String name, String url);
		public event OnLoadAnimHandler LoadAnimEvent;


		/*!
		 * @param closet	ArrayList with clothing items in closet
		 * Event is generated when a closet configuration file has finished loading.
		 * The garments in the closet may not be loaded yet.
		 */
		public delegate void OnLoadClosetHandler(String url, ArrayList items);
		public event OnLoadClosetHandler LoadClosetEvent;

		/*!
		 * @param message	error message
		 * @param level		error severity level (0 = warning)
		 * Event is generated when an error occurs in the avatar system.
		 * Usually this is a file which failed to load.
		 */
		public delegate void OnErrorHandler(System.String message, int level);
		public event OnErrorHandler ErrorEvent;

		#endregion
	
		#region Avatar interface

		/*!
		 * @param url	URL with 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 * This scene defines the background environment the avatar occupies.
		 */
		public virtual void LoadScene(String url)
		{
			if (interiorRoot != null)
				interiorRoot.FileName = url;
		}

		/*!
		 * @param url	URL with 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 * This scene defines a hairstyle for the avatar.
		 */
		public virtual void LoadHair(String url)
		{
			currentAvatar.LoadHair(url);
		}

		/*!
		 * @param name	name to assign to loaded avatar.
		 * @param url	URL of 3D avatar content to load.
		 * Load a 3D avatar content file (Havok or Vixen format)
		 * and handle a LoadAvatarEvent when load is complete.
		 */
		public virtual void LoadAvatar(String name, String url)
		{
			Puppet p = FindAvatar(name);

			if (p != null)
				ShowAvatar(name);
			else
			{
				currentAvatar = new Puppet(name, url, config.avatar);
				avatarRoot.Append(currentAvatar.BodyModel);
				avatars.Add(currentAvatar);
			}
		}

		/*!
		 * Display the named avatar. This function will only display the avatar
		 * if it has already been loaded. If the name is null, the current avatar is shown.
		 * 
		 * @see LoadAvatar HideAvatar
		 */
		public virtual void ShowAvatar(String name)
		{
			Puppet p = FindAvatar(name);

			if (p == null)
				return;
			if (p != currentAvatar)
				HideAvatar();
			currentAvatar = p;
			p.Show();
		}


		/*!
		 * Hide the named avatar if it is currently displayed.
		 * If the name is null, the current avatar is hidden.
		 * 
		 * @see LoadAvatar ShowAvatar
		 */
		public virtual void HideAvatar()
		{
			if (currentAvatar != null)
				currentAvatar.Hide();
		}

		/*!
		 * @param url		URL to 3D animation file to load.
		 * @param target	name of avatar/garment to apply animation to
		 *				if null, the current avatar is assumed
		 * Load a 3D animation content file (BVH, Havok or Vixen format)
		 * and apply it to the named avatar or garment. The name of the
		 * animation is the base name of the file without the extension.
		 * 
		 * @see PlayAnimation PauseAnimation
		 */
		public virtual void LoadAnimation(String url, String targetname)
		{
			String		ext = url.Substring(url.LastIndexOf('.')).ToLower();
			SharedObj	targobj = null;
			Garment		g = null;

			if (ext == ".scp")
			{
				scriptor.LoadScript(url);
				return;
			}
			if (ext == ".xml")
			{
				if (targetname != null)
					g = FindGarment(targetname);
				else
					g = currentGarment;
				LoadMayaCache(g, url, null, scriptor);
			}
			else
			{
				Puppet p = null;

				if (targetname != null)
				{
					p = FindAvatar(targetname);
					g = FindGarment(targetname);
				}
				else
				{
					if (currentAvatar == null)
						return;
					p = currentAvatar;
				}
				if (p != null)
				{
					targetname = p.AnimSkeleton.Name;
					targetname = targetname.Substring(targetname.IndexOf('.'));
					targobj = p.AnimSkeleton;
				}
				else if (g != null)
				{
					targobj = g.ClothMesh;
					targetname = g.ClothMesh.Name;
				}
				scriptor.Load(url, targetname, 0, targobj);
			}
		}

		/*!
		 * Plays the named animation on the current avatar.
		 * The animation will not be played if it has not already been loaded.
		 * @param name	name of animation to play
		 * @param blend	time in seconds to blend to this animation (0 = no blending)
		 * 
		 * @see LoadAnimation PauseAnimation
		 */
		public virtual void PlayAnimation(String name, float blend)
		{
			SharedWorld world = SharedWorld.Get();

			if (nowPlaying != null)
				return;
			try
			{
				Group g = scriptor.Find(name + '.', Group.FIND_START | Group.FIND_CHILD);

				world.SuspendScene();
				if (g != null)
				{
					scriptor.End();
					nowPlaying = g as Engine;
					scriptor.Begin(g.Name);
				}
				else nowPlaying = null;
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("PlayAnimation EXCEPTION " + ex.Message);
				nowPlaying = null;
			}
			world.ResumeScene();
		}

		/*!
		 * Pauses the currently playing animation.
		 * 
		 * @see LoadAnimation PlayAnimation
		 */
		public virtual void PauseAnimation(String name)
		{
			try
			{
				if (name == null)
					scriptor.End();
				else
					scriptor.End(name);
				nowPlaying = null;
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("PauseAnimation EXCEPTION " + ex.Message);
			}
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
			try
			{
				Object temp;
				string result = null;
				dynamic anims = config.animation;

				temp = anims[category];
				if (temp == null)
					return null;
				if (temp.GetType() == typeof(String))
					result = temp as String;
				else do
				{
					dynamic arr = temp;
					Int32 n = _random.Next(arr.Length);
					result = arr[n] as string;
					if ((arr.Length > 1) && (result == NowPlaying))
						result = null;
				}
				while (result == null);
				nowPlaying = null;
				PlayAnimation(result, blend);
			}
			catch (Exception) { }
			return null;
		}
	
		/*!
		 * @param name	name to assign to loaded garment.
		 * @param url	URL to 3D content file for garment.
		 * Load a 3D clothing content file (Havok format)
		 * and handle a LoadGarmentEvent when load is complete.
		 */
		public virtual void LoadGarment(String name, String url)
		{
			closet.Load(name, url);
		}

		/*!
		 * Display the named garment on the avatar. This function will only display the clothing
		 * if it has already been loaded.
		 * 
		 * @see LoadGarment HideGarment
		 */
		public virtual void ShowGarment(String name)
		{
			Garment g = closet.Find(name);

			if (g != null)
			{
				if (!g.IsLoaded)
					g.Load(null);
				else
					g.Show();
				currentGarment = g;
			}
			else
				SharedWorld.LogError("Cannot find garment " + name);
		}

		/*!
		 * Hides the named garment if it is currently draped on the avatar.
		 * 
		 * @see ShowGarment LoadGarment
		 */
		public virtual void HideGarment(String name)
		{
			Garment g = closet.Find(name);

			if (g != null)
				g.Hide();
			else
				SharedWorld.LogError("Cannot find garment " + name);
		}

		/*!
		 * Load a closet description from a URL which returns a JSON string.
		 * The garments in the closet are defined but their 3D content is not loaded.
		 * 
		 * @see LoadGarment
		 */
		public virtual void LoadCloset(String url)
		{
			WebClient	client = new WebClient();
			Uri			uri = new Uri(url);
			string json_data = client.DownloadString(uri);
			try
			{
				if (closet.LoadJSON(json_data))
				{
					if (LoadClosetEvent != null)
						LoadClosetEvent(url, ((dynamic)closet).Items as ArrayList);
				}
			}
			catch (Exception ex)
			{
				throw (new System.IO.FileFormatException(new Uri(url), ex));
			}
		}


		/*!
		 * Load a configuration file from a URL which returns a JSON string.
		 */
		public virtual dynamic LoadConfig(String url)
		{
			WebClient client = new WebClient();
			Uri uri = new Uri(url);
			string json_data = client.DownloadString(uri);
			try
			{
				dynamic result = Json.Decode(json_data);
				if (result != null)
				{
					config = result;
					return result;
				}
			}
			catch (Exception ex)
			{
				throw (new System.IO.FileFormatException(new Uri(url), ex));
			}
			return null;
		}
		#endregion

		public virtual void Initialize()
		{
			Scene scene = MakeScene();
            if (config.timeinc != null)
            {
                double t = Double.Parse(config.timeinc);
                scene.TimeInc = (float)t;
            }
		}

		/*!
		 * @param name	name of avatar to fine.
		 * Searches for the named avatar in the list of previously loaded avatars.
		 * @returns avatar with given name or null if not found
		 */
		public Puppet FindAvatar(String name)
		{
			if ((currentAvatar != null) &&
				(currentAvatar.Name == name))
				return currentAvatar;
			foreach (Puppet p in avatars)
			{
				if (p.Name == name)
					return p;
			}
			return null;
		}

		/*!
		 * @param name	name of garment to fine.
		 * Searches for the named grament in the current closet.
		 * @returns Garment with given name or null if not found
		 */
		public Garment FindGarment(String name)
		{
			if (closet != null)
				return closet.Find(name);
			return null;
		}

		/*!
		 * @param name	name of garment to fine.
		 * Searches for the named grament in the current closet.
		 * @returns Garment with given name or null if not found
		 */
		public Engine FindAnimation(String name)
		{
			Group g = scriptor.Find(name + '.', Group.FIND_START | Group.FIND_CHILD);
			if (g == null)
				return null;
			if (!typeof(Engine).IsAssignableFrom(g.GetType()))
				return null;
			return g as Engine;
		}

		protected virtual Engine MakeEngines()
		{
			simRoot = new Engine();
			simRoot.Name = "magicmirror.simroot";
			physics = new Physics();
			physics.Name = "magicmirrot.physics";
			physics.Active = false;
			simRoot.Append(physics);
			scriptor = new Scriptor();
			scriptor.Name = "magicmirror.scriptor";
			simRoot.Append(scriptor);
			cameraController = new Transformer();
			cameraController.Name = "magicmirror.camera.xform";
			simRoot.Append(cameraController);
			if (!SuppressNavigation)
			{
				navigator = new Flyer();
				if (typeof(Flyer).IsAssignableFrom(navigator.GetType()))
				{
					navigator.DegreesOfFreedom = Flyer.Y_ROT | Flyer.X_TRANS | Flyer.Y_TRANS | Flyer.Z_TRANS;
					navigator.Speed = 15.0f;
				}
				else
				{
					navigator.Speed = 0.1f;
					navigator.TurnSpeed *= 0.05f;
				}
				navigator.Name = "magicmirror.navigator";
				navigator.SetFlags((uint)SharedObj.DOEVENTS);
				cameraController.Append(navigator);
			}
			return simRoot;
		}

		protected virtual Model MakeModels()
		{
			interiorRoot = new ExtModel();
			interiorRoot.Name = "magicmirror.environment";
			interiorRoot.Options |= ExtModel.REMOVE;
			interiorRoot.ExtName = "root";
			avatarRoot = new Model();
			avatarRoot.Name = "magicmirror.avatars";
			garmentRoot = new Model();
			garmentRoot.Name = "magicmirror.garments";
			sceneRoot = new Model();
			sceneRoot.Name = "magicmirror.root";
			sceneRoot.Append(interiorRoot);
			sceneRoot.Append(avatarRoot);
			sceneRoot.Append(garmentRoot);
			return sceneRoot;
		}

		protected virtual Scene MakeScene()
		{
			AmbientLight ambient = new AmbientLight();

 			sceneRoot = new Model();
			mainScene = new Scene();
			mainScene.Name = "magicmirror.scene";
			mainScene.BackColor = new Col4(0.2f, 0.2f, 0.2f);
			if (Scene.RenderOptions == "embree")
				ambient.Color = new Col4(0.0f, 0.0f, 0.0f, 0.0f);
			else
				ambient.Color = new Col4(0.1f, 0.1f, 0.1f, 0.0f);
			mainScene.Ambient = ambient;
			mainScene.Models = MakeModels();
			mainScene.Engines = MakeEngines();
			return mainScene;
		}

		protected virtual Skeleton ConnectAvatar(SharedWorld world, Scene scene)
		{
			Skeleton skeleton = null;
			Scene displayScene = world.GetScene();
			try
			{
				if (currentAvatar.IsActive)
					return currentAvatar.BodyPoser;
				if (currentAvatar.IsHavokFile)
					skeleton = currentAvatar.ConnectBody(physics);
				else
				{
					skeleton = currentAvatar.ConnectBody(scene.Engines);
					skeleton.Remove(false);
					physics.Append(skeleton);
				}
				world.Observe(Event.LOAD_SCENE, skeleton);
				if (skeleton != null)
				{
					if (scriptor != null)
					{
						scriptor.Directory = config.mediadir;
						scriptor.Target = currentAvatar.AnimSkeleton;
						if (!currentAvatar.AnimSkeleton.Name.Contains("amy"))
							currentAvatar.AnimSkeleton.GetBone(0).SetOptions(Transformer.RELATIVE);
						if (config.script != null)
							scriptor.LoadScript(config.script);
						currentAvatar.IsActive = true;
						if (config.animation != null)
						{
							dynamic animinfo = config.animation;
							if (animinfo.scale != null)
							{
								string s = animinfo.scale;
								double d = Double.Parse(s);
								if (d > 0)
									Scriptor.SetAnimationScale((float)d);
							}
						}
					}
				 }
				else
					return null;
			}
			catch (Exception)
			{
				// do nothing for now
			}
			return skeleton;
		}

		public bool ConnectHair(SharedWorld world, Scene scene)
		{
			Skeleton hairskel = null;
			Skeleton bodyskel = null;
			Scene displayScene = world.GetScene();
			try
			{
				bodyskel = currentAvatar.BodyPoser;
				if (bodyskel == null)
				{
					SharedWorld.LogError("MagicMirror: Cannot connect hair - avatar skeleton missing");
					return false;
				}
				hairskel = currentAvatar.ConnectHair(displayScene.Engines);
				if (hairskel == null)
				{
					SharedWorld.LogError("MagicMirror: could not connect hair " + scene.Name);
					return false;
				}
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("MagicMirror: exception connecting hair " + ex.Message);
				return false;
			}
			return true;
		}

		/*!
		 * @param ev	Vixen event to handle.
		 * @param world	current world
		 * Handles vixen events - content loading and animation.
		 * This function also calls event handlers for AvatarEvents.
		 */
		public virtual void OnVixen(Event ev, SharedWorld world)
		{
			int code = ev.Code;

			if (code == Event.STOP)
			{
				OnStopAnim(ev.Sender);
				return;
			}
			if (code == Event.LOAD_SCENE)
			{
				LoadSceneEvent le = ev as LoadSceneEvent;
				String fname = le.FileName;
				SharedObj obj = le.Object;

				if (obj != null)
				{
					Scene scene = obj as Scene;
					string name = obj.Name;
					int p = name.IndexOf('.');

					if (p > 0)
						name = name.Substring(0, p);
					if (scene != null)
					{
						if (scene.Models != null)
							OnSceneLoad(world, scene, fname);
						else if (scene.Engines != null)
							RaiseAnimLoadEvent(name, fname);
						return;
					}
					if (((obj as Skeleton) != null) || ((obj as MeshAnimator) != null))
						RaiseAnimLoadEvent(name, fname);
				}
				return;
			}
			if (code == Event.LOAD_TEXT)
			{
				LoadTextEvent le = ev as LoadTextEvent;

				OnLoadText(le.FileName, le.Text);
				return;
			}
			if (code == Event.SCENE_CHANGE)
			{
				SceneEvent se = ev as SceneEvent;
				OnSceneChange(world, se.Target as Scene);
				return;
			}
			if (code == Event.ERROR)
			{
				ErrorEvent err = ev as ErrorEvent;
				RaiseErrorEvent(err.ErrString, err.ErrLevel);
				return;
			}
		}
		
		/*!
		 * @param sender	Engine which stopped
		 * Called to process a Vixen STOP event. This occurs
		 * when any Vixen Engine stops running. The animation
		 * playing on the avatar comes from the BoneAnimator engine.
		 * If a STOP event from the BoneAnimator is detected,
		 * the StopEvent handler is invoked.
		 */
		protected virtual void OnStopAnim(SharedObj sender)
		{
			if (StopEvent != null)
				StopEvent(sender.Name);
			if ((nowPlaying != null) && (sender.Name == nowPlaying.Name))
				nowPlaying = null;
		}

		/*!
		 * @param world	current world
		 * @param scene	new scene which has replaced current 3D scene
		 * 
		 * Called when the 3D scene is replaced with completely new content.
		 * If this is the first scene change, the event was generated during
		 * initialization and the InitSceneEvent handler is invoked.
		 * For any scene change the SetSceneEvent handler is invoked (not
		 * just the first one).
		 */
		protected virtual void OnSceneChange(SharedWorld world, Scene scene)
		{
			if (scene.SameAs(mainScene))
			{
				world.Observe(Event.ENTER);
				world.Observe(Event.LEAVE);
				world.Observe(Event.STOP);
				world.Observe(Event.SELECT);
				world.Observe(Event.DESELECT);
				world.Observe(Event.LOAD_TEXT);
				if (cameraController != null)
				{
					world.Observe(Event.NAVINPUT, cameraController);
					world.Observe(Event.NAVIGATE, cameraController);
					cameraController.Target = scene.Camera;
					SharedWorld.Trace("MagicMirror: " + cameraController.Name + " -> target " + scene.Camera.Name + "\n");
				}
				if (navigator != null)
				{
					world.Observe(Event.MOUSE, navigator);
					world.Observe(Event.NAVINPUT, navigator);
				}
				var a = config.avatar;
				if (a != null)
				{
					if ((a.name != null) && (a.filename != null))
						LoadAvatar(a.name, a.filename);
				}
				if (InitSceneEvent != null)
					InitSceneEvent();
				return;
			}
			if (SetSceneEvent != null)
				SetSceneEvent(scene.Name);
		}

		/*!
		 * @param world	current world
		 * @param scene	avatar which finished loading
		 * 
		 * Called when an avatar has finished loading. 
		 * This function always invokes the LoadAvatarEvent handler.
		 */
		public virtual void OnAvatarLoad(SharedWorld world, Scene scene)
		{
			Skeleton skel = ConnectAvatar(world, scene);
            if (skel == null)
                return;
            SharedWorld.Trace("Loaded avatar, creating closet");
			if (closet == null)
			{
				closet = new Closet(garmentRoot, skel);
				closet.scriptor = scriptor;
			}
            if (config.interior != null)
                LoadScene(Path.Combine(config.mediadir, config.interior));
            if (config.closet != null)
                LoadCloset(Path.Combine(config.mediadir, config.closet));
            if (config.hair != null)
                LoadHair(Path.Combine(config.mediadir, config.hair));
			if (LoadAvatarEvent != null)
				LoadAvatarEvent(currentAvatar.Name);
		}

		/*!
		 * @param world	current world
		 * @param scene	scene which finished loading
		 * 
		 * Called when a 3D content file has finished loading. This event
		 * will always come before OnSetScene but this function is not
		 * guaranteed to be called before th 3D scene has been replaced.
		 * This function always invokes the LoadSceneEvent handler.
		 */
		public virtual void OnSceneLoad(SharedWorld world, Scene scene, String scenefile)
		{
			System.String name = scene.Name.ToLower();

			if (name.EndsWith(".scene"))
				name = name.Substring(0, name.Length - 6);
			SharedWorld.Trace("Loaded " + name);
			if (LoadSceneEvent != null)
				LoadSceneEvent(name, scenefile);
			if (scenefile.Contains("avatar"))
			{
				OnAvatarLoad(world, scene);
				return;
			}
			if (name.Contains("hair"))
			{
				ConnectHair(world, scene);
				return;
			}
			else if (!sceneLoaded && (interiorRoot.FileName == scenefile))
			{
				sceneLoaded = true;
				world.SuspendScene();
				Camera mainCam = world.GetScene().Camera;
				Box3 vvol = scene.Camera.ViewVol;
				float aspect = mainCam.Aspect;
				mainCam.Copy(scene.Camera);
				mainCam.Aspect = aspect;
				if (cameraController != null)
					cameraController.Target = mainCam;
				world.ResumeScene();
				if (scriptor != null)
					scriptor.LoadScript(scenefile.Substring(0, scenefile.Length - 3) + "scp");
				return;
			}
			/*
			 * Not background or avatar, assume it is clothing
			 */
			if (closet == null)
				return;
			world.SuspendScene();
			dynamic g = closet.OnLoad(scene, world, config.avatar);
			if (g == null)
			{
				world.ResumeScene();
				return;
			}
			if (g.script != null)
				PlayAnimation(g.script, 0);
			if (g.ClothSim != null)
			{
				if (g.ClothSim.Parent() == null)
					physics.Append(g.ClothSim);
				if (typeof(ClothSim).IsAssignableFrom(g.ClothSim.GetType()))
					((ClothSim) g.ClothSim).GroundPlane = groundPlane;
			}
			currentGarment = g;
			world.ResumeScene();
			if (LoadGarmentEvent != null)
				LoadGarmentEvent(g.name, scenefile);
		}

		protected void OnLoadText(string url, string text)
		{
			if (url.EndsWith(".xml"))			// Is a Maya animation cache?
				LoadMayaCache(currentGarment, url, text, scriptor);
		}

		/*!
		 * Loads a Maya geometry cache for cloth presimulation.
		 * @param	g			Garment which is being simulated by the cache
		 * @param	url			URL or path to the cache description file (XML)
		 * @param	text		if not null, this parameter is used as the XML cache description
		 * @param	scriptor	Scriptor object to add this animation to
		 *
		 * @returns true if cache successfully loaded and connected, else false
		 */
		public static bool LoadMayaCache(Garment g, String url, String text, Scriptor scriptor)
		{
			dynamic d = g;

			if (g == null)
				return false;
			if ((g.ClothSim == null) || !g.ClothSim.IsClass((uint)SerialID.VX_MeshAnimator))
				return false;

			LoadSceneEvent		loadevent;
			string				name = Path.GetFileNameWithoutExtension(url);
			MeshAnimator		clothsim = g.ClothSim.Clone() as MeshAnimator;
			Animator			anim;
			MayaIO.MayaCacheFile import = new MayaIO.MayaCacheFile(clothsim);
			String				engname = d.name + ".meshanim";
			String				animname;

			/*
			 * Make a MeshAnimator to control the cloth vertices in the cloth mesh.
			 * The mesh sequence will be loaded into this engine.
			 */
			clothsim.Name = name;
			if (clothsim.Target == null)
				clothsim.Target = g.ClothMesh;
			/*
			 * Make an Animator to control the mesh animation we are loading.
			 * It is attached to the Scriptor that should control it.
			 */
			name += "." + engname;
			animname = name + ".anim";
			anim = scriptor.MakeAnim(name, g.ClothMesh, false);
			anim.SetEngineName(engname);
			anim.SetFileName(url);
			SharedWorld.Get().Observe(Event.LOAD_SCENE, anim);
			/*
			 * Make a load event to signal the mesh animation has been loaded.
			 * The event will come from the Animator and will contain the
			 * MeshAnimator with the mesh sequences loaded.
			 */
			loadevent = new LoadSceneEvent(Event.LOAD_SCENE);
			loadevent.Sender = anim;
			if (loadevent.Sender == null)
				loadevent.Sender = clothsim;
			loadevent.Object = clothsim;
			loadevent.FileName = url;
			/*
			 * Start the import of the Maya me4sh sequence.
			 * This occurs asynchronously in a separate thread.
			 */
			if (text != null)
			{
				import.FileName = url;
				import.LoadString(text, loadevent);
			}
			else
				import.LoadFile(url, loadevent);
			return true;
		}

		protected void RaiseSceneLoadEvent(String name, String url)
		{
			if (LoadSceneEvent != null)
				LoadSceneEvent(name, url);
		}

		protected void RaiseAnimLoadEvent(String name, String url)
		{
			if (LoadAnimEvent != null)
				LoadAnimEvent(name, url);
		}

		protected void RaiseSelectGarmentEvent(String name)
		{
			if (SelectGarmentEvent != null)
				SelectGarmentEvent(name);
		}

		protected void RaiseErrorEvent(String message, int level)
		{
			if (this.ErrorEvent != null)
				this.ErrorEvent(message, level);
		}

		protected void LogErrorEvent(String message, int level)
		{
			Vixen.ErrorEvent err = new Vixen.ErrorEvent(World3D.Get());

			err.ErrString = message;
			err.ErrLevel = level;
			err.ErrCode = 0;
			err.Log();
		}
	}
}
