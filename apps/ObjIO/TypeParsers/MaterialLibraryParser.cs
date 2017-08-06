using ObjIO;

namespace ObjIO.TypeParsers
{
    public class MaterialLibraryParser : TypeParser
    {
        private readonly MaterialLibraryLoaderFacade _libraryLoaderFacade;

        public MaterialLibraryParser(MaterialLibraryLoaderFacade libraryLoaderFacade)
        {
            _libraryLoaderFacade = libraryLoaderFacade;
        }

        protected override string Keyword
        {
            get { return "mtllib"; }
        }

        public override void Parse(string line)
        {
            _libraryLoaderFacade.Load(line);
        }
    }
}