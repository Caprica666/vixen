#include "vixen.h"
#include "vxmedia.h"

VXSoundData::VXSoundData(const TCHAR *filename)
{
	m_WaveLoader = NULL;
	m_WaveID = 0;
	m_SourceID = 0;
	m_Data = NULL;

	if (filename)
		Load(filename);
}

VXSoundData::~VXSoundData()
{
	if (m_Data)
	{
		delete m_Data;
		m_Data = NULL;
	}
	if (m_File)
	{
		fclose(m_File);
		m_File = NULL;
	}
}

size_t VXSoundData::LoadBuffer(int bufferid)
{
	size_t bytesread = 0;
	size_t datasize = m_BufferSize;

	// Read more audio data (if there is any)
	if (m_File)
	{
		size_t offset = ftell(m_File);

		if ((offset - m_DataOffset + dataSize) > m_DataSize)
			dataSize = m_DataSize - (offset - m_DataOffset);
		bytesread = fread(m_Data, 1, dataSize, m_File);
	}
	if (bytesread)		// Copy audio data to Buffer
	{
		alBufferData(bufferid, m_AudioFormat, m_Data, bytesread, m_Frequency);
		return bytesread;
	}
	return 0;
}

const TCHAR* VXSoundData::GetAudioFormat()
{
	size_t format;

	if (!pfnGetEnumValue)
		return 0;
	if (m_Type == WF_EX)
	{
		if (m_WFEXT.Format.nChannels == 1)
		{
			switch (m_WFEXT.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_MONO_IMA4");
				case 8:		return TEXT("AL_FORMAT_MONO8");
				case 16:	return TEXT("AL_FORMAT_MONO16");
			}
		}
		else if (m_WFEXT.Format.nChannels == 2)
		{
			switch (m_WFEXT.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_STEREO_IMA4");
				case 8:		return TEXT("AL_FORMAT_STEREO8");
				case 16:	return TEXT("AL_FORMAT_STEREO16");
			}
		}
		else if ((m_WFEXT.Format.nChannels == 4) &&
				 (m_WFEXT.Format.wBitsPerSample == 16))
			return TEXT("AL_FORMAT_QUAD16");
	}
	else if (m_Type == WF_EXT)
	{
		if ((m_WFEXT.Format.nChannels == 1) &&
			((m_WFEXT.dwChannelMask == SPEAKER_FRONT_CENTER) ||
			 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)) ||
			 (m_WFEXT.dwChannelMask == 0)))
		{
			switch (m_WFEXT.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_MONO_IMA4");
				case 8:		return TEXT("AL_FORMAT_MONO8");
				case 16:	return TEXT("AL_FORMAT_MONO16");
			}
		}
		else if ((m_WFEXT.Format.nChannels == 2) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
		{
			switch (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_STEREO_IMA4");
				case 8:		return TEXT("AL_FORMAT_STEREO8");
				case 16:	return TEXT("AL_FORMAT_STEREO16");
			}
		}
		else if ((m_WFEXT.Format.nChannels == 2) &&
				 (m_WFEXT.Format.wBitsPerSample == 16) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
			return TEXT("AL_FORMAT_REAR16");
		else if ((m_WFEXT.Format.nChannels == 4) &&
				 (m_WFEXT.Format.wBitsPerSample == 16) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					return TEXT("AL_FORMAT_QUAD16");
		else if ((m_WFEXT.Format.nChannels == 6) &&
				 (m_WFEXT.Format.wBitsPerSample == 16) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
			return= TEXT("AL_FORMAT_51CHN16");
		else if ((m_WFEXT.Format.nChannels == 7) &&
				 (m_WFEXT.Format.wBitsPerSample == 16) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
			return TEXT("AL_FORMAT_61CHN16");
		else if ((m_WFEXT.Format.nChannels == 8) &&
				 (m_WFfEXT.Format.wBitsPerSample == 16) &&
				 (m_WFEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
			return TEXT("AL_FORMAT_71CHN16");
	}
	return NULL;
}

bool VXSoundData::Load(const TCHAR *filename)
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;
	WAVERESULT		wr = WR_BADWAVEFILE;

	if (!filename)
		return false;
	// Open the wave file for reading
	fopen_s(&m_File, filename, "rb");
	if (m_File == NULL)
		VX_ERROR(("ERROR: VXSoundData cannot open %s", filename), false);
	// Read Wave file header
	fread(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), m_File);
	if (STRNCASECMP(waveFileHeader.szRIFF, "RIFF", 4) ||
		STRNCASECMP(waveFileHeader.szWAVE, "WAVE", 4))
		VX_ERROR(("ERROR: VXSoundData unsupported audio format %s", filename), false);
	while (fread(&riffChunk, 1, sizeof(RIFFCHUNK), m_File) == sizeof(RIFFCHUNK))
	{
		if (!STRNCASECMP(riffChunk.szChunkName, "fmt ", 4))
		{
			if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
			{
				fread(&waveFmt, 1, riffChunk.ulChunkSize, m_File);
				
				// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
				if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
				{
					m_FileType = WF_EX;
					memcpy(&m_WFEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
				}
				else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
				{
					m_FilefType = WF_EXT;
					memcpy(&m_WFEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
				}
			}
			else
			{
				fseek(m_File, riffChunk.ulChunkSize, SEEK_CUR);
			}
		}
		else if (!STRNCASECMP(riffChunk.szChunkName, "data", 4))
		{
			m_DataSize = riffChunk.ulChunkSize;
			m_DataOffset = ftell(pWaveInfo->pFile);
			fseek(m_File, riffChunk.ulChunkSize, SEEK_CUR);
		}
		else
		{
			fseek(m_File, riffChunk.ulChunkSize, SEEK_CUR);
		}

		// Ensure that we are correctly aligned for next chunk
		if (riffChunk.ulChunkSize & 1)
			fseek(m_File, 1, SEEK_CUR);
	}
	if (m_Data)
		delete m_Data;
	if (m_DataSize && m_DataOffset && ((m_FileType == WF_EX) || (m_FileType == WF_EXT)))
	{
		m_Frequency = m_WEEXT.Format.nSamplesPerSec;
		// Queue 250ms of audio data
		m_BufferSize = m_WFEXT.nAvgBytesPerSec >> 2;
		// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
		m_BufferSize -= (m_BufferSize % m_WFEXT.nBlockAlign);
		m_Data = new char[m_BufferSize];
		m_AudioFormat = alGenEnumType(GetAudioFormat());
		return true;
	}
	else
	{
		fclose(m_File);
		m_File = NULL;
		m_AudioFormat = 0;
		VX_ERROR(("ERROR: VXSoundData cannot read from %s", filename), false);
	}
}


