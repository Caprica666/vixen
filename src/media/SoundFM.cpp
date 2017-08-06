#include "vixen.h"
#include "vxmedia.h"
#include "fmod.hpp"

#define SOUND_BUFFER_SIZE sizeof(int16)*(16384)

namespace Vixen {

VX_IMPLEMENT_CLASSID(SoundPlayer, Engine, VX_SoundPlayer);

FMOD::System*	SoundPlayer::s_System = NULL;

/*!
 * @fn SoundPlayer::SoundPlayer()
 * Constructs an SoundPlayerBase object that describes the position, 
 * orientation, speed, and environment of a sound in the 3D world.
 *
 * @see VXSoundListener Engine 
 */

SoundPlayer::SoundPlayer()
:	Engine(),
	m_SoundBuffers(SOUND_BUFFER_SIZE, 1)
{
	if (s_System == NULL)
		Init();
}

void SoundPlayer::Shutdown()
{
 	if (s_System)	// clean up sound system
	{
		s_System->close();
		s_System->release();
		s_System = NULL;
	}
}

SoundPlayer::~SoundPlayer()
{
	Close();		// clean up sound buffer
}

void SoundPlayer::SetFileName(const TCHAR* filename)
{
	m_FileName = filename;
	SetChanged(true);
}
Core::Buffer*	SoundPlayer::NewBuffer()
{
	return m_SoundBuffers.NewBuffer();
}

bool SoundPlayer::LoadBuffer(Core::Buffer* newbuf)
{
	if ((newbuf == NULL) || (newbuf->NumBytes == 0))
		return false;
	m_SoundBuffers.Submit(newbuf);
	return true;
}

bool SoundPlayer::LoadFile()
{
    FMOD_RESULT result;
	unsigned int dur;
	char	fbuf[VX_MaxPath];

	if (m_FileName.IsEmpty())
		return false;
	SetChanged(false);
	if (!Init())
		return false;
	Close();
	m_FileName.AsMultiByte(fbuf, VX_MaxPath);
    result = s_System->createStream(fbuf, FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D | FMOD_CREATESTREAM, NULL, &m_Stream);
	if (result != FMOD_OK)
		VX_ERROR(("SoundPlayer: ERROR cannot load %s err=%d", m_FileName, result), false);
	result = m_Stream->getLength(&dur, FMOD_TIMEUNIT_MS);
	if (result == FMOD_OK)
		m_Duration = float(dur) / 1000.0f;	// duration in seconds
	else
		m_Duration = 0;
	return true;
}

void SoundPlayer::Close()
{
	if (m_Player)
		m_Player->stop();
	if (m_Stream)		// free sound stream
	{
		m_Stream->release();
		m_Stream = NULL;
	}
}

bool SoundPlayer::Init()
{
    FMOD_RESULT result;
	unsigned int fmod_version = 0;

	if (s_System != NULL)
		return true;
	result = FMOD::System_Create(&s_System);
    if (result != FMOD_OK)
		VX_ERROR(("SoundPlayer: ERROR cannot create FMOD player"), false);
    result = s_System->getVersion(&fmod_version);
    if ((result != FMOD_OK) ||
		(fmod_version < FMOD_VERSION))
         VX_ERROR(("SoundPlayer: ERROR  %d wrong version of FMOD, need %d", fmod_version, FMOD_VERSION), false);
    result = s_System->init(2, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK)
		VX_ERROR(("SoundPlayer: ERROR cannot initialize FMOD player"), false);
	return true;
}

bool SoundPlayer::Eval(float t)
{
	bool	paused = false;
	bool	playing = false;
	Core::Buffer*	newbuf = NULL;
	
	if (m_Player == NULL)
		return true;
	m_Player->getPaused(&paused);
	m_Player->isPlaying(&playing);
	if (paused || playing)
		return true;
	if (newbuf = m_SoundBuffers.Process())
		PlayBuffer(newbuf);
	return true;
}

bool SoundPlayer::OnStart()
{
	if ((m_Stream == NULL) || HasChanged())
	{
		if (!LoadFile())
			return Engine::OnStart();
	}
	Play();
	return Engine::OnStart();
}

bool SoundPlayer::OnStop()
{
	Pause();
	return Engine::OnStop();
}

bool SoundPlayer::OnReset()
{
	Rewind();
	Play();
	return Engine::OnReset();
}

float SoundPlayer::GetVolume() const
{
	float v;

	if (m_Player && (m_Player->getVolume(&v) == FMOD_OK))
		return v;
	else
		return 0.0f;
}
void SoundPlayer::SetVolume(float v)
{
	if (m_Player != NULL)
		m_Player->setVolume(v);
}

bool SoundPlayer::Rewind()
{
	if (m_Player != NULL)
	{
		m_Player->setPosition(0, FMOD_TIMEUNIT_MS);
		return true;
	}
	return false;
}

bool SoundPlayer::Pause()
{
	if (m_Player == NULL)
		return false;
    m_Player->setPaused(true);
	return true;
}

bool SoundPlayer::PlayBuffer(Core::Buffer* newbuf)
{
    FMOD_RESULT result;
	unsigned int dur;
	FMOD_CREATESOUNDEXINFO ext;
	FMOD::Sound* stream;

	memset(&ext, 0, sizeof(ext));
	ext.length = newbuf->NumBytes;
	ext.format = FMOD_SOUND_FORMAT_PCM16;
    result = s_System->createStream(newbuf->GetData(), FMOD_HARDWARE | FMOD_OPENMEMORY | FMOD_LOOP_NORMAL | FMOD_2D | FMOD_CREATESTREAM, &ext, &stream);
	if (result != FMOD_OK)
		VX_ERROR(("SoundPlayer: ERROR cannot load sound buffer err=%d", result), false);
	result = stream->getLength(&dur, FMOD_TIMEUNIT_MS);
	if (result == FMOD_OK)
		m_Duration = float(dur) / 1000.0f;	// duration in seconds
	if (m_Stream)							// free current sound stream
		m_Stream->release();
	m_Stream = stream;
	return Play();
}

bool SoundPlayer::Play()
{
	FMOD_RESULT result;

	if (s_System == NULL)
		return false;
	if (m_Player == NULL)
	{
		result = s_System->playSound(FMOD_CHANNEL_FREE, m_Stream, false, &m_Player);
		if (result != FMOD_OK)
			VX_ERROR(("SoundPlayer: ERROR cannot play FMOD sound err=%d", result), false);
		return true;
	}
    m_Player->setPaused(false);
	return true;
}

}	// end Vixen
