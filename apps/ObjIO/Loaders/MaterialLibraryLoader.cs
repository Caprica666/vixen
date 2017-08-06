using System;
using System.Collections.Generic;
using System.IO;
using ObjIO.Data;

namespace ObjIO
{
    public class MaterialLibraryLoader : LoaderBase
    {
        private readonly IMaterialLibrary _materialLibrary;
		private Vixen.Appearance _appearance;

        private readonly Dictionary<string, Action<string>> _parseActionDictionary = new Dictionary<string, Action<string>>();
        private readonly List<string> _unrecognizedLines = new List<string>();

        public MaterialLibraryLoader(IMaterialLibrary materialLibrary)
        {
            _materialLibrary = materialLibrary;

            AddParseAction("newmtl", PushMaterial);
            AddParseAction("Ka", d => CurrentMaterial.Ambient = ParseCol(d));
            AddParseAction("Kd", d => CurrentMaterial.Diffuse = ParseCol(d));
            AddParseAction("Ks", d => CurrentMaterial.Specular = ParseCol(d));
            AddParseAction("Ns", d => CurrentMaterial.Shine = d.ParseInvariantFloat());

            AddParseAction("d", d => CurrentMaterial.Diffuse *= d.ParseInvariantFloat());
            AddParseAction("Tr", d => CurrentMaterial.Diffuse.a = d.ParseInvariantFloat());

 //           AddParseAction("illum", i => CurrentMaterial.IlluminationModel = i.ParseInvariantInt());

 //           AddParseAction("map_Ka", m => CurrentMaterial.AmbientTextureMap = m);
            AddParseAction("map_Kd", m => DiffuseMap.Texture = new Vixen.Texture(m));

			AddParseAction("map_Ks", m => SpecularMap.Texture = new Vixen.Texture(m));
//            AddParseAction("map_Ns", m => CurrentMaterial.SpecularHighlightTextureMap = m);
            
//            AddParseAction("map_d", m => CurrentMaterial.AlphaTextureMap = m);

//            AddParseAction("map_bump", m => CurrentMaterial.BumpMap = m);
//            AddParseAction("bump", m => CurrentMaterial.BumpMap = m);

//            AddParseAction("disp", m => CurrentMaterial.DisplacementMap = m);

//            AddParseAction("decal", m => CurrentMaterial.StencilDecalMap = m);
        }

        private Vixen.PhongMaterial CurrentMaterial
		{
			get
			{
				dynamic phong = _appearance.Material.ConvertTo(Vixen.SerialID.VX_PhongMaterial);
				return phong as Vixen.PhongMaterial;
			}
		}

		private Vixen.Sampler DiffuseMap
		{
			get
			{
				Vixen.Sampler sampler = _appearance.GetSampler(0);
				if (sampler == null)
				{
					sampler = new Vixen.Sampler();
					_appearance.SetSampler(0, sampler);
				}
				return sampler;
			}
		}

		private Vixen.Sampler SpecularMap
		{
			get
			{
				Vixen.Sampler sampler = _appearance.GetSampler(1);
				if (sampler == null)
				{
					sampler = new Vixen.Sampler();
					_appearance.SetSampler(1, sampler);
				}
				return sampler;
			}
		}
		
		private void AddParseAction(string key, Action<string> action)
        {
            _parseActionDictionary.Add(key.ToLowerInvariant(), action);
        }

        protected override void ParseLine(string keyword, string data)
        {
            var parseAction = GetKeywordAction(keyword);

            if (parseAction == null)
            {
                _unrecognizedLines.Add(keyword + " " + data);
                return;
            }

            parseAction(data);
        }

        private Action<string> GetKeywordAction(string keyword)
        {
            Action<string> action;
            _parseActionDictionary.TryGetValue(keyword.ToLowerInvariant(), out action);

            return action;
        }

        private void PushMaterial(string materialName)
        {
			_appearance = new Vixen.Appearance();
			_appearance.Name = materialName;
			_appearance.Material = new Vixen.PhongMaterial();
            _materialLibrary.Push(_appearance);
        }

		private Vixen.Vec3 ParseVec3(string data)
        {
            string[] parts = data.Split(' ');

            float x = parts[0].ParseInvariantFloat();
            float y = parts[1].ParseInvariantFloat();
            float z = parts[2].ParseInvariantFloat();

			return new Vixen.Vec3(x, y, z);
        }

		private Vixen.Col4 ParseCol(string data)
		{
			string[] parts = data.Split(' ');

			float x = parts[0].ParseInvariantFloat();
			float y = parts[1].ParseInvariantFloat();
			float z = parts[2].ParseInvariantFloat();

			return new Vixen.Col4(x, y, z, 1.0f);
		}

		private string ParseFileName(string data)
		{
			return null;
		}

        public void Load(Stream lineStream)
        {
            StartLoad(lineStream);
        }
    }
}