
#include "vixen.h"
#include "vxmedia.h"

namespace Vixen {

VX_IMPLEMENT_CLASS(MediaBase, Engine);

static const TCHAR* opnames[] = {
		TEXT("SetFileName"),
		TEXT("SetCaptureSource"),
		TEXT("SetOptions"),
		TEXT("SetFPS"),
		TEXT("SetPlaying"),
};

const TCHAR** MediaBase::DoNames = opnames;


/*!
 * @fn MediaBase::MediaBase()
 * Constructs an MediaBase object that plays video or audio files.
 *
 * @see MediaSource Engine
 */
MediaBase::MediaBase()
:	Engine(), m_VideoSize(0, 0), m_TexSize(0, 0)
{
	m_IsPlaying = false;
	m_VideoFrameTime = 0;
	m_NextTime = 0;
	m_NeedsInit = true;
	m_NeedsLoad = false;
}


MediaBase::~MediaBase()
{
	Close();
}


/*!
 * @fn MediaBase::SetFileName(const TCHAR* filename)
 * @brief Set the name of the media file to play.
 * @param filename	string with file name
 *
 * This function loads the file from disk but does not play it.
 * You can play it by calling Engine::Start
 *
 * @return true if audio file could be opened, false on error
 *
 * @see Engine::Start Engine::Stop
 */
void MediaBase::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MediaSource, MEDIA_SetFileName) << this << filename;
	VX_STREAM_END()

	m_FileName = filename;
	m_NeedsLoad = true;
	Load();
}

/*!
 * @fn void SetOptions(int opts)
 * @param opts	video capture / playback options. If MediaSource::LIVE is set, live capture
 *				mode is enabled and the video source is a camera. Otherwise it is a file.
 * Designates the video source to use.
 *
 * @see SetFileName SetCaptureSource
 */
void MediaBase::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MediaSource, MEDIA_SetOptions) << this << int32(opts);
	VX_STREAM_END()

	m_Options = opts;
}

/*!
 * @fn void MediaBase::SetFPS(int fps)
 * @param fps	desired frame rate for video playback
 *
 * If the desired frame rate is less than the actual playback rate,
 * the engine will skip video frames to match the desired rate.
 *
 * @see SetFileName SetCaptureSource
 */
void MediaBase::SetFPS(int fps) 
{ 
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MediaSource, MEDIA_SetFPS) << this << int32(fps);
	VX_STREAM_END()

	if (fps > 0)
		m_VideoFrameTime = 1.0f / float(fps); 
};

int MediaBase::GetFPS() const
{
	if (m_VideoFrameTime > 0)
		return (int) (1.0f / m_VideoFrameTime);
	else
		return 0;
}
	
/*!
 * @fn bool MediaBase::ComputeTexSize(int& width, int& height)
 * @param width	on input it has the width of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 * @param height on input it has the height of the video frame in pixels
 *				on output it gets the smallest power of two that fits
 *
 * @returns true if successfully computed, false if input dimension is zero
 */
bool MediaBase::ComputeTexSize(int& width, int& height)
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

/*!
 * @fn bool MediaBase::InitImage(Texture* image, int width, int height, int depth)
 * @brief  Creates a memory data area to hold the pixels of a texture that is being dynamically updated. 
 * @param image		image to create a data area for
 * @param width		minimum pixel width of texture
 * @param height	minimum pixel height of texture
 * @param depth		bits per pixel
 *
 * The typical usage case is for video display.
 * The texture data area created will be guaranteed to accomodate the
 * input pixel dimensions but it will be a power of 2 to make it
 * suitable for graphics display.
 *
 * If this texture is used within the scene, it will be downloaded into
 * the graphics card each time it is updated. This data area will remain
 * until the bitmap associated with the texture is released. This function
 * does not initialize the data area created - it is random junk.
 *
 * @see MediaBase::CopyImage Texture::SetBitmap Texture::SetFormat
 */
bool MediaBase::InitImage(Texture* image, int width, int height, int depth)
{
	int		texwidth, texheight;
	char*	videobuffer;
	int		format = image->GetFormat() | Bitmap::HASCOLOR | Bitmap::NOFREE_DATA;

	switch (depth)
	{
		case 24: break;
		case 32: format |= Bitmap::HASALPHA; break;
		default:
		VX_ERROR(("MediaSource: ERROR bad depth %d, must be 24 or 32", depth), false);
	}
	if (!ComputeTexSize(texwidth = width, texheight = height))
		VX_ERROR(("MediaSource: ERROR bad video dimensions %d X %d video frame", width, height), false);
	if (!image || !image->IsClass(VX_Image))
		return false;
	videobuffer = (char*) malloc(texwidth * texheight * depth / sizeof(int32));
	if (videobuffer == NULL)
		VX_ERROR(("MediaSource: ERROR cannot allocate memory for %d X %d video frame", texwidth, texheight), false);
	image->MakeLock();
	ObjectLock	lock(image);
	image->SetWidth(texwidth);
	image->SetHeight(texheight);
	image->SetDepth(depth);
	image->SetFormat(format);
	image->SetBitmap(Bitmap::DXBITMAP, videobuffer);
	return true;
}

/*!
 * @fn bool MediaBase::CopyImage(Texture* image, const char* srcpixels, int srcwidth, int srcheight, int srcstride)
 * @brief Copies source pixels into the given texture.
 * @param image		image to update
 * @param srcpixels	-> pixels of input bitmap
 * @param srcwidth	pixel width of input bitmap
 * @param srcheight	pixel height of input bitmap
 * @param srcstride	number of bytes per horizontal line of input bitmap
 *
 * his function assumes the destination texture already has a memory data area associated
 * with it that can accomodate the input pixel dimensions.
 * If this texture is used within the scene, the new pixels will be downloaded into
 * the graphics card during the next frame.
 *
 * @see MediaBase::InitImage Texture::UpdateData Texture::SetBitmap
 */
bool MediaBase::CopyImage(Texture* image, const char* srcpixels, int srcwidth, int srcheight, int srcstride)
{
	ObjectLock	locki(image);
	Bitmap*		bitmap = image->GetBitmap();

	if (bitmap == NULL)
		return false;

	ObjectLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	int			w = image->GetWidth();
	int			h = image->GetHeight();

	if (videobuffer == NULL)
		return false;
	VX_ASSERT(w >= srcwidth);
	VX_ASSERT(h >= srcheight);
	for (int y = 0; y < srcheight; y++)
	{
		const char* src = srcpixels + y * srcstride;
		char*		dst = videobuffer + (srcheight - y - 1) * w * 3;
		memcpy(dst, src, srcwidth * 3);
	}
	bitmap->Type = Bitmap::DXBITMAP;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
	return true;
}

bool MediaBase::Load()
{
	if (!m_NeedsLoad)
		return false;
	if (m_NeedsInit && !Init(NULL))
		VX_ERROR(("ERROR: cannot make media player for %s", m_FileName), false);
	m_NeedsInit = false;
	if (!m_FileName.IsEmpty())
	{
		Close();
		if (!Open(m_FileName))
			VX_ERROR(("ERROR: cannot open %s", m_FileName), false);
	}
	m_NeedsLoad = false;
	if (IsActive())
		Start(0);
	return true;
}

bool MediaBase::Eval(float t)
{
	GroupIter<Engine> iter(this, Group::CHILDREN);
	SoundPlayer*	sound = NULL;
	Engine*			e;
	Texture*		image = (Texture*) GetTarget();

	if (!m_IsPlaying)			// are we paused?
		return true;			// don't decode anything
	while (e = iter.Next())		// find a sound player child engine
		if (e->IsClass(VX_SoundPlayer))
		{
			sound = (SoundPlayer*) e;
			break;
		}
	if (image && !image->IsClass(VX_Image))
		image = NULL;
	if (m_VideoFrameTime == 0)
	{
		ProcessFrame(image, sound);
		return true;
	}
	while (m_Elapsed >= m_NextTime)		// current time > last video frame time?
	{
		ProcessFrame(image, sound);		// grab another frame and update target image
		m_NextTime += m_VideoFrameTime;	// compute time of next video frame
	}
	return true;
}

bool MediaBase::OnStart()
{
	if (m_NeedsLoad)
		Load();
	if (Play())
		m_IsPlaying = true;
	return Engine::OnStart();
}

bool MediaBase::OnStop()
{
	if (m_IsPlaying && Pause())
		m_IsPlaying = false;
	return Engine::OnStop();
}

bool MediaBase::OnReset()
{
	if (GetState() & Engine::REVERSE)
	{
		m_IsPlaying = false;
		Play();
	}
	else
		Rewind();
	return Engine::OnReset();
}

bool MediaBase::Copy(const SharedObj* obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(obj);
	if (!Engine::Copy(obj))
		return false;
	const MediaBase*	src = (const MediaBase*) obj;

	m_FileName = src->m_FileName;
	m_VideoSize = src->m_VideoSize;
	m_TexSize = src->m_TexSize;
	m_CaptureSource = src->m_CaptureSource;
	m_VideoFrameTime = src->m_VideoFrameTime; 
	m_IsPlaying = false;
	m_NeedsInit = true;
	return true;
}

bool MediaBase::Do(Messenger& s, int op)
{
	
	TCHAR			fname[VX_MaxPath];
	const TCHAR*	imagefile = NULL;
	Core::String	file;
	int32			n;

	switch (op)
	{	
/*
 * When loading a texture from a stream, we prepend
 * the directory of the file that was opened
 */
		case MEDIA_SetFileName:
		s >> fname;
		file = fname;
		imagefile = s.GetPath(file);
		if (imagefile)
			file = imagefile;
		SetFileName(file);
		break;

		case MEDIA_SetCaptureSource:
		s >> fname;
		file = fname;
		imagefile = s.GetPath(file);
		if (imagefile)
			file = imagefile;
		SetCaptureSource(file);
		break;

		case MEDIA_SetFPS:
		s >> n;
		SetFPS(n);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << MediaBase::DoNames[op - MEDIA_SetFileName]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Image override for SharedObj::Save
 *
 ****/
int MediaBase::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h <= 0)
		return h;

	const TCHAR* name = GetFileName();
	int	fps = GetFPS();

	if (name && *name)
		s << OP(VX_MediaSource, MEDIA_SetFileName) << h << name;
	name = m_CaptureSource;
	if (name && *name)
		s << OP(VX_MediaSource, MEDIA_SetCaptureSource) << h << name;
	if (fps > 0)
		s << OP(VX_MediaSource, MEDIA_SetFPS) << h << fps;
	return h;
}

DebugOut& MediaBase::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_FileName.IsEmpty())
		endl(dbg << "	<FileName>" << m_FileName << "</FileName>");
	if (!m_CaptureSource.IsEmpty())
		endl(dbg << "	<CaptureSource>" << m_CaptureSource << "</CaptureSource>");
	if (m_VideoFrameTime > 0)
		endl(dbg << "	<FrameTime>" << m_VideoFrameTime << "</FrameTime>");
	endl(dbg << "	<VideoSize>" << m_VideoSize.x <<"," << m_VideoSize.y << "</VideoSize>");
	return Engine::Print(dbg, opts & PRINT_Trailer);
}
}	// end Vixen