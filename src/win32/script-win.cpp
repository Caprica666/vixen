#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

#ifndef VX_SOUND

void Sequencer::LoadSound(const TCHAR* filename, SharedObj* targobj, int animopts) {}

#else

#include "vxmedia.h"

void Sequencer::LoadSound(const TCHAR* filename, SharedObj* targobj, int animopts)
{
	TCHAR			engbuf[VX_MaxPath];
	TCHAR			animname[VX_MaxPath];
	SoundPlayer*	sound;
	VXSoundListener* listener;
	Scene*		scene;
	Animator*	anim;

	Core::Stream::ParseDirectory(filename, animname);
	if (*animname)
		STRCPY(engbuf, animname);
	if (targobj && targobj->GetName())
	{
		STRCAT(engbuf, TEXT("."));
		STRCPY(engbuf, STRCHR(targobj->GetName(), TEXT('.')) + 1);
		strcat(engbuf, TEXT(".sound"));
	}
	STRCAT(animname, TEXT("."));
	STRCAT(animname, engbuf);
	STRCPY(engbuf, animname);
	anim = MakeAnim(animname, targobj);
	anim->MakeName(engbuf);
	anim->SetOptions(animopts);
	listener = (VXSoundListener*) (Engine*) ScriptParser::FindEngine("scene.camera.listener");
	if (listener == NULL)
	{
		listener = new VXSoundListener;
		anim->Append(listener);
	}
	if ((listener->GetTarget() == NULL) &&
		(scene = GetMainScene()))
		listener->SetTarget(scene->GetCamera());
	sound = (SoundPlayer*) (Engine*) ScriptParser::FindEngine(engbuf);
	if (sound == NULL)
	{
		sound = new SoundPlayer;
		anim->Append(sound);
	}						
	if (sound->IsClass(VX_SoundPlayer))
	{
		sound->SetListener(listener);
		sound->LoadSound();
		sound->SetName(engbuf);
		sound->SetTarget(targobj);
		sound->SetFileName(filename);
	}
	anim->SetRootEngine(sound);
	if ((animopts & Animator::AUTO_PLAY) == 0)
		anim->Stop();
}

#endif	// VX_SOUND

#ifndef VX_VIDEO

void Sequencer::LoadVideo(const TCHAR* filename, SharedObj* targobj, int animopts) {}

#else

#include "vxmedia.h"

void Sequencer::LoadVideo(const TCHAR* filename, SharedObj* targobj, int animopts)
{
	char			engbuf[VX_MaxPath];
	char			animname[VX_MaxPath];
	VXVideoImage*	video;
	Animator*		anim;

	Core::Stream::ParseDirectory(filename, animname);
	if (*animname)
		STRCPY(engbuf, animname);
	if (targobj && targobj->GetName())
	{
		STRCAT(engbuf, ".");
		STRCPY(engbuf, strchr(targobj->GetName(), '.') + 1);
		STRCAT(engbuf, ".video");
	}
	STRCAT(animname, ".");
	STRCAT(animname, engbuf);
	STRCPY(engbuf, animname);
	anim = MakeAnim(animname, targobj);
	anim->MakeName(engbuf);
	anim->SetOptions(animopts);
	video = (VXVideoImage*) (Engine*) ScriptParser::FindEngine(engbuf);
	if (video == NULL)
	{
		video = new VXVideoImage;
		anim->Append(video);
	}		
	if (video->IsClass(VX_VideoImage))
	{
		video->SetName(engbuf);
		video->SetTarget(targobj);
		video->SetFileName(filename);
	}
	anim->SetRootEngine(video);
	if ((animopts & Animator::AUTO_PLAY) == 0)
		anim->Stop();
}


#endif //VX_VIDEO
}	// end Vixen