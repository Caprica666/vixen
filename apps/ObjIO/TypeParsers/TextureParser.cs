using ObjIO.Data;

namespace ObjIO.TypeParsers
{
    public class TextureParser : TypeParser
    {
        private readonly ITextureDataStore _textureDataStore;

        public TextureParser(ITextureDataStore textureDataStore)
        {
            _textureDataStore = textureDataStore;
        }

        protected override string Keyword
        {
            get { return "vt"; }
        }

        public override void Parse(string line)
        {
            string[] parts = line.Split(' ');

            float u = parts[0].ParseInvariantFloat();
            float v = parts[1].ParseInvariantFloat();

            _textureDataStore.AddTexture(u, v);
        }
    }
}