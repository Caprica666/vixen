#include "vixen.h"
#include "vxmedia.h"
#include "alc.h"
#include "ks.h"
#include "ksmedia.h"
#include "media/openal/aldlist.h"

VX_IMPLEMENT_CLASSID(VXSoundPlayer, Engine, VX_SoundSource);

/*!
 * @fn VXSoundPlayer::VXSoundPlayer()
 * Constructs an VXSoundPlayer object that plays audio files.
 *
 * @see VXMediaSource
 */
#define WF_EX	1
#define	WF_EXT	2

typedef struct
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned long	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
    GUID            guidSubFormat;
} WAVEFMT;

VXSoundPlayer::VXSoundPlayer() : Engine()
{
	m_SourceID = 0;
	m_Stream = NULL;
	m_Data = NULL;
	Init();
}


VXSoundPlayer::~VXSoundPlayer()
{
	Close();
    // Clean up buffers and sources
	if (m_SourceID)
	{
		alSourceStop(m_SourceID);
		alDeleteSources(1, &m_SourceID);
		alDeleteBuffers(SOUND_NumBuffers, m_Buffers);
		m_SourceID = 0;
	}
}

bool VXSoundPlayer::LoadFile()
{
	if (!ReadHeader())
		return false;
	return LoadData();
}

void VXSoundPlayer::Close()
{
	if (m_Stream)
		fclose(m_Stream);
	m_Stream = NULL;
	if (m_Data)
		delete []  m_Data;
	m_Data = NULL;
}

bool VXSoundPlayer::Init()
{
	ALDeviceList *pDeviceList = NULL;
	ALCcontext *pContext = NULL;
	ALCdevice *pDevice = NULL;
	ALint i;
	ALboolean bReturn = AL_FALSE;

	pDeviceList = new ALDeviceList();
	if ((pDeviceList) && (pDeviceList->GetNumDevices()))
	{
		for (i = 0; i < pDeviceList->GetNumDevices(); i++) 
			VX_PRINTF(("%d. %s%s\n", i + 1, pDeviceList->GetDeviceName(i), i == pDeviceList->GetDefaultDevice() ? "(DEFAULT)" : ""));
		pDevice = alcOpenDevice(pDeviceList->GetDeviceName(0));
		if (pDevice)
		{
			pContext = alcCreateContext(pDevice, NULL);
			if (pContext)
			{
				VX_PRINTF(("Opened %s Device\n", alcGetString(pDevice, ALC_DEVICE_SPECIFIER)));
				alcMakeContextCurrent(pContext);
			}
			else
			{
				alcCloseDevice(pDevice);
				delete pDeviceList;
				VX_ERROR(("ERROR: Failed to initialize OpenAL"), false);
			}
		}
		delete pDeviceList;
	}
	alGenBuffers(SOUND_NumBuffers, m_Buffers);	// Generate some AL Buffers for streaming
    alGenSources(1, &m_SourceID);				// Generate a Source to playback the Buffers
	return true;
}


bool VXSoundPlayer::OnStart()
{
	if (m_SourceID == 0)
		return Init();
	if (!m_IsOpen)
	{
		LoadData();
		PlayBuffers();
	}
	return Engine::OnStart();
}

bool VXSoundPlayer::OnStop()
{
	return Engine::OnStop();
}

bool VXSoundPlayer::OnReset()
{
	return Engine::OnReset();
}


bool VXSoundPlayer::PlayBuffers()
{
	int queuedBuffers = 0;
	int state;

	// Check the status of the Source.  If it is not playing, then playback was completed,
	// or the Source was starved of audio data, and needs to be restarted.
	alGetSourcei(m_SourceID, AL_SOURCE_STATE, &state);
	if (state == AL_PLAYING)
		return true;
	// If there are Buffers in the Source Queue then the Source was starved of audio
	// data, so needs to be restarted (because there is more audio data to play)
	alGetSourcei(m_SourceID, AL_BUFFERS_QUEUED, &queuedBuffers);
	if (queuedBuffers)
	{
		alSourcePlay(m_SourceID);
		return true;
	}
	return false;
}


bool VXSoundPlayer::LoadData()
{
	int BuffersProcessed = 0;
	unsigned long BytesWritten;
	int	data_loaded = false;

	alGetSourcei(m_SourceID, AL_BUFFERS_PROCESSED, &BuffersProcessed);

	// For each processed buffer, remove it from the Source Queue, read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the Source Queue
	while (BuffersProcessed)
	{
		// Remove the Buffer from the Queue. (bufferid contains the Buffer ID for the unqueued Buffer)
		uint32 bufferid = 0;
		size_t bytesread = 0;
		alSourceUnqueueBuffers(m_SourceID, 1, &bufferid);

		// Read more audio data (if there is any)
		if (m_Stream)
		{
			size_t offset = ftell(m_Stream);
			size_t datasize = m_BufferSize;

			if ((offset - m_DataOffset + datasize) > m_DataSize)
				datasize = m_DataSize - (offset - m_DataOffset);
			bytesread = fread(m_Data, 1, datasize, m_Stream);
		}
		if (bytesread)		// Copy audio data to Buffer
		{
			alBufferData(bufferid, m_AudioFormat, m_Data, bytesread, m_Frequency);
			alSourceQueueBuffers(m_SourceID, 1, &bufferid);
			data_loaded = true;
		}
		BuffersProcessed--;
	}
	return data_loaded;
}

const TCHAR* VXSoundPlayer::GetAudioFormat()
{
	size_t format;

	if (m_FileType == WF_EX)
	{
		if (m_WaveInfo.Format.nChannels == 1)
		{
			switch (m_WaveInfo.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_MONO_IMA4");
				case 8:		return TEXT("AL_FORMAT_MONO8");
				case 16:	return TEXT("AL_FORMAT_MONO16");
			}
		}
		else if (m_WaveInfo.Format.nChannels == 2)
		{
			switch (m_WaveInfo.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_STEREO_IMA4");
				case 8:		return TEXT("AL_FORMAT_STEREO8");
				case 16:	return TEXT("AL_FORMAT_STEREO16");
			}
		}
		else if ((m_WaveInfo.Format.nChannels == 4) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16))
			return TEXT("AL_FORMAT_QUAD16");
	}
	else if (m_FileType == WF_EXT)
	{
		if ((m_WaveInfo.Format.nChannels == 1) &&
			((m_WaveInfo.dwChannelMask == SPEAKER_FRONT_CENTER) ||
			 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)) ||
			 (m_WaveInfo.dwChannelMask == 0)))
		{
			switch (m_WaveInfo.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_MONO_IMA4");
				case 8:		return TEXT("AL_FORMAT_MONO8");
				case 16:	return TEXT("AL_FORMAT_MONO16");
			}
		}
		else if ((m_WaveInfo.Format.nChannels == 2) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
		{
			switch (m_WaveInfo.Format.wBitsPerSample)
			{
				case 4:		return TEXT("AL_FORMAT_STEREO_IMA4");
				case 8:		return TEXT("AL_FORMAT_STEREO8");
				case 16:	return TEXT("AL_FORMAT_STEREO16");
			}
		}
		else if ((m_WaveInfo.Format.nChannels == 2) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
			return TEXT("AL_FORMAT_REAR16");
		else if ((m_WaveInfo.Format.nChannels == 4) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					return TEXT("AL_FORMAT_QUAD16");
		else if ((m_WaveInfo.Format.nChannels == 6) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
			return TEXT("AL_FORMAT_51CHN16");
		else if ((m_WaveInfo.Format.nChannels == 7) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
			return TEXT("AL_FORMAT_61CHN16");
		else if ((m_WaveInfo.Format.nChannels == 8) &&
				 (m_WaveInfo.Format.wBitsPerSample == 16) &&
				 (m_WaveInfo.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
			return TEXT("AL_FORMAT_71CHN16");
	}
	return NULL;
}

bool VXSoundPlayer::ReadHeader()
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;

	if (m_FileName.IsEmpty())
		return false;
	// Open the wave file for reading
	fopen_s(&m_Stream, m_FileName, "rb");
	if (m_Stream == NULL)
		VX_ERROR(("ERROR: VXSoundData cannot open %s", m_FileName), false);
	// Read Wave file header
	fread(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), m_Stream);
	if (STRNCASECMP(waveFileHeader.szRIFF, "RIFF", 4) ||
		STRNCASECMP(waveFileHeader.szWAVE, "WAVE", 4))
		VX_ERROR(("ERROR: VXSoundData unsupported audio format %s", m_FileName), false);
	while (fread(&riffChunk, 1, sizeof(RIFFCHUNK), m_Stream) == sizeof(RIFFCHUNK))
	{
		if (!STRNCASECMP(riffChunk.szChunkName, "fmt ", 4))
		{
			if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
			{
				fread(&waveFmt, 1, riffChunk.ulChunkSize, m_Stream);
				
				// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
				if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
				{
					m_FileType = WF_EX;
					memcpy(&m_WaveInfo.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
				}
				else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
				{
					m_FileType = WF_EXT;
					memcpy(&m_WaveInfo, &waveFmt, sizeof(WAVEFORMATEXT));
				}
			}
			else
			{
				fseek(m_Stream, riffChunk.ulChunkSize, SEEK_CUR);
			}
		}
		else if (!STRNCASECMP(riffChunk.szChunkName, "data", 4))
		{
			m_DataSize = riffChunk.ulChunkSize;
			m_DataOffset = ftell(m_Stream);
			fseek(m_Stream, riffChunk.ulChunkSize, SEEK_CUR);
		}
		else
		{
			fseek(m_Stream, riffChunk.ulChunkSize, SEEK_CUR);
		}

		// Ensure that we are correctly aligned for next chunk
		if (riffChunk.ulChunkSize & 1)
			fseek(m_Stream, 1, SEEK_CUR);
	}
	if (m_Data)
	{
		delete [] m_Data;
		m_Data = NULL;
	}
	if (m_DataSize && m_DataOffset && ((m_FileType == WF_EX) || (m_FileType == WF_EXT)))
	{
		m_Frequency = m_WaveInfo.Format.nSamplesPerSec;
		// Queue 250ms of audio data
		m_BufferSize = m_WaveInfo.Format.nAvgBytesPerSec >> 2;
		// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
		m_BufferSize -= (m_BufferSize % m_WaveInfo.Format.nBlockAlign);
		m_Data = new char[m_BufferSize];
		m_AudioFormat = alGetEnumValue(GetAudioFormat());
		return true;
	}
	else
	{
		fclose(m_Stream);
		m_Stream = NULL;
		m_AudioFormat = 0;
		VX_ERROR(("ERROR: VXSoundData cannot read from %s", m_FileName), false);
	}
}


