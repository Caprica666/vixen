using System.Collections.Generic;

namespace ObjIO.Data
{
    public class Group
    {
        private readonly List<Face> _faces = new List<Face>();
		private Vixen.VertexArray _vertices = new Vixen.VertexArray("position float 3");
		private Vixen.FloatArray _textures = new Vixen.FloatArray();
		private Vixen.VertexArray _normals = new Vixen.VertexArray("position float 3");
		private Dictionary<Vertex, long> _vtxcache;
		private long _numverts = 0;
        
        public Group(string name)
        {
			Name = name;
        }

        public string Name { get; set; }
        public Vixen.Appearance Material { get; set; }
        public IList<Face> Faces { get { return _faces; } }

        public void AddFace(Face face)
        {
            _faces.Add(face);
        }

		public void AddVertex(float[] v)
		{
			_vertices.Add(v);
		}

		public void AddNormal(float[] v)
		{
			_normals.Add(v);
		}

		public void AddTexture(float[] v)
		{
			_textures.Add(v[0]);
			_textures.Add(v[1]);
			float f = _textures[0];
		}

		public Vixen.Shape MakeShape(string basename)
		{
			Vixen.Shape shape = new Vixen.Shape();

			shape.Name = basename + '.' + Name;
			shape.Appearance = Material;
			shape.Geometry = MakeMesh();
			return shape;
		}

		protected Vixen.TriMesh MakeMesh()
		{
			Vixen.TriMesh mesh;
			Vixen.VertexArray verts;
			string desc = "position float 3, normal float 3";
			_vtxcache = new Dictionary<Vertex, long>();

			if (_textures.Count > 0)
				desc += ", texcoord float 3";
			_numverts = 0;
			mesh = new Vixen.TriMesh(desc);
			verts = mesh.Vertices;
			foreach (Face f in Faces)
			{
				mesh.AddIndex(AddVertex(f[0], mesh));
				mesh.AddIndex(AddVertex(f[1], mesh));
				mesh.AddIndex(AddVertex(f[2], mesh));
			}
			verts.MaxVerts = _vtxcache.Keys.Count;
			foreach (KeyValuePair<Vertex, long> pair in _vtxcache)
			{
				Vertex src = pair.Key as Vertex;
				long i = pair.Value * mesh.VertexSize;
				verts.AddVertices(src.data, 1);
			}
			if (_normals.Count == 0)
				mesh.MakeNormals();
			return mesh;
		}

		private long AddVertex(FaceVertex fv, Vixen.Mesh mesh)
		{
			Vertex v = new Vertex(mesh.VertexSize);

			float[] p = _vertices[fv.VertexIndex - 1];
			long n = _numverts;
			long i = 0;

			v.data[i] = p[0];
			v.data[i + 1] = p[1];
			v.data[i + 2] = p[2];
			i += 3;
			if (_normals.Count > 0)
			{
				p = _normals[fv.NormalIndex - 1];
				v.data[i] = p[0];
				v.data[i + 1] = p[1];
				v.data[i + 2] = p[2];
			}
			else
			{
				v.data[i] = 0;
				v.data[i + 1] = 0;
				v.data[i + 2] = 0;
			}
			i += 3;
			if (_textures.Count > 0)
			{
				long ti = (fv.TextureIndex - 1) * 2;
				v.data[i + 1] = _textures[ti];
				v.data[i + 2] = _textures[ti + 1];
			}
			if (_vtxcache.ContainsKey(v))
				return _vtxcache[v];
			_vtxcache.Add(v, n);
			++_numverts;
			return n;
		}

	}
}