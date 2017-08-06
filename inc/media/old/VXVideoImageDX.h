
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
#define _D3DRMOBJ_H_
#define __D3DRM_H__
#include <qedit.h>

#define BUFFERS 2
enum USERS
{
	TheImage,
	TheRGBVideo,
	TheAlphaVideo,
};

typedef struct
{
	unsigned char* buffer;
	int32	count;
} ImageBuffer;

enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// If the compiler can't find streams.h you need to add the
// following directory to your include search path:
// C:\MSSDK\SAMPLES\MULTIMEDIA\DIRECTSHOW\BASECLASSES
// C:\MSSDK is the directory in which the DirectX SDK is installed
//
#include <streams.h>

class CGrabCB;


class VXVideoImage : public VXVideoImageBase
{
// Construction
public:
	VX_DECLARE_CLASS(VXVideoImage);

	VXVideoImage();
	VXVideoImage(char* videofilename);
	~VXVideoImage();

	bool	Load();
	bool	UpdateImage();
	void	UnLoad();
	void	IncreaseVolume();
	void	DecreaseVolume();
	void	InitBuffers(int size);
	void*	GetCurrentBuffer(int kind);

	bool	OnStart();
	bool	OnStop();
	bool	Eval(float);


protected:
	bool	ChangeVideo();
	bool	ChangeVideoFile(char*);
	bool	ChangeLiveVideo();
	bool	InitVideo();
	bool	RenewDShow();
	HRESULT FindCaptureDevice(IBaseFilter ** );
	HRESULT	GetInterfaces(void);
	bool	CreateLiveVideoGrabber();
	bool	ImageCanBeUsed(ImageBuffer &ib);

protected:
	bool						m_Init;
	bool						m_Startup;
    IDirectDraw					*pDD;
    IDirectDrawSurface			*pPrimarySurface;
    IMultiMediaStream			*pMMStream;
	IAMMultiMediaStream			*pAMStream;
	IDirect3DSurface8			*m_TextureSurface;

	IMediaStream				*pPrimaryVidStream;
	IMediaSample				*pMediaSample;
    IDirectDrawMediaStream		*pDDStream;
    IDirectDrawStreamSample		*pSample;
    RECT						rect;
	int32						m_ImageSize;
	float						m_Duration, m_PauseEnd, m_PauseStart;


	HWND						ghApp;
	DWORD						g_dwGraphRegister;

	IMediaControl				*g_pMC;
	IMediaEvent					*g_pME;
	IMediaSeeking				*g_pMS;
	IGraphBuilder				*g_pGraph;
	ICaptureGraphBuilder2		*g_pCapture;
	PLAYSTATE					g_psCurrent;
	IBaseFilter					*pF;
	ISampleGrabber				*pGrab;
	bool						m_LiveVideo;
	int							m_Texheight, m_Texwidth;
	CGrabCB						*cb;
	CGrabCB						*cbAlpha;
	bool						m_DoStop;
	intptr						m_OwnerThread;
	char						m_CaptureDevices[30][85];
	ImageBuffer					m_Buffer[BUFFERS];
	int							m_CurrentImage;
	IAMBufferNegotiation		*g_Buffer;

};

class CGrabCB: public CUnknown, public ISampleGrabberCB
{
public:
    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
    {
        if( riid == IID_ISampleGrabberCB )
        {
            return GetInterface((ISampleGrabberCB*)this, ppv);
        }
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    // ISampleGrabberCB methods
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
    {
        return S_OK;
    }

    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
    {
		BYTE		*Dest = NULL, *pDst = NULL, *pSrc = NULL;
		if(m_Option & VIDEO_AlphaStream)
			Dest = (BYTE*) m_VideoEngine->GetCurrentBuffer(TheAlphaVideo);
		else
			Dest = (BYTE*) m_VideoEngine->GetCurrentBuffer(TheRGBVideo);
		if(Dest == NULL)
			return S_OK;
		pDst = Dest;
		pSrc		= (BYTE*)pBuffer;
		int stride = m_X*m_D;
		BYTE r=0, g=0, b=0, a=0;
		// UGLY HACK Cannot go to m_Y, must use m_Y - 1 or memcpy crashes,not sure why (ND 5/2001)
		for(int i = 0; i < m_Y - 1; i++)
		{
			for(int j = 0; j < m_X; j++)
			{
				if(m_Option & VIDEO_AlphaStream)
				{

					r = *pSrc++;
					g = *pSrc++;
					b = *pSrc++;
					pSrc++;
					pDst+=3;

					a = (r+g+b)/3;
					*pDst++ = a;

				/*	if((r > 128) || (g > 128) || (b > 128))
						*pDst++ = 255;
					else
						*pDst++ = 0;
					*/
				}
				else
				{
					memcpy(pDst, pSrc, 3);
					pDst+=4;
					pSrc+=4;
				}
			}
			pDst += m_Pitch - stride;
		}

		if((m_Option & VIDEO_LiveRGBAlpha) == 0)
			m_VideoEngine->GetCurrentBuffer(TheImage);

		return S_OK;
    }

	int					m_X;
	int					m_Y;
	int					m_D;
	int					m_Pitch;
	int32				m_Option;
	Ref<VXVideoImage>	m_VideoEngine;
	int					m_ImageSize;

    // Constructor
    CGrabCB( ) : CUnknown("SGCB", NULL)
    {
		m_X = m_Y = m_D = m_Pitch = 0;
		m_VideoEngine = (VXVideoImage*)NULL;
		m_ImageSize = 0;
	}

	~CGrabCB() {}

	void SetOption(int32 option) {m_Option = option;}
	void SetX(int x) {m_X = x;}
	void SetY(int y) {m_Y = y;}
	void SetD(int d) {m_D = d;}
	void SetPitch(int p) {m_Pitch = p;}
	void SetImageSize(int s) {m_ImageSize = s;}
	void SetVideoEngine(VXVideoImage *eng)
	{
		m_VideoEngine = eng;
		m_VideoEngine->InitBuffers(m_ImageSize);
	}
};

