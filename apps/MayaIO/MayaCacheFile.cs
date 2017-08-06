using System;
using System.IO;
using System.Xml;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Vixen;

namespace MayaIO
{

	public class CacheChannel
	{
		public string Name = "";
		public string Type = "";
		public string Interp = "";
		public string SampleType = "";
		public float SampleRate = 0;
		public float StartTime = 0;
		public float EndTime = 0;

		public CacheChannel(string channelName, string channelType,
							string interpretation, string samplingType,
							float samplingRate, float startTime, float endTime)
			: base()
		{
			object[] args = new object[7];
			Name = channelName;
			Type = channelType;
			Interp = interpretation;
			SampleType = samplingType;
			SampleRate = samplingRate;
			StartTime = startTime;
			EndTime = endTime;
		}
	}

	/*!
	* @class MayaCacheFile
	* Imports geometry cache files from Maya
	*/
	public class MayaCacheFile
	{
		public string BaseName = "";
		public string Directory = "";
		public string FileName = "";
		public string Type = "";
		public float StartTime = 0;
		public float EndTime = 0;
		public float TimePerFrame = 0;
		public float Version = 0.0f;

		protected Dictionary<string, CacheChannel> Channels = new Dictionary<string, CacheChannel>();
		protected MeshAnimator	_root;
		protected bool			_swapbytes = true;
		protected LoadEvent		_event = null;
		static public	bool	RaiseError = false;
		static protected Thread	_loadthread = null;
		static protected ArrayList _queue = new ArrayList();

		public MayaCacheFile(MeshAnimator root)
			: base()
		{
			_root = root;
			if (root == null)
				_root = new MeshAnimator();
		}

		public MayaCacheFile()
			: base()
		{
			_root = new MeshAnimator();
		}

		/*!
		* @fn void LoadFile(string fileName)
		* @param fileName	full path to the .xml description file,
		*					or just the filename of the .xml file, with or without extension
		*					if it is in the current directory
		*/
		public void LoadFile(string filename, LoadEvent ev)
		{
			if (RaiseError)
			{
				Log("LOAD ABORTED DUE TO EXCEPTION " + filename);
				return;
			}
			try
			{
				XmlDocument dom = new XmlDocument();

				FileName = filename;
				dom.Load(filename);
				ParseDescription(dom);
			}
			catch (Exception ex)
			{
				RaiseError = true;
				throw (ex);
			}
			_event = ev;
			if (ev != null)
				StartAsyncLoad();
			else
				ParseData();
		}

		/*!
		* @fn void LoadString(string xmlstring)
		* @param xmlstring	string containing XML description
		*/
		public void LoadString(string xmlstring, LoadEvent ev)
		{
			if (RaiseError)
			{
				string s = "LOAD ABORTED DUE TO EXCEPTION ";

				if (ev != null)
					s += ev.FileName;
				Log(s);
				return;
			}
			try
			{
				XmlDocument dom = new XmlDocument();

				dom.LoadXml(xmlstring);
				ParseDescription(dom);
			}
			catch (Exception ex)
			{
				RaiseError = true;
				throw (ex);
			}
			_event = ev;
			if (ev != null)
				StartAsyncLoad();
			else
				ParseData();
		}

		static protected void ParseDataAsync(object input)
		{
			ArrayList queue = input as ArrayList;

			while ((queue.Count > 0) && !MayaCacheFile.RaiseError)
			{
				MayaCacheFile cache = null;

				lock (queue)
				{
					cache = queue[0] as MayaCacheFile;
					queue.Remove(cache);
				}
				try
				{
					cache.ParseData();
				}
				catch (Exception)
				{
					ErrorEvent ev = new ErrorEvent();

					MayaCacheFile.RaiseError = true;
					ev.ErrString = "FILE OPEN FAILED ";
					ev.ErrString += cache.BaseName;
					ev.ErrLevel = 2;
					ev.Log();
				}
			}
			_loadthread = null;
		}

		protected void StartAsyncLoad()
		{
			lock (_queue)
			{
				if (_loadthread == null)
				{
					RaiseError = false;
					_loadthread = new Thread(new ParameterizedThreadStart(ParseDataAsync));
				}
				_queue.Add(this);
				if (!_loadthread.IsAlive)
					_loadthread.Start(_queue);
			}
		}

		public void ParseData()
		{
			BaseName = Path.GetFileNameWithoutExtension(FileName);
			Directory = Path.GetDirectoryName(FileName) + '/';
			if (Version > 2.0)
				throw new FileLoadException("This function can only parse cache files of version 2 or lower");
			Log(BaseName + " Maya Cache version " + Version);
			Log(String.Format("Cache has {0} channels, starting at time {1} seconds and ending at {2} seconds",
					new object[] { Channels.Count, StartTime, EndTime }));
			if (_root.Name == null)
				_root.Name = BaseName + ".meshanim";
			_root.TimeInc = TimePerFrame;
			_root.Active = false;
			foreach (KeyValuePair<string, CacheChannel> p in Channels)
			{
				CacheChannel channel = p.Value as CacheChannel;
				Log(String.Format("Channel Name {0}, Type {1}, interpretation {2}, sampling Type {3}",
					new object[] { channel.Name, channel.Type, channel.Interp, channel.SampleType }));
				Log("sample rate " + (1.0 / channel.SampleRate) + " FPS " + channel.SampleRate + " sec / frame");
				Log(String.Format("startTime {0} seconds, endTime {1} seconds for {2} frames",
					new object[] { channel.StartTime, channel.EndTime, (channel.EndTime - channel.StartTime) / channel.SampleRate }));
			}
			if (Type == "OneFilePerFrame")
				ParseDataFilePerFrame();
			else if (Type == "OneFile")
				ParseDataOneFile();
			else throw new FileLoadException("Invalid cache file type " + Type);
			if (_event != null)
			{
				_event.Log();
				_event = null;
			}
		}

		private void Log(string s)
		{
			Vixen.SharedWorld.Trace(s);
		}

		private Int32 ReadInt(BinaryReader fd)
		{
			if (_swapbytes)
			{
				Byte[] bytes = fd.ReadBytes(4);

				Array.Reverse(bytes);
				return BitConverter.ToInt32(bytes, 0);
			}
			else return fd.ReadInt32();
		}

		private float ReadFloat(BinaryReader fd)
		{
			if (_swapbytes)
			{
				Byte[] bytes = fd.ReadBytes(4);

				Array.Reverse(bytes);
				return BitConverter.ToSingle(bytes, 0);
			}
			else return fd.ReadSingle();
		}

		private float ReadDouble(BinaryReader fd)
		{
			if (_swapbytes)
			{
				Byte[] bytes = fd.ReadBytes(8);

				Array.Reverse(bytes);
				return (float)BitConverter.ToDouble(bytes, 0);
			}
			else return (float)fd.ReadDouble();
		}

		/*!
		* @fn ParseDescription(XmlDocument dom)
		* @param dom	XmlDocument containing the contents of the cache file descriptor
		* Parses the contents of the XML description remembers it.
		*/
		protected void ParseDescription(XmlDocument dom)
		{
			XmlNodeList nodes = null;

			nodes = dom.GetElementsByTagName("Autodesk_Cache_File");
			foreach (XmlNode node in nodes[0].ChildNodes)
			{
				string name = node.Name;
				if (name == "cacheType")
					Type = node.Attributes[0].Value;
				if (name == "time")
				{
					string[] timeRange = node.Attributes[0].Value.Split(new Char[] { '-' });
					StartTime = Single.Parse(timeRange[0]) / 6000.0f;
					EndTime = Single.Parse(timeRange[1]) / 6000.0f;
				}
				else if (name == "cacheTimePerFrame")
					TimePerFrame = Single.Parse(node.Attributes[0].Value) / 6000.0f;
				else if (name == "cacheVersion")
					Version = Single.Parse(node.Attributes[0].Value);
				else if (name == "Channels")
					ParseChannels(node.ChildNodes);
			}

		}

		/*!
		* @fn ParseChannels(XmlNodeList channels)
		* @param channels	node list with channel information
		* helper method to extract channel information
		*/
		protected void ParseChannels(XmlNodeList channels)
		{
			foreach (XmlNode channel in channels)
			{
				if (channel.Name.Contains("channel"))
				{
					string channelName = "";
					string channelType = "";
					string channelInterp = "";
					string sampleType = "";
					float sampleRate = 0;
					float startTime = 0;
					float endTime = 0;
					CacheChannel channelObj = null;

					for (int index = 0; index < channel.Attributes.Count; ++index)
					{
						string attrName = channel.Attributes[index].Name;
						string attrVal = channel.Attributes[index].Value;
						if (attrName == "ChannelName")
							channelName = attrVal;
						if (attrName == "ChannelInterpretation")
							channelInterp = attrVal;
						if (attrName == "EndTime")
							endTime = (float)Double.Parse(attrVal) / 6000.0f;
						if (attrName == "StartTime")
							startTime = (float) Double.Parse(attrVal) / 6000.0f;
						if (attrName == "SamplingRate")
							sampleRate = (float) Double.Parse(attrVal) / 6000.0f;
						if (attrName == "SamplingType")
							sampleType = attrVal;
						if (attrName == "ChannelType")
							channelType = attrVal;
					}
					channelObj = new CacheChannel(channelName, channelType, channelInterp, sampleType, sampleRate, startTime, endTime);
					Channels[channelName] = channelObj;
				}
			}
		}

		protected Int32 ParseChannel(BinaryReader fd, float time)
		{
			//
			// channel name is next. the tag for this must be CHNM
			//
			String chnmTag = new String(fd.ReadChars(4));
			String channelName;
			String sizeTag;
			String dataFormatTag;
			Int32 chnmSize;
			Int32 arrayLength;
			Int32 bufferLength;
			Int32 bytesRead = 0;
			Int32 mask;
			Int32 chnmSizeToRead;
			Int32 paddingSize;
			Int32 index;
			CacheChannel channel = null;

			if (chnmTag != "CHNM")
				return 0;
			bytesRead += 4;
			//
			// Next comes a 32 bit int that tells us how long the channel name is
			//
			chnmSize = ReadInt(fd);
			bytesRead += 4;
			//
			// The string is padded out to 32 bit boundaries,
			// so we may need to read more than chnmSize
			//
			mask = 3;
			chnmSizeToRead = (chnmSize + mask) & (~mask);
			--chnmSize;
			channelName = new String(fd.ReadChars(chnmSize));
			channel = Channels[channelName];
			paddingSize = chnmSizeToRead - chnmSize;
			if (paddingSize > 0)
				fd.ReadChars(paddingSize);
			bytesRead += chnmSizeToRead;
			//
			// Next is the SIZE field, which tells us the length of the data array
			//
			sizeTag = new String(fd.ReadChars(4));
			if (sizeTag != "SIZE")
				throw new FileLoadException("SIZE section missing in cache file " + FileName);
			bytesRead += 4;
			//
			// Next 32 bit int is the size of the array size variable,
			// this is always 4, so we'll ignore it for now
			// though we could use it as a sanity check.
			//
			fd.ReadChars(4);
			bytesRead += 4;
			//
			//finally the actual size of the array
			//
			arrayLength = ReadInt(fd);
			bytesRead += 4;
			//
			// data format tag
			//
			dataFormatTag = new String(fd.ReadChars(4));
			//
			// buffer length - how many bytes is the actual data
			//
			bufferLength = ReadInt(fd);
			bytesRead += 8;
			Vixen.VertexArray verts = new Vixen.VertexArray("position float 3", arrayLength);
			float[] floatArray = new float[arrayLength * verts.VertexSize];

			if (dataFormatTag == "FVCA")		// FVCA == Float Vector Array
			{
				if (bufferLength != arrayLength * 3 * 4)
					throw new FileLoadException("size inconsistency in cache file " + FileName);
				for (int i = 0; i < arrayLength; i++)
				{
					int j = i * verts.VertexSize;
					float x = ReadFloat(fd);
					float y = ReadFloat(fd);
					float z = ReadFloat(fd);
					floatArray[j] = x;
					floatArray[j + 1] = y;
					floatArray[j + 2] = z;
				}
			}
			else if (dataFormatTag == "DVCA")	// DVCA == Double Vector Array
			{
				if (bufferLength != arrayLength * 3 * 8)
					throw new FileLoadException("size inconsistency in cache file " + FileName);
				for (int i = 0; i < arrayLength; i++)
				{
					int j = i * verts.VertexSize;
					float x = ReadDouble(fd);
					float y = ReadDouble(fd);
					float z = ReadDouble(fd);
					floatArray[j] = x;
					floatArray[j + 1] = y;
					floatArray[j + 2] = z;
				}
			}
			else
				throw new FileLoadException("Unknown data tag " + dataFormatTag + " in cache file " + FileName);
			bytesRead += bufferLength;
			verts.AddVertices(floatArray, arrayLength);
			float t = time + TimePerFrame / 2.0f - channel.StartTime;
			index = (Int32) (t / TimePerFrame);
			_root.SetSource(index, verts);
			Log("\ti = " + index + " t = " + t);
			return bytesRead;
		}


		/*!
		* @fn ParseDataOneFile(CacheFile cacheFile)
		* Parse and display the contents of the data file, for the
		* One large file case ("OneFile")
		*/
		protected void ParseDataOneFile()
		{
			string			fileName = Directory + BaseName + ".mc";
			BinaryReader	fd = new BinaryReader(new FileStream(fileName, FileMode.Open, FileAccess.Read), Encoding.ASCII);
			String			tag = new String(fd.ReadChars(4));
			Int32			bytesRead = 0;

			// blockTag must be FOR4
			if (tag != "FOR4")
				throw new FileLoadException("Incorrect block tag in cache file " + fileName);
			Int32 offset = ReadInt(fd);
			tag = new String(fd.ReadChars(4));
			if (tag != "CACH")
				throw new FileLoadException("Incorrect block tag in cache file " + fileName);
			tag = new String(fd.ReadChars(4));
			if (tag == "VRSN")
			{
				String s = new String(fd.ReadChars(8));
				bytesRead += 8;
				tag = new String(fd.ReadChars(4));
			}
			if (tag == "STIM")
			{
				float t = ReadDouble(fd);
				bytesRead += 8;
				tag = new String(fd.ReadChars(4));
			}
			if (tag == "ETIM")
			{
				float t = ReadDouble(fd);
				bytesRead += 8;
			}
			//
			// The 1st block is the header, not used. Ignore the header for now.
			// 
			while (true)
			{
				Int32	blockSize;
				float	time;

				tag = new String(fd.ReadChars(4));
				if (tag.Length < 4)	// EOF condition...we are done 
					return;
				if (tag != "FOR4")
					throw new FileLoadException("Incorrect block tag in cache file " + fileName);
				blockSize = ReadInt(fd);
				bytesRead = 0;
				tag = new String(fd.ReadChars(4));
				bytesRead += 4;
				if (tag != "MYCH")
					throw new FileLoadException("MYCH section missing in cache file " + fileName);
				tag = new String(fd.ReadChars(4));
				bytesRead += 4;
				if (tag != "TIME")
					throw new FileLoadException("TIME section missing in cache file " + fileName);        
				//
				// Next 32 bit int is the size of the time variable,
				// this is always 4, so we'll ignore it for now
				// though we could use it as a sanity check.
				//
				fd.ReadChars(4);
				bytesRead += 4;
				//
				// Next 32 bit int is the time itself, in ticks
				// 1 tick = 1/6000 of a second
				//
				time = (float) ReadInt(fd);
				bytesRead += 4;
				time /= 6000.0f;
				while (bytesRead < blockSize)
				{
					Int32 n = ParseChannel(fd, time);

					if (n == 0)
						break;
					bytesRead += n;
				}
			}
		}


	   /*!
		* @fn ParseDataFilePerFrame(CacheFile cacheFile)
		* Parse and display the contents of the data file, for the
		* file per frame case ("OneFilePerFrame")
		*/
		protected void ParseDataFilePerFrame()
		{
			String[] allFilesInDir = System.IO.Directory.GetFiles(Directory);
			ArrayList dataFiles = new ArrayList();
			foreach (String afile in allFilesInDir)	// find all matching data files
			{
				if (Path.GetExtension(afile) != ".mc" || !afile.Contains(BaseName))
					continue;
				String	fileName = Path.GetFileName(afile);
				String	baseName = Path.GetFileNameWithoutExtension(afile);
				Int32	frameNumber = 0;
				Int32	tickNumber = 0;
				float	time;
				Int32	bytesRead = 0;
				String	blockTag;
				String	tag;
				Int32	offset;
				Int32	blockSize;
				BinaryReader fd;

				try
				{
					Int32	n = baseName.IndexOf("Frame") + 5;
					String	frameAndTickNumberStr = baseName.Substring(n, baseName.Length - n);
					Int32	m = frameAndTickNumberStr.IndexOf("Tick");

					frameNumber = Int32.Parse(frameAndTickNumberStr);
					if (m > 0)
						tickNumber = Int32.Parse(frameAndTickNumberStr.Substring(m, frameAndTickNumberStr.Length - m));
				}
				catch (Exception)
				{
				}
				time = frameNumber * TimePerFrame + (tickNumber / 6000.0f);
				//Log("Data found at time " + time + " seconds");
				fd = new BinaryReader(new FileStream(afile, FileMode.Open, FileAccess.Read), Encoding.ASCII);
				blockTag = new String(fd.ReadChars(4));
				if (blockTag != "FOR4")			// blockTag must be FOR4
					throw new FileLoadException("block tag not found where expected cache file " + fileName);
				offset = ReadInt(fd);
				bytesRead = 0;
				blockSize = ReadInt(fd);
				tag = new String(fd.ReadChars(4));
				if (tag == "VRSN")
				{
					String s = new String(fd.ReadChars(8));
					bytesRead += 8;
					tag = new String(fd.ReadChars(4));
				}
				if (tag == "STIM")
				{
					float t = ReadDouble(fd);
					bytesRead += 8;
					tag = new String(fd.ReadChars(4));
				}
				if (tag == "ETIM")
				{
					float t = ReadDouble(fd);
					bytesRead += 8;
					tag = new String(fd.ReadChars(4));
				}
				if (tag == "FOR4")
				{
					blockSize = ReadInt(fd);
					bytesRead += 4;
					tag = new String(fd.ReadChars(4));
				}
				if (tag != "MYCH")
					throw new FileLoadException("Invalid section tag in cache file " + fileName);
				bytesRead = 4;
				//
				// Note that unlike the oneFile case, for file per frame there is no
				// TIME tag at this point.  The time of the data is embedded in the file name itself
				//
				while (bytesRead < blockSize)
				{
					Int32 n = ParseChannel(fd, time);

					if (n == 0)
						break;
					bytesRead += n;
				}
			}
		}
	}
}

