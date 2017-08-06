using System.Collections.Generic;
using System.Linq;
using ObjIO.Data;

namespace ObjIO.Data
{

	internal class Vertex
	{
		public float[] data;

		public Vertex(int n)
		{
			data = new float[n];
		}
	};

	internal class VertexComparer : IEqualityComparer<Vertex>
	{
		public int GetHashCode(Vertex obj)
		{
			return obj.GetHashCode();
		}

		public bool Equals(Vertex v1, Vertex v2)
		{
			if (v1.data.Length != v2.data.Length)
				return false;
			for (int i = 0; i < v1.data.Length; i++)
			{
				if (v1.data[i] != v2.data[i])
					return false;
			}
			return true;
		}
	};

    public class DataStore : IGroupDataStore, INormalDataStore, IVertexDataStore, ITextureDataStore, IElementGroup, IFaceGroup, IMaterialLibrary
    {
        private Group _currentGroup;

		private string _name;
        private readonly List<Group> _groups = new List<Group>();
		private readonly List<Vixen.Appearance> _materials = new List<Vixen.Appearance>();

		public DataStore(string name)
			: base()
		{
			_name = name;
			_currentGroup = new Group(null);
			_groups.Add(_currentGroup);
		}

        public IList<Vixen.Appearance> Materials
        {
            get { return _materials; }
        }

        public IList<Group> Groups
        {
            get { return _groups; }
        }

        public void AddFace(Face face)
        {
            _currentGroup.AddFace(face);
        }

        public void PushGroup(string groupName)
        {
			if ((_currentGroup != null) && (_currentGroup.Name == null))
				_currentGroup.Name = groupName;
			else
			{
				_currentGroup = new Group(groupName);
				_groups.Add(_currentGroup);
			}
        }


        public void Push(Vixen.Appearance material)
        {
            _materials.Add(material);
        }

        public void SetMaterial(string materialName)
        {
            var material = _materials.SingleOrDefault(x => x.Name.EqualsInvariantCultureIgnoreCase(materialName));
 			material.Name = materialName;
            _currentGroup.Material = material;
        }


		public void AddVertex(float x, float y, float z)
		{
			float[] v = new float[3] { x, y, z };
			_currentGroup.AddVertex(v);
		}

		public void AddNormal(float x, float y, float z)
		{
			float[] v = new float[3] { x, y, z };
			_currentGroup.AddVertex(v);
		}

		public void AddTexture(float x, float y)
		{
			float[] v = new float[2] { x, y };
			_currentGroup.AddTexture(v);
		}

		public Vixen.Scene MakeScene()
		{
			Vixen.Scene scene = new Vixen.Scene();
			Vixen.Model root = null;
			Vixen.Shape shape = null;

			scene.Name = _name + ".scene";
			if (_groups.Count == 0)
				return scene;
			scene.Models = root;
			if (_groups.Count > 1)
				root = new Vixen.Model();
			foreach (Group g in _groups)
			{
				if (g.Name == null)
					g.Name = _name;
				shape = g.MakeShape(_name);
				if (root != null)
					root.Append(shape);
			}
			if (root == null)
				root = shape;
			root.Name = _name + ".root";
			scene.Models = root;
			scene.ShowAll();
			return scene;
		}
    }
}