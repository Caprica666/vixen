
/*
 * @file vxvideoimagedx.h
 * @brief DirectShow video texture.
 *
 * Video images are used to map video sources onto textures in
 * the 3D scene. This version uses DirectShow for video capture and playback.
 *
 * @author Judith Stanley
 * @ingroup vixen
 *
 * @see vxspeech.h vxvideoimage.h
 */
#pragma once

#include <dshow.h>
#include <streams.h>

#define _D3DRMOBJ_H_
#define __D3DRM_H__
#include <qedit.h>

class IMFPMediaPlayer;

class VXVideoImage : public VXVideoImageBase
{
public:
	VX_DECLARE_CLASS(VXVideoImage);

	VXVideoImage();
	VXVideoImage(const TCHAR* filename);
	~VXVideoImage();

	virtual	bool	Load();
	virtual	bool	UpdateImage(void* newdata);
	virtual	void	UnLoad();
	virtual	void	IncreaseVolume();
	virtual	void	DecreaseVolume();
	virtual bool	OnStart();
	virtual bool	OnStop();
	virtual bool	Eval(float);

protected:
	virtual bool	ChangeVideo();
	bool			ChangeVideoFile(const TCHAR*);
	bool			ChangeLiveVideo();
	bool			InitVideo();
	HRESULT			GetInterfaces(void);
	bool			CreateLiveVideoGrabber();
	bool			InitTexture(int width, int height);

	bool					m_Init;
	int32					m_ImageSize;
	int32					m_Texheight;
	int32					m_Texwidth;
	int32					m_Stride;
	float					m_Duration;
	float					m_PauseEnd;
	float					m_PauseStart;
	bool					m_LiveVideo;
	bool					m_HasVideo;

	MediaCaptureCallback*	m_CaptureCB;
	MediaPlayerCallback*	m_PlayerCB;
};

