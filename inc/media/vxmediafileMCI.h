#pragma once
/*!
 * @class VXMediaFile
 *
 * A VXMediaFile represents a file with media that can be
 * played, such as MP3, WAV, AVI or MPG
 */
class VXMediaFile : public VXEngine
{
	VX_DECLARE_CLASS(VXMediaFile);
public:
/*
 *	Constructors and Destructors
 */
	VXMediaFile();
	~VXMediaFile();

	//! Load the file
	virtual bool	Load(const TCHAR* filename = NULL);
	
	// Engine overrides
	virtual bool OnStart();
	virtual bool OnStop();
	virtual bool OnReset();
	virtual void SetTimeOfs(float timeofs, int who = 0);
	virtual void SetSpeed(float timeofs, int who = 0);

protected:
	//! Execute MCI media command string
	bool		Exec(const TCHAR* command) const;
	bool		Exec(const TCHAR* command, float val) const;

	bool		Play();			//!< play the sound
	bool		Pause();		//!< pause sound if it is playing
	bool		Rewind();		//!< play again from the beginning
	void		Close();		//!< close file and stop playing
	VString		m_FileName;		//!< name of sound file to play
	bool		m_IsPlaying;	//!< true if sound is playing
	bool		m_IsOpen;		//!< true if sound file is open
};

