#pragma once

#include "mplvideo.h"
#include "mplerror.h"
#include "mpliansi.h"
#include "mplsstd.h"

class VXVideoImage : public VXVideoImageBase
{
// Construction
public:
	VX_DECLARE_CLASS(VXVideoImage);

	VXVideoImage();
	VXVideoImage(const TCHAR* videofilename);
	~VXVideoImage();

// Implementation
public:
	MplVideoStream const*		GetVideoStream() { return m_stream; };
	MplVideoFrame const*		GetVideoFrame() { return m_frame; };
	bool						UpdateImage();
	void						UnLoad();
	bool						ChangeVideo();

		//// Engine overrides
	bool						OnStart();
	bool						OnStop();
	bool						Eval(float);


	// Generated message map functions
public:
	const MplVideoFrame*	m_frame;
	MplVideoStream*	m_stream;
	MplIOSystem*	m_raw_in;
	MplSplitter*	m_splitter;
	int				m_width, m_height, m_size;
};
