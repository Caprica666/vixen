
/*!
 * @file sound.h
 * @brief Sound classes for DirectX audio.
 *
 * Sound sources for 3D sound can be attached to objects within the
 * scene and travel with them to produce spatialized sound.
 *
 * @author Judith Stanley
 * @ingroup vixen
 *
 * @see speech.h
 */

#pragma once

#include "dsound.h"
#include "eax.h"
#include "media/sounddata.h"

class SoundData;

typedef struct IKsPropertySet *LPKSPROPERTYSET;

typedef enum _SoundProperties
{
	LISTENER_UpOrientation,
	LISTENER_ForwardOrientation,
	LISTENER_Position,
	LISTENER_Velocity,
	LISTENER_Environment,
	LISTENER_DistanceFactor,
	LISTENER_Rolloff,
	LISTENER_Doppler,
	LISTENER_None,
	SOURCE_Position,
	SOURCE_Velocity,
	SOURCE_InsideConeAngle,
	SOURCE_OutsideConeAngle,
	SOURCE_ConeOrientation,
	SOURCE_Frequency,
	SOURCE_Distance,
	SOURCE_MinDistance,
	SOURCE_Panning,
	SOURCE_OutsideConeVolume,
	SOURCE_None,
}SoundProperties;

typedef enum _SoundEnvironments
{
	SOUND_Generic,
	SOUND_Room,
	SOUND_Bathroom,
	SOUND_Livingroom,
	SOUND_Stoneroom,
	SOUND_Hallway,
	SOUND_CarpetedHallway,
	SOUND_Auditorium,
	SOUND_ConcertHall,
	SOUND_Alley,
	SOUND_Forest,
	SOUND_City,
	SOUND_Mountains,
	SOUND_Quarry,
	SOUND_Plain,
	SOUND_Parkinglot,
	SOUND_SewerPipe,
	SOUND_PaddedCell,
	SOUND_Cave,
	SOUND_Hangar,
	SOUND_Arena,
	SOUND_StoneCorridor,
	SOUND_Underwater,
	SOUND_Drugged,
	SOUND_Dizzy,
	SOUND_Psychotic,
}
SoundEnvironments;

#define NumEnvironments 26

typedef enum _SoundFormat
{
	SOUND_FORMAT_WAV,
}
SoundFormat;

enum ListenerOps
{
	SOUNDLISTENER_SetEnvironment = ENG_NextOp,
	SOUNDLISTENER_SetFileName,
	SOUNDLISTENER_NextOp = ENG_NextOp + 20,
};

/*!
 * @class SoundListener
 * @brief Represents listening environment for sound.
 *
 * As the camera in the scene represents the viewer's eyes,
 * the PSSoundListener represents the viewer's ears.
 * There can only be one PSSoundListener for the entire application.
 * The listener gets clues about position and orientation from the camera.
 * It updates information about the environment that the listener is in.
 * This information will be used by the sound card when it mixes other sounds
 * the listener hears when it moves to a different environment
 * within the scene. Because PSSoundListener inherits from PSEngine,
 * the current camera can be set as it's target and then the position,
 * velocity, and orientation parameters will be set implicitly.
 *
 * @see SoundSource Engine
 */
class SoundListener : public Engine
{
public:
	VX_DECLARE_CLASS(SoundListener);
	SoundListener();
	~SoundListener();

	bool	CreateMidi();			//!< Creates DirectSound Midi objects and loads a midi file.
	bool	InitDev();				//!< Sets the default environmental variables.
	bool	ChangeMusic();			//! Loads a new audio file.


	void	Set(int32, void*);			//!< Set listener property.
	void	Get(int32, const void*);	//!< Get listener property.
	void	SetEnvironment(int32);		//!< Sets the type of physical environment around the listener.
	const char*	GetFileName() const;	//!< Return name of audio file.
	void		SetFileName(const char* filename);	//! Sets the audio file name.
	LPDIRECTSOUND8	GetListener() const;			//!< Gets the LPDIRECTSOUND8 object.

	// Overrides
	bool			OnStart();
	bool			OnStop();
	bool			Eval(float);
	bool			Copy(const PSObj*);
	bool			Do(PSMessenger&, int);
	int				Save(PSMessenger&, int) const;
	DebugOut&		Print(DebugOut& = psm_debug) const;

protected:
	//! Creates the DirectSoundListener object.
	bool	CreateListener();

	//! Releases the DirectSoundListener objects.
	void	CleanUp();

	//! Sets the listener cooperative level and buffer format
	HRESULT	Initialize(HWND, DWORD, DWORD, DWORD, DWORD);

	//! Sets the channels, frequency and bit rate for the primary buffer..
	HRESULT			SetPrimaryBufferFormat(DWORD, DWORD, DWORD);

	//! Assigns the 3D listener interface associated with primary buffer.
	HRESULT			Get3DListenerInterface();

private:
	LPDIRECTSOUND				pDirectSoundObj;		// Object to start DirectSound for the midi
	LPDIRECTSOUNDBUFFER			pPrimaryBuf;			// The DirectSound primary buffer
	LPDIRECTSOUND8				m_pDS;					// The IDirectSound8 object for the listener
	LPDIRECTSOUND3DLISTENER8	pListener;				// The 3D interface for the primary buffer
	LPKSPROPERTYSET				p3DEAXproperty;			// the 3D EAX property interface (pReverb)
	DS3DLISTENER				ListenerParameters;		// The settings for the listener
	IDirectMusicPerformance*	pPerf;					// The central object of DirectMusic
	IDirectMusicLoader*			pLoader;				// The music loader
	IDirectMusicSegment*		pSegment;				// The midi segment
	IDirectMusicSegmentState*	pSegState;				// The variable that receives a pointer to the segment state
	bool						m_b3D;					// Boolean tells whether the sound card supports 3D
	bool						m_bEAX;					// Boolean tells whether the sound card supports EAX
	HWND						m_hwnd;					// Window handle
	Vec3						m_UpOrientation;		// Vector describing the up orientation of the listener
	Vec3						m_ForwardOrientation;	// Vector describing the forward orientation of the listener
	Vec3						m_Position;				// Vector describing the position of the listener
	Vec3						m_Velocity;				// Vector describing the velocity of the listener
	int32						m_Environment;			// The current physical environment setting for EAX properties
	float						m_Distance;				// The number of meters in a vector unit
	float						m_Rolloff;				// The amount of attenuation that is applied globaly
	float						m_Doppler;				// The amount of Doppler shift
	uint32						m_envId;				// The current EAX physical environment setting
	Core::String				m_FileName;				// The current midi file name
	bool						m_change;				// True when a midi file has changed
	bool						m_init;					// True when the DirectListener objects are ready
	bool						m_DistanceFactorChanged;// True when the distance factor has been changed
};

inline const char*	SoundListener::GetFileName() const
{	return m_FileName.IsEmpty() ? NULL : (const char*) m_FileName; }

inline LPDIRECTSOUND8	SoundListener::GetListener() const
{	return m_pDS; }

/*!
 * @class PSSoundSource
 * @brief Represents a sound in the environment.
 *
 * There can be multiple PSSoundSources. The sound is associated
 * with an object in the environment and will move or re-orient
 * itself with that object. In 3D, a sound item has a cone of sound
 * (sort of like a spot light beam) so if the listener is outside of
 * the cone then the sound is fainter than inside the cone. Higher
 * frequency sounds would tend to have a narrow cone and low frequency
 * would have a wider cone. The properties of the sound source are mixed
 * with properties of the listener by the audtio hardware to get
 * the end result of a sound in the environment. Because PSSoundSource
 * inherits from PSEngine, each sound can have a target object and so
 * can update its own position and velocity implicitly.
 *
 * You can use the scripting language to attach sound sources to objects
 * at load time or while running.
 *
 * @see PSSoundListener PSScriptor PSSequencer::LoadSound
 */
enum SourceOps
{
	SOUNDSOURCE_SetFileName = ENG_NextOp,
	SOUNDSOURCE_SetListener,
	SOUNDSOURCE_OnPause,
	SOUNDSOURCE_OnResume,
	SOUNDSOURCE_SetInt,
	SOUNDSOURCE_SetFloat,
	SOUNDSOURCE_SetVec,
	SOUNDSOURCE_NextOp = ENG_NextOp + 20,
};

class SoundSource : public Engine
{
public:
	VX_DECLARE_CLASS(SoundSource);
	SoundSource();
	~SoundSource();

	bool		LoadSound();		//!< Transfer audio file into memory.
	void		Pause();			//!< Pause the sound.
	void		Resume();			//!< Resume the sound.
	bool		Change();			//!< Change audio file.
	void		Set(int, long);				//!< Set integer sound property.
	void		Set(int, float);				//!< Set floating sound property.
	void		Set(int, const PSVec3&);	//!< Set vector sound property.
	void		Get(int, const void*);		//!< Get sound property.
	const char*	GetFileName() const;		//!< Get name of audio file.
	bool		SetFormat(WAVEFORMATEX&);	//!< Sets WAVE format.
	void		SetFileName(const char* filename);	//! Set name of audio file to load.
	void		SetListener(const PSSoundListener* listener);	//!< Set sound listener.



	// Overrides
	bool		OnStart();
	bool		OnStop();
	bool		OnReset();
	bool		Eval(float);
	void		SetActive(bool);
	bool		Copy(const PSObj*);
	bool		Do(PSMessenger&, int);
	int			Save(PSMessenger&, int) const;
	DebugOut&		Print(DebugOut& = psm_debug) const;

protected:
	void		CleanUp();			//!< Releases the DirectSound objects.
	bool		LoadWaveFile();		//!< Load .WAV file from disk.
	bool		LoadBuffer();		//!< Loads  sound data into internal buffer.
	HRESULT		RestoreBuffers();	//!< Restores sound buffers when lost.
	bool		InitDev();			//!< Initializes the sound settings.
	bool		CreateSound();		//!< Creates the sound buffer to hold sound data.

private:
	Ref<SoundListener>		m_Listener;				// to get to the primary sound buffer when needed
	LPDIRECTSOUNDBUFFER		pSecondaryBuf;			// instantiate a secondary buffer for each sound source
	LPDIRECTSOUND3DBUFFER	p3DBuf;					// the 3D secondary buffer interface
	LPKSPROPERTYSET			p3DEAXproperty;			// the 3D EAX property interface (pReverb)
	DS3DBUFFER				Sound3DProperties;		// the parameters for setting the 3D sound properties
	bool					m_b3D;					// boolean tells whether the sound card supports 3D
	bool					m_bEAX;					// boolean tells whether the sound card supports EAX
	int32					m_size;					// size of the data buffer
	BYTE*					m_data;					// points to the memory where the data is stored.
	int32					m_format;				// enumerated type for the format
	Core::String			m_FileName;				// name of the wave file
	Vec3					m_Position;				// vector describing the position of the sound
	Vec3					m_Velocity;				// vector describing the velocity of the sound
	Vec3					m_ConeOrientation;		// vector describing the sound cone orientation
	uint32					m_InsideConeAngle;		// the inside sound cone angle
	uint32					m_OutsideConeAngle;		// the outside sound cone angle
	uint32					m_Frequency;			// the frequency of the sound
	int32					m_Format;				// the wave file format
	float					m_Distance;				// the distance the sound should travel
	float					m_MinDistance;			// the minimum distance that the sound volume doesn't change
	long					m_Panning;				// whether left, right or center panning
	long					m_OCVolume;				// the outside cone volume
	SoundData*				m_SoundData;			// holds the wave file data
	int						m_init;					// holds flags for when the DirectSound objects have been created
};


inline const char*	SoundSource::GetFileName() const
{	return m_FileName.IsEmpty() ? NULL : (const char*) m_FileName; }

