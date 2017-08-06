// VXVideoImageBase.cpp : implementation file
//
#include "vixen.h"
#ifdef VIXEN_VIDEO

#include "vxmedia.h"

#define		MAX_X 1024
#define		MAX_Y 1024

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

VXVideoImageBase::VXVideoImageBase() : VXEngine()
{
	m_IsPlaying = false;
	m_VideoFrameTime = 1.0f / 30.0f;	// default to 30 frames / sec
	m_NextTime = 0;
	m_Change = false;
	m_Options = 0;
}


VXVideoImageBase::~VXVideoImageBase()
{
}

/*!
 * @fn void SetFileName(const TCHAR* filename)
 * @param filename	name of file containing video to play
 * Designates the file which contains video to play.
 * If not in live capture mode, this file will be the video source.
 *
 * @see VXVideoImage::SetOptions
 */
void VXVideoImageBase::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetFileName) << this << filename;
	VX_STREAM_END()

	if (filename == NULL)
	{
		m_FileName.Empty();
		return;
	}
	else
		m_FileName = filename;
	m_Change = true;
	m_IsPlaying = true;
	ChangeVideo();
	if (IsActive())
		Start(0);
}

/*!
 * @fn void SetCaptureSource(const TCHAR* source)
 * @param source	string indicating which video capture source to use
 * Designates the video capture source to use when in live capture mode.
 *
 * @see VXVideoImage::SetOptions
 */
void VXVideoImageBase::SetCaptureSource(const TCHAR* source)
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
	m_Change = true;
	m_IsPlaying = true;
	if (IsActive())
		Start(0);
}

/*!
 * @fn void SetOptions(int opts)
 * @param opts	video capture / playback options. If VIDEO_Live is set, live capture
 *				mode is enabled and the video source is a camera. Otherwise it is a file.
 * Designates the video source to use.
 *
 * @see VXVideoImage::SeFileName VXVideoImage::SetCaptureSource
 */
void VXVideoImageBase::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetOptions) << this << int32(opts);
	VX_STREAM_END()

	m_Options = opts;
	m_Change = true;
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
void VXVideoImageBase::SetFPS(int fps) 
{ 
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VideoImage, VIDEO_SetFPS) << this << int32(fps);
	VX_STREAM_END()

	if (fps > 0)
		m_VideoFrameTime = 1.0f / float(fps); 
};

int VXVideoImageBase::GetFPS() const
{
	if (m_VideoFrameTime > 0)
		return (int) (1.0f / m_VideoFrameTime);
	else
		return 0;
}
	
/*!
 * @fn bool ComputeTexSize(int& width, int& height)
 * @param width	on input it has the width of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 * @param height on input it has the height of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 *
 * @returns true if successfully computed, false if input dimension is zero
 */
bool VXVideoImageBase::ComputeTexSize(int& width, int& height)
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

bool VXVideoImageBase::ChangeVideo()
{
	m_Change = false;
	m_NextTime = 0;
	return true;
}

bool VXVideoImageBase::OnStart()
{
	m_IsPlaying = true;
	m_NextTime = m_VideoFrameTime;
	return true;
}

bool VXVideoImageBase::OnStop()
{
	m_IsPlaying = false;
	return true;
}
	
bool VXVideoImageBase::Eval(float val)
{
	if (!m_IsPlaying)					// no video playing?
		return true;
	if (m_Change)						// video source changed?
	{
		ChangeVideo();					// start new video source
		return true;
	}
	if (m_VideoFrameTime == 0)
	{
		UpdateImage();
		return true;
	}
	while (m_Elapsed >= m_NextTime)		// current time > last video frame time?
	{
		UpdateImage();					// grab another frame and update target image
		m_NextTime += m_VideoFrameTime;	// compute time of next video frame
	}
	return true;
}


	
bool VXVideoImageBase::Do(VXMessenger& s, int op)
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
					   << VXVideoImageBase::DoNames[op - VIDEO_SetFileName]
					   << " " << this);
#endif
	return true;
}

void VXVideoImageBase::SetImage(VXImage* image)
{
	SetTarget(image);
}

/****
 *
 * class Image override for VXObj::Save
 *
 ****/
int VXVideoImageBase::Save(VXMessenger& s, int opts) const
{
	int32 h = VXEngine::Save(s, opts);
	if (h <= 0)
		return h;

	const TCHAR* name = GetFileName();
	int	fps = GetFPS();

	if (name && *name)
		s << OP(VX_VideoImage, VIDEO_SetFileName) << h << name;
	name = m_CaptureSource;
	if (name && *name)
		s << OP(VX_VideoImage, VIDEO_SetCaptureSource) << h << name;
	if (fps > 0)
		s << OP(VX_VideoImage, VIDEO_SetFPS) << h << fps;
	return h;
}

VDebug& VXVideoImageBase::Print(VDebug& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return VXObj::Print(dbg, opts);
	VXEngine::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_FileName.IsEmpty())
		endl(dbg << "	<FileName>" << m_FileName << "</FileName>");
	if (!m_CaptureSource.IsEmpty())
		endl(dbg << "	<CaptureSource>" << m_CaptureSource << "</CaptureSource>");
	if (m_VideoFrameTime > 0)
		endl(dbg << "	<FrameTime>" << m_VideoFrameTime << "</FrameTime>");
	endl(dbg << "	<VideoSize>" << m_VideoX <<"," << m_VideoY << "</VideoX>");
	return VXEngine::Print(dbg, opts & PRINT_Trailer);
}

bool VXVideoImageBase::Copy(const VXObj* obj)
{
	VXObjLock dlock(this);
	VXObjLock slock(obj);
	if (!VXEngine::Copy(obj))
		return false;
	const VXVideoImageBase*	src = (const VXVideoImageBase*) obj;
	SetFileName(src->GetFileName());

	m_VideoX = src->m_VideoX;
	m_VideoY = src->m_VideoY;
	m_Depth = src->m_Depth;
	m_FileName = src->m_FileName;
	m_CaptureSource = src->m_CaptureSource;
	m_IsPlaying = src->m_IsPlaying;
	m_VideoFrameTime = src->m_VideoFrameTime; 
	m_Change = src->m_Change;
	return true;
}


#endif