

/****************************************************************************
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or 
 *  nondisclosure agreement with Intel Corporation and may not be copied 
 *  or disclosed except in accordance with the terms of that agreement. 
 *
 *      Copyright (c) 1998-2013 Intel Corporation. All Rights Reserved.
 *
 *
 ****************************************************************************/
#include "dx9/vixendx.h"
#include "vxmedia.h"
#include <dshow.h>
#include <vmr9.h>

namespace Vixen {

VX_IMPLEMENT_CLASSID(MediaSource, MediaBase, VX_MediaSource);

#define MAX_VIDEO_SURFACES	4
#define	VIDEO_USERID	0xACDCACDC

class DXVideo  : public SharedObj, IVMRSurfaceAllocator9, IVMRImagePresenter9
{
public:
	DXVideo();
	virtual ~DXVideo();
	void	SetTexture(Texture* tex)	{	ObjectLock lock(this); m_Texture = tex; }
	void	CloseGraph();
	bool	MakeGraph(const TCHAR* videofile);
	void	Play();
	void	Pause();
	void	Stop();
	IDirect3DDevice9* GetDevice();

	ULONG	AddRef()
	{
		IncUse();
		return GetUse();
	}

	ULONG	Release()
	{
		return Delete();
	}

    // IVMRSurfaceAllocator9
    virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
            /* [in] */ DWORD_PTR dwUserID,
            /* [in] */ VMR9AllocationInfo *lpAllocInfo,
            /* [out][in] */ DWORD *lpNumBuffers);
            
    virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
        /* [in] */ DWORD_PTR dwID);
    
    virtual HRESULT STDMETHODCALLTYPE GetSurface( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ DWORD SurfaceIndex,
        /* [in] */ DWORD SurfaceFlags,
        /* [out] */ IDirect3DSurface9 **lplpSurface);
    
    virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
        /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

    // IVMRImagePresenter9
    virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE PresentImage( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9PresentationInfo *lpPresInfo);
    
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void** ppvObject);

	int		Width;		// width of video
	int		Height;		// height of video
	float	FrameTime;	// average time per frame

protected:
	void DeleteSurfaces();
	HRESULT SetAllocatorPresenter(IBaseFilter *filter);

private:
	vint32								m_refCount;
	Ref<Texture>						m_Texture;
	LPD3D								m_D3D;
	LPD3DDEVICE							m_D3DDev;
	DXRef<IVMRSurfaceAllocatorNotify9>	m_lpIVMRSurfAllocNotify;
	IDirect3DSurface9*					m_surfaces[MAX_VIDEO_SURFACES];
	DXRef<IDirect3DTexture9>			m_privateTexture;
	DXRef<IGraphBuilder>				m_graph;
	DXRef<IBaseFilter>					m_filter;
	DXRef<IMediaControl>				m_mediaControl;
	DXRef<IVMRSurfaceAllocator9>		m_allocator;
};

/*!
 * @fn MediaSource::MediaSource()
 * Constructs a MediaSource object that plays video or audio files.
 *
 * @see MediaBase Engine 
 */
MediaSource::MediaSource() : MediaBase()
{
	m_Video = NULL;
}

bool MediaSource::Init(void* ptr)
{
	m_Video = new DXVideo();
	return true;
}

bool MediaSource::Rewind()
{
	m_Video->Stop();
	m_Video->Play();
	return true;
}

bool	MediaSource::Play()
{
	m_Video->Play();
	return true;
}

bool	MediaSource::Pause()
{
	m_Video->Pause();
	return true;
}

void MediaSource::Close()
{
	m_Video->CloseGraph();
}

bool MediaSource::Open(const char* filename)
{
	Texture*	image = (Texture*) GetTarget();
	int			texwidth, texheight;		

	if (m_Video->GetDevice() == NULL)		// DirectX not initialized yet
	{
		if (IsActive())
			Start(0);
		VX_ERROR(("MediaSource: ERROR DirectX not initialized, cannot create media source for ", filename), false);
	}
	if (!m_Video->MakeGraph(filename))
		VX_ERROR(("MediaSource: ERROR cannot capture from %s", filename), false);
	m_VideoSize.x = (float) m_Video->Width;
	m_VideoSize.y = (float) m_Video->Height;
	m_VideoFrameTime = m_Video->FrameTime;
	if (ComputeTexSize(texwidth = (int) m_VideoSize.x, texheight = (int) m_VideoSize.x))
		m_TexSize.Set((float) texwidth, (float) texheight);
	return true;
}

/*
 * If a video is playing or capturing and we have a target image,
 * grab the next frame and stuff it into the image.
 */
bool MediaSource::ProcessFrame(Texture* image, SoundPlayer* sound)
{
	if (image)
	{
		m_Video->SetTexture(image);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DXVideo::DXVideo() : m_D3DDev(NULL), m_refCount(1)
{
	FrameTime = 1.0f / 30.0f;
	Width = 1;
	Height = 1;
	for (int i = 0; i < MAX_VIDEO_SURFACES; ++i)
		m_surfaces[i] = NULL;
	GetDevice();
}

DXVideo::~DXVideo()
{
	DeleteSurfaces();
	CloseGraph();
}

bool DXVideo::MakeGraph(const TCHAR* path)
{
	WCHAR				wpath[VX_MaxPath];
	Core::String		tmp(path);
	HRESULT				hr;
	IBasicVideo*		basicVideo = NULL;
	IGraphBuilder*		graph = NULL;
	IBaseFilter*		filter = NULL;
	IVMRFilterConfig9*	config = NULL;
	IMediaControl*		control = NULL;
	
	tmp.AsWide(wpath, VX_MaxPath);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &filter);
	if (FAILED(hr))
		VX_ERROR(("ERROR: MediaSource: Video Mixing Renderer does not exist, cannot create DX media source"), false);
	m_filter = filter;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**) &graph);
	if (FAILED(hr))
		VX_ERROR(("ERROR: MediaSource: Cannot create filter graph"), false);
	m_graph = graph;
	hr = m_filter->QueryInterface(IID_IVMRFilterConfig9, reinterpret_cast<void**>(&config));
	if (SUCCEEDED(hr))
	{		
		 hr = config->SetRenderingMode(VMR9Mode_Renderless);
		if (SUCCEEDED(hr))
			hr = config->SetNumberOfStreams(2);
		 config->Release();
	}
	if (SUCCEEDED(hr))
		hr = SetAllocatorPresenter(filter);
	if (SUCCEEDED(hr))
		hr = graph->AddFilter(filter, L"Video Mixing Renderer 9");
	if (SUCCEEDED(hr))
		hr = graph->RenderFile(wpath, NULL);
	if (SUCCEEDED(hr))
		hr = graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&control));
	if (SUCCEEDED(hr))
	{
		m_mediaControl = control;
		hr = graph->QueryInterface(IID_IBasicVideo, (LPVOID *) &basicVideo);
	}
	if (SUCCEEDED(hr))
	{
		REFTIME	frametime;
		long	height;
		long	width;

		hr = basicVideo->get_AvgTimePerFrame(&frametime);
		hr = basicVideo->get_VideoHeight(&height);
		hr = basicVideo->get_VideoWidth(&width);
		basicVideo->Release();
		if (SUCCEEDED(hr))
		{
			Width = width;
			Height = height;
			FrameTime = (float) frametime;
		}
		return true;
	}
	CloseGraph();
	VX_ERROR(("ERROR: MediaSource: Cannot create Filter Graph"), false);
}

void DXVideo::CloseGraph()
{
	if (m_mediaControl != NULL ) 
	{
		OAFilterState state;
		do
		{
			m_mediaControl->Stop();
			m_mediaControl->GetState(0, & state );
		}
		while( state != State_Stopped ) ;
	}
	m_allocator    = NULL;
	m_mediaControl = NULL;
	m_filter       = NULL;
	m_graph        = NULL;
}

void DXVideo::Play()
{
	if (m_mediaControl != NULL)
		m_mediaControl->Run();
}

void DXVideo::Pause()
{
	if (m_mediaControl != NULL)
		m_mediaControl->Pause();
}

void DXVideo::Stop()
{
	if (m_mediaControl != NULL)
		m_mediaControl->Stop();
}

HRESULT DXVideo::SetAllocatorPresenter(IBaseFilter *filter)
{
	if( filter == NULL )
		return E_FAIL;

	HRESULT hr;
	IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify;
	hr = filter->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, reinterpret_cast<void**>(&lpIVMRSurfAllocNotify));

	// create our surface allocator
	m_allocator = this;
	if (FAILED(hr))
	{
		m_allocator = NULL;
		return hr;
	}
	// let the allocator and the notify know about each other
	hr = lpIVMRSurfAllocNotify->AdviseSurfaceAllocator(VIDEO_USERID, m_allocator);
	if (SUCCEEDED(hr))
		hr = m_allocator->AdviseNotify(lpIVMRSurfAllocNotify);
	lpIVMRSurfAllocNotify->Release();
	return hr;
}

IDirect3DDevice9* DXVideo::GetDevice()
{
	if (m_D3DDev != NULL)
		return m_D3DDev;

	World3D*	world = World3D::Get();
	DXScene*	scene = (DXScene*) world->GetScene(0);

	if (scene)
	{
		m_D3D = scene->GetD3D();
		m_D3DDev = scene->GetDevice();
	}
	return m_D3DDev;
}

void DXVideo::DeleteSurfaces()
{
	ObjectLock	lock(this);
	m_privateTexture = NULL;
	for (size_t i = 0; i < MAX_VIDEO_SURFACES; ++i)
	{
		if (m_surfaces[i] != NULL)
			m_surfaces[i]->Release();
		m_surfaces[i] = NULL;
	}
}


//IVMRSurfaceAllocator9
HRESULT DXVideo::InitializeDevice( 
            /* [in] */ DWORD_PTR dwUserID,
            /* [in] */ VMR9AllocationInfo *lpAllocInfo,
            /* [out][in] */ DWORD *lpNumBuffers)
{
	DWORD dwWidth = 1;
	DWORD dwHeight = 1;
	float fTU = 1.f;
	float fTV = 1.f;
	HRESULT hr = S_OK;
	IDirect3DDevice9* device = GetDevice();

	if (lpNumBuffers == NULL)
		return E_POINTER;
	if( m_lpIVMRSurfAllocNotify == NULL )
		return E_FAIL;
	Width = lpAllocInfo->dwWidth;
	Height = lpAllocInfo->dwHeight;
	while( dwWidth < lpAllocInfo->dwWidth )
		dwWidth = dwWidth << 1;
	while( dwHeight < lpAllocInfo->dwHeight )
		dwHeight = dwHeight << 1;
	fTU = (float)(lpAllocInfo->dwWidth) / (float)(dwWidth);
	fTV = (float)(lpAllocInfo->dwHeight) / (float)(dwHeight);
	lpAllocInfo->dwWidth = dwWidth;
	lpAllocInfo->dwHeight = dwHeight;

    // NOTE:
    // we need to make sure that we create textures because
    // surfaces can not be textured onto a primitive.
	lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;
	DeleteSurfaces();
	VX_ASSERT(*lpNumBuffers <= MAX_VIDEO_SURFACES);
	hr = m_lpIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, (IDirect3DSurface9**) m_surfaces);
    // If we couldn't create a texture surface and 
    // the format is not an alpha format,
    // then we probably cannot create a texture.
    // So what we need to do is create a private texture
    // and copy the decoded images onto it.
    if (FAILED(hr) && !(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget))
    {
        DeleteSurfaces();            
		// is surface YUV ?
		if (lpAllocInfo->Format > '0000') 
		{
			D3DDISPLAYMODE dm; 
			hr = device->GetDisplayMode(NULL,  & dm);
			if (FAILED(hr))
				return hr;
			// create the private texture
			hr = device->CreateTexture(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight,
					1, 
					D3DUSAGE_RENDERTARGET,
					dm.Format, 
					D3DPOOL_DEFAULT, 
					(IDirect3DTexture9**) &m_privateTexture, NULL );
			if (FAILED(hr))
				return hr;
		}
		lpAllocInfo->dwFlags &= ~VMR9AllocFlag_TextureSurface;
		lpAllocInfo->dwFlags |= VMR9AllocFlag_OffscreenSurface;
		return m_lpIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, (IDirect3DSurface9**) m_surfaces);
	}
	return S_OK;
}

HRESULT DXVideo::TerminateDevice( 
        /* [in] */ DWORD_PTR dwID)
{
	DeleteSurfaces();
	return S_OK;
}
    
HRESULT DXVideo::GetSurface( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ DWORD SurfaceIndex,
        /* [in] */ DWORD SurfaceFlags,
        /* [out] */ IDirect3DSurface9 **lplpSurface)
{
	if (lplpSurface == NULL)
		return E_POINTER;
	if (SurfaceIndex >= MAX_VIDEO_SURFACES)
		return E_FAIL;

	ObjectLock	lock(this);
	*lplpSurface = m_surfaces[SurfaceIndex];
	(*lplpSurface)->AddRef();
	return S_OK;
}
    
HRESULT DXVideo::AdviseNotify( 
        /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{
	ObjectLock	lock(this);
	HRESULT hr;

	m_lpIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;
	HMONITOR hMonitor = m_D3D->GetAdapterMonitor( D3DADAPTER_DEFAULT );
	hr = m_lpIVMRSurfAllocNotify->SetD3DDevice(m_D3DDev, hMonitor);
	return hr;
}

HRESULT DXVideo::StartPresenting( 
    /* [in] */ DWORD_PTR dwUserID)
{
	return S_OK;
}

HRESULT DXVideo::StopPresenting( 
    /* [in] */ DWORD_PTR dwUserID)
{
	return S_OK;
}

HRESULT DXVideo::PresentImage( 
    /* [in] */ DWORD_PTR dwUserID,
    /* [in] */ VMR9PresentationInfo *lpPresInfo)
{
	HRESULT hr = E_FAIL;
	IDirect3DTexture9* texhandle = NULL;
	Texture*	image = m_Texture;

	if (lpPresInfo->lpSurf == NULL)
		return hr;
	if (image == NULL)
		return hr;
    // if we created a  private texture
    // blt the decoded image onto the texture.	m_Bitmap->Type = Bitmap::TEXHANDLE;
	if (m_privateTexture != NULL)
		texhandle = m_privateTexture;
	// this is the case where we have got the textures allocated by VMR
	// all we need to do is to get them from the surface
	else
		lpPresInfo->lpSurf->GetContainer(__uuidof(IDirect3DTexture9), (LPVOID*) &texhandle);
	m_Texture->SetBitmap(Bitmap::TEXHANDLE, texhandle);
	return hr;
}

// IUnknown
HRESULT DXVideo::QueryInterface( 
        REFIID riid,
        void** ppvObject)
{
	HRESULT hr = E_NOINTERFACE;

	if( ppvObject == NULL)
		hr = E_POINTER;
    else if (riid == IID_IVMRSurfaceAllocator9)
	{
		*ppvObject = static_cast<IVMRSurfaceAllocator9*>( this );
		AddRef();
		hr = S_OK;
	} 
	else if (riid == IID_IVMRImagePresenter9)
	{
		*ppvObject = static_cast<IVMRImagePresenter9*>( this );
		AddRef();
		hr = S_OK;
	} 
    else if(riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>( static_cast<IVMRSurfaceAllocator9*>( this ) );
		AddRef();
		hr = S_OK;    
	}
	return hr;
}

}	// end Vixen