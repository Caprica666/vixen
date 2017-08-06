using System.Collections.Generic;
using System.IO;
using ObjIO.Data;
using ObjIO.TypeParsers;
using ObjIO;

namespace ObjIO
{
    public class ObjLoader : LoaderBase
    {
        private readonly DataStore _dataStore;
        private readonly List<TypeParser> _typeParsers = new List<TypeParser>();

        private readonly List<string> _unrecognizedLines = new List<string>();

        public ObjLoader(
            DataStore dataStore, 
            FaceParser faceParser, 
            GroupParser groupParser,
            NormalParser normalParser, 
            TextureParser textureParser, 
            VertexParser vertexParser,
            MaterialLibraryParser materialLibraryParser, 
            UseMaterialParser useMaterialParser)
        {
            _dataStore = dataStore;
            SetupTypeParsers(
                vertexParser,
                faceParser,
                normalParser,
                textureParser,
                groupParser,
                materialLibraryParser,
                useMaterialParser);
        }

        private void SetupTypeParsers(params TypeParser[] parsers)
        {
            foreach (var parser in parsers)
            {
                _typeParsers.Add(parser);
            }
        }

        protected override void ParseLine(string keyword, string data)
        {
            foreach (var typeParser in _typeParsers)
            {
                if (typeParser.CanParse(keyword))
                {
                    typeParser.Parse(data);
                    return;
                }
            }

            _unrecognizedLines.Add(keyword + " " + data);
        }

        public Vixen.Scene Load(Stream lineStream)
        {
            StartLoad(lineStream);

            return CreateResult();
        }
		
		private Vixen.Scene CreateResult()
        {
			Vixen.Scene result = _dataStore.MakeScene();
			return result;
        }
    }
}