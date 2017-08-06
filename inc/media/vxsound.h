#pragma once
/*!
 *
 * @class VXSoundListenerBase
 *
 * As the camera in the scene represents the viewer's eyes, 
 * the VXSoundListenerBase represents the viewer's ears. 
 * There can only be one VXSoundListenerBase for the entire application. 
 * The VXSoundListenerBase gets clues 
 * about position and orientation from the camera. It updates information 
 * about the environment that the listener is in. This information will 
 * be used by the sound card when it mixes other sound items that the 
 * listener hears in each room or when it moves to a different environment 
 * within the scene. Because VXSoundListenerBase inherits from VXEngine, 
 * the current camera can be set as it's target and then the position, 
 * velocity, and orientation parameters will be set implicitly.
 *
 * @code
 *	OnStart 	    starts up the listener
 *	OnStop		    stops the listener
 *	Suspend         suspends the listener
 *	Eval            checks on listener properties and commits source settings
 * @endcode
 *
 * @see VXSoundSourceBase VXEngine
 *
 */
#include "vixen.h"
#include "vxmedia.h"

typedef enum _SoundType
{
	SOUND_Ambient,
	SOUND_Object,
	SOUND_Background,
}SoundType;

typedef enum _SoundTrigger
{
	SOUND_Global,
	SOUND_Proximity,
	SOUND_Trigger,
}SoundTrigger;

typedef enum _SoundCycle
{
	SOUND_LoopForever,
	SOUND_PlayOnce,
	SOUND_PlayNTimes,
}SoundCycle;
	
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
	SOURCE_Volume,
	SOURCE_Frequency,
	SOURCE_Distance,
	SOURCE_Panning,
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
}SoundEnvironments;

#define NumEnvironments 26


class VXSoundListenerBase : public VXEngine
{
public:
/*
 *	Constructors and Destructors
 */
	VXSoundListenerBase();

	// Set and Get Listener Properties
	virtual void Set(int, void*);
	virtual void Get(int, const void*);

	//// Engine overrides
	virtual bool OnStart();
	virtual bool OnStop();
	virtual bool Eval(float);

protected: 
	VXVec3		m_UpOrientation, m_ForwardOrientation, m_Position, m_Velocity;
	int32		m_Environment;
	int32		m_Format;
	float		m_Distance, m_Rolloff, m_Doppler;
	uint32		m_envId;
};


/*!
 * @class VXSoundSource
 *
 * A VXSoundSource represents a sound in the environment. 
 * There can be multiple VXSoundSources. The sound is associated 
 * with an object in the environment and will move or re-orient 
 * itself with that object. In 3D, a sound item has a cone of sound 
 * (sort of like a spot light beam) so if the listener is outside of 
 * the cone then the sound is fainter than inside the cone. Higher 
 * frequency sounds would tend to have a narrow cone and low frequency 
 * would have a wider cone. The properties of the VXSoundSource are mixed 
 * with properties of the VXSoundListener by the sound hardware to get 
 * the end result of a sound in the environment. Because VXSoundSource 
 * inherits from VXEngine, each sound can have a target object and so 
 * can update its own position and velocity implicitly. The EAX propery 
 * settings which are low level will probably not be used much but the 
 * functionality will be available when necessary to use.
 *
 * @code
 *	OnStart 	    starts up the listener
 *	OnStop		    stops the listener
 *	Suspend         suspends the listener
 *	Eval            checks on listener properties and commits source settings
 * @endcode
 */
class VXSoundSourceBase : public VXEngine
{
public:
/*
 *	Constructors and Destructors
 */
	VXSoundSourceBase();
	~VXSoundSourceBase();

	// Set and Get Listener Properties
	virtual void Set(int, void*);
	virtual void Get(int, const void*);

	// Sound file loading
	virtual bool LoadSound(const TCHAR* filename);

	//// Engine overrides
	virtual bool OnStart();
	virtual bool OnStop();
	virtual bool Eval(float);

protected: 
	VXVec3f			m_Position, m_Velocity, m_ConeOrientation;
	uint32			m_InsideConeAngle, m_OutsideConeAngle, m_Frequency;
	int32			m_Format;
	float			m_Distance;
	long			m_Volume, m_Panning;
	VXSoundData*	m_SoundData;
};

