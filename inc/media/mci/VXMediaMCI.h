#pragma once
/*!
 * @class VXMediaSource
 *
 * A VXMediaSource represents a file with media that can be
 * played, such as MP3, WAV, AVI or MPG
 */
class VXMediaSource : public Engine
{
	VX_DECLARE_CLASS(VXMediaSource);
public:
/*
 *	Constructors and Destructors
 */
	VXMediaSource();
	const TCHAR*	GetFileHandle() const;
	
	// Engine overrides
	virtual void SetTimeOfs(float timeofs, int who = 0);
	virtual void SetSpeed(float timeofs, int who = 0);

protected:
	//! Execute MCI media command string
	bool		Exec(const TCHAR* command) const;
	bool		Exec(const TCHAR* command, float val) const;

	virtual bool		Open(const TCHAR*); //! Load the file
	virtual	bool		Play();				//!< play the sound
	virtual	bool		Pause();			//!< pause sound if it is playing
	virtual	bool		Rewind();			//!< play again from the beginning
	virtual	void		Close();			//!< close file and stop playing

	bool				m_IsOpen;			//!< true if sound file is open
	VString				m_FileHandle;
};

