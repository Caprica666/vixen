
#include "vixen.h"
#include "vxmedia.h"
#include "highgui.h"

//#define USE_FFMPEG

#define	NONE	0
#define	ERR		-1
#define	VIDEO	1
#define	AUDIO	2

#ifdef USE_FFMPEG
#define HAVE_FFMPEG_SWSCALE 1
#define __STDC_CONSTANT_MACROS 1
#undef snprintf
#define snprintf _snprintf

extern "C" {
#ifdef _MSC_VER
#include "msc_stdint.h"
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

typedef struct CvCapture CvCapture;

struct CvCapture
{
    virtual ~CvCapture() {}
    virtual double getProperty(int) { return 0; }
    virtual bool setProperty(int, double) { return 0; }
    virtual bool grabFrame() { return true; }
    virtual IplImage* retrieveFrame(int) { return 0; }
    virtual IplImage* queryFrame() { return grabFrame() ? retrieveFrame(0) : 0; }
	virtual int getCaptureDomain() { return CV_CAP_ANY; } // Return the type of the capture object: CV_CAP_VFW, etc...
};

class CvCapture_FFMPEG_proxy : public CvCapture
{
public:
    CvCapture_FFMPEG_proxy();

    void* ffmpegCapture;
};

class CvCapture_FFMPEG : public CvCapture
{
public:
    CvCapture_FFMPEG() { init(); }
    virtual ~CvCapture_FFMPEG() { close(); }

    virtual bool open( const char* filename );
    virtual void close();

    virtual double getProperty(int);
    virtual bool setProperty(int, double);
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);

protected:
    void init();
    bool reopen();
    bool slowSeek( int framenumber );

public:
    AVFormatContext*	ic;
    int					video_stream;
    AVStream*			video_st;
    AVFrame*			picture;
    int64_t             picture_pts;
    AVFrame             rgb_picture;
    AVPacket            packet;
    IplImage            frame;
#if defined(HAVE_FFMPEG_SWSCALE)
    struct SwsContext*	img_convert_ctx;
#endif
    char*				filename;
};
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(MediaSource, MediaBase, VX_MediaSource);

/*!
 * @fn MediaSource::MediaSource()
 * Constructs a MediaSource object that plays video or audio files.
 *
 * @see MediaBase Engine 
 */
MediaSource::MediaSource() : MediaBase()
{
	m_Stream = NULL;
	m_FFMPEG = NULL;
	m_AudioStream = NULL;
	m_AudioStreamIndex = -1;
	m_AudioBuffer = NULL;
}


bool MediaSource::Rewind()
{
	return false;
}


void MediaSource::Close()
{
	m_AudioStreamIndex = -1;
	m_FFMPEG = NULL;
	if (m_Stream)
	{
		cvReleaseCapture(&m_Stream);
		m_Stream = NULL;
	}
}

bool MediaSource::Open(const char* filename)
{
	Texture*	image = (Texture*) GetTarget();
	float		videofps;
	int			texwidth, texheight;		

	if (m_Options & LIVE)
		m_Stream = cvCreateCameraCapture(0);
	else
		m_Stream = cvCreateFileCapture(filename);
	if (!m_Stream)
		VX_ERROR(("MediaSource: ERROR cannot capture from %s", filename), false);
	InitAudio();
	m_VideoSize.x = (float) cvGetCaptureProperty(m_Stream, CV_CAP_PROP_FRAME_WIDTH);
	m_VideoSize.y = (float) cvGetCaptureProperty(m_Stream, CV_CAP_PROP_FRAME_HEIGHT);
	videofps = (float) (int) cvGetCaptureProperty(m_Stream, CV_CAP_PROP_FPS);

	if (videofps > 0)
		m_VideoFrameTime = (1.0f / videofps);
	cvSetCaptureProperty(m_Stream, CV_CAP_PROP_CONVERT_RGB, 1.0);
	if (ComputeTexSize(texwidth = (int) m_VideoSize.x, texheight = (int) m_VideoSize.x))
		m_TexSize.Set((float) texwidth, (float) texheight);
	return MediaBase::InitImage(image, (int) m_VideoSize.x, (int) m_VideoSize.y);
}

/*
 * If a video is playing or capturing and we have a target image,
 * grab the next frame and stuff it into the image.
 */
bool MediaSource::UpdateImage(Texture* image)
{
	IplImage*	cvimg;

	m_VideoFrameImage = cvimg = cvRetrieveFrame(m_Stream);
	if (image)
		return MediaBase::CopyImage(image, cvimg->imageData, cvimg->width, cvimg->height, cvimg->widthStep);
	return false;
}

#ifdef USE_FFMPEG
bool MediaSource::InitAudio()
{
	CvCapture_FFMPEG_proxy*	proxy = dynamic_cast<CvCapture_FFMPEG_proxy*> ( m_Stream );
	CvCapture_FFMPEG*		ffmpeg;

	if ((proxy == NULL) || (proxy->ffmpegCapture == NULL))
		return false;
	ffmpeg = (CvCapture_FFMPEG*) proxy->ffmpegCapture;
	ffmpeg->packet.data = NULL;
    for (int i = 0; i < ffmpeg->ic->nb_streams; i++)
	{
		AVCodecContext* enc = ffmpeg->ic->streams[i]->codec;
		if ((enc->codec_type == CODEC_TYPE_AUDIO) && (m_AudioStreamIndex < 0))
		{
            AVCodec* codec = avcodec_find_decoder(enc->codec_id);
 			m_FFMPEG = proxy->ffmpegCapture;
 			m_AudioStreamIndex = i;
            m_AudioStream = ffmpeg->ic->streams[i];
			if (!codec)
				m_AudioStream = ffmpeg->video_st;
			else if (avcodec_open(enc, codec) < 0)
				VX_ERROR(("MediaSource: ERROR cannot find audio codec"), false);
			if ((m_AudioStream->duration != AV_NOPTS_VALUE) && (m_Duration == 0))
				m_Duration = float(m_AudioStream->duration * m_AudioStream->time_base.num) / float(m_AudioStream->time_base.den); 
			return true;
		}
	}
	return false;
}

int MediaSource::QueryFrame()
{
	CvCapture_FFMPEG* ffmpeg = (CvCapture_FFMPEG*) m_FFMPEG;
	
	if (ffmpeg == NULL)					// not streaming audio?
		if (cvGrabFrame(m_Stream))		// just use OpenCV asis
			return VIDEO;
		else
			return ERR;
	if (ffmpeg->packet.data != NULL)    // free last packet if any
       av_free_packet (&ffmpeg->packet);
    while (1)							// get the next frame
	{
        int ret = av_read_frame(ffmpeg->ic, &ffmpeg->packet);
        if (ret == AVERROR(EAGAIN))
            continue;
        if (ret < 0)
            return ERR;      
		if (ffmpeg->packet.stream_index == ffmpeg->video_stream)
		{
			int got_picture;

			avcodec_decode_video(ffmpeg->video_st->codec,
                             ffmpeg->picture, &got_picture,
                             ffmpeg->packet.data, ffmpeg->packet.size);

			if (got_picture)            // save time stamp of new picture
			{
				ffmpeg->picture_pts = ffmpeg->packet.pts;
				return VIDEO;
			}
        }
//		else if (ffmpeg->packet.stream_index == m_AudioStreamIndex)
//			return AUDIO;				// return audio type
		else
			av_free_packet (&ffmpeg->packet);
    }
    return NONE;
}

bool MediaSource::ProcessFrame(Texture* image, SoundPlayer* sound)
{
	CvCapture_FFMPEG* ffmpeg = (CvCapture_FFMPEG*) m_FFMPEG;

	while (true)
	{
		switch (QueryFrame())
		{
			case AUDIO:			// process audio
			if (!DecodeAudio(sound))
				return false;
			break;

			case VIDEO:			// process video
			return UpdateImage(image);

			default:
			return false;
		}
	}
	return true;	
}

bool MediaSource::DecodeAudio(SoundPlayer* sound)
{
	CvCapture_FFMPEG* ffmpeg = (CvCapture_FFMPEG*) m_FFMPEG;
    if ((ffmpeg == NULL) ||
		(ffmpeg->packet.data == NULL))		// any sound data?
		return false;
	if (ffmpeg->packet.stream_index != m_AudioStreamIndex)
		return false;						// not audio data
	if (m_AudioBuffer == NULL)
		m_AudioBuffer = sound->NewBuffer();
	if (m_AudioBuffer == NULL)
	{
		av_free_packet(&ffmpeg->packet);
		ffmpeg->packet.data = NULL;			// free unused audio packet
		VX_ERROR(("MediaSource: cannot allocate audio buffer for streaming"), false);
	}
	int		bufsize = m_AudioBuffer->NumBytes;
	char*	buffer = m_AudioBuffer->GetData() + sound->GetBufferSize() - m_AudioBuffer->NumBytes;
	size_t	bytesused = avcodec_decode_audio3(m_AudioStream->codec, (int16*) buffer, &bufsize, &ffmpeg->packet);
	
	if (bytesused <= 0)
		return false;	
	m_AudioBuffer->NumBytes -= bytesused;
	bufsize = m_AudioBuffer->NumBytes;
	buffer = m_AudioBuffer->GetData() + sound->GetBufferSize() - m_AudioBuffer->NumBytes;
	if (m_AudioBuffer->NumBytes < 1024)
	{
		memset(buffer, 0, bufsize);			// zero the remainder
		sound->LoadBuffer(m_AudioBuffer);	// queue it up to play
		m_AudioBuffer = NULL;
	}
	return true;
}

#else
bool MediaSource::InitAudio() { return false; }

int MediaSource::QueryFrame()
{
	if (cvGrabFrame(m_Stream))
		return VIDEO;
	else
		return ERR;
}

bool MediaSource::ProcessFrame(Texture* image, SoundPlayer* sound)
{
	if (QueryFrame() == VIDEO)
		return UpdateImage(image);
	return false;
}

bool MediaSource::DecodeAudio(SoundPlayer* sound) { return false; }

#endif
}	// end Vixen