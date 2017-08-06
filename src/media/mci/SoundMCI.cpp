

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

VX_IMPLEMENT_CLASSID(VXSoundPlayer, Engine);

/*!
 * @fn VXSoundPlayer::VXSoundPlayer()
 * Constructs an VXSoundPlayer object that plays audio.
 *
 * @see VXMediaSource Engine 
 */
VXSoundPlayer::VXSoundPlayer() : VXMediaSource()
{
}

bool VXSoundPlayer::OnStart()
{
	if (m_FileName.IsEmpty())
	{
		VXMediaSource* parent = (VXMediaSource*) Parent();
		if (parent->IsClass(VX_MediaSource))
			m_FileName = parent->GetFileName();
	}
	return VXMediaBase::OnStart();
}

void VXSoundPlayer::SetVolume(float vol)
{
	long	err;
	TCHAR	buf[128];
	int64	v = (int64) vol;
	VXMediaSource*	media = (VXMediaSource*) Parent();

	if (m_FileName.IsEmpty() || (vol < 0))
		return;
	SPRINTF(buf, TEXT("setaudio \"%s\" volume to %dl"), m_FileName, v);
	if ((err = mciSendString(buf, NULL, 0, NULL)) != 0)
		VX_ERROR_RETURN(("ERROR: SoundPlayer cannot set volume err= %d", err));
}
