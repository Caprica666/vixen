using System.IO;
using ObjIO.Data;
using ObjIO.TypeParsers;

namespace ObjIO
{
    public class Import
    {
		private string _name = "default";

		public void LoadAsync(string pathname)
		{
			Vixen.Scene scene = LoadFile(pathname);
			if (scene != null)
			{
				Vixen.LoadSceneEvent loadevent = new Vixen.LoadSceneEvent(Vixen.Event.LOAD_SCENE, Vixen.SharedWorld.MainScene);
				loadevent.Object = scene;
				loadevent.Log();
			}
		}

		public Vixen.Scene LoadFile(string pathname)
		{
			_name = Path.GetFileNameWithoutExtension(pathname);
			MaterialStreamProvider streamHelper = new MaterialStreamProvider(Path.GetDirectoryName(pathname));
			ObjLoader loader = Create(streamHelper);
			Vixen.Scene scene = loader.Load(streamHelper.Open(pathname));
			return scene;
		}

		public Vixen.Scene LoadStream(Stream instream, string name)
		{
			_name = name;

			MaterialStreamProvider streamHelper = new MaterialStreamProvider();
			ObjLoader loader = Create(streamHelper);
			Vixen.Scene scene = loader.Load(instream);
			return scene;
		}
		
		public ObjLoader Create()
        {
            return Create(new MaterialStreamProvider());
        }

        public ObjLoader Create(MaterialStreamProvider materialStreamProvider)
        {
			var dataStore = new DataStore(_name);          
            var faceParser = new FaceParser(dataStore);
            var groupParser = new GroupParser(dataStore);
            var normalParser = new NormalParser(dataStore);
            var textureParser = new TextureParser(dataStore);
            var vertexParser = new VertexParser(dataStore);

            var materialLibraryLoader = new MaterialLibraryLoader(dataStore);
            var materialLibraryLoaderFacade = new MaterialLibraryLoaderFacade(materialLibraryLoader, materialStreamProvider);
            var materialLibraryParser = new MaterialLibraryParser(materialLibraryLoaderFacade);
            var useMaterialParser = new UseMaterialParser(dataStore);

            return new ObjLoader(dataStore, faceParser, groupParser, normalParser, textureParser, vertexParser, materialLibraryParser, useMaterialParser);
        }
    }
}