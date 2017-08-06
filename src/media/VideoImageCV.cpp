#include "vixen.h"

#ifdef VIXEN_VIDEO
#include "highgui.h"
#include "vxmedia.h"


VX_IMPLEMENT_CLASSID(VXVideoImage, VXEngine, VX_VideoImage);

VXVideoImage::VXVideoImage() : VXVideoImageBase()
{
	m_width = m_height = 1;
	m_size = 0;
	m_videoframeimage = NULL;
}


VXVideoImage::~VXVideoImage()
{
	Unload();
}

bool VXVideoImage::ChangeVideo()
{
	VXImage*	image = GetImage();
	char*		videobuffer;
	float		videofps;

	m_Change = false;
	Unload();
	if (m_Options & VIDEO_Live)
		m_stream = cvCreateCameraCapture(0);
	else
		m_stream = cvCreateFileCapture(m_FileName);
	if (!m_stream)
		VX_ERROR(("VXVideoImage: ERROR cannot capture from %s", m_FileName), false);
	m_VideoX = (int) cvGetCaptureProperty(m_stream, CV_CAP_PROP_FRAME_WIDTH);
	m_VideoY = (int) cvGetCaptureProperty(m_stream, CV_CAP_PROP_FRAME_HEIGHT);
	videofps = (float) (int) cvGetCaptureProperty(m_stream, CV_CAP_PROP_FPS);
	if (videofps > 0)
		m_VideoFrameTime = (1.0f / videofps);
	cvSetCaptureProperty(m_stream, CV_CAP_PROP_CONVERT_RGB, 1.0);
	if (!ComputeTexSize(m_width = m_VideoX, m_height = m_VideoY))
		VX_ERROR(("VXVideoImage: ERROR bad video dimensions %d X %d video frame", m_VideoX, m_VideoY), false);
	if (!image)
		return false;
	videobuffer = (char*) malloc(m_width * m_height * 3);
	if (videobuffer == NULL)
		VX_ERROR(("VXVideoImage: ERROR cannot allocate memory for %d X %d video frame", m_width, m_height), false);
	VXObjLock	lock(image);
	image->SetWidth(m_width);
	image->SetHeight(m_height);
	image->SetDepth(24);
	image->SetFormat(IMAGE_HasColor | IMAGE_NoFree);
	image->SetBitmap(VXBitmap::IMAGE_DXBitmap, videobuffer);
	return VXVideoImageBase::ChangeVideo();
}

bool VXVideoImage::UpdateImage()
{
	IplImage*	cvimg;
	VXImage*	image = GetImage();

	/*
	 * If a video is playing or capturing and we have a target image,
	 * grab the next frame and stuff it into the image
	 */
	if (!m_stream || !m_IsPlaying)		// is it playing?
		return true;
	m_videoframeimage = cvimg = cvQueryFrame(m_stream);
	if (cvimg == NULL)					// are we at the end?
	{
		if (GetControl() & ENG_Cycle)	// user wants replay?
		{
			cvSetCaptureProperty(m_stream, CV_CAP_PROP_POS_AVI_RATIO, 0.0);
			return true;				// start at the beginning again
		}
	}
	if (image == NULL)				// do we have a target?
		return false;
	/*
	 * Copy the pixel data from the video frame to the texture.
	 * The texture is a power of 2 - it is probably not the same size as the video frame.
	 * The bitmap associated with the texture contains the actual pixels.
	 * We have already allocated the data buffer for these pixels in ChangeVideo
	 */
	VXObjLock	locki(image);
	VXBitmap*	bitmap = image->GetBitmap();

	if (bitmap == NULL)
		return false;

	VXObjLock	lockb(bitmap);
	char*		videobuffer = (char*) bitmap->Data;
	bool		is_bottom_left = cvimg->origin;

	if (videobuffer == NULL)
		return false;
	VX_ASSERT(image->GetWidth() == m_width);
	VX_ASSERT(image->GetHeight() == m_height);
	VX_ASSERT(cvimg->width == m_VideoX);
	VX_ASSERT(cvimg->height == m_VideoY);

	for (int y = 0; y < m_VideoY; y++)
	{
		const char* src = cvimg->imageData + y * cvimg->widthStep;
		char *dst = is_bottom_left ?
					(videobuffer + y * m_width * 3) :
					(videobuffer + (m_VideoY - y - 1) * m_width * 3);
		memcpy(dst, src, m_VideoX * 3);
	}
	bitmap->Type = VXBitmap::IMAGE_DXBitmap;
	bitmap->Depth = 24;
	bitmap->SetChanged(true);
	return true;
}

void VXVideoImage::Unload()
{
	VXVideoImageBase::Unload();
	m_videoframeimage = NULL;
	if (m_stream)
	{
		cvReleaseCapture(&m_stream);
		m_stream = NULL;
	}
}

#endif VIXEN_VIDEO