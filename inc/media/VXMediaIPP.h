#pragma once
/*!
 * @class MediaSource
 *
 * A MediaSource represents a file with media that can be
 * played, such as MP3, WAV, AVI or MPG
 */
class AVSync;

namespace Vixen {

	class VixenVideoRender;

	class MediaSource : public MediaBase
	{
		VX_DECLARE_CLASS(MediaSource);
	public:
	/*
	 *	Constructors and Destructors
	 */
		MediaSource();

		virtual	bool	Eval(float t);
	protected:
		virtual	bool	Play();						//!< play the sound
		virtual	bool	Pause();					//!< pause sound if it is playing
		virtual	bool	Rewind();					//!< play again from the beginning
		virtual	void	Close();					//!< close file and stop playing
		virtual bool	Open(const TCHAR* filename);//!< Load the media file

		AVSync*				m_Player;				//!< -> IPP media player
		VixenVideoRender*	m_VideoRender;
	};

}	// end Vixen

