
/*
 * @file vxvideoimage.h
 * @brief Base class for video images.
 *
 * Video images are used to map video sources onto textures in
 * the 3D scene. The base class abstracts the platform-independent
 * video capabilities so that different low-level video APIs may be used.
 * The current implementation uses DirectShow.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxspeech.h vxmediafileIPP.h
 */
#pragma once

#define	VIDEO_Live	1

class VixenVideoRender;

class VXVideoImage : public VXMediaFile
{
public:
	VXVideoImage();
	~VXVideoImage();

public:
	//! Returns the pixel dimensions of the video frame
	VXVec2			GetVideoSize() const			{ return VXVec2(m_VideoX, m_VideoY); }

	//! Returns pixel dimensions of video texture (power of 2)
	VXVec2			GetTexSize() const				{ return VXVec2(m_TexWidth, m_TexHeight); }

	//! Returns bits per pixel of video frame (24, 32)
	int				GetDepth() const				{ return m_Depth; }

	//! Returns a pointer to the target image
	VXImage*		GetImage()						{ return (VXImage*) GetTarget(); }

	//! Sets the image to get the video data each frame
	void			SetImage(VXImage* image);

	//! Returns video playback and capture options
	int				GetOptions() const				{ return m_Options; }

	//! Sets the video capture and playback options
	void			SetOptions(int opts);

	//! Returns the name of the video capture source
	const TCHAR*	GetCaptureSource() const		{ return m_CaptureSource; }

	//! Sets the name of the video capture source
	void			SetCaptureSource(const TCHAR*);

	//! Returns the desired video playback rate
	int				GetFPS() const;

	//! Returns the desired video playback rate
	void			SetFPS(int);

	//! Internal function to compute power of two texture dimensions
	static bool		ComputeTexSize(int& width, int& height);
	
	// Engine overrides
	virtual void	SetFileName(const TCHAR* filename);
	virtual	bool	Copy(const VXObj*);
	virtual	bool	Do(VXMessenger&, int);
	virtual	int		Save(VXMessenger&, int) const;
	virtual	VDebug&	Print(VDebug& = vixen_debug, int opts = VXObj::PRINT_Default) const;

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
	virtual bool	Load();
	virtual	bool	Init(void*);

	VString				m_CaptureSource;
	int32				m_TexWidth;
	int32				m_TexHeight;
	int32				m_Depth;
	int32				m_Options;
	int32				m_fps;
	int32				m_Realfps;
	int32				m_SkipCount;
	int32				m_SkipAmount;
	double				m_FrameCount;
	float				m_PrevTime;
	float				m_PrevVal;
	bool				m_Change;
	VixenVideoRender*	m_VideoRender;
};

