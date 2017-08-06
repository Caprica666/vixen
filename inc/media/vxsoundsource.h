#pragma once
/*!
 * @class VXSoundSource
 *
 * A VXSoundSource represents a sound in the environment, usually played
 * from an audio file such as MP3 or WAV.
 */
class VXSoundSource : public VXMediaFile
{
public:
	VX_DECLARE_CLASS(VXSoundSource);

	VXSoundSource();

	//! Set the sound volume
	virtual void	SetVolume(float vol);

	//! Get the sound volume
	virtual float	GetVolume() const;
};

