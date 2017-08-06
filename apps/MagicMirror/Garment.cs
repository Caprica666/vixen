using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Web.Helpers;
using System.IO;

using Vixen;

namespace MagicMirror
{
	/*!
	 * Implements a virtual Closet - a set of cloth simulations which can
	 * be loaded from a file and attached to an avatar.
	 *
	 * @see Garment
	 */
	public class Closet : DynamicJsonObject
	{
		public		Scriptor	scriptor = null;
		protected	Garment		currentGarment = null;
		protected	Model		garmentRoot = null;
		protected	Skeleton	skeleton = null;

		public Closet()
			: base(new Dictionary<string, object>() { { "items", new ArrayList() }})
		{
		}

		public Closet(Model root, Skeleton skel)
			: base(new Dictionary<string, object>() { { "items", new ArrayList() } })

		{
			garmentRoot = root;
			skeleton = skel;
		}

		public IEnumerator GetEnumerator()
		{
			dynamic closet = this;
			ArrayList garments = closet.items as ArrayList;
			return garments.GetEnumerator();
		}

		public void Add(Garment g)
		{
			dynamic tmp = this;
			tmp.items.Add(g);
		}

		public void Clear()
		{
			dynamic tmp = this;
			tmp.items.Clear();
		}

		public void CopyFrom(dynamic src)
		{
			Clear();
			foreach (dynamic d in src)
			{
				Garment g = new Garment(d, garmentRoot, skeleton, scriptor);
				Add(g);
			}
		}

		public Garment Find(String garmentName)
		{
			if (garmentName.EndsWith(".scene"))
				garmentName = garmentName.Substring(0, garmentName.Length - 6);
			garmentName = garmentName.ToLower();
			foreach (dynamic g in this)
			{
				if (garmentName.Contains(g.Name.ToLower()))
					return g;
			}
			return null;
		}

		public Garment OnLoad(Scene scene, SharedWorld world, dynamic avatarconfig)
		{
			Garment g = Find(scene.Name);
			dynamic tmp = g;

			if (g == null)
			{
				SharedWorld.LogError(scene.Name + " not a garment - ignoring this load event");
				return null;
			}
			if (g.IsLoaded)
			{
				SharedWorld.LogError(scene.Name + " already loaded - ignoring this load event");
				return g;
			}
			g.Connect(world, scene, avatarconfig);
			if ((scriptor != null) && (tmp.script != null))
				scriptor.LoadScript(tmp.script);
			Select(g);
			return g;
		}

		public Garment Load(String garmentName, String url)
		{
			dynamic g = Find(garmentName);

			if (g != null)
			{
				g.Load(g.filename);
				Select(g);
				return g;
			}
			g = new Garment(garmentRoot, skeleton, garmentName);
			this.Add(g);
			return null;
		}

		public void Select(String garmentName, String url)
		{
			Garment g = Load(garmentName, url);
			if (g != null)
			{
				Select(g);
				return;
			}		
		}

		public void Select(Garment g)
		{
			if (currentGarment != null)
				currentGarment.Hide();
			currentGarment = g;
			g.Show();
		}

		public string FindAnimation(string name)
		{
			if (currentGarment == null)
				return null;
			return currentGarment.FindAnimation(name);
		}

		public String AsJSON()
		{
			return Json.Encode(this);
		}

		public bool LoadJSON(String json_data)
		{
			dynamic result = Json.Decode(json_data);
			if (result != null)
			{
				this.CopyFrom(result.closet);
				return true;
			}
			return true;
		}
	}

	/*!
	 * Implements a virtual Garment - a cloth simulation that can be controlled
	 * by both bones and physics.
	 *
	 * @see Physics Skeleton
	 */
	public class Garment : DynamicJsonObject
	{
		public bool		IsActive = false;
		public Model	ClothMesh		{ get { return clothMesh; } }
		public Engine	ClothSim		{ get { return clothSim; } }
		public bool		IsLoaded		{ get { return clothSim != null; } }

		protected Scriptor		scriptor;
		protected String		enableAnims = "";
		protected String		disableAnims = "";
		protected Model			closetRoot = null;
		protected Model			clothMesh = null;
		protected Skeleton		skeleton = null;
		protected Engine		clothSim = null;
		protected ExtModel		clothRoot = null;
		protected Dictionary<string, string> members = null;


		public Garment(dynamic src, Model root, Skeleton skel, Scriptor scp)
			: base(new Dictionary<string, object>()
			{
				{ "name", src.name },
				{ "filename", src.filename },
				{ "iconfile", src.iconfile },
				{ "script", src.script },
				{ "animation", src.animation }
			})
		{
			closetRoot = root;
			skeleton = skel;
			scriptor = scp;
			clothRoot = new ExtModel();
			clothRoot.Options |= ExtModel.REMOVE;
			clothRoot.Active = false;
			root.Append(clothRoot);
		}

		public Garment(Model root, Skeleton skel, String name) : base(new Dictionary<string, object>()
		{
			{ "name", name },
			{ "filename", null },
			{ "iconfile", null },
			{ "animation", null },
			{ "script", null }
		})
		{
			skeleton = skel;
			closetRoot = root;
			clothRoot = new ExtModel();
			clothRoot.Options |= ExtModel.REMOVE;
			clothRoot.Active = false;
			root.Append(clothRoot);
		}

		public string FindAnimation(string name)
		{
			dynamic anims = (this as dynamic).animation;
			char[]	delims = { (char) '/', (char) '\\', (char) '.' };
			if (anims == null)
				return null;
			foreach (string s in anims)
			{
				String t = s;
				if (t.IndexOfAny(delims) >= 0)
					t = Path.GetFileNameWithoutExtension(s);
				if (t.StartsWith(name))
					return t;
			}
			return null;
		}

		public void Show()
		{
			clothRoot.Active = true;
			IsActive = true;
			if ((scriptor != null) && (enableAnims != ""))
				scriptor.Exec(enableAnims);
			else if (clothSim != null)
				clothSim.Enable(Engine.ACTIVE);
		}

		public void Hide()
		{
			clothRoot.Active = false;
			if (clothSim != null)
				clothSim.Disable(Engine.ACTIVE);
			if (!IsActive)
				return;
			IsActive = false;
			if ((scriptor != null) && (disableAnims != ""))
				scriptor.Exec(disableAnims);
		}

		public Model Load(String url)
		{
			dynamic g = this;
			if (IsLoaded)
				return clothRoot;
			if (url != null)
			{
				g.filename = url;
				clothRoot.FileName = url;
			}
			else
				clothRoot.FileName = g.filename;
			clothRoot.Load();
			return clothRoot;
		}

		/*
		 * void LoadAnimations(dynamic avatarconfig)
		 * Load the mesh animations associated with this garment.
		 * @param avatarconfig	"avatar" section of the config file
		 * 
		 */
		public void LoadAnimations(dynamic avatarconfig)
		{
			dynamic anims = (this as dynamic).animation;
			char[] delims = { (char)'/', (char)'\\', (char)'.' };
			dynamic g = this;
			String garment_name = g.name;

			if (anims == null)
				return;
			String avatar_name = avatarconfig.rig + ".skeleton.anim";
			foreach (string s in anims)
			{
				String base_name = Path.GetFileNameWithoutExtension(s);
				String script = "";

				if (s.IndexOfAny(delims) < 0)
					continue;
				if (!base_name.EndsWith("_" + garment_name))
					throw new FileLoadException("Incorrect animation naming in closet file, animation names must end with garment name suffix", s);
				String anim_name = base_name.Substring(0, base_name.Length - garment_name.Length - 1);
				String meshanim_name = g.name + ".meshanim";
				String anim_eng_name = base_name + '.' + meshanim_name + ".anim";

				enableAnims += "enable " + anim_eng_name + "\n";
				disableAnims += "disable " + anim_eng_name + "\n";
				if (s.EndsWith("xml"))
				{
					Animator anim = scriptor.MakeAnim(anim_eng_name, clothMesh, false);
					anim.SetEngineName(meshanim_name);
					anim.SetFileName(s);
					SharedWorld.Get().Observe(Event.LOAD_SCENE, anim);
					AvatarScene.LoadMayaCache(g, s, null, scriptor);
				}
				else
					script += "load " + s + " " + meshanim_name + " -s  -t " + garment_name + "." + garment_name + "\n";
				script += "onevent loadscene " + anim_eng_name + ", begin " + anim_eng_name + " -with " + anim_name + "." + avatar_name;
				scriptor.Exec(script);
				Console.WriteLine("LoadAnimations: " + script);
			}
		}

		public Engine Connect(SharedWorld world, Scene scene, dynamic avatarconfig)
		{
			Engine  simroot = Physics.Get();
			Model   mod;
			string baseName = null;
			string animrig = "hip";
			string havokrig = null;

			if (avatarconfig != null)
			{
				if (avatarconfig.rig != null)
					animrig = avatarconfig.rig;
				if (avatarconfig.havokrig != null)
					havokrig = avatarconfig.havokrig;
			}
			if (simroot == null)
			{
				simroot = scene.Engines;
				if (simroot == null)
					return null;
			}
			world.SuspendScene();
			try
			{
				baseName = ((dynamic) this).name;
				string skelname = null;
				Skeleton clothskel = null;

				if (havokrig != null)
				{
					skelname += baseName + "." + havokrig + ".skeleton";
					clothskel = (Skeleton) Scriptor.Find(skelname);
					if (clothskel != null)
					{
						mod = (Model) clothskel.Target;
						clothskel.Remove(true);
					}
					else
						mod = (Model) clothRoot.Find(baseName + "." + havokrig, Group.FIND_DESCEND | Group.FIND_EXACT);
					if (mod != null)
						mod.Remove(true);
				}
				skelname = baseName + "." + animrig + ".skeleton";
				clothskel = (Skeleton) Scriptor.Find(skelname);
				if (clothskel != null)
				{
					Engine e = clothskel.First();
					Engine skin = null;

					clothMesh = (Model) clothskel.Target;
					if (clothMesh == null)
						clothMesh = (Model) clothRoot.Find(baseName + "." + baseName, Group.FIND_DESCEND | Group.FIND_EXACT);
					/*
					 * Grab all the skins under this skeleton and group them
					 * under a single Engine.
					 */
					while (e != null)
					{
						Engine next = e.Next();
						if (e.IsClass((uint)SerialID.VX_Skin))
						{
							Skin s = e as Skin;

							e.Remove(false);			// unlink from skeleton
							s.Skeleton = skeleton;
							if (skin != null)			// more than one skin?
							{
								if (clothSim == null)	// group them
								{
									clothSim = new Engine();
									clothSim.Control = Engine.CONTROL_CHILDREN;
									clothSim.Name = baseName + ".skin";
									clothSim.Append(skin);
									clothSim.Target = clothMesh;
								}
								clothSim.Append(s);
							}
							skin = e;
						}
						e = next;
					}
					if (clothSim == null)
						clothSim = skin;
					clothskel.Remove(true);
				}
				if (clothSim == null)
					clothSim = (Engine) Scriptor.Find(baseName + ".cloth");
				if (clothSim == null)
					clothSim = (Engine) Scriptor.Find(baseName + ".skin");
				if (clothSim == null)
				{
					simroot = scene.Engines;
					if (simroot != null)
					{
						clothSim = (Engine)Scriptor.Find(baseName + ".meshanim");
						if (clothSim != null)
							clothSim.Remove(false);
					}
				}
				if ((clothMesh == null) && (clothSim != null))
				{
					SharedObj tmp = clothSim.Target;

					if ((tmp != null) && (typeof(Model).IsAssignableFrom(tmp.GetType())))
						clothMesh = tmp as Model;
				}
				if (clothMesh == null)
					clothMesh = (Model) clothRoot.Find(baseName + "." + baseName, Group.FIND_DESCEND | Group.FIND_EXACT);
			}
			catch (Exception)
			{
				// do nothing, simulation tree does not have the skeleton
			}
			world.ResumeScene();
			if (scriptor != null)
				LoadAnimations(avatarconfig);
			if (clothSim == null)
				SharedWorld.LogError("No cloth simulation found for " + baseName);
			if (clothMesh == null)
				SharedWorld.LogError("No cloth mesh found for " + baseName);
			return clothSim;
		}
	}
}
