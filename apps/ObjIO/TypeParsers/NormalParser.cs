using ObjIO.Data;

namespace ObjIO.TypeParsers
{
    public class NormalParser : TypeParser
    {
        private readonly INormalDataStore _normalDataStore;

        public NormalParser(INormalDataStore normalDataStore)
        {
            _normalDataStore = normalDataStore;
        }

        protected override string Keyword
        {
            get { return "vn"; }
        }

        public override void Parse(string line)
        {
            string[] parts = line.Split(' ');

            float x = parts[0].ParseInvariantFloat();
            float y = parts[1].ParseInvariantFloat();
            float z = parts[2].ParseInvariantFloat();

            _normalDataStore.AddNormal(x, y, z);
        }
    }
}