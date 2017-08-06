#pragma once
/*!
 * @class VXMediaFile
 *
 * A VXMediaFile represents a file with media that can be
 * played, such as MP3, WAV, AVI or MPG
 */
class AVSync;

class VXMediaFile : public VXEngine
{
	VX_DECLARE_CLASS(VXMediaFile);
public:
/*
 *	Constructors and Destructors
 */
	VXMediaFile();
	~VXMediaFile();

	//! Set the sound volume
	virtual void	SetVolume(float vol);
	//! Get the sound volume
	virtual float	GetVolume() const;
	//! Returns the name of the video file playback source
	const TCHAR*	GetFileName() const	{ return m_FileName.IsEmpty() ? NULL : (const TCHAR*) m_FileName; }
	//! Establish name of media file
	virtual	void	SetFileName(const TCHAR* filename);	
	//! Determine if media is playing (false if paused or no media)
	bool	IsPlaying() const			{ return m_IsPlaying; }

	// Engine overrides
	virtual bool	OnStart();
	virtual bool	OnStop();
	virtual bool	OnReset();
	virtual	bool	Copy(const VXObj* obj);

protected:
	virtual	bool	Init(void*);	//!< initialize before playing
	virtual bool	Load();			//!< Load the media file
	virtual	bool	Play();			//!< play the sound
	virtual	bool	Pause();		//!< pause sound if it is playing
	virtual	bool	Rewind();		//!< play again from the beginning
	virtual	void	Close();		//!< close file and stop playing

	VString		m_FileName;		//!< name of sound file to play
	AVSync*		m_Player;		//!< -> IPP media player
	bool		m_IsPlaying;	//!< true if media is playing (not paused)
	int32		m_VideoX;		//!< pixel width of video frame
	int32		m_VideoY;		//!< pixel height of video frame
};

