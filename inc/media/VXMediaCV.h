
/*
 * @file vxmediacv.h
 * @brief Media decoding based on FFMPEG package.
 *
 * Media sources are used to map video frames onto textures in
 * the 3D scene and decode audio frames to play on sound hardware.
 * This version uses the OpenCV (Open Computer Vision) to sample
 * video streams from cameras or files. It currently cannot play
 * audio even with the SoundPlayer attached.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmediabase.h
 */
#pragma once

struct CvCapture;
struct AVStream;

typedef struct _IplImage IplImage;

namespace Vixen {

/*!
 * @class MediaSource
 * @brief plays video files or captures a video stream from a camera.
 *
 * The input to the media source is either a file on the local disk
 * (no internet streaming right now) or a video camera on the local machine.
 * The output of the media source is a Texture whose content changes
 * as the video is player. This texture can be used on any object in
 * the 3D scene to show the video playing (the object can be a mesh).
 * This class is just an implementation of the MediaBase API using
 * Open Computer Vision.
 *
 * @see MediaBase SoundPlayer
 */
class MediaSource : public MediaBase
{
public:
	VX_DECLARE_CLASS(MediaSource);
	MediaSource();

protected:
	virtual	void	Close();
	virtual	bool	Rewind();
	virtual	bool	Open(const char* filename);
	virtual	bool	ProcessFrame(Texture* image, SoundPlayer* sound);
	bool			DecodeAudio(SoundPlayer* sound);
	bool			InitAudio();
	int				QueryFrame();
	bool			UpdateImage(Texture* image);

	CvCapture*		m_Stream;
	void*			m_FFMPEG;
	int				m_AudioStreamIndex;
	Core::Buffer*	m_AudioBuffer;
	AVStream*		m_AudioStream;
	IplImage*		m_VideoFrameImage;
};

}	// end Vixen