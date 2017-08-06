#include "vixen.h"
#ifdef VIXEN_VIDEO
#include "vxmedia.h"
#include <windowsx.h>
#include <mfplay.h>
#include <mferror.h>

#define		MAX_X 1024
#define		MAX_Y 1024
#define		WM_GRAPHNOTIFY  WM_APP + 1

VX_IMPLEMENT_CLASSID(VXVideoImage, Engine, VX_VideoImage);

#define SAFE_RELEASE(x) { if(x) { x->Release(); x = NULL; }}

class MediaPlayerCallback : public IMFAsyncCallback
{
public:
    static HRESULT CreateInstance(HWND hEvent, MediaPlayerCallback **pCallback);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFAsyncCallback methods
    STDMETHODIMP  GetParameters(DWORD*, DWORD*)		{ return E_NOTIMPL; }
    STDMETHODIMP  Invoke(IMFAsyncResult* pAsyncResult);

    // Playback
    HRESULT       OpenURL(const WCHAR *sURL);
    HRESULT       Play();
    HRESULT       Pause();
    HRESULT       Stop();
    HRESULT       Shutdown();
    HRESULT       HandleEvent(UINT_PTR pUnkPtr);
    PlayerState   GetState() const { return m_state; }

protected:
    // Constructor is private. Use static CreateInstance method to instantiate.
    MediaPlayerCallback(HWND hEvent);

    // Destructor is private. Caller should call Release.
    virtual ~MediaPlayerCallback(); 

    HRESULT Initialize();
    HRESULT CreateSession();
    HRESULT CloseSession();
    HRESULT StartPlayback();

    // Media event handlers
    virtual HRESULT OnTopologyStatus(IMFMediaEvent *pEvent);
    virtual HRESULT OnPresentationEnded(IMFMediaEvent *pEvent);
    virtual HRESULT OnNewPresentation(IMFMediaEvent *pEvent);

    // Override to handle additional session events.
    virtual HRESULT OnSessionEvent(IMFMediaEvent*, MediaEventType)   {  return S_OK; }

protected:
    long                    m_nRefCount;        // Reference count.
    IMFMediaSession*		m_pSession;
    IMFMediaSource*			m_pSource;
    HWND                    m_hwndEvent;        // App window to receive events.
    HANDLE                  m_hCloseEvent;      // Event to wait on while closing.
    PlayerState             m_state;            // Current state of the media session.
};


class MediaCaptureCallback : public IMFSourceReaderCallback
{
public:
    static HRESULT CreateInstance(MediaCaptureCallback **ppCallback, VXVideoImage* owner);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)	{ return S_OK; }
    STDMETHODIMP OnFlush(DWORD)						{ return S_OK; }

    HRESULT		SetDevice(IMFActivate *pActivate);
    HRESULT		CloseDevice();
	HRESULT		GetDefaultStride(IMFMediaType *pType);
protected:
    // Constructor is private. Use static CreateInstance method to create.
    MediaCaptureCallback();

    // Destructor is private. Caller should call Release.
    virtual		~MediaCaptureCallback();

    HRESULT		Initialize();
    void		NotifyError(HRESULT hr);

	VXVideoImage*			m_VideoImage;
    long                    m_nRefCount;        // Reference count.
    CRITICAL_SECTION        m_critsec;
    IMFSourceReader*		m_Reader;
	IMFMediaType			m_MediaType;
    WCHAR*					m_pwszSymbolicLink;
    UINT32                  m_cchSymbolicLink;
};

VXVideoImage::VXVideoImage() : VXVideoImageBase(const TCHAR* filename)
{
	m_Init = false;
	m_ImageSize = 0;
	m_Duration = 0.0f;
	m_MediaControl = NULL;
	m_LiveVideo = false;
	m_Texheight = m_Texwidth = 1;
	if (filename)
		VXVideoImageBase::SetFileName(videofilename);
}


VXVideoImage::~VXVideoImage()
{
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
	TCHAR		fname[VX_MaxPath];
	Texture*	image;

	if (m_Options == 0)				// video from a file
	{
		if (m_CaptureCB)			// stop capturing
			m_CaptureCB->Stop();
		if (m_PlayerCB == NULL)
		{
			if (FAILED(CoInitialize(NULL)))
				return false;
			if (!MediaPlayerCallback::CreateInstance(&m_PlayerCB, this))
				return false;
			m_Init = true;
		}
		if (!m_FileName.IsEmpty() && FAILED(m_PlayerCB->OpenURL(fname)))
			return false;
	}
	else							// live capture
	{
		if (m_PlayerCB != NULL)		// stop video file playback
			m_PlayerCB->Stop();
		if (m_CaptureCB == NULL)
			if (!CreateLiveVideoGrabber())
				return false;
	}
	m_Change = false;
	return true;
}

bool VXVideoImage::InitTexture(int width, int height)
{
	Texture*	image = GetImage();
	char*		data;

	if ((image == NULL) || (m_VideoX == 0) || (m_VideoY == 0))
		return false;
	m_Texwidth = m_Texheight = 1;
	m_VideoX = width;
	m_VideoY = height;
	while(m_Texwidth < m_VideoX)
		m_Texwidth *= 2;
	while(m_Texheight < m_VideoY)
		m_Texheight *= 2;
	m_ImageSize = m_Texwidth * m_Texheight * 4;
	data = (unsigned char*) malloc(m_ImageSize);
	if (data == NULL)
		VX_ERROR(("VXVideoImage: ERROR out of memory for %dx%d video texture", m_Texwidth, m_Texheight), false);
	image->Lock();
	image->SetWidth(m_Texwidth);
	image->SetHeight(m_Texheight);
	image->SetDepth(32);
	image->SetFormat(IMAGE_HasColor | IMAGE_HasAlpha | IMAGE_NoFree);
	image->SetBitmap(Bitmap::IMAGE_DXBitmap, data);
	image->Unlock();
	return true;
}

bool VXVideoImage::UpdateImage(void* srcdata)
{
	Texture*	image = GetImage();
	Bitmap*	bitmap;

	if (image == NULL)
		return false;
	image->Lock();
	bitmap = image->GetBitmap();
	if ((bitmap->ByteSize == m_ImageSize) && (bitmap->Data != NULL))
		MFCopyImage(bitmap->Data, m_Stride, srcdata, m_Stride, m_ImageSize);
	image->Unlock();
	return true;
}

bool VXVideoImage::OnStart()
{
	if (m_LiveVideo && m_CaptureCB)
		m_CaptureCB->Play();
	else if (m_PlayerCB)
		m_PlayerCB->Play();
}
	
bool VXVideoImage::OnStop()
{
	if (m_LiveVideo && m_CaptureCB)
		m_CaptureCB->Pause();
	else if (m_PlayerCB)
		m_PlayerCB->Pause();
	return true;
}
	

bool VXVideoImage::CreateLiveVideoGrabber()
{  
    IMFAttributes*	attributes = NULL;
	IMFActivate*	devices = NULL;
	UINT32			devcount = 0;
    HRESULT			hr = S_OK;

	if (m_CaptureCB != NULL)
		return true;
	if (FAILED(CoInitialize(NULL)))
		return false;
	if (!MediaPlayerCallback::CreateInstance(&m_CaptureCB, this))
		return false;
    // Initialize an attribute store to specify enumeration parameters.
    hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr))
		return false;
    // Ask for source type = video capture devices.
    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
		goto done;
    // Enumerate devices.
    hr = MFEnumDeviceSources(attributes, &devices, &devcount);
    if (FAILED(hr))
		goto done;
    if (devcount > 0)			// pick the first one
	{
        if (SUCCEEDED(m_CaptureCB->SetDevice(devices[0]))
			InitTexture(m_VideoX, m_VideoY);
	}
done:
    SafeRelease(&pAttributes);
    for (DWORD i = 0; i < devcount; i++)
        SafeRelease(&devices[i]);
    CoTaskMemFree(devices);
    if (FAILED(hr))
		VX_ERROR(("VXVideoImage:: ERROR - Cannot create a video capture device %0x", hr), false);
	return true;
}

/*
 * Create a media source reader callback object
 */
HRESULT MediaCaptureCallback::CreateInstance(MediaCaptureCallback** pCallback, VXVideoImage* owner)
{
    if ((pCallback == NULL) || (owner == NULL))
        return E_POINTER;

    MediaCaptureCallback *pPlayer = new (std::nothrow) MediaCaptureCallback(pCallback, owner);

    // The CPlayer constructor sets the ref count to 1.

    if (pPlayer == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = pPlayer->Initialize();
    if (SUCCEEDED(hr))
    {
		pPlayer->m_VideoImage = owner;
        *pCallback = pPlayer;
        (*pCallback)->AddRef();
    }
    SafeRelease(&pPlayer);
    return hr;
}


bool MediaCaptureCallback::SetDevice(IMFActivate *pActivate)
{
    HRESULT hr = S_OK;

    IMFMediaSource  *pSource = NULL;
    IMFAttributes   *pAttributes = NULL;
    IMFMediaType    *pType = NULL;

    EnterCriticalSection(&m_critsec);

    // Create the media source for the device.
    if (SUCCEEDED(hr))
        hr = pActivate->ActivateObject(__uuidof(IMFMediaSource), (void**) &pSource);
    // Get the symbolic link.
    if (SUCCEEDED(hr))
        hr = pActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &m_pwszSymbolicLink, &m_cchSymbolicLink);
	// Create an attribute store to hold initialization settings.
    if (SUCCEEDED(hr))
        hr = MFCreateAttributes(&pAttributes, 2);
    if (SUCCEEDED(hr))
        hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
    // Set the callback pointer.
    if (SUCCEEDED(hr))
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,cthis);
    if (SUCCEEDED(hr))
        hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &m_Reader);

    // Try to find a suitable output type.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; ; i++)
        {
			GUID subtype = { 0 };
			hr = m_Reader->GetNativeMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &pType);
            if (FAILED(hr))
				break;
			if (SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &subtype))
			{
				if ((subtype == MFVideoFormat_RGB32) ||
					(subtype == MFVideoFormat_RGB24))
				{
					hr = GetDefaultStride(pType);
					SafeRelease(&pType);
					break;
				}
			}
            SafeRelease(&pType);
        }
    }
    if (SUCCEEDED(hr))							// Ask for the first sample.
        hr = m_Reader->ReadSample((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
    if (FAILED(hr))
    {
        if (pSource)
        {
            pSource->Shutdown();
			VX_ERROR(("VXVideoImage:: ERROR cannot create video source callback 0x%x", hr), false);
            // NOTE: The source reader shuts down the media source
            // by default, but we might not have gotten that far.
        }
        CloseDevice();
    }
    SafeRelease(&pSource);
    SafeRelease(&pAttributes);
    SafeRelease(&pType);
    LeaveCriticalSection(&m_critsec);
    return true;
}

/*
 * The media foundation calls this function every time there is a new video frame
 */
HRESULT MediaCaptureCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,DWORD dwStreamFlags, LONGLONG timestamp, IMFSample *pSample)
{
    HRESULT hr = S_OK;
    IMFMediaBuffer *pBuffer = NULL;

    if (FAILED(hrStatus))
		VX_ERROR(("VXVideoImage: ERROR cannot read video sample %0x", hr), hr);
	// Get the video frame buffer from the sample.
    EnterCriticalSection(&m_critsec);
	if (pSample)
	{
		hr = pSample->GetBufferByIndex(0, &pBuffer);
		// Lock the video buffer. This method returns a pointer to the first scan
		// line in the image, and the stride in bytes.
		VideoBufferLock buffer(pBuffer);
		hr = buffer.LockBuffer(m_Stride, m_VideoY, &pbScanline0, &lStride);
		if (owner)
			owner->UpdateImage(pbScanline0);
		buffer.UnlockBuffer();
	}
    if (SUCCEEDED(hr))    // Request the next frame.
        hr = m_pReader->ReadSample((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
    SafeRelease(&pBuffer);
    LeaveCriticalSection(&m_critsec);
    if (FAILED(hr))
		VX_ERROR(("VXVideoImage: ERROR cannot read video sample %0x", hr), hr);
    return hr;
}

//-----------------------------------------------------------------------------
// GetDefaultStride
//
// Gets the default stride for a video frame, assuming no extra padding bytes.
//
//-----------------------------------------------------------------------------

HRESULT MediaCaptureCallback::GetDefaultStride(IMFMediaType *pType)
{
    LONG lStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*) &lStride);
    if (FAILED(hr))
    {
        // Attribute not set. Try to calculate the default stride.
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        // Get the subtype and the image size.
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_VideoX, &m_VideoY);
        if (SUCCEEDED(hr))
            hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);

        // Set the attribute for later reference.
        if (SUCCEEDED(hr))
            (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
    }
    if (SUCCEEDED(hr))
        m_Stride = lStride;
    return hr;
}


//-------------------------------------------------------------------
//  CloseDevice
//
//  Releases all resources held by this object.
//-------------------------------------------------------------------

HRESULT MediaCaptureCallback::CloseDevice()
{
    EnterCriticalSection(&m_critsec);
    SafeRelease(&m_Reader);
    CoTaskMemFree(m_pwszSymbolicLink);
    m_pwszSymbolicLink = NULL;
    m_cchSymbolicLink = 0;
    LeaveCriticalSection(&m_critsec);
    return S_OK;
}

bool MediaCaptureCallback::OnNewSample(IMFMediaBuffer* buffer)
{
	Texture*	image;

	if (m_VideoImage == NULL)
		return false;
	image = (Texture*) m_VideoImage->GetTarget();
	if (!image->IsClass(Texture))			// no target image to update?
		return false;
	// Lock the video buffer. This method returns a pointer to the first scan
	// line in the image, and the stride in bytes.
	VideoBufferLock buffer(buffer);
	if (SUCCEEDED(buffer.LockBuffer(m_lDefaultStride, m_height, &pbScanline0, &lStride))
	{
		m_VideoImage->UpdateImage(pbScanline0, lStride);
		return true;
	}
	VX_ERROR(("VXVideoImage: ERROR cannot lock video buffer to capture sample"), false);
}


HRESULT MediaPlayerCallback::CreateInstance(MediaPlayerCallback** pCallback, HWND hEvent, VXVideoImage* owner)
{
    if ((pCallback == NULL) || (owner == NULL))
        return E_POINTER;

    MediaPlayerCallback* callback = new (std::nothrow) MediaPlayerCallback(hEvent, owner);
    if (callback == NULL)
        return E_OUTOFMEMORY;
    HRESULT hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr))
    {
        m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hCloseEvent == NULL)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    if (SUCCEEDED(hr))
	{
		callback->m_VideoImage = owner;
        *pCallback = callback;
	}
    else
        callback->Release();
    return hr;
}

HRESULT MediaPlayerCallback::OpenURL(const WCHAR *sURL)
{
	// 1. Create a new media session.
    // 2. Create the media source.
    // 3. Create the topology.
    // 4. Queue the topology [asynchronous]
    // 5. Start playback [asynchronous - does not happen in this method.]

    IMFTopology *pTopology = NULL;
    IMFPresentationDescriptor* pSourcePD = NULL;

    // Create the media session.
    HRESULT hr = CreateSession();
    if (FAILED(hr))
		VX_ERROR(("VXVideoImage: ERROR cannot create media session"), false);

    // Create the media source.
    hr = CreateMediaSource(sURL, &m_MediaSource);
    if (FAILED(hr))
		goto done;

    // Create the presentation descriptor for the media source.
    hr = m_pSource->CreatePresentationDescriptor(&pSourcePD);
    if (FAILED(hr))
        goto done;

    // Create a partial topology.
    hr = CreatePlaybackTopology(m_pSource, pSourcePD, m_hwndVideo, &pTopology);
    if (FAILED(hr))
        goto done;

    // Set the topology on the media session.
    hr = m_pSession->SetTopology(0, pTopology);
    if (FAILED(hr))
        goto done;

    m_state = OpenPending;
    // If SetTopology succeeds, the media session will queue an 
    // MESessionTopologySet event.
done:
    if (FAILED(hr))
	{
		VX_ERROR(("VXVideoImage: ERROR cannot open %s", sURL), false);
        m_state = Closed;
	}
    SafeRelease(&pSourcePD);
    SafeRelease(&pTopology);
    return hr;
}


HRESULT MediaPlayerCallback::CreateSession()
{
    // Close the old session, if any.
    HRESULT hr = CloseSession();
    if (FAILED(hr))
        goto done;
    VX_ASSERT(m_state == Closed);

    // Create the media session.
    hr = MFCreateMediaSession(NULL, &m_pSession);
    if (FAILED(hr))
        goto done;

    // Start pulling events from the media session
    hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL);
    if (FAILED(hr))
        goto done;
    m_state = Ready;

done:
    return hr;
}


//  Callback for the asynchronous BeginGetEvent method.
HRESULT MediaPlayerCallback::Invoke(IMFAsyncResult *pResult)
{
    MediaEventType meType = MEUnknown;  // Event type
    IMFMediaEvent *pEvent = NULL;
    HRESULT hr = m_pSession->EndGetEvent(pResult, &pEvent);

    if (FAILED(hr))					    // Get the event from the event queue.
        goto done;
    hr = pEvent->GetType(&meType);    // Get the event type. 
    if (FAILED(hr))
        goto done;
	// The session was closed. 
	// The application is waiting on the m_hCloseEvent event handle. 
    if (meType == MESessionClosed)
         SetEvent(m_hCloseEvent);
    else
    {
        // For all other events, get the next event in the queue.
        hr = m_pSession->BeginGetEvent(this, NULL);
        if (FAILED(hr))
            goto done;
    }
    // Check the application state. 
    // If a call to IMFMediaSession::Close is pending, it means the 
    // application is waiting on the m_hCloseEvent event and
    // the application's message loop is blocked. 
    // Otherwise, post a private window message to the application. 
    if (m_state != Closing)
    {
        pEvent->AddRef();        // Leave a reference count on the event.
        PostMessage(m_hwndEvent, WM_APP_PLAYER_EVENT, (WPARAM)pEvent, (LPARAM)meType);
    }
done:
    SafeRelease(&pEvent);
    return S_OK;
}

HRESULT MediaPlayerCallback::HandleEvent(UINT_PTR pEventPtr)
{
    HRESULT hrStatus = S_OK;  
	HRESULT	hr;
    MediaEventType meType = MEUnknown;  
    IMFMediaEvent *pEvent = (IMFMediaEvent*)pEventPtr;

    if (pEvent == NULL)
        return E_POINTER;
    hr = pEvent->GetType(&meType);			// Get the event type.
    if (FAILED(hr))
        goto done;
    // Get the event status. If the operation that triggered the event 
    // did not succeed, the status is a failure code.
    hr = pEvent->GetStatus(&hrStatus);
    if (SUCCEEDED(hr) && FAILED(hrStatus))	// Check if the async operation succeeded.
        hr = hrStatus;
    if (FAILED(hr))
        goto done;
    switch(meType)
    {
		case MESessionTopologyStatus:
        hr = OnTopologyStatus(pEvent);
        break;

		case MEEndOfPresentation:
        hr = OnPresentationEnded(pEvent);
        break;

		case MENewPresentation:
        hr = OnNewPresentation(pEvent);
        break;

		default:
        hr = OnSessionEvent(pEvent, meType);
        break;
    }
done:
    SafeRelease(&pEvent);
    return hr;
}

HRESULT MediaPlayerCallback::OnNewPresentation(IMFMediaEvent *pEvent)
{
    IMFPresentationDescriptor *pPD = NULL;
    IMFTopology *pTopology = NULL;
    HRESULT hr = GetEventObject(pEvent, &pPD);

    if (FAILED(hr))    // Get the presentation descriptor from the event.
        return ht;
    hr = CreatePlaybackTopology(m_pSource, pPD, &pTopology);
    if (FAILED(hr))    // Create a partial topology.
        goto done;
    hr = m_pSession->SetTopology(0, pTopology);
    if (FAILED(hr))    // Set the topology on the media session.
        goto done;
    m_state = OpenPending;

done:
    SafeRelease(&pTopology);
    SafeRelease(&pPD);
    return S_OK;
}


//  Handler for MEEndOfPresentation event.
HRESULT MediaPlayerCallback::OnPresentationEnded(IMFMediaEvent *pEvent)
{
    // The session puts itself into the stopped state automatically.
    m_state = Stopped;
    return S_OK;
}

HRESULT MediaPlayerCallback::OnTopologyStatus(IMFMediaEvent *pEvent)
{
    UINT32 status; 
    HRESULT hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);

    if (SUCCEEDED(hr) && (status == MF_TOPOSTATUS_READY))
        hr = StartPlayback();
    return hr;
}

HRESULT MediaPlayerCallback::StartPlayback()
{
    VX_ASSERT(m_pSession != NULL);

    PROPVARIANT varStart;
    PropVariantInit(&varStart);

    HRESULT hr = m_pSession->Start(&GUID_NULL, &varStart);
    if (SUCCEEDED(hr))
    {
        // Note: Start is an asynchronous operation. However, we
        // can treat our state as being already started. If Start
        // fails later, we'll get an MESessionStarted event with
        // an error code, and we will update our state then.
        m_state = Started;
    }
    PropVariantClear(&varStart);
    return hr;
}

//  Start playback from paused or stopped.
HRESULT MediaPlayerCallback::Play()
{
    if (m_state != Paused && m_state != Stopped)
        return MF_E_INVALIDREQUEST;
    if (m_pSession == NULL || m_pSource == NULL)
        return E_UNEXPECTED;
    return StartPlayback();
}

//  Pause playback.
HRESULT MediaPlayerCallback::Pause()    
{
    if (m_state != Started)
        return MF_E_INVALIDREQUEST;
    if (m_pSession == NULL || m_pSource == NULL)
        return E_UNEXPECTED;
    HRESULT hr = m_pSession->Pause();
    if (SUCCEEDED(hr))
        m_state = Paused;
    return hr;
}

// Stop playback.
HRESULT MediaPlayerCallback::Stop()
{
    if (m_state != Started && m_state != Paused)
        return MF_E_INVALIDREQUEST;
    if (m_pSession == NULL)
        return E_UNEXPECTED;
    HRESULT hr = m_pSession->Stop();
    if (SUCCEEDED(hr))
        m_state = Stopped;
    return hr;
}

#endif