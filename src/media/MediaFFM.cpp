

/****************************************************************************
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or 
 *  nondisclosure agreement with Intel Corporation and may not be copied 
 *  or disclosed except in accordance with the terms of that agreement. 
 *
 *      Copyright (c) 1998-2011 Intel Corporation. All Rights Reserved.
 *
 *
 ****************************************************************************/
#include "vixen.h"
#include "vxmedia.h"

#define HAVE_FFMPEG_SWSCALE 1
#define __STDC_CONSTANT_MACROS 1
#undef snprintf
#define snprintf _snprintf

extern "C" {

#include "stdint.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
}

VX_IMPLEMENT_CLASSID(VXMediaSource, VXMediaBase, VX_MediaSource);

struct FFMPEG
{
	enum FrameType
	{
		NONE = 0,
		VIDEO = 1,
		AUDIO = 2,
		ERR = -1
	};

    AVFormatContext* Context;
    int				VideoStreamIndex;
    int				AudioStreamIndex;
    AVStream*		VideoStream;
	AVStream*		AudioStream;
    AVFrame*		Frame;
	AVFrame			RGBData;
    AVPacket		Packet;
	int64			FrameTime;
	int				VideoWidth;
	int				VideoHeight;
	float			Duration;
	float			FPS;
	VBuffer*		AudioBuffer;

	FFMPEG();
	~FFMPEG();
	void		Close();
	bool		Open(const char* filename);
	FrameType	QueryFrame();
	bool		DecodeVideo(Texture*);
	bool		DecodeAudio(VXSoundPlayer*);
};


/*!
 * @fn VXMediaSource::VXMediaSource()
 * Constructs an VXMediaSource object that plays video or audio files.
 *
 * @see VXVideoImage Engine 
 */
VXMediaSource::VXMediaSource() : VXMediaBase()
{
	m_Player = NULL;
}

bool VXMediaSource::Init(void* ptr)
{
	if (m_Player == NULL)
		m_Player = new FFMPEG();
	return true;
}

bool VXMediaSource::Play()
{
	if (m_Player == NULL)
		return false;
	return true;
}

bool VXMediaSource::Rewind()
{
	return false;
}

bool VXMediaSource::Pause()
{
	return true;
}

void VXMediaSource::Close()
{
	if (m_Player)
		m_Player->Close();
	m_IsPlaying = false;
}

bool VXMediaSource::Open(const char* filename)
{
	if (m_Player == NULL)
		return false;
	if (!m_Player->Open(filename))
		return false;
	if (m_Player->Duration > 0)
		m_Duration = m_Player->Duration;
	if (m_Player->FPS > 0)
		m_VideoFrameTime = 1.0f / m_Player->FPS;
	return true;
}

bool VXMediaSource::ProcessFrame(Texture* image, VXSoundPlayer* sound)
{
	while (1)
	{
		switch (m_Player->QueryFrame())
		{
			case FFMPEG::AUDIO:			// process audio
			if (!m_Player->DecodeAudio(sound))
				return false;
			break;

			case FFMPEG::VIDEO:			// process video
			return m_Player->DecodeVideo(image);
			break;

			default:					// error or no more
			return false;
		}
	}
	return true;	
}


FFMPEG::FFMPEG()
{
	FrameTime = 0;
	Context = NULL;
	VideoStream = NULL;
	AudioStream = NULL;
	Frame = NULL;
	VideoStreamIndex = -1;
	AudioStreamIndex = -1;
	VideoWidth = 0;
	VideoHeight = 0;
	AudioBuffer = NULL;
	memset(&RGBData, 0, sizeof(RGBData));
	Packet.data = NULL;
	av_register_all();
	avcodec_init();
    avcodec_register_all();
	avdevice_register_all();
}

FFMPEG::~FFMPEG()
{
	Close();
}

void FFMPEG::Close()
{
    if (Frame)
	{
		av_free(Frame);
		Frame = NULL;
	}
	if (RGBData.data[0])
	{
        free(&RGBData.data[0]);
		memset(&RGBData, 0, sizeof(RGBData));
	}
    // free last packet if exist
    if (Packet.data)
	{
        av_free_packet(&Packet);
		Packet.data = NULL;
    }
    if (VideoStream && VideoStream->codec)
    {
        avcodec_close(VideoStream->codec);
        VideoStream = NULL;
    }
     if (AudioStream && AudioStream->codec)
    {
        avcodec_close(AudioStream->codec);
        AudioStream = NULL;
    }
   if (Context)
    {
        av_close_input_file(Context);
        Context = NULL;
	}
	VideoStreamIndex = AudioStreamIndex = -1;
	FrameTime = 0;
	Duration = 0;
    av_register_all();
}

bool FFMPEG::Open(const char* filename)
{
	AVCodecContext*	enc;
	AVCodec*		codec;
	int				err;

    err = av_open_input_file(&Context, filename, NULL, 0, NULL);
	if (err < 0)
		VX_ERROR(("VXMediaSource: ERROR cannot open %s", filename), false);
    av_find_stream_info(Context);
	Duration = 0;
	FPS = 0;
    for (int i = 0; i < Context->nb_streams; i++)
	{
		enc = Context->streams[i]->codec;
        Frame = avcodec_alloc_frame();
		/*
		 * If the input file contains a video stream, find a suitable video codec
		 */
        if ((enc->codec_type == CODEC_TYPE_VIDEO) && (VideoStreamIndex < 0))
		{
            AVCodec *codec = avcodec_find_decoder(enc->codec_id);
            if (!codec || avcodec_open(enc, codec) < 0)
				VX_ERROR(("VXMediaSource: ERROR cannot find video codec for %s", filename), false);
            VideoStreamIndex = i;
            VideoStream = Context->streams[i];
			VideoWidth = enc->width;
			VideoHeight = enc->height;
			FPS = float(VideoStream->r_frame_rate.num) /float(VideoStream->r_frame_rate.den);
			if (VideoStream->duration != AV_NOPTS_VALUE)
				Duration = float(VideoStream->duration * VideoStream->time_base.num) / float(VideoStream->time_base.den);
            RGBData.data[0] = (uint8_t*) malloc(avpicture_get_size(PIX_FMT_BGR24, enc->width, enc->height));
            avpicture_fill((AVPicture*) &RGBData, RGBData.data[0],
							PIX_FMT_BGR24, enc->width, enc->height);
		}
		/*
		 * If input file contains an audio stream, find a decoder for it
		 */
        else if ((enc->codec_type == CODEC_TYPE_AUDIO) && (AudioStreamIndex < 0))
		{
            codec = avcodec_find_decoder(enc->codec_id);
            if (!codec || avcodec_open(enc, codec) < 0)
				VX_ERROR(("VXMediaSource: ERROR cannot find audio codec for %s", filename), false);
            AudioStreamIndex = i;
            AudioStream = Context->streams[i];
			if ((AudioStream->duration != AV_NOPTS_VALUE) && (Duration == 0))
				Duration = float(AudioStream->duration * AudioStream->time_base.num) / float(AudioStream->time_base.den); 
		}
	}
	if ((VideoStreamIndex + AudioStreamIndex) < 0)
		return false;
	return true;
}

FFMPEG::FrameType  FFMPEG::QueryFrame()
{
    bool	valid = false;
    int		got_picture = 0;
	int		got_sound = 0;
	int		ret;

    if (!Context)
        return ERR;
    if (Packet.data != NULL)    // free last packet we got
	{
        av_free_packet(&Packet);
		Packet.data = NULL;
    }
    while ((ret = av_read_frame(Context, &Packet)) >= 0) // get the next frame
	{
        if (ret == AVERROR(EAGAIN))
            continue;
		// If this is a video packet, return type VIDEO
		if (VideoStream && (Packet.stream_index == VideoStreamIndex))
		{
			avcodec_decode_video2(VideoStream->codec,
								 Frame, &got_picture,
								 &Packet);
			if (got_picture)
			{
				FrameTime = Packet.pts;
				return VIDEO;		// got video
			}
			else
				return ERR;			// no more video
		}
		// If this is an audio packet, return type AUDIO
		if (AudioStream && (Packet.stream_index == AudioStreamIndex))
			return AUDIO;			// return audio type
		// Discard unknown packet types
		else
		{
			av_free_packet(&Packet);
			Packet.data = NULL;
		}
    }
    return NONE;
}

bool FFMPEG::DecodeVideo(Texture* image)
{
	if (Packet.data == NULL)
		return false;
	if (Packet.stream_index != VideoStreamIndex)
		return false;
    av_free_packet(&Packet);		// free video packet
	Packet.data = NULL;				// data already decoded
	if (image == NULL)
		return false;
	SwsContext* img_convert_ctx = sws_getContext(VideoStream->codec->width, VideoStream->codec->height,
						VideoStream->codec->pix_fmt,
						VideoStream->codec->width, VideoStream->codec->height,
						PIX_FMT_BGR24, SWS_BICUBIC,
						NULL, NULL, NULL);
	sws_scale(img_convert_ctx, Frame->data,
				Frame->linesize, 0,
				VideoStream->codec->height,
				RGBData.data, RGBData.linesize);
	sws_freeContext(img_convert_ctx);
	/*
	 * Copy the pixel data from the video frame to the texture.
	 * The texture is a power of 2 - it is probably not the same size as the video frame.
	 * The bitmap associated with the texture contains the actual pixels.
	 * We have already allocated the data buffer for these pixels in ChangeVideo
	 */
	ObjectLock	locki(image);
	Bitmap*	bitmap = image->GetBitmap();

	if (bitmap == NULL)
		return false;

	ObjectLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	bool		is_bottom_left = false;

	if (videobuffer == NULL)
		return false;
	VX_ASSERT(image->GetWidth() >= VideoWidth);
	VX_ASSERT(image->GetHeight() >= VideoHeight);

	for (int y = 0; y < VideoHeight; y++)
	{
		const char* src = (char*) RGBData.data[0] + y * RGBData.linesize[0];
		char *dst = is_bottom_left ?
					(videobuffer + y * VideoWidth * 3) :
					(videobuffer + (VideoHeight - y - 1) * VideoWidth * 3);
		memcpy(dst, src, VideoWidth * 3);
	}
	bitmap->Type = Bitmap::IMAGE_DXBitmap;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
	return true;	
}

bool FFMPEG::DecodeAudio(VXSoundPlayer* sound)
{
    if (Packet.data == NULL)    // any sound data?
		return false;
	if (Packet.stream_index !=AudioStreamIndex)
		return false;			// not audio data
	if (sound == NULL)
	{
        av_free_packet(&Packet);
		Packet.data = NULL;		// free unused audio packet
		return false;
    }
	if (AudioBuffer == NULL)
		AudioBuffer = sound->NewBuffer();
	if (AudioBuffer == NULL)
	{
		av_free_packet(&Packet);
		Packet.data = NULL;		// free unused audio packet
		VX_ERROR(("VXMediaSource: cannot allocate audio buffer for streaming"), false);
	}
	int		bufsize = sound->GetBufferSize() - AudioBuffer->NumBytes;
	char*	buffer = AudioBuffer->GetData() + AudioBuffer->NumBytes;
	size_t	bytesused;

	while ((Packet.size > 0) &&
		   ((bytesused = avcodec_decode_audio3(AudioStream->codec, (int16*) buffer, &bufsize, &Packet)) > 0))
	{
		Packet.size -= bytesused;
		Packet.data += bytesused;
		AudioBuffer->NumBytes = bufsize;
		bufsize = sound->GetBufferSize() - AudioBuffer->NumBytes;
		buffer = AudioBuffer->GetData() + AudioBuffer->NumBytes;
		if (bufsize < 1024)
		{
			memset(buffer, 0, bufsize);		// zero the remainder
			sound->LoadBuffer(AudioBuffer);	// queue it up to play
			AudioBuffer = sound->NewBuffer();
			if (AudioBuffer == NULL)		// allocate new sound buffer
				break;
		}
	}
	av_free_packet(&Packet);
	Packet.data = NULL;
	return true;
}