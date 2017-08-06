using System.IO;

namespace ObjIO
{
    public class MaterialStreamProvider
    {
		string _name = null;
		string _basedir = "";

		public string Name { get { return _name; } }
		public string Directory { get { return _basedir; } }

		public MaterialStreamProvider(string basedir)
		{
			_basedir = basedir;
		}

		public MaterialStreamProvider()
		{
		}

        public Stream Open(string materialFilePath)
        {
			_name = Path.GetFileNameWithoutExtension(materialFilePath);
			if (!Path.IsPathRooted(materialFilePath))
				materialFilePath = _basedir + '/' + materialFilePath;
            return File.Open(materialFilePath, FileMode.Open, FileAccess.Read);
        }
    }
}