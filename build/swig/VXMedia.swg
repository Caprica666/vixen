/*
 * SWIG interface file for animation engines
 */
%ignore FMOD::Sound;
%ignore FMOD::Channel;
%ignore FMOD::System;
%ignore CvCapture;
%ignore AVStream;

namespace Vixen
{
	%ignore Core::Buffer;
	%ignore SoundPlayer::NewBuffer;
	%ignore SoundPlayer::GetBuffer;
	%ignore SoundPlayer::GetBufferSize;
	%ignore SoundPlayer::LoadBuffer;
	%apply	SharedObj*	{ SoundPlayer* };
	%apply	SharedObj*	{ MediaBase* };
}

ATTR_DEFINE(Vixen::SoundPlayer, float, Volume);
ATTR_DEFINE(Vixen::SoundPlayer, const char*, FileName);
ATTR_DEFINE(Vixen::MediaBase, int, Options);
ATTR_DEFINE(Vixen::MediaBase, int, FPS);
ATTR_DEFINE(Vixen::MediaBase, const char*, FileName);
ATTR_DEFINE(Vixen::MediaBase, const char*, CaptureSource);
ATTR_READONLY(Vixen::MediaBase, Vec2, TexSize);
ATTR_READONLY(Vixen::MediaBase, Vec2, VideoSize);
%{
#include "vxmedia.h"
%}

%include "media/VXSoundPlayFM.h"
%include "media/VXMediaBase.h"
%include "media/VXMediaCV.h"

namespace Vixen
{
}



