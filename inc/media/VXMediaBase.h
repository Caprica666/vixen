
/*
 * @file vxmediabase.h
 * @brief Base class for media streaming.
 *
 * Media sources are used to map video frames onto textures in
 * the 3D scene and decode audio frames to play on sound hardware.
 * The base class abstracts the platform-independent video and audio
 * capabilities so that different media APIs may be easily used.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxsoundplayerfm.h
 */
#pragma once

namespace Vixen {

class SoundPlayer;

/*!
 * @class MediaBase
 * @brief Base class with device-independent infrastructure for media streaming.
 *
 * Media sources are used to map video frames onto textures in
 * the 3D scene and decode audio frames to play on sound hardware.
 * The base class abstracts the platform-independent video and audio
 * capabilities so that different media APIs may be easily used.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see SoundPlayer
 */
class MediaBase : public Engine
{
public:
	VX_DECLARE_CLASS(MediaBase);
	MediaBase();
	~MediaBase();

public:
	//! Returns the pixel dimensions of the video frame
	const Vec2&		GetVideoSize() const			{ return m_VideoSize; }

	//! Returns video playback and capture options
	int				GetOptions() const				{ return m_Options; }

	//! Sets the video capture and playback options
	void			SetOptions(int opts);

	//! Returns the name of the video file playback source
	const TCHAR*	GetFileName() const				{ return m_FileName.IsEmpty() ? NULL : (const TCHAR*) m_FileName; }

	//! Sets the name of the video file playback source
	void			SetFileName(const TCHAR*);

	//! Returns the name of the video capture source
	const TCHAR*	GetCaptureSource() const		{ return m_CaptureSource; }

	//! Sets the name of the video capture source
	void			SetCaptureSource(const TCHAR*)	{ }

	//! Returns the desire video playback rate
	int				GetFPS() const;

	//! Returns the desire video playback rate
	void			SetFPS(int);

	//! Returns minimum texture size for video frame (power of 2)
	const Vec2&		GetTexSize() const			{ return m_TexSize; }

	//! Internal function to compute power of two texture dimensions
	static bool		ComputeTexSize(int& width, int& height);

	//! Internal function to create in-memory texture data area	
	static bool		InitImage(Texture* image, int width, int height, int depth = 24);

	//! Internal function to copy into the texture data area
	static bool		CopyImage(Texture* image, const char* srcpixels, int srcwidth, int srcheight, int srcstride);

	// Engine overrides
	virtual bool		Eval(float);
	virtual	bool		Copy(const SharedObj*);
	virtual	bool		Do(Messenger&, int);
	virtual	int			Save(Messenger&, int) const;
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Media option values
	 * @see SetOptions SetCaptureSource SetFileName
	 */
	enum
	{
		LIVE = 1,	//!< capture live video, use capture source instead of video file
	};
	/*
	 * Do opcodes (for binary file input)
	 */
	enum Opcode
	{
		MEDIA_SetFileName = ENG_NextOp,
		MEDIA_SetCaptureSource,
		MEDIA_SetOptions,
		MEDIA_SetFPS,
		MEDIA_NextOp = ENG_NextOp + 20,
	};

protected:
	virtual bool	OnStart();
	virtual bool	OnStop();
	virtual bool	OnReset();
	virtual	bool	Load();
	virtual	bool	Init(void* ptr)	{ return true; }
	virtual	bool	Play()			{ return true; }
	virtual	void	Close()			{ }
	virtual	bool	Pause()			{ return true; }
	virtual	bool	Rewind()		{ return false; }
	virtual	bool	Open(const TCHAR* filename)	{ return false; }
	virtual	bool	ProcessFrame(Texture*, SoundPlayer*)	{ return false; }

	Core::String	m_FileName;
	Core::String	m_CaptureSource;
	Vec2			m_VideoSize;
	Vec2			m_TexSize;
	int32			m_Options;
	bool			m_NeedsInit;
	bool			m_NeedsLoad;
	float			m_VideoFrameTime;
	float			m_NextTime;
	bool			m_IsPlaying;
};

}	// end Vixen