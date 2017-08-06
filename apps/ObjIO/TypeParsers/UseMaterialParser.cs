using ObjIO.Data;

namespace ObjIO.TypeParsers
{
    public class UseMaterialParser : TypeParser
    {
        private readonly IElementGroup _elementGroup;

        public UseMaterialParser(IElementGroup elementGroup)
        {
            _elementGroup = elementGroup;
        }

        protected override string Keyword
        {
            get { return "usemtl"; }
        }

        public override void Parse(string line)
        {
            _elementGroup.SetMaterial(line);
        }
    }
}