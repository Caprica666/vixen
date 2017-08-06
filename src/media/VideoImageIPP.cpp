// VXVideoImageIPP.cpp : IPP based implementation of VXVideoImage
//
#include "vixen.h"
#include "vxmedia.h"
#include "avsync.h"
#include "unified_video_render.h"

#define		MAX_X 1024
#define		MAX_Y 1024

/*
 * A UMC video renderer that provides access to the current video frame
 */
class VixenVideoRender : public UMC::UnifiedVideoRender
{
public:
	VixenVideoRender() : UMC::UnifiedVideoRender() { }
	void		SetImage(VXImage* image)	{ m_Image = image; }
	VXImage*	GetImage() const			{ return m_Image; }

	virtual UMC::Status RenderFrame(void);

protected:
	void	UpdateImage(void* videobuffer);

	Ref<VXImage>	m_Image;
};

static const TCHAR* opnames[] = {
		TEXT("SetFileName"),
		TEXT("SetCaptureSource"),
		TEXT("SetOptions"),
		TEXT("SetFPS"),
		TEXT("SetPlaying"),
};

const TCHAR** VXVideoImage::DoNames = opnames;


#define SAFE_UNLOAD(x) { if(x) {delete x; x = NULL; }}

/////////////////////////////////////////////////////////////////////////////
// VXVideoImageBase

#define SKIP_MAX	100

VXVideoImage::VXVideoImage() : VXMediaFile()
{
		m_TexWidth = m_TexHeight = 0;
		m_fps = m_Realfps = 30;
		m_FrameCount = 0;
		m_PrevTime = VGetTime();
		m_PrevVal = 0;
		m_SkipCount = m_SkipAmount = 0;
		m_Options = 0;
		m_VideoRender = NULL;
		m_Change = false;
}

VXVideoImage::~VXVideoImage()
{
//	if (m_VideoRender)
//		delete m_VideoRender;
}

/*!
 * @fn void SetFileName(const TCHAR* filename)
 * @param filename	name of file containing video to play
 * Designates the file which contains video to play.
 * If not in live capture mode, this file will be the video source.
 *
 * @see VXVideoImage::SetOptions
 */
void VXVideoImage::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetFileName) << this << filename;
	VX_STREAM_END()

	VXMediaFile::SetFileName(filename);
	m_Change = true;
}

bool VXVideoImage::Load()
{
	VXImage*	image = GetImage();
	char*		videobuffer;

	if (!VXMediaFile::Load())
		return false;
	if (!ComputeTexSize(m_TexWidth = m_VideoX, m_TexHeight = m_VideoY))
		VX_ERROR(("VXVideoImage: ERROR bad video dimensions %d X %d video frame", m_VideoX, m_VideoY), false);
	if (!image)
		return false;
	videobuffer = (char*) malloc(m_TexWidth * m_TexWidth * 3);
	if (videobuffer == NULL)
		VX_ERROR(("VXVideoImage: ERROR cannot allocate memory for %d X %d video frame", m_TexWidth, m_TexWidth), false);
	VXObjLock	lock(image);
	image->SetWidth(m_TexWidth);
	image->SetHeight(m_TexWidth);
	image->SetDepth(24);
	image->SetFormat(IMAGE_HasColor | IMAGE_NoFree);
	image->SetBitmap(VXBitmap::IMAGE_DXBitmap, videobuffer);
	if (IsActive())
		Start(0);
	return true;
}

/*
 * Add our own UMC video renderer so we can capture the video frames in UpdateImage
 */
bool VXVideoImage::Init(void* p)
{
	AVSync::CommonCtl*	params = (AVSync::CommonCtl*) p;
	AVSync::Stat		stat;
	UMC::Status			umcRes = UMC::UMC_OK;
	VixenVideoRender*	vvr = m_VideoRender;
	VXImage*			image = (VXImage*) GetTarget();

	if (vvr == NULL)
		m_VideoRender = vvr = new VixenVideoRender;
	VX_ASSERT(params->pExternalInfo);
	params->pExternalInfo->m_pVideoRender = vvr;
	vvr->SetImage(image);
	return VXMediaFile::Init(p);
}

/*!
 * @fn void SetCaptureSource(const TCHAR* source)
 * @param source	string indicating which video capture source to use
 * Designates the video capture source to use when in live capture mode.
 *
 * @see VXVideoImage::SetOptions
 */
void VXVideoImage::SetCaptureSource(const TCHAR* source)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetCaptureSource) << this << source;
	VX_STREAM_END()

	if (source == NULL)
	{
		m_CaptureSource.Empty();
		return;
	}
	else
		m_CaptureSource = source;
}

/*!
 * @fn void SetOptions(int opts)
 * @param opts	video capture / playback options. If VIDEO_Live is set, live capture
 *				mode is enabled and the video source is a camera. Otherwise it is a file.
 * Designates the video source to use.
 *
 * @see VXVideoImage::SeFileName VXVideoImage::SetCaptureSource
 */
void VXVideoImage::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetOptions) << this << int32(opts);
	VX_STREAM_END()

	m_Options = opts;
}

/*!
 * @fn void SetFPS(int fps)
 * @param fps	desired frame rate for video playback
 *
 * If the desired frame rate is less than the actual playback rate,
 * the engine will skip video frames to match the desired rate.
 *
 * @see VXVideoImage::SeFileName VXVideoImage::SetCaptureSource
 */
void VXVideoImage::SetFPS(int fps) 
{ 
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetFPS) << this << int32(fps);
	VX_STREAM_END()
	m_fps = fps; 
};

/*!
 * @fn bool ComputeTexSize(int& width, int& height)
 * @param width	on input it has the width of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 * @param height on input it has the height of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 *
 * @returns true if successfully computed, false if input dimension is zero
 */
bool VXVideoImage::ComputeTexSize(int& width, int& height)
{
	int	videox = width;
	int videoy = height;

	if ((videox == 0) || (videoy == 0))
		return false;
	height = 64, width = 64;
	while (width < videox)
		width *= 2;
	while (height < videoy)
		height *= 2;
	return true;
}
	
bool VXVideoImage::Do(VXMessenger& s, int op)
{
	
	char			fname[VX_MaxPath];
	const TCHAR*	imagefile = NULL;
	int32			n;

	switch (op)
	{	
/*
 * When loading a texture from a stream, we prepend
 * the directory of the file that was opened
 */
		case VIDEO_SetFileName:
		s >> fname;
		imagefile = s.GetPath(fname);
		if (imagefile)
			strcpy(fname, imagefile);
		SetFileName(fname);
		break;

		case VIDEO_SetCaptureSource:
		s >> fname;
		imagefile = s.GetPath(fname);
		if (imagefile)
			strcpy(fname, imagefile);
		SetCaptureSource(fname);
		break;

		case VIDEO_SetFPS:
		s >> n;
		SetFPS(n);
		break;

		default:
		return VXEngine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << VXVideoImage::DoNames[op - VIDEO_SetFileName]
					   << " " << this);
#endif
	return true;
}

void VXVideoImage::SetImage(VXImage* image)
{
	SetTarget(image);
	if (m_VideoRender)
		m_VideoRender->SetImage(image);
}

/****
 *
 * class Image override for VXObj::Save
 *
 ****/
int VXVideoImage::Save(VXMessenger& s, int opts) const
{
	int32 h = VXMediaFile::Save(s, opts);
	if (h <= 0)
		return h;

	const TCHAR* name = GetFileName();

	if (name && *name)
		s << OP(VX_VideoImage, VIDEO_SetFileName) << h << name;
	name = m_CaptureSource;
	if (name && *name)
		s << OP(VX_VideoImage, VIDEO_SetCaptureSource) << h << name;
	if (m_fps)
		s << OP(VX_VideoImage, VIDEO_SetFPS) << h << m_fps;
	return h;
}

VDebug& VXVideoImage::Print(VDebug& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return VXObj::Print(dbg, opts);
	VXMediaFile::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_FileName.IsEmpty())
		endl(dbg << "	<FileName>" << m_FileName << "</FileName>");
	if (!m_CaptureSource.IsEmpty())
		endl(dbg << "	<CaptureSource>" << m_CaptureSource << "</CaptureSource>");
	if (m_fps > 0)
		endl(dbg << "	<FPS>" << m_fps << "</FPS>");
	endl(dbg << "	<VideoSize>" << m_VideoX <<"," << m_VideoY << "</VideoX>");
	return VXMediaFile::Print(dbg, opts & PRINT_Trailer);
}

bool VXVideoImage::Copy(const VXObj* obj)
{
	VXObjLock dlock(this);
	VXObjLock slock(obj);
	if (!VXMediaFile::Copy(obj))
		return false;
	const VXVideoImage*	src = (const VXVideoImage*) obj;

	m_Depth = src->m_Depth;
	m_CaptureSource = src->m_CaptureSource;
	m_fps = src->m_fps; 
	m_Realfps = src->m_Realfps;
	m_PrevTime = src->m_PrevTime;
	m_FrameCount = src->m_FrameCount;
	m_PrevVal = src->m_PrevVal;
	m_SkipCount = src->m_SkipCount; 
	m_SkipAmount = src->m_SkipAmount; 
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
        Ipp32s  iReadI  = m_iReadIndex; // wee need read index after 'unlock'
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

void VixenVideoRender::UpdateImage(void* videoframe)
{
	VXImage*	image = GetImage();

	if ((image == NULL) ||				// do we have a target?
		(image->GetBitmap() == NULL))
		return;
	/*
	 * Copy the pixel data from the video frame to the texture.
	 * The texture is a power of 2 - it is probably not the same size as the video frame.
	 * The bitmap associated with the texture contains the actual pixels.
	 * We have already allocated the data buffer for these pixels in ChangeVideo
	 */
	VXObjLock	locki(image);
	VXBitmap*	bitmap = image->GetBitmap();
	VXObjLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	bool		is_bottom_left = false;
	int			wvideo = m_disp.right - m_disp.left;
	int			hvideo = m_disp.bottom - m_disp.top;
	int			wimage = image->GetWidth();
	int			himage = image->GetHeight();

	if (videobuffer == NULL)
		return;
	if ((wvideo == 0) || (hvideo == 0))
		return;
	for (int y = 0; y < hvideo; y++)
	{
		const char* src = (char*) videoframe + y * wvideo;
		char *dst = is_bottom_left ?
					(videobuffer + y * wimage * 3) :
					(videobuffer + (wvideo - y - 1) * wimage * 3);
		memcpy(dst, src, wvideo * 3);
	}
	bitmap->Type = VXBitmap::IMAGE_DXBitmap;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
}