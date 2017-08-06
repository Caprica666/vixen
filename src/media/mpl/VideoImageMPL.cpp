
#include "vixen.h"
#ifdef VIXEN_VIDEO

#include "vxmedia.h"

VX_IMPLEMENT_CLASSID(VXVideoImage, Engine, VX_VideoImage);

VXVideoImage::VXVideoImage() : VXVideoImageBase()
{
	m_width = m_height = 1;
	m_size = 0;
}


VXVideoImage::VXVideoImage(const TCHAR* videofilename) : VXVideoImageBase()
{
	m_width = m_height = 1;
	m_size = 0;
	VXVideoImageBase::SetFileName(videofilename);
}

VXVideoImage::~VXVideoImage()
{
}

bool VXVideoImage::ChangeVideo()
{
	Texture*	image = GetImage();
	MplResult	rc;

	m_frame = NULL;
	m_splitter = NULL;
	m_stream = NULL;
	m_raw_in = NULL;
	m_Change = false;
	if (MPL_FAILED(rc = mplCreateIOSystemAnsiFile(&m_raw_in, m_FileName)) ||
		MPL_FAILED(rc = mplCreateSplitter(&m_splitter, m_raw_in)) ||
		MPL_FAILED(rc = mplCreateVideoStream(&m_stream, m_splitter )))
		VX_ERROR(("VXVideoImage: ERROR cannot open %s", m_FileName), false);
	if (MPL_FAILED(mplSetVideoSP_Threads(m_stream, 1)) ||
		MPL_FAILED(mplSetVideoFrameNumber(m_stream, 1)) ||
		MPL_FAILED(mplGetVideoFrame(m_stream, &m_frame )))
		VX_ERROR(("VXVideoImage: ERROR cannot play video stream"), false);
	m_VideoX = m_frame->component[0].width;
	m_VideoY = m_frame->component[0].height;

	m_height = 1, m_width = 1;
	while(m_width < m_VideoX)
		m_width *= 2;
	while(m_height < m_VideoY)
		m_height *= 2;
	image->SetWidth(m_width / 2);
	image->SetHeight(m_height / 2);
	image->SetDepth(32);
	if(m_data != NULL)
	{
		free(m_data);
		m_data = NULL;
	}
	m_data = (unsigned char*)malloc(m_width*m_height*4);
	image->SetFormat(IMAGE_HasColor | IMAGE_HasAlpha | IMAGE_NoFree);
	image->SetBitmap(Bitmap::IMAGE_DXBitmap, m_data);
	return true;
}

bool VXVideoImage::UpdateImage()
{
	MplResult		rc;
	if (m_Pause)
	{
		m_bIsPlaying = false;
		m_Pause = 0;
		return true;
	}
	if (m_Resume)
	{
		m_bIsPlaying = true;
		m_Resume = 0;
		return true;
	}
	if(!m_bIsPlaying)
		return true;

	Texture* image = GetImage();
	if (image == NULL)
		return true;
	rc = mplGetNextVideoFrame(m_stream, &m_frame ); // decode the next m_frame in m_stream
	if ( MPL_FAILED( rc ) )
	{
		if (rc == MPL_ERR_FRAME_LIST_END)
		{
			long currentFrame = 2;
			// loop when hitting the end
			if (MPL_FAILED(mplSetVideoFrameNumber(m_stream, currentFrame)) ||
				MPL_FAILED(mplGetVideoFrame(m_stream, &m_frame )))
				return false;
		}
		else rc = mplGetNextVideoFrame(m_stream, &m_frame );
	}
	mplConvertVideoFrame(m_frame, 32, m_data, m_width, m_VideoY,
			MPL_SHOW_Y|MPL_SHOW_U|MPL_SHOW_V);

	image->SetBitmap(Bitmap::IMAGE_DXBitmap, m_data);
	return true;
}


bool VXVideoImage::OnStart()
{
	return VXVideoImageBase::OnStart();
}

bool VXVideoImage::OnStop()
{
	Texture* image = GetImage();
	m_bIsPlaying = false;
	memset(m_data,0,m_size);
	if (image)
		image->SetBitmap(Bitmap::IMAGE_DXBitmap, m_data);
	return VXVideoImageBase::OnStop();
}

bool VXVideoImage::Eval(float val)
{
	return VXVideoImageBase::Eval(val);
}


void VXVideoImage::UnLoad()
{
	m_frame = NULL;
	m_splitter = NULL;
	m_stream = NULL;
	m_raw_in = NULL;
}

#endif VIXEN_VIDEO