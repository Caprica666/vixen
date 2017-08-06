
/*
 * @file vxvideoimage.h
 * @brief Base class for video images.
 *
 * Video images are used to map video sources onto textures in
 * the 3D scene. The base class abstracts the platform-independent
 * video capabilities so that different low-level video APIs may be used.
 * The current implementation uses DirectShow.
 *
 * @author Judith Stanley
 * @ingroup vixen
 *
 * @see vxspeech.h vxvideoimagedx.h vxvideoimagempl.h
 */
#pragma once

#define	VIDEO_Live	1

class VXVideoImageBase : public Engine
{
public:
	VXVideoImageBase();
	~VXVideoImageBase();

public:
	//! Returns the pixel dimensions of the video frame
	Vec2			GetVideoSize() const			{ return Vec2(m_VideoX, m_VideoY); }

	//! Returns bits per pixel of video frame (24, 32)
	int				GetDepth() const				{ return m_Depth; }

	//! Returns a pointer to the target image
	Texture*		GetImage()						{ return (Texture*) GetTarget(); }

	//! Sets the image to get the video data each frame
	void			SetImage(Texture* image);

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
	void			SetCaptureSource(const TCHAR*);

	//! Returns the desire video playback rate
	int				GetFPS() const;

	//! Returns the desire video playback rate
	void			SetFPS(int);

	//! Internal function to update the target image with the new video frame data
	virtual bool	UpdateImage()					{ return true; }

	//! Internal function to stop and  unload the currently playing video
	virtual void	Unload()						{ };

	//! Internal function to change the video source
	virtual bool	ChangeVideo();

	//! Internal function to compute power of two texture dimensions
	static bool		ComputeTexSize(int& width, int& height);
	
	virtual void	IncreaseVolume() {};
	virtual void	DecreaseVolume() {};


	// Engine overrides
	virtual bool	OnStart();
	virtual bool	OnStop();
	virtual bool	Eval(float);
	virtual	bool	Copy(const SharedObj*);
	virtual	bool	Do(Messenger&, int);
	virtual	int		Save(Messenger&, int) const;
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Do opcodes (for binary file input)
	 */
	enum Opcode
	{
		VIDEO_SetFileName = ENG_NextOp,
		VIDEO_SetCaptureSource,
		VIDEO_SetOptions,
		VIDEO_SetFPS,
		VIDEO_NextOp = ENG_NextOp + 20,
	};

protected:
	VString				m_FileName;
	VString				m_CaptureSource;
	int32				m_VideoX;
	int32				m_VideoY;
	int32				m_Depth;
	int32				m_Options;
	float				m_VideoFrameTime;
	float				m_NextTime;
	bool				m_IsPlaying;
	bool				m_Change;
};

