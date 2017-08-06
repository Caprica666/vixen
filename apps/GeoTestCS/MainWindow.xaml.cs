using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

using Vixen;

namespace GeoTest
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow() : base()
		{
			InitializeComponent();
		}
	}

	public class GeoCanvas : Canvas3D
	{
		public GeoCanvas() : base()
		{
		}

        private Shape MakeShape(string name, Col4 diffuse, string texfile, int texop)
        {
            Shape shape = new Shape();
 
            try
            {
                Appearance app = new Appearance(diffuse);
                TriMesh mesh;
                
                app.Set(Appearance.LIGHTING, 1);
                shape.Appearance = app;
                shape.Name = name;              
                if (texfile != "")
                {
                    Texture tex = new Texture(GetMediaPath(texfile));
                    Sampler sampler = new Sampler();
                    app.SetSampler(0, sampler);
                    sampler.Set(Sampler.TEXTUREOP, texop);
                    sampler.Texture = tex;
                    mesh = new TriMesh(VertexPool.NORMALS | VertexPool.TEXCOORDS);
                }
                else
                    mesh = new TriMesh(VertexPool.NORMALS);
                shape.Geometry = mesh;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return shape;
        }

        public override Scene MakeScene()
        {
            Scene scene = new Scene();
            Model root = new Model();
            Shape shape;
            string name = "testgeo";
			string bricktex = GetMediaPath("brick.jpg");
			TriMesh mesh;
			ExtModel extmodel;

			scene.Models = root;
			scene.Name = name + ".scene";
			root.Name = name + ".root";
			shape = MakeShape(name + ".block", new Col4(1.0f, 0.5f, 0.5f), bricktex, Sampler.DIFFUSE);
			mesh = (TriMesh) shape.Geometry;
			GeoUtil.Block(mesh, new Vec3(20.0f, 10.0f, 10.0f));
			shape.Translate(-40.0f, 0.0f, 0.0f);
			root.Append(shape);
			shape = MakeShape(name + ".sphere", new Col4(0.5f, 1.0f, 0.5f), bricktex, Sampler.DIFFUSE);
			GeoUtil.Ellipsoid((TriMesh)shape.Geometry, new Vec3(10.0f, 10.0f, 10.0f), 9);
			root.Append(shape);
			shape = MakeShape(name + ".torus", new Col4(1.0f, 0.5f, 1.0f), bricktex, Sampler.DIFFUSE);
			GeoUtil.Torus((TriMesh)shape.Geometry, 5.0f, 10.0f, 15);
			shape.Translate(30.0f, 20.0f, 0.0f);
			shape.Turn(new Vec3(1.0f, 0f, 0f), 3.14159f / 2);
			root.Append(shape);
			extmodel = new ExtModel();
			extmodel.Name = "testgeo.external";
			extmodel.FileName = ContentFile;
			root.Append(extmodel);
			scene.ShowAll();
			return scene;
		}
	}
}
