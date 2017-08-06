
/*
 * @file vxmediaFFM.h
 * @brief Media decoding based on FFMPEG package.
 *
 * Media sources are used to map video frames onto textures in
 * the 3D scene and decode audio frames to play on sound hardware.
 * This version uses the FFMPEG package for decoding audio and video.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmediabase.h
 */
#pragma once

struct FFMPEG;

class VXMediaSource : public VXMediaBase
{
public:
	VX_DECLARE_CLASS(VXMediaSource);
	VXMediaSource();

protected:
	virtual	bool	Init(void* ptr);
	virtual	bool	Play();
	virtual	bool	Pause();
	virtual	void	Close();
	virtual	bool	Rewind();
	virtual	bool	Open(const char* filename);
	virtual	bool	ProcessFrame(Texture* image, VXSoundPlayer* sound);

	FFMPEG*		m_Player;	
};

