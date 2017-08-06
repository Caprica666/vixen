namespace ObjIO
{
    public class MaterialLibraryLoaderFacade
    {
        private readonly MaterialLibraryLoader _loader;
        private readonly MaterialStreamProvider _materialStreamProvider;

        public MaterialLibraryLoaderFacade(MaterialLibraryLoader loader, MaterialStreamProvider materialStreamProvider)
        {
            _loader = loader;
            _materialStreamProvider = materialStreamProvider;
        }

        public void Load(string materialFileName)
        {
            using (var stream = _materialStreamProvider.Open(materialFileName))
            {
                if (stream != null)
                {
                    _loader.Load(stream);    
                }
            }
        }
    }
}