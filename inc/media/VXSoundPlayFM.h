#pragma once

namespace FMOD
{
	class Sound;
	class Channel;
	class System;
};

namespace Vixen {

/*!
 * @class SoundPlayer
 * @brief plays sound from an audio file such as MP3 or WAV.
 *
 * This version of the class uses the cross-platform FMOD sound library.
 * The sound player can be appended to a media source to play the
 * sound track from a video.
 *
 * @see MediaSource
 */
class SoundPlayer : public Engine
{
public:
	VX_DECLARE_CLASS(SoundPlayer);

	SoundPlayer();
	~SoundPlayer();

	//! Get the name of the sound file
	const char*		GetFileName() const	{ return m_FileName; }

	//! Set the name of the sound file to play
	virtual void	SetFileName(const TCHAR* filename);

	//! Load the sound file data
	virtual bool	LoadFile();

	//! Load raw sound data from a buffer
	virtual	bool	LoadBuffer(Core::Buffer*);

	//! Allocate a new buffer for streaming audio
	Core::Buffer*	NewBuffer();

	//! Get size of sound buffers
	size_t			GetBufferSize() const	{ return m_SoundBuffers.GetDataSize(); }

	//! Get the volume of the sound playing
	virtual	float	GetVolume() const;

	//! Set the volume of the sound playing
	virtual	void	SetVolume(float);

	// Engine overrides
	virtual bool Eval(float t);
	virtual bool OnStart();
	virtual bool OnStop();
	virtual bool OnReset();
	//virtual void SetTimeOfs(float timeofs, int who = 0);
	//virtual void SetSpeed(float timeofs, int who = 0);

protected:
	bool		Play();						//!< play the sound
	bool		Pause();					//!< pause sound if it is playing
	bool		Rewind();					//!< play again from the beginning
	void		Close();					//!< close file and stop playing
	bool		PlayBuffer(Core::Buffer*);	//!< play sound from memory
	static bool	Init();						//!< initialize sound player
	static void	Shutdown();					//!< shut down sound player

	Core::String			m_FileName;		//!< name of sound file to play
	Core::BufferQueue		m_SoundBuffers;	//!< queue of sound buffers
	FMOD::Sound*			m_Stream;		//!< sound data
	FMOD::Channel*			m_Player;		//!< sound player
	static FMOD::System*	s_System;		//!< sound system
};

}	// end Vixen