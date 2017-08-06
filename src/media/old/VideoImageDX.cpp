#include "vixen.h"
#ifdef VIXEN_VIDEO
#include "vxmedia.h"

#define		MAX_X 1024
#define		MAX_Y 1024
#define		WM_GRAPHNOTIFY  WM_APP + 1

VX_IMPLEMENT_CLASSID(VXVideoImage, Engine, VX_VideoImage);

#define SAFE_RELEASE(x) { if(x) { x->Release(); x = NULL; }}


VXVideoImage::VXVideoImage() : VXVideoImageBase()
{
	m_Init = false;
	m_Startup = false;
    pDD = NULL;    
    pPrimarySurface = NULL;
    pMMStream = NULL;
	pAMStream = NULL;
	m_TextureSurface = NULL;

	pPrimaryVidStream = NULL;    
	pMediaSample = NULL;
    pDDStream = NULL;
    pSample = NULL;
	m_ImageSize = 0;
	m_Duration = 0.0f;
	m_OwnerThread = 0;
	m_DoStop = false;
	g_pMC = NULL;
	g_pME = NULL;
	g_pMS = NULL;
	g_pGraph = NULL;
	g_pCapture = NULL;
	g_psCurrent = Stopped;
	pF = NULL;
	pGrab = NULL;
	m_LiveVideo = false;
	m_Texheight = m_Texwidth = 1;
	cb = NULL;
	cbAlpha = NULL;
	for(int i = 0; i < BUFFERS; i++)
	{
		m_Buffer[i].buffer = NULL;
		m_Buffer[i].count = -2;
	}
	m_CurrentImage = -1;
}


VXVideoImage::VXVideoImage(char* videofilename) : VXVideoImageBase()
{
	VXVideoImageBase::SetFileName(videofilename);
	m_Init = false;
	m_Startup = false;
	pDD = NULL;    
    pPrimarySurface = NULL;
    pMMStream = NULL;
	pAMStream = NULL;
	m_TextureSurface = NULL;

	pPrimaryVidStream = NULL;    
	pMediaSample = NULL;
    pDDStream = NULL;
    pSample = NULL;
	m_ImageSize = 0;
	m_Duration = 0.0f;
	ghApp=0;
	g_dwGraphRegister=0;
	m_OwnerThread = 0;
	m_DoStop = false;

	g_pMC = NULL;
	g_pME = NULL;
	g_pMS = NULL;
	g_pGraph = NULL;
	g_pCapture = NULL;
	g_psCurrent = Stopped;
	pF = NULL;
	pGrab = NULL;
	m_LiveVideo = false;
	m_Texheight = m_Texwidth = 1;
	cb = NULL;
	cbAlpha = NULL;
	for(int i = 0; i < BUFFERS; i++)
	{
		m_Buffer[i].buffer = NULL;
		m_Buffer[i].count = -2;
	}
	m_CurrentImage = -1;
}

VXVideoImage::~VXVideoImage()
{
}

bool VXVideoImage::InitVideo()
{

    if(FAILED(CoInitialize(NULL)))
		return false;

	m_Init = true;
	SetFlags(SharedObj::DOEVENTS);
	m_State = ENG_Idle;
	m_OwnerThread = ::GetCurrentThreadId();
	return true;
	
}

bool VXVideoImage::RenewDShow()
{
  	SAFE_RELEASE(pPrimarySurface);   
	SAFE_RELEASE(pMMStream);    
	SAFE_RELEASE(pMMStream);    
	SAFE_RELEASE(m_TextureSurface);    
	SAFE_RELEASE(pPrimaryVidStream);    
	SAFE_RELEASE(pDDStream);    
	SAFE_RELEASE(pDD);   
	SAFE_RELEASE(pSample);    
	SAFE_RELEASE(pMediaSample); 
    SAFE_RELEASE(g_pCapture);   
	SAFE_RELEASE(g_pGraph);   
	SAFE_RELEASE(g_pMC);   
	SAFE_RELEASE(g_pME);   
	SAFE_RELEASE(g_pMS);
	for(int i = 0; i < BUFFERS; i++)
	{
		if(m_Buffer[i].buffer)
		{
			free(m_Buffer[i].buffer);
			m_Buffer[i].buffer = NULL;
			m_Buffer[i].count = -2;
		}
	}
	m_CurrentImage = -1;
	return true;
}

void VXVideoImage::IncreaseVolume()
{
	DWORD		volume = 0;
	MMRESULT	mr;
	DWORD		inc = 0xffff/16;

	mr = waveOutGetVolume(NULL, &volume);
	if(mr != MMSYSERR_NOERROR )
		return;
	WORD left_channel = (volume & 0xFFFF0000) >> 16;
	WORD right_channel = volume & 0x0000FFFF;
	if((left_channel >= (0xffff - inc)) || (right_channel >= (0xffff - inc)))
		return;
	left_channel += inc;
	right_channel += inc;
	volume = 0;
	volume |= left_channel;
	volume <<=16;
	volume |= right_channel;
	mr = waveOutSetVolume(NULL, volume);
}

void VXVideoImage::DecreaseVolume()
{
	DWORD		volume;
	MMRESULT	mr;
	WORD		inc = 0xffff/16;

	mr = waveOutGetVolume(NULL, &volume);
	if(mr != MMSYSERR_NOERROR )
		return;
	WORD left_channel = (volume & 0xFFFF0000) >> 16;
	WORD right_channel = volume & 0x0000FFFF;
	if((left_channel <= (0x0000 + inc)) || (right_channel <= (0x0000 + inc)))
		return;
	left_channel -= inc;
	right_channel -= inc;
	volume = 0;
	volume |= left_channel;
	volume <<=16;
	volume |= right_channel;
	mr = waveOutSetVolume(NULL, volume);
}

bool VXVideoImage::ChangeVideo()
{
	char		fname[VX_MaxPath];
	Texture*	image = GetImage();

	if(!m_Init)	
		if(!InitVideo())
			return false;
	if (image == NULL)
		return true;

	LPD3DTEXTURE	Texture = (LPD3DTEXTURE)image->GetTexHandle();
	if(!Texture)
		return true;
	if (m_Options == 0)
	{
		if(m_FileName.IsEmpty())
			return true;
		strcpy(fname, m_FileName);
		ChangeVideoFile(fname);
	}
	else
		ChangeLiveVideo();

	if(Texture)
		Texture->Release();

	VXDXScene*	scene = (VXDXScene*) GetMainScene();
	LPD3DDEVICE dev = scene->GetDevice();
	if(!dev)
	{
		return true;	
	}

	if(FAILED(dev->CreateTexture(m_Texwidth,m_Texheight,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture)))
		return false; 
	image->SetBitmap(IMAGE_TexHandle, Texture);
	image->SetChanged(false);
	m_Change = false;
	return true;
}

bool VXVideoImage::ChangeLiveVideo()
{
	m_LiveVideo = true;
	if((m_Options & VIDEO_AlphaStream) == 0)
		RenewDShow();

	if(FAILED(GetInterfaces()))
		return false;

	IBaseFilter *pSrcFilter = NULL;
	Texture* image = GetImage();
	// Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.

	// Attach the filter graph to the capture graph
    if(FAILED(g_pCapture->SetFiltergraph(g_pGraph)))
		return false;
   
    if(FAILED(FindCaptureDevice(&pSrcFilter)))
		return false;
	
	if(FAILED(g_pGraph->AddFilter(pSrcFilter, L"VideoCapture")))
		return false;

	if(!CreateLiveVideoGrabber())
		return false;

	g_Buffer = NULL;
	if(FAILED(g_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pSrcFilter,
				IID_IAMBufferNegotiation, (void **)&g_Buffer)))
		return false;

	ALLOCATOR_PROPERTIES BufferProps;
	BufferProps.cbBuffer = 2;
	if(FAILED(g_Buffer->SuggestAllocatorProperties(&BufferProps)))
		return false;
	g_Buffer->Release();


	if(FAILED(g_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                 pSrcFilter, NULL, pF)))
		return false;


	AM_MEDIA_TYPE MediaType;
	if(FAILED(pGrab->GetConnectedMediaType(&MediaType)))
		return false; 



	m_VideoX = m_VideoY = 0;
	m_Texwidth = m_Texheight = 1;
	// Get a pointer to the video header.
	VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)MediaType.pbFormat;
	m_VideoX = pVideoHeader->bmiHeader.biWidth;
	m_VideoY = pVideoHeader->bmiHeader.biHeight;

	while(m_Texwidth < m_VideoX)
		m_Texwidth *= 2;
	while(m_Texheight < m_VideoY)
		m_Texheight *= 2;

	m_ImageSize = m_Texwidth*m_Texheight*4;
	image->SetWidth(m_Texwidth);
	image->SetHeight(m_Texheight);
	
	if(FAILED(pGrab->SetBufferSamples(FALSE)))
		return false;

	if(FAILED(pGrab->SetOneShot((m_Options & VIDEO_LiveGrab) != 0)))
		return false;

	cb = new CGrabCB;
	cb->SetX(m_VideoX);
	cb->SetY(m_VideoY);
	cb->SetD(4);
	cb->SetPitch(m_Texwidth*4);
	cb->SetOption(m_Options);
	cb->SetImageSize(m_ImageSize);
	if((m_Options & VIDEO_AlphaStream) == 0)
		cb->SetVideoEngine(this);
	else
		cb->SetVideoEngine((VXVideoImage*)this->m_Parent);
	if(FAILED(pGrab->SetCallback(cb, 1)))
		return false;

	if((m_Options & VIDEO_LiveRGBAlpha) == 0)
	{
		if(m_Options & VIDEO_AlphaStream)
		{
			VXVideoImage* ParentVideo = (VXVideoImage*)this->m_Parent;
			ParentVideo->g_pMC->Run();
		}
		g_pMC->Run();
	}

	SetDuration(0);

	return true;
}

HRESULT VXVideoImage::GetInterfaces(void)
{
    HRESULT hr;
	
    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
        IID_IGraphBuilder, (void **) &g_pGraph);
    if (FAILED(hr))
        return hr;

    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
        IID_ICaptureGraphBuilder2, (void **) &g_pCapture);
    if (FAILED(hr))
        return hr;

    // Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &g_pMC);
    if (FAILED(hr))
        return hr;

	// Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaEvent,(LPVOID *) &g_pME);
    if (FAILED(hr))
        return hr;

	// Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaSeeking,(LPVOID *) &g_pMS);
    if (FAILED(hr))
        return hr;

    return hr;
}

bool VXVideoImage::ChangeVideoFile(char* fname)
{
	if(m_LiveVideo)
	{
		m_LiveVideo = false;
//		g_pMC->Stop();
	}
	if(pDD)
		RenewDShow();
	
    if(FAILED(CoCreateInstance(CLSID_AMMultiMediaStream, NULL, CLSCTX_INPROC_SERVER,
        IID_IAMMultiMediaStream, (void **)&pAMStream)))
		return false;
	
    if(FAILED(pAMStream->Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, NULL)))
		return false;
	
	if(FAILED(DirectDrawCreate(NULL, &pDD, NULL)))
		return false;
    if(FAILED(pDD->SetCooperativeLevel(GetDesktopWindow(), DDSCL_NORMAL)))
		return false;
	
    if(FAILED(pAMStream->AddMediaStream(pDD, &MSPID_PrimaryVideo, 0, NULL))) //pDD
		return false;
    if(FAILED(pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL)))
		return false;

   
	WCHAR    wPath[MAX_PATH];        // wide (32-bit) string name
	MultiByteToWideChar(CP_ACP, 0, fname, -1, wPath, sizeof(wPath)/sizeof(wPath[0]));    

	if(FAILED(pAMStream->OpenFile(wPath, 0)))
		return false;    

    pMMStream = pAMStream;

    if(FAILED(pMMStream->GetMediaStream(MSPID_PrimaryVideo, &pPrimaryVidStream)))
		return false;

	int64 pDuration;
	HRESULT hr = pMMStream->GetDuration(&pDuration);
	pDuration /= 10000000;
	m_Duration = (float)pDuration;
	SetDuration(m_Duration);

    if(FAILED(pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&pDDStream)))
		return false;

	DDSURFACEDESC ddsd;
	DDSURFACEDESC ddsd2;
	ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd);
 
   if(FAILED(pDDStream->GetFormat(&ddsd, NULL, NULL, NULL)))
		return false;

	rect.top = rect.left = 0;
    rect.bottom = m_VideoX = ddsd.dwWidth;
    rect.right = m_VideoY = ddsd.dwHeight;

	m_Texwidth = m_Texheight = 1;

	while(m_Texwidth < m_VideoX)
		m_Texwidth *= 2;
	while(m_Texheight < m_VideoY)
		m_Texheight *= 2;

	m_ImageSize = m_Texwidth*m_Texheight*4;

	ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	ddsd2.dwHeight = m_Texheight;
	ddsd2.dwWidth = m_Texwidth;
    if(FAILED(pDD->CreateSurface(&ddsd2, &pPrimarySurface, NULL)))
		return false; 
  
    rect.top = rect.left = 0;
    rect.bottom = ddsd.dwHeight;
    rect.right = ddsd.dwWidth;
    if(FAILED(pDDStream->CreateSample(pPrimarySurface, &rect, 0, &pSample)))
		return false;

	return true;
}

bool VXVideoImage::UpdateImage()
{
	HRESULT hr;
	
	if(m_Startup)
	{
		m_Startup = false;

		if(!m_LiveVideo)
		{
			// Set stream position to zero
			if(FAILED(pMMStream->Seek(0)))
				return false;	
			if(FAILED(pMMStream->SetState(STREAMSTATE_RUN)))
				return false;
		}	
	}
	if(m_Pause)
	{
		if(m_LiveVideo)
			g_pMC->Stop();
		else
		{
			if(FAILED(pMMStream->SetState(STREAMSTATE_STOP)))
				return false;	
			m_PauseStart = GetElapsed();
			SetDuration(0);
		}
		m_bIsPlaying = false;
		m_Pause = 0;
		return true;
	}
	if(m_Resume)
	{
		if(m_LiveVideo)
			g_pMC->Run();
		else
		{
			if(FAILED(pMMStream->SetState(STREAMSTATE_RUN)))
				return false;
			m_PauseEnd = GetElapsed();
			m_Duration += (m_PauseEnd - m_PauseStart);
			SetDuration(m_Duration);
		}
		m_bIsPlaying = true;
		m_Resume = 0;
		return true;
	}

	if(m_LiveVideo)
		return true;
	
	STREAM_STATE pCurrentState;
	if(!pMMStream || FAILED(pMMStream->GetState(&pCurrentState)))
			return false;
	
	if(pCurrentState == STREAMSTATE_STOP)
	{	
		// Set stream position to zero
		if(FAILED(pMMStream->Seek(0)))
			return false;	
		if(FAILED(pMMStream->SetState(STREAMSTATE_RUN)))
			return false;
	}
	
	// Draw video to DirectDraw surface
	// Update the media stream

	if(hr = pSample->Update( 0 , NULL, NULL, 0) != S_OK)
	{
		if(hr == E_ABORT)
			OutputDebugString(" The update aborted.");
	 
		if(hr ==  E_INVALIDARG ) 
			OutputDebugString("One of the parameters is invalid.");
			
		if(hr ==  E_POINTER ) 
			OutputDebugString("One of the parameters is invalid.");
			
		if(hr ==  MS_E_BUSY ) 
			OutputDebugString("this sample already has a pending update.");
		
		if(hr ==  MS_S_ENDOFSTREAM ) 
			OutputDebugString("Reached the end of the stream; the sample wasn't updated.  ");
			
		if(hr ==  MS_S_PENDING )  
			OutputDebugString("The asynchronous update is pending. ");
			
		return false;
	}



	//m_TextureSurface
	Texture*	image = GetImage();
	if (image == NULL)
		return false;

	LPD3DTEXTURE	Texture = (LPD3DTEXTURE)image->GetTexHandle();
	D3DLOCKED_RECT d3dlr;

	if(FAILED(Texture->LockRect(0, &d3dlr, 0, 0)))
		return false;

	BYTE		*pDst = NULL, *pSrc = NULL;
	pDst		= (BYTE*)d3dlr.pBits;
	int stride	= 0;


	DDSURFACEDESC ddsd;
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	// pPrimarySurface is the DirectDraw surface associated
	if(FAILED(pPrimarySurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL)))
		return false;

	pSrc		= (BYTE*)ddsd.lpSurface;
	int d = ddsd.ddpfPixelFormat.dwRGBBitCount/8;
	stride = ddsd.lPitch;
	int x = stride/d;
	int y = ddsd.dwHeight;
	int size = stride*y;
	int dst_start = y - m_VideoY;
	pSrc += m_VideoY*stride;
	if(d3dlr.Pitch == ddsd.lPitch)
	{
		for(int i = 0; i < m_VideoY; i++)
		{
			pSrc -= stride;
			memcpy(pDst,pSrc,m_VideoX*d);
			pDst += stride;
			
		}
	
	}
	if(FAILED(pPrimarySurface->Unlock(NULL)))
		return false;

		
	if(FAILED(Texture->UnlockRect(0)))
		return false;	
	
	Texture->AddDirtyRect(NULL);

	return true;
}

bool VXVideoImage::OnStart()
{
//	m_Startup = true;
	if (m_DoStop)
		Stop();
	VXVideoImageBase::OnStart();
	return true;
}
	
bool VXVideoImage::OnStop()
{
	Texture* image = GetImage();

	if (image == NULL)
		return true;
	SetStartTime(0.0f);
	if (m_OwnerThread != ::GetCurrentThreadId())
	   {
		m_DoStop = true;
		return false;						// don't stop from UI threads
	   }
	m_DoStop = false;
	LPD3DTEXTURE	Texture = (LPD3DTEXTURE)image->GetTexHandle();
    D3DLOCKED_RECT	d3dlr;

	if (Texture == NULL)
		return true;
	if(m_LiveVideo)
	{
		g_pMC->Stop();
		m_LiveVideo = false;
	}
	RenewDShow();

	if(FAILED(Texture->LockRect(0, &d3dlr, 0, 0)))
		return false;	

	BYTE		*pDst = NULL;

	pDst		= (BYTE*)d3dlr.pBits;
	ZeroMemory(pDst, m_ImageSize);
	if(FAILED(Texture->UnlockRect(0)))
		return false;	
	Texture->AddDirtyRect(NULL);
	return true;//VXVideoImageBase::OnStop();
}
	
bool VXVideoImage::Eval(float val)
{
	if (m_DoStop)
		Stop();
	VXVideoImageBase::Eval(val);
	return true;
}

void VXVideoImage::UnLoad()
{
	VXVideoImageBase::UnLoad();
	return;
}

void Msg(char *szFormat, ...)
{
    TCHAR szBuffer[512];

    va_list pArgs;
    va_start(pArgs, szFormat);
    vsprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    MessageBox(NULL, szBuffer, "PlayCap Message", MB_OK | MB_ICONERROR);
}

HRESULT VXVideoImage::FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;
  
	ICreateDevEnum *pSysDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, 
		(void **)&pSysDevEnum);
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	IMoniker *pMoniker;
	ULONG cFetched;
	int count = 0;
	int index = -1;
	while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		IPropertyBag *pPropBag;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);

		VARIANT varName;
		varName.vt = VT_BSTR;
		hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		if (hr != NOERROR) 
			continue;
		WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, m_CaptureDevices[count], 80, NULL, NULL);
		if ((strstr(m_CaptureDevices[count], "SDK Sample") == NULL) &&
			(strstr(m_CaptureDevices[count], m_CaptureSource) != NULL))
		{
		// Bind Moniker to a filter object
			hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
			if (FAILED(hr))
			{
				Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
				return hr;
			}
		// Copy the found filter pointer to the output parameter.
		// Do NOT Release() the reference, since it will still be used
		// by the calling function.
			*ppSrcFilter = pSrc;
			return hr;
		}
		index = count;
		SysFreeString(varName.bstrVal);
		count++;
		pPropBag->Release();
		pMoniker->Release();
	}
	pEnumCat->Release();
	pSysDevEnum->Release();
	Msg(TEXT("Unable to access video capture device!"));   
	return E_FAIL;
}

bool VXVideoImage::CreateLiveVideoGrabber()
{  
	AM_MEDIA_TYPE   mt;

	if(FAILED(CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, 
		IID_IBaseFilter, (LPVOID *)&pF)))
		return false;

	if(FAILED(pF->QueryInterface(IID_ISampleGrabber, (void **)&pGrab)))
		return false;

	if(FAILED(g_pGraph->AddFilter(pF, L"Grabber")))
		return false;

	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB32;
	mt.formattype = FORMAT_VideoInfo; 

	if(FAILED(pGrab->SetMediaType(&mt)))
		return false;

	return true;
}

void VXVideoImage::InitBuffers(int size)
{
	if(size)
	{
		for(int i = 0; i < BUFFERS; i++)
		{
			if(m_Buffer[i].buffer == NULL)
				m_Buffer[i].buffer = (unsigned char*)malloc(size);
		}
	}
}

void* VXVideoImage::GetCurrentBuffer(int kind)
{
	int i;
	Lock();

	for(i = 0; i < BUFFERS; i++)
	{
		switch(kind)
		{
		case TheRGBVideo:
		case TheAlphaVideo:
			if(m_Buffer[i].count != m_CurrentImage)
				goto AssignBuffer;
			break;

		case TheImage:
			if((m_Buffer[i].count != m_CurrentImage) && (ImageCanBeUsed(m_Buffer[i])))
				goto NullBuffer;
			break;
		}
	}
	
NullBuffer:
	Unlock();
	return NULL;

AssignBuffer:
	Unlock();
	return m_Buffer[i].buffer;
}

bool VXVideoImage::ImageCanBeUsed(ImageBuffer &ib)
{
	PSImage* image = GetImage();
	if((image->GetFlags() & CHANGED) == 0) 
	{
		image->SetBitmap(IMAGE_DXBitmap, ib.buffer); 
		ib.count = m_CurrentImage = (m_CurrentImage + 1) % BUFFERS;
		return true;
	}
	return false;
}

#endif