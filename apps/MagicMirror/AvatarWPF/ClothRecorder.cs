using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Navigation;
using Vixen;

namespace MagicMirror.Viewer
{

	public  class ClothRecorder
	{
		Model	_root = null;
		Engine	_simroot = null;

		public float TimeInc = 1.0f /30.0f;
		public string BaseName;
		public string AnimName;

		public Engine AnimRoot
		{
			get { return _simroot; }
			set
			{
				if (value != null)
					throw new System.ArgumentException("Cannot set AnimRoot to non-null value");
				if (_simroot != null)
				{
					_simroot.Remove(true);
					_simroot = null;
				}
			}
		}

		protected MeshAnimator _meshAnimRoot
		{
			get
			{
				if (_simroot == null)
					return null;
				Engine tmp = _simroot.First();
				return tmp as MeshAnimator;
			}
		}

		public Model MeshRoot
		{
			get { return _root; }
			set
			{
				_root = value;
				if (_root != null)
					if (_simroot == null)
					{
						MeshAnimator tmp = new MeshAnimator();
						tmp.Name = BaseName + ".meshanim";
						tmp.TimeInc = TimeInc;
						tmp.Enable(Engine.CONTROL_CHILDREN);
						_simroot = new Engine();
						_simroot.Append(tmp);
						AddMeshAnimators(_root, tmp);
					}
					else
						SetTarget(_root);
			}
		}

		public ClothRecorder(float timeinc = 0.0f)
		{
			if (timeinc > 0)
				TimeInc = timeinc;
		}

		public bool SaveAnimation()
		{
			String			name = AnimName;
			MeshAnimator	tmp;
			bool			rc;

			if (_simroot == null)
				return false;
			if (BaseName != null)
				if (name != null)
					name += "_" + BaseName;
				else
					name = BaseName;
			if (name == null)
				name = "default";
			tmp = _simroot.First() as MeshAnimator;
			if (tmp == null)
				return false;
			_simroot.Name = name + ".simroot";
			tmp.Name = name + "." + BaseName + ".meshanim";
			tmp.Stop();
			tmp.Disable(MeshAnimator.RECORD, Engine.CONTROL_CHILDREN);
			SetTarget(null);
			rc = SharedWorld.Get().SaveAsVix(_simroot, name + ".vix");
			if (!rc)
				SharedWorld.LogError("Cannot save cloth animations " + name + ".vix");
			SetTarget(_root);
			Clear();
			return rc;
		}

		public bool SaveAnimation(MeshAnimator meshanim)
		{
			String name = AnimName;
			bool rc;
			Engine simroot;
			Engine oldroot;

			if (meshanim == null)
				return false;
			if (BaseName != null)
				name += "_" + BaseName;
			oldroot = meshanim.Parent();
			meshanim.Remove(false);
			simroot = new Engine();
			simroot.Name = name + ".simroot";
			meshanim.Stop();
			meshanim.Disable(MeshAnimator.RECORD, Engine.CONTROL_CHILDREN);
			simroot.Append(meshanim);
			rc = SharedWorld.Get().SaveAsVix(simroot, name + ".vix");
			if (!rc)
				SharedWorld.LogError("Cannot save cloth animations " + name + ".vix");
			meshanim.Remove(false);
			oldroot.Append(meshanim);
			return rc;
		}

		public bool SaveCloth()
		{
			Model parent;
			Model tmproot;
			Model clothroot = (Model) _root.Find(BaseName + "." + BaseName, Group.FIND_EXACT | Group.FIND_DESCEND);
			bool rc;

			if (clothroot == null)
			{
				SharedWorld.LogError("Cannot find cloth model " + BaseName + "." + BaseName);
				return false;
			}
			parent = clothroot.Parent();
			clothroot.Remove(false);
			tmproot = new Model();
			tmproot.Name = BaseName + ".root";
			tmproot.Append(clothroot);
			rc = SharedWorld.Get().SaveAsVix(tmproot, BaseName + ".vix");
			if (!rc)
				SharedWorld.LogError("Cannot save cloth meshes " + BaseName + ".vix");
			clothroot.Remove(false);
			parent.Append(clothroot);
			return rc;
		}

		public void Clear()
		{
			MeshAnimator tmp = _meshAnimRoot;
			if (tmp != null)
				tmp.Clear();
		}

		public void Record(bool record)
		{
			Scene scene = SharedWorld.MainScene;
			MeshAnimator animroot = _meshAnimRoot;

			if (animroot != null)
			{
				animroot.Stop();
				if (record)
				{
					if (TimeInc > 0)
						scene.TimeInc = TimeInc;
					animroot.Enable(MeshAnimator.RECORD);
				}
				else
				{
					animroot.Disable(MeshAnimator.RECORD);
					scene.TimeInc = 0.0f;
				}
			}
		}

		public void Start()
		{
			MeshAnimator animroot = _meshAnimRoot;

			if (animroot != null)
			{
				animroot.Enable(Engine.ACTIVE);
				animroot.Start();
			}
		}

		public void Pause()
		{
			MeshAnimator animroot = _meshAnimRoot;

			if (animroot != null)
				animroot.Stop();
		}

		protected void AddMeshSource(Model srcroot, Engine dstroot)
		{
			if (srcroot == null)
				return;
			if (dstroot.IsClass((uint) SerialID.VX_MeshAnimator))
				try
				{
					MeshAnimator meshanim = dstroot as MeshAnimator;
					Shape dstshape = meshanim.Target as Shape;
					String name = dstshape.Name;
					Shape srcshape;
					Mesh mesh;
					VertexArray verts;
					int p = name.IndexOf('.');

					if (p > 0)
						name = name.Substring(p);
					srcshape = srcroot.Find(name, Group.FIND_DESCEND | Group.FIND_END) as Shape;
					mesh = srcshape.Geometry as Mesh;
					verts = mesh.Vertices;
					SharedWorld.Trace(meshanim.Name + " -> " + srcshape.Name);
					meshanim.SetSource(-1, verts);
				}
				catch (Exception ex)
				{
					SharedWorld.LogError(ex.Message);
				}
			dstroot = dstroot.First() as Engine;
			while (dstroot != null)
			{
				AddMeshSource(srcroot, dstroot);
				dstroot = dstroot.Next() as Engine;
			}
		}

		protected void AddMeshAnimators(Model root, Engine simroot)
		{
			try
			{
				if (root.IsClass((uint)SerialID.VX_Shape))
				{
					Vixen.Shape shape = root as Vixen.Shape;
					Vixen.Geometry geo = shape.Geometry;

					if ((geo != null) && (geo.GetNumVtx() > 0))
					{
						MeshAnimator meshanim;

						if (simroot.IsClass((uint)SerialID.VX_MeshAnimator))
							meshanim = simroot as MeshAnimator;
						else
						{
							meshanim = new MeshAnimator();
							meshanim.Name = root.Name + ".meshanim";
						}
						meshanim.Target = geo;
						meshanim.Control = Engine.CONTROL_CHILDREN;
						meshanim.TimeInc = TimeInc;
						meshanim.Active = false;
						SharedWorld.Trace(meshanim.Name + " -> " + shape.Name);
						if (simroot != meshanim)
						{
							simroot.Append(meshanim);
							simroot = meshanim;
						}
					}
				}
			}
			catch (Exception ex)
			{
				SharedWorld.LogError(ex.Message);
			}
			root = root.First();
			while (root != null)
			{
				AddMeshAnimators(root, simroot);
				root = root.Next();
			}
		}

		protected void SetTarget(Model root)
		{
			MeshAnimator tmp = _meshAnimRoot;
			if (tmp != null)
				tmp.Target = root;
		}
	}
}
