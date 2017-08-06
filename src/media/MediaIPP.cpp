

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

#include "avsync.h"
#include "codec_pipeline.h"
#include "umc_media_data_ex.h"
#include "null_audio_render.h"
#include "umc_module_context.h"
#include "umc_audio_codec.h"
#include "null_video_renderer.h"

namespace Vixen {

#define		MAX_X 1024
#define		MAX_Y 1024

/*
 * A UMC video renderer that provides access to the current video frame
 */
class VixenVideoRender : public UMC::NULLVideoRender
{
public:
	VixenVideoRender() : UMC::NULLVideoRender() { }
	void		SetImage(Texture* image)	{ m_Image = image; }
	Texture*	GetImage() const			{ return m_Image; }

	virtual UMC::Status RenderFrame(void);
	virtual	UMC::Status ResizeDisplay(UMC::RECT& disp, UMC::RECT& range);
protected:
	void	UpdateImage(void* videobuffer);

	Ref<Texture>	m_Image;
};


VX_IMPLEMENT_CLASSID(MediaSource, MediaBase, VX_MediaSource);

/*!
 * @fn VXMediaSource::VXMediaSource()
 * Constructs an VXMediaSource object that plays video or audio files.
 *
 * @see VXVideoImage Engine 
 */
MediaSource::MediaSource() : MediaBase()
{
	m_Player = NULL;
}


bool MediaSource::Open(const TCHAR* filename)
{
	AVSync::CommonCtl		params;
	UMC::HWNDModuleContext	modContext;
    UMC::LocalReaderContext	readContext;
	AVSync::ExternalInfo	exinfo;
	AVSync::Stat			status;
	UMC::RECT				videosize;
	VixenVideoRender*		vvr = m_VideoRender;
	Texture*				image = (Texture*) GetTarget();
	UMC::UnifiedVideoRenderParams vrinfo;

	m_Player = new AVSync();
	if (m_Player == NULL)
		VX_ERROR(("ERROR: cannot make media player for %s", filename), false);
	STRCPY(readContext.m_szFileName, filename);
	modContext.m_hWnd = (HWND) GetMainScene()->GetWindow();
	params.pExternalInfo = &exinfo;
    params.pReadContext = &readContext;
    params.pRenContext = &modContext;
	params.uiPrefVideoRender = UMC::DEF_VIDEO_RENDER;
    params.uiPrefAudioRender = UMC::DEF_AUDIO_RENDER;
	// TODO: check if there is a sound player and disable audio if not
	params.cformat = UMC::RGB24;
	params.ulSplitterFlags = UMC::AUDIO_SPLITTER | UMC::VIDEO_SPLITTER;

	if (vvr == NULL)
		m_VideoRender = vvr = new VixenVideoRender;
	params.pExternalInfo->m_pVideoRender = vvr;
	vvr->SetImage(image);
	if (m_Player->Init(params) != UMC::UMC_OK)
		VX_ERROR(("MediaSource ERROR: cannot open %s", m_FileName), false);
	m_VideoY = m_Player->GetDstFrmHeight();
	m_VideoX = m_Player->GetDstFrmWidth();
	vrinfo.disp.right = vrinfo.range.right = m_VideoX;
	vrinfo.disp.bottom = vrinfo.range.bottom = m_VideoY;
	vrinfo.out_data_template.Init(m_VideoX, m_VideoY, UMC::RGB24, 24);
	vvr->Init(&vrinfo);
	if (m_Player->GetStat(status) != UMC::UMC_OK)
		VX_ERROR(("MediaSource ERROR: cannot read media stream %s", m_FileName), false);
	m_Duration = (float) status.dfDuration;
	return true;
}

bool MediaSource::Play()
{
	if (m_Player == NULL)
		return true;
	if (!m_Player->IsPlaying())
		m_Player->Start();
	else
		m_Player->Resume();
	return true;
}

bool MediaSource::Rewind()
{
	if (m_Player == NULL)
		return true;
	if (m_Player->IsPlaying())
		m_Player->Stop();
	m_Player->Start();
	return true;
}

bool MediaSource::Pause()
{
	if (m_Player == NULL)
		return true;
	if (m_Player->IsPlaying())
		m_Player->Pause();
	return true;
}

void MediaSource::Close()
{
	if (m_Player == NULL)
		return;
	if (m_Player->IsPlaying())
		m_Player->Stop();
	m_IsPlaying = false;
	delete m_Player;
	m_Player = NULL;
}

bool MediaSource::Eval(float t)
{
	return true;
}

UMC::Status VixenVideoRender::RenderFrame()
{
	UMC::Status umcRes = UMC::UMC_OK;
	UMC::VideoData out_data;

    if(m_iReadIndex < 0)
        umcRes = UMC::UMC_ERR_NOT_INITIALIZED;
    if (UMC::UMC_OK == umcRes)
    {
        m_SuncMut.Lock();
        Ipp32s  iReadI  = m_iReadIndex;
        bool    bShow   = m_bShow;
        void*   overlay = m_Buffers[m_iReadIndex].surface;
        m_SuncMut.Unlock();

		if (bShow)
			UpdateImage(overlay);
        if (UMC::UMC_OK == umcRes)
            m_iLastFrameIndex = iReadI;
        m_hFreeBufSema.Signal();
    }
	return umcRes;
}

UMC::Status VixenVideoRender::ResizeDisplay(UMC::RECT& disp, UMC::RECT& range)
{
	int			texwidth = disp.right;
	int			texheight = disp.bottom;
	Texture*	image = m_Image;

	VX_ASSERT(disp.left == 0);
	VX_ASSERT(disp.top == 0);
	if (MediaBase::InitImage(image, texwidth, texheight))
		return NULLVideoRender::ResizeDisplay(disp, range);
	return false;
}

/*
 * Copy the pixel data from the video frame to the texture.
 * The texture is a power of 2 - it is probably not the same size as the video frame.
 * The bitmap associated with the texture contains the actual pixels.
 * We have already allocated the data buffer for these pixels in ChangeVideo
 */
void VixenVideoRender::UpdateImage(void* videoframe)
{
	Texture*	image = GetImage();
	int			wvideo = m_disp.right - m_disp.left;
	int			hvideo = m_disp.bottom - m_disp.top;

	if ((wvideo == 0) || (hvideo == 0))
		return;
	if (image)
		MediaBase::CopyImage(image, (const char*) videoframe, wvideo, hvideo, wvideo * 3);
}

} // end Vixen