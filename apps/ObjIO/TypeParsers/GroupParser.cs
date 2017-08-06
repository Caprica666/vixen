using ObjIO.Data;

namespace ObjIO.TypeParsers
{
    public class GroupParser : TypeParser
    {
        private readonly IGroupDataStore _groupDataStore;

		public GroupParser(IGroupDataStore groupDataStore)
        {
            _groupDataStore = groupDataStore;
        }

        protected override string Keyword
        {
            get { return "g"; }
        }

        public override void Parse(string line)
        {
            _groupDataStore.PushGroup(line);
        }
    }
}