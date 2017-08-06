

/****************************************************************************
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or 
 *  nondisclosure agreement with Intel Corporation and may not be copied 
 *  or disclosed except in accordance with the terms of that agreement. 
 *
 *      Copyright (c) 1998-2011 Intel Corporation. All Rights Reserved.
 *
 *
 ****************************************************************************/

#include "vixen.h"
#include "vxmedia.h"

VX_IMPLEMENT_CLASSID(VXMediaSource, VXMediaBase, VX_MediaSource);

/*!
 * @fn VXMediaSource::VXMediaSource()
 * Constructs an VXMediaSource object that plays audio files.
 *
 * @see VXVideoImage Engine 
 */
VXMediaSource::VXMediaSource() : VXMediaBase()
{
	m_IsOpen = false;
}


VXMediaSource::~VXMediaSource()
{
	Close();
}


/*!
 * @fn VXMediaSource::Open(const TCHAR* filename)
 * @param filename	name of media file (.WAV, .MP3, .MPG, etc) to play
 *
 * This function loads the file from disk but does not play it.
 * You can play it by calling Engine::Start
 *
 * @return true if audio file could be opened, false on error
 *
 * @see Engine::Start Engine::Stop
 */
bool VXMediaSource::Open(const TCHAR* filename)
{	
	Core::String	command;
	TCHAR			lengthbuf[128];
	long			err;

	command = TEXT("open \"") + m_FileName + TEXT("\"  alias MediaFile");
	if (Exec(command) &&
		Exec("set MediaFile time format milliseconds") &&
		Exec("set MediaFile seek exactly on") &&
		((err = mciSendString("status MediaFile length", lengthbuf, 128, NULL)) == 0))
	{
		m_Duration = Core::String(lengthbuf).toDouble();
		m_IsOpen = true;
		m_IsPlaying = false;
		return true;
	}
	VX_ERROR(("ERROR: cannot open %s", m_FileName), false);
}


bool VXMediaSource::Play()
{
	Core::String	command;

	if (m_IsPlaying)
		return true;
	command = TEXT("play MediaFile");
	if (GetControl() & ENG_Cycle)
		command += " repeat";
	if (GetState() & ENG_Reverse)
		command += " reverse";
	return Exec(command);
}

bool VXMediaSource::Rewind()
{
	if (Exec(TEXT("seek MediaFile to start")))
		return Play();
	return false;
}

bool VXMediaSource::Pause()
{
	Exec(TEXT("pause MediaFile"));
	return true;
}

void VXMediaSource::SetTimeOfs(float timeofs, int who)
{
	Engine::SetTimeOfs(timeofs, who);
	if (m_IsOpen && (timeofs <= m_Duration))
		Exec(TEXT("seek MediaFile to %dl"), timeofs);
}

void VXMediaSource::SetSpeed(float speed, int who)
{
	Engine::SetSpeed(speed, who);
	if (m_IsOpen)
		Exec(TEXT("set MediaFile speed %dl"), speed);
}

void VXMediaSource::Close()
{
	Exec(TEXT("close MediaFile"));
	m_IsPlaying = false;
	m_IsOpen = false;
}

bool VXMediaSource::Exec(const TCHAR* command, float v) const
{
	long	err;
	int64	val = (int64) v;
	TCHAR	buf[128];

	SPRINTF(buf, command, v);
	err = mciSendString(buf, NULL, 0, NULL);
	if (err == 0)
		return true;
	mciGetErrorString(err, buf, 128);
	VX_ERROR(("ERROR: MediaFile command '%s' %s", command, buf), false);
}

bool VXMediaSource::Exec(const TCHAR* command) const
{
	long	err;
	TCHAR	buf[128];

	err = mciSendString(command, NULL, 0, NULL);
	if (err == 0)
		return true;
	mciGetErrorString(err, buf, 128);
	VX_ERROR(("ERROR: MediaFile command '%s' %s", command, buf), false);
}