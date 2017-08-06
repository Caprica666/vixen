#pragma once

struct CvCapture;
typedef struct _IplImage IplImage;

class VXVideoImage : public VXVideoImageBase
{
// Construction
public:
	VX_DECLARE_CLASS(VXVideoImage);

	VXVideoImage();
	~VXVideoImage();

// Implementation
public:
	//! Returns pixel dimensions of video texture (power of 2)
	VXVec2			GetTexSize() const			{ return VXVec2(m_width, m_height); }

	//! Returns a pointer to the video data for this frame
	IplImage*		GetVideoFrameData() const	{ return m_videoframeimage; }

	virtual	bool	UpdateImage();
	virtual	void	Unload();
	virtual	bool	ChangeVideo();

public:
	CvCapture*		m_stream;
	IplImage*		m_videoframeimage;
	int				m_width;
	int				m_height;
	int				m_size;
};
