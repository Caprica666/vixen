using ObjIO.Data;

namespace ObjIO.TypeParsers
{
    public abstract class TypeParser
    {
        protected abstract string Keyword { get; }

        public bool CanParse(string keyword)
        {
            return keyword.EqualsInvariantCultureIgnoreCase(Keyword);
        }

        public abstract void Parse(string line);
    }
}