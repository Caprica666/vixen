

/****************************************************************************
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or 
 *  nondisclosure agreement with Intel Corporation and may not be copied 
 *  or disclosed except in accordance with the terms of that agreement. 
 *
 *      Copyright (c) 1998 Intel Corporation. All Rights Reserved.
 *
 *
 ****************************************************************************/

#include "multimedia\soundbase.h"

VXSoundListenerBase::VXSoundListenerBase()  : Engine()
{
}


/*!
 * @fn void VXSoundListenerBase::Set(int attr, void* val)
 *
 * Set the given sound listener property. The listener property
 * controls how the environment and source sounds are mixed to produce the total sound.
 * If you do not explicitly set an listener property, it will be
 * in the default state. You can /e unset an property again by setting
 * it's value to LISTENER_None. The current set of listener properties are:
 * @code
 *	LISTENER_UpOrientation		VXVec3f for the upward orientation
 *	LISTENER_ForwardOrientation	VXVec3f for the forward orientation
 *	LISTENER_Position			VXVec3f for the position orientation
 *	LISTENER_Velocity			VXVec3f for the velocity vector
 *	LISTENER_Format				format of the sound file
 *	LISTENER_Environment		string describing sound environment "generic", "bathroom", ...
 *	LISTENER_None				reset defaults
 * @endcode
 *
 * @see VXSoundListener::Get
 */
void VXSoundListenerBase::Set(int attr, void* val)
{
	switch(attr)
	{
		case LISTENER_UpOrientation:
		m_UpOrientation = *((Vec3*)val);
		break;

		case LISTENER_ForwardOrientation:
		m_ForwardOrientation = *((Vec3*)val);
		break;

		case LISTENER_Position:
		m_Position = *((Vec3*)val);
		break;

		case LISTENER_Velocity:
		m_Velocity = *((Vec3*)val);
		break;

		case LISTENER_Environment:
		m_Environment = (const char*) val;
		break;

		case LISTENER_DistanceFactor:
		m_Distance = *((float*)val);
		break;

		case LISTENER_Rolloff:
		m_Rolloff = *((float*)val);
		break;

		case LISTENER_Doppler:
		m_Doppler = *((float*)val);
		break;

		case LISTENER_None:
		//TODO
		break;
	}
}

/*++++
 *
 * Name: SoundListener_Get
 * void Get(int32 attr, const void* val)
 *
 * Description:
 * Gets the value of the given listener property.
 *
 * returns: int32
 *	Value of the attribute or APP_None if attribute is not set
 *
 * Also: Appearance_Set
 *
 ----*/
void VXSoundListenerBase::Get(int attr, const void* val) 
{
	switch(attr)
	{
		case LISTENER_UpOrientation:
		*((Vec3*)val) = m_UpOrientation;
		break;

		case LISTENER_ForwardOrientation:
		*((Vec3*)val) = m_ForwardOrientation;
		break;

		case LISTENER_Position:
		*((Vec3*)val) = m_Position;
		break;

		case LISTENER_Velocity:
		*((Vec3*)val) = m_Velocity;
		break;

		case LISTENER_Environment:
		strcpy((char*) val, (const char*) m_Environment);
		break;

		case LISTENER_DistanceFactor:
		*((float*)val) = m_Distance;
		break;

		case LISTENER_Rolloff:
		*((float*)val) = m_Rolloff;
		break;

		case LISTENER_Doppler:
		*((float*)val) = m_Doppler;
		break;

		case LISTENER_None:
		//TODO
		break;
	}
}

bool VXSoundListenerBase::OnStart()
{
	return Engine::OnStart();
}

bool VXSoundListenerBase::OnStop()
{
	return Engine::OnStop();
}

bool VXSoundListenerBase::Eval(float val)
{
	Camera* target = (Camera*)GetTarget();
	
	if (!target || !target->IsClass(VX_Camera))
		return true;
	m_Position = target->GetCenter();
	m_UpOrientation = target->GetDirection();

	const Matrix* trans = target->GetTransform();
	m_ForwardOrientation.x = trans->Get(2,0);
	m_ForwardOrientation.y = trans->Get(2,1);
	m_ForwardOrientation.z = trans->Get(2,2);
	return true;
}

/*!
 * @fn VXSoundSourceBase::VXSoundSourceBase()
 * Constructs an VXSoundSourceBase object that describes the position, 
 * orientation, speed, and environment of a sound in the 3D world.
 *
 * @see VXSoundListener Engine 
 */
VXSoundSourceBase::VXSoundSourceBase() : Engine()
{
	//TODO default settings go here
	m_SoundData = NULL;
}


VXSoundSourceBase::~VXSoundSourceBase()
{
	if (m_SoundData)
		delete m_SoundData;
	m_SoundData = NULL;
}


/*!
 * void VXSoundSourceBase::Set(int attr, void* val)
 *
 * Set the given sound source property. The source property
 * controls how the properties of the sound source.
 * If you do not explicitly set an source property, it will be
 * in the default state. You can <unset> an property again by setting
 * it's value to <SOUND_None>. The current set of source properties are:
 * @code
 *	<<SOURCE_Position>>				Vec3 for the upward orientation
 *	<<SOURCE_Velocity>>				Vec3 for the forward orientation
 *	<<SOURCE_InsideConeAngle>>		Vec3 for the position orientation
 *	<<SOURCE_OutsideConeAngle>>		Vec3 for the velocity vector
 *	<<SOURCE_ConeOrientation>>		type of environment
 *	<<SOURCE_Volume>>				format of the sound file
 *	<<SOURCE_Frequency>>			reset defaults
 *	<<SOURCE_Distance>>				Vec3 for the upward orientation
 *	<<SOURCE_Format>>				Vec3 for the forward orientation
 *	<<SOURCE_Panning>>				Vec3 for the position orientation
 *	<<SOUND_None>>					Vec3 for the velocity vector
 * @endcode
 *
 * @see VXSoundSourceBase::Get
 */
void VXSoundSourceBase::Set(int attr, void* val)
{
	switch (attr)
	{
		case SOURCE_Position:
	//	Vec3* valPtr = val;
		m_Position = *((Vec3*)val);
		break;

		case SOURCE_Velocity:
		m_Velocity = *((Vec3*)val);
		break;

		case SOURCE_InsideConeAngle:
		m_InsideConeAngle = *((uint32*)val);
		break;

		case SOURCE_OutsideConeAngle:
		m_OutsideConeAngle = *((uint32*)val);
		break;

		case SOURCE_ConeOrientation:
		m_ConeOrientation = *((Vec3*)val);
		break;

		case SOURCE_Volume:
		m_Volume = *((long*)val);
		break;

		case SOURCE_Frequency:
		m_Frequency = *((uint32*)val);
		break;

		case SOURCE_Distance:
		m_Distance = *((float*)val);
		break;

		case SOURCE_Panning:
		m_Panning = *((long*)val);
		break;

		case SOURCE_None:
		//TODO
		break;
	}
}

/*!
 * @fn void VXSoundSourceBase::Get(int attr, const void* val) 
 * Gets the value of the given source property.
 *
 * @returns Value of the attribute or SOURCE_None if attribute is not set
 *
 * @see VXSoundSourceBase::Set
 */
void VXSoundSourceBase::Get(int attr, const void* val) 
{
	switch(attr)
	{
		case SOURCE_Position:
		*((Vec3*)val) = m_Position;
		break;

		case SOURCE_Velocity:
		*((Vec3*)val) = m_Velocity;
		break;

		case SOURCE_InsideConeAngle:
		*((uint32*)val) = m_InsideConeAngle;
		break;

		case SOURCE_OutsideConeAngle:
		*((uint32*)val) = m_OutsideConeAngle;
		break;

		case SOURCE_ConeOrientation:
		*((Vec3*)val) = m_ConeOrientation;
		break;

		case SOURCE_Volume:
		*((long*)val) = m_Volume;
		break;

		case SOURCE_Frequency:
		*((uint32*)val) = m_Frequency;
		break;

		case SOURCE_Distance:
		*((float*)val) = m_Distance;
		break;

		case SOURCE_Panning:
		*((long*)val) = m_Panning;
		break;

		case SOURCE_None:
		//TODO
		break;
	}
	
}

bool VXSoundSourceBase::LoadSound(const TCHAR* filename)
{	
	Core::String fname(filename);

	if (m_SoundData)
	{
		delete (m_SoundData);
		m_SoundData = NULL;
	}
	m_SoundData = new VXSoundData(filename);
	if (fname.Right(4) == Core::String(TEXT(".wav")))
	{
		m_SoundData->LoadWAVFile();
		if (m_SoundData->GetData())
			return true;
	}
	return false;
}

bool VXSoundSourceBase::OnStart()
{
	return Engine::OnStart();
}

bool VXSoundSourceBase::OnStop()
{
	return Engine::OnStop();
}

bool VXSoundSourceBase::Eval(float val)
{
	Model* target = (Model*) GetTarget();

	if (target && target->IsClass(VX_Model))
		m_Position = target->GetCenter();
	return true;
}
