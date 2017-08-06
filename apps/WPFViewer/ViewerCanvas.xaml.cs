using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Interop;
using System.IO;
using System.Reflection;

using Vixen;

namespace Vixen.Viewer
{
	/*!
	 * 3D WPF control for Vixen room viewer.
	 */
	public partial class ViewerCanvas : Canvas3D
	{
		private Microsoft.Win32.OpenFileDialog openFileDialog;
		private Microsoft.Win32.SaveFileDialog saveFileDialog;
		protected Skeleton _skeleton = null;
		protected string _animName = null;
		protected string _skelName = null;
		protected Scriptor _scriptor = null;

		public ViewerCanvas()
			: base()
		{
			openFileDialog = new Microsoft.Win32.OpenFileDialog();
			saveFileDialog = new Microsoft.Win32.SaveFileDialog();
		}

		#region Events
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
		 * @param name	name of animation loaded (same name as passed to LoadAnimation)
		 * @param url	path to animation file loaded
		 * Event generated when the a 3D avatar content file has finished loading.
		 */
		public delegate void OnLoadAnimHandler(String name, String url);
		public event OnLoadAnimHandler LoadAnimEvent;

		public delegate void OBJLoadHandler(Scene scene, String filename);
		public event OBJLoadHandler OBJLoaded;
		#endregion


		public Scriptor AnimScriptor
		{
			get
			{
				if (_scriptor != null)
					return _scriptor;
				_scriptor = new Scriptor();
				_scriptor.Name = "viewer.animation.scriptor";
				Viewer.SuspendScene();
				SharedWorld.MainScene.Engines.Append(_scriptor);
				Viewer.ResumeScene();
				return _scriptor;
			}
		}

		/*!
		 * @param url	URL of 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 * @see LoadAnimation
		 */
		public void LoadScene(string url)
		{
			try
			{
				Viewer3D viewer = Viewer as Viewer3D;
				Scene scene = SharedWorld.MainScene;
				Engine simroot = scene.Engines;

				SharedWorld.DoAsyncLoad = true;
				string ext = Path.GetExtension(url).ToLower();
				if (ext == ".obj")
				{
					ObjIO.Import objload = new ObjIO.Import();
					Scene newscene = objload.LoadFile(url);
					if (OBJLoaded != null)
						OBJLoaded(newscene, url);
					else
						Viewer.SetScene(newscene);
				}
				/*
				else if (ext == ".xml")
				{
					ModImport.ModImport import = new ModImport.ModImport();

					import.Load(url);
					Scene newscene = import.ParseScene();
					Viewer.SetScene(newscene);
				}
				 * */
				else if (ext == ".vix")
					viewer.LoadAsync(url, scene);
				else if (ext == ".hkt")
				{
					Physics physics = null;

					if (simroot != null)
						physics = simroot.Find(".physics", Group.FIND_END | Group.FIND_CHILD) as Physics;
					if (physics == null)
					{
						physics = new Physics();
						physics.Name = "viewer.physics";
						physics.Active = false;
						if (simroot != null)
							simroot.Append(physics);
						else
							scene.Engines = physics;
					}
					physics.Active = false;
					viewer.LoadAsync(url, scene);
				}
				else
					throw new System.ArgumentException("invalid extension", ext);
			}
			catch (Exception ex)
			{
				throw new System.IO.FileNotFoundException(ex.Message + ", Cannot open vixen file", url);
			}
		}

		/*!
		 * @param url	URL to 3D animation file to load.
		 * Load a 3D animation content file (BVH or Vixen format)
		 * and apply it to the current skeleton.
		 * 
		 * @see PlayAnimation PauseAnimation
		 */
		public void LoadAnimation(string url)
		{
			Viewer3D viewer = Viewer as Viewer3D;
			Scriptor scriptor = AnimScriptor;
			String ext = url.Substring(url.LastIndexOf('.')).ToLower();
			string skelname = ConnectSkeleton();

			AnimScriptor.Directory = Path.GetDirectoryName(url);
			AnimScriptor.Target = _skeleton;
			if (ext == ".scp")
				scriptor.LoadScript(url);
			else
			{
				_animName = Path.GetFileNameWithoutExtension(url) + skelname;
				scriptor.Load(url, skelname, Animator.AUTO_PLAY, _skeleton);
			}
		}

		/*!
		 * Plays the named animation on the current avatar.
		 * The animation will not be played if it has not already been loaded.
		 * 
		 * @see LoadAnimation PauseAnimation
		 */
		public void PlayAnimation(string name)
		{
			try
			{
				Viewer3D viewer = Viewer as Viewer3D;
				string skelname = ConnectSkeleton();
				Scriptor scp = _scriptor;
				if (skelname != null)
				{
					if (name != null)
						scp.Begin(name + skelname);
					else
						scp.Begin(_animName);
				}
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("PlayAnimation EXCEPTION " + ex.Message);
			}
		}

		/*!
		 * Pauses the currently playing animation.
		 * 
		 * @see LoadAnimation PlayAnimation
		 */
		public void PauseAnimation(string name)
		{
			try
			{
				Viewer3D viewer = Viewer as Viewer3D;
				string skelname = ConnectSkeleton();
				Scriptor scp = AnimScriptor;
				if (skelname != null)
				{
					if (name != null)
						scp.End(name + skelname);
					else
						scp.End(_animName);
				}
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("PauseAnimation EXCEPTION " + ex.Message);
			}
		}

		/*
		 * Come here when "Save Scene" menu item has been chosen.
		 * Brings up file save dialog for scenes.
		 */
		public void SaveScene(string extension, string dir)
		{
			saveFileDialog.InitialDirectory = dir;
			saveFileDialog.DefaultExt = ".vix";
			saveFileDialog.Filter = "Vixen scenes (.vix)|*.vix";
			Nullable<bool> result = saveFileDialog.ShowDialog();
			if (result.Value)
			{
				Scene scene = new Scene();
				string filebase = Path.GetFileNameWithoutExtension(saveFileDialog.FileName);

				Viewer.SuspendScene();
				scene.Models = SharedWorld.MainScene.Models.GetAt(1);
				if (scene.Models != null)
					scene.Models.Name = filebase + ".root";
				scene.Engines = SharedWorld.MainScene.Engines.GetAt(1);
				if (scene.Engines != null)
					scene.Engines.Name = filebase + ".simroot";
				scene.Camera = SharedWorld.MainScene.Camera;
				Viewer.SaveAsVix(SharedWorld.MainScene, saveFileDialog.FileName);
				Viewer.ResumeScene();
			}
		}

		public Scene LoadOBJ(string filename)
		{
			ObjIO.Import objload = new ObjIO.Import();
			return objload.LoadFile(filename);
		}

		public void OpenScene(string extensions, string dir)
		{
			string filter = "";

			if (extensions.Contains("vix"))
				filter += "Vixen scene files (*.vix)|*.vix|";
			if (extensions.Contains("hkt"))
				filter += "Havok scene files (*.hkt)|*.hkt|";
			if (extensions.Contains("obj"))
				filter += "Wavefront OBJ files (*.obj)|*.obj|";
			if (extensions.Contains("xml"))
				filter += "OptiTex modml files (*.xml)|*.xml|";
			if (extensions.Contains("bvh"))
				filter += "Biovision motion files (*.bvh)|*.bvh|";
			if (extensions.Contains("scp"))
				filter += "Vixen script files (*.scp)|*.scp|";
			filter += "All Files (*.*)|*.*";
			openFileDialog.Filter = filter;
			if (dir != null)
			{
				openFileDialog.InitialDirectory = dir;
				Directory.SetCurrentDirectory(dir);
			}
			Nullable<bool> result = openFileDialog.ShowDialog();
			if (!result.Value)
				return;
			SharedWorld.DoAsyncLoad = true;
			LoadScene(openFileDialog.FileName);
		}

		public void OpenAnimation(string extensions, string dir)
		{
			string filter = "";

			if (extensions.Contains("bvh"))
				filter += "Biovision motion files (*.bvh)|*.bvh|";
			if (extensions.Contains("vix"))
				filter += "Vixen scene files (*.vix)|*.vix|";
			if (extensions.Contains("scp"))
				filter += "Vixen script files (*.scp)|*.scp|";
			filter += "All Files (*.*)|*.*";
			openFileDialog.Filter = filter;
			if (dir != null)
			{
				openFileDialog.InitialDirectory = dir;
				Directory.SetCurrentDirectory(dir);
			}
			Nullable<bool> result = openFileDialog.ShowDialog();
			if (!result.Value)
				return;
			SharedWorld.DoAsyncLoad = true;
			LoadAnimation(openFileDialog.FileName);
		}

		protected string ConnectSkeleton()
		{
			if (_skelName != null)
				return _skelName;
			try
			{
				Group g = SharedWorld.MainScene.Engines.Find(".skeleton", Group.FIND_ACTIVE | Group.FIND_DESCEND | Group.FIND_END);
				Viewer3D viewer = Viewer as Viewer3D;
				Skeleton skel = g.ConvertTo(SerialID.VX_Skeleton) as Skeleton;
				string skelname = skel.Name;

				_skelName = skelname.Substring(skelname.IndexOf('.'));
				_skeleton = skel;
				AnimScriptor.Target = skel;
				return _skelName;
			}
			catch (Exception)
			{
				return null;
			}
		}


		/*!
		 * @param ev	Vixen event to handle.
		 * @param world	current world
		 * Handles vixen events - content loading and animation.
		 * This function also calls event handlers for AvatarEvents.
		 */
		protected override void OnVixen(VixenEventArgs vargs)
		{
			Event ev = vargs.VixEvent;
			int code = ev.Code;

			if (code == Event.STOP)
			{
				if (StopEvent != null)
				{
					StopEvent(ev.Sender.Name);
				}
			}
			else if (code == Event.LOAD_SCENE)
				try
				{
					LoadSceneEvent le = ev as LoadSceneEvent;
					String fname = le.FileName;
					SharedObj obj = le.Object;

					if (obj != null)
					{
						string name = obj.Name;
						int p = name.IndexOf('.');

						if (p > 0)
							name = name.Substring(0, p);
						if (obj.IsClass((uint)SerialID.VX_Scene))
						{
							Scene scene = obj as Scene;
							OnSceneLoad(scene, fname);
							if (LoadSceneEvent != null)
								LoadSceneEvent(name, fname);
						}
						else if (obj.IsClass((uint)SerialID.VX_Skeleton))
						{
							if (LoadAnimEvent != null)
								LoadAnimEvent(name, fname);
						}
					}
				}
				catch (Exception) { }
			else if (code == Event.SCENE_CHANGE)
				try
				{
					SceneEvent se = ev as SceneEvent;
					if (SetSceneEvent != null)
					{
						string name = se.Target.Name;
						int p = name.IndexOf('.');

						if (p > 0)
							name = name.Substring(0, p);
						SetSceneEvent(name);
					}
				}
				catch (Exception) { }
		}
	}
}

