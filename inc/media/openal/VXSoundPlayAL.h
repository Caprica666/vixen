#pragma once
#define SOUND_NumBuffers	2


/*!
 * @class VXSoundPlayer
 *
 * A VXSoundPlayer represents a sound in the environment, usually played
 * from an audio file such as MP3 or WAV.
 */
class VXSoundPlayer : public Engine
{
public:
	VX_DECLARE_CLASS(VXSoundPlayer);

	VXSoundPlayer();
	~VXSoundPlayer();

	//! Set the sound volume
	//virtual void	SetVolume(float vol);

	//! Load the file
	virtual bool	Load(const TCHAR* filename = NULL);
	
	// Engine overrides
	virtual bool OnStart();
	virtual bool OnStop();
	virtual bool OnReset();
	//virtual void SetTimeOfs(float timeofs, int who = 0);
	//virtual void SetSpeed(float timeofs, int who = 0);

protected:
	bool		Play();			//!< play the sound
	bool		Pause();		//!< pause sound if it is playing
	bool		Rewind();		//!< play again from the beginning
	void		Close();		//!< close file and stop playing
	bool		ReadHeader();	//!< read WAV header
	bool		Init();			//!< initialize OpenAL
	bool		LoadData();
	bool		PlayBuffers();
	const TCHAR* GetAudioFormat();

	VString		m_FileName;		//!< name of sound file to play
	bool		m_IsPlaying;	//!< true if sound is playing
	bool		m_IsOpen;		//!< true if sound file is open
	float		m_Frequency;	//!< sound frequency
	int32		m_FileType;		//!< type of audio file
	uint32		m_AudioFormat;	//!< OpenAL audio format
	uint32		m_SourceID;		//!< id of sound source
	size_t		m_DataOffset;	//!< offset into sound file
	size_t		m_BufferSize;	//!< size of sound data buffer
	size_t		m_DataSize;		//!< size of data chunk
	void*		m_Data;			//!< sound data buffer
	FILE*		m_Stream;		//!< open sound file
	uint32		m_Buffers[SOUND_NumBuffers];
	/*
	 * Wave file format header info
	 */
	struct WAVEFORMATEXT
	{
		struct WAVEFORMATEX
		{
			WORD    wFormatTag;
			WORD    nChannels;
			DWORD   nSamplesPerSec;
			DWORD   nAvgBytesPerSec;
			WORD    nBlockAlign;
			WORD    wBitsPerSample;
			WORD    cbSize;
		}	Format;
		union
		{
			WORD	wValidBitsPerSample;	// bits of precision
			WORD	wSamplesPerBlock;		// valid if wBitsPerSample==0
			WORD	wReserved;				// If neither applies, set to zero
		} Samples;
		DWORD		dwChannelMask;			// which channels are present in stream
		GUID		SubFormat;
	} m_WaveInfo;

};

