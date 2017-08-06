#ifdef PSM_SOUND

#include "win32/psmwin.h"
#include "psmedia.h"
#include "media/sounddata.h"

#define	SOUND_IsLoaded	1
#define	SOUND_IsCreated	2
#define	SOUND_IsPlaying	4
#define	SOUND_IsReady	3

static bool sound_debug = true;

// Safely release the DirectSound Objects
#define SAFE_RELEASE(x) {if(x) {x->Release(); x = NULL; }}

//Convert from multibyte format to Unicode using the following macro:
#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP, \
        MB_PRECOMPOSED, y, -1, x, _MAX_PATH );

///
/// PSM2D3DVec
///
/// Converts a vector from the ISM PSVec3 format to the D3D D3DVECTOR format.
///

D3DVECTOR PSM2D3DVec( PSVec3 iVec ){
	D3DVECTOR dVec;
	dVec.x = -iVec.x; 	dVec.y = iVec.y; 	dVec.z = -iVec.z;
	return dVec;
} // PSM2D3DVec

///
/// D3D2PSMVec
///
/// Converts a vector from the D3D D3DVECTOR format to the ISM PSVec3 format.
///

PSVec3 D3D2PSMVec( D3DVECTOR dVec ){
	PSVec3 iVec;
	iVec.x = -dVec.x; iVec.y = dVec.y; iVec.z = -dVec.z;
	return iVec;
} // D3D2PSMVec

#ifdef _DEBUG
static const char* ListenerOpnames[] =
	{ "SetEnvironment", "SetFileName"};
const char** PSSoundListener::DoNames = ListenerOpnames;
#endif

PS_IMPLEMENT_CLASSID(PSSoundListener, PSEngine, PSM_SoundListener);

/*!
 * @fn PSSoundListener::PSSoundListener()
 *
 * Constructs an PSSoundListener object that describes the position, 
 * orientation, speed, and environment of the user's hearing in the 3D world.
 *
 * @see PSSoundSource PSEngine
 */
PSSoundListener::PSSoundListener()  : PSEngine()
{
	m_b3D = true;
	m_bEAX = true;
	m_hwnd = NULL;
	pDirectSoundObj = NULL;
	pPrimaryBuf = NULL;
	p3DEAXproperty = NULL;
	m_FileName.Empty();
	pPerf = NULL;
	pSegment = NULL;
	pSegState = NULL;
	ListenerParameters.dwSize = sizeof(DS3DLISTENER);
	m_init = 0;
	m_change = false;
	m_pDS = NULL;
	pListener = NULL;
	m_DistanceFactorChanged = false;
}

/*!
 * @fn PSSoundListener::~PSSoundListener()
 *
 * Destructs the PSSoundListener object and cleans up the 
 * DirectSound objects and buffers
 *
 * @see PSSoundSource
 */
PSSoundListener::~PSSoundListener()
{
	CleanUp();
}


HRESULT PSSoundListener::Initialize( HWND  hWnd, 
                                   DWORD dwCoopLevel, 
                                   DWORD dwPrimaryChannels, 
                                   DWORD dwPrimaryFreq, 
                                   DWORD dwPrimaryBitRate )
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    SAFE_RELEASE( m_pDS );

    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) )
        return hr;

    // Set DirectSound coop level 
    if( FAILED( hr = m_pDS->SetCooperativeLevel( hWnd, dwCoopLevel ) ) )
        return hr;
    
    // Set primary buffer format
    SetPrimaryBufferFormat( dwPrimaryChannels, dwPrimaryFreq, dwPrimaryBitRate );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: PSSoundListener::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format 
//       For example, to set the primary buffer format to 22kHz stereo, 16-bit
//       then:   dwPrimaryChannels = 2
//               dwPrimaryFreq     = 22050, 
//               dwPrimaryBitRate  = 16
//-----------------------------------------------------------------------------
HRESULT PSSoundListener::SetPrimaryBufferFormat( DWORD dwPrimaryChannels, 
                                               DWORD dwPrimaryFreq, 
                                               DWORD dwPrimaryBitRate )
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
       
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
        return hr;

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = WAVE_FORMAT_PCM; 
    wfx.nChannels       = (WORD) dwPrimaryChannels; 
    wfx.nSamplesPerSec  = dwPrimaryFreq; 
    wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
        return hr;

    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: PSSoundListener::Get3DListenerInterface()
// Desc: Assigns the 3D listener interface associated with primary buffer.
//-----------------------------------------------------------------------------
HRESULT PSSoundListener::Get3DListenerInterface()
{
    HRESULT             hr;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    // Obtain primary buffer, asking it for 3D control
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbdesc, &pDSBPrimary, NULL ) ) )
        return hr;

    if( FAILED( hr = pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, 
                                                  (VOID**)&pListener ) ) )
    {
        SAFE_RELEASE( pDSBPrimary );
        return hr;
    }
		
    // Release the primary buffer, since it is not need anymore
    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}


/*++++
 *
 * Name: SoundListener_Create
 *	bool Create( HWND, SoundFormat)
 *	Destroy();
 *
 * Description:
 * Creates the primary buffer for the API's Sound Listener
 *
 * Also: PSSoundSource SoundListener_Set SoundListener_Get
 *
 ----*/
bool PSSoundListener::CreateMidi()
{
	HRESULT			hr;

	if(m_FileName.GetLength() != 0)
	{
	
		// Direct Music initializations
		if (FAILED(CoInitialize(NULL)))
			return false;

		if (FAILED(CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, 
			IID_IDirectMusicPerformance2, (void**)&pPerf)))
				return false;

		//Initialize the DirectMusic with our DirectSound pointer...
		if(FAILED(hr = pPerf->Init(NULL, pDirectSoundObj, m_hwnd)))
			return false;

		//Add a midi port
		if (FAILED(pPerf->AddPort(NULL)))
			return false;

		//Create a midi loader
		if (FAILED(CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
			IID_IDirectMusicLoader, (void**)&pLoader)))
				return false;

	//Load the midi

		WCHAR				wszMidiFileName[_MAX_PATH];
		char				szMidiFileName[_MAX_PATH] ;
		strcpy(szMidiFileName, m_FileName);

		MULTI_TO_WIDE(wszMidiFileName, szMidiFileName);
	//   if(FAILED( hr = pLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE)))
	//		return false;
		pSegment = NULL;

		//describe the object to be loaded, in a DMUS_OBJECTDESC structure:
		DMUS_OBJECTDESC ObjDesc;
		ObjDesc.guidClass = CLSID_DirectMusicSegment;
		ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);	
		wcscpy( ObjDesc.wszFileName, wszMidiFileName );
		ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

		//Now load the object and query it for the IDirectMusicSegment interface. 
		//This is done in a single call to IDirectMusicLoader::GetObject. 
		//Note that loading the object also initializes the tracks and does everything 
		//else necessary to get the MIDI data ready for playback.
		if( FAILED( hr = pLoader->GetObject(&ObjDesc, IID_IDirectMusicSegment2, (void**) &pSegment)))
			return false;

		//To ensure that the segment plays as a standard MIDI file, 
		//you now need to set a parameter on the band track.
		//let DirectMusic find the track, by passing -1 (or 0xFFFFFFFF) in the dwGroupBits method parameter.
		if( FAILED( hr = pSegment->SetParam(GUID_StandardMIDIFile,
				-1, 0, 0, (void*)pPerf)))
				return false;

		//The next step is to download the instruments. This is necessary even for playing 
		//a simple MIDI file, because the default software synthesizer needs the DLS data 
		//for the General MIDI instrument set. If you skip this step, the MIDI file will play silently. 
		//Again, you call SetParam on the segment, this time specifying the GUID_Download parameter:
		if( FAILED( hr = pSegment->SetParam(GUID_Download, -1, 0, 0, (void*)pPerf)))
			return false;

		if( FAILED( hr = pPerf->PlaySegment(pSegment, 0, 0, &pSegState)))
			return false;
	}

	return true;
}


void PSSoundListener::CleanUp()
{
	SAFE_RELEASE(pSegState);
	SAFE_RELEASE(pSegment);
	SAFE_RELEASE(pLoader);
	SAFE_RELEASE(pPerf);
	SAFE_RELEASE(pListener);
	SAFE_RELEASE(pPrimaryBuf);
	SAFE_RELEASE(pDirectSoundObj);
}
		
/*++++
 *
 * Name: SoundListener_Set
 *	void Set(int32 attr, void* val)
 *
 * Description:
 * Set the given sound listener property. The listener property
 * controls how the environment and source sounds are mixed to produce the total sound.
 * If you do not explicitly set an listener property, it will be
 * in the default state. You can <unset> an property again by setting
 * it's value to <LISTENER_None>. The current set of listener properties are:
 * T+
 *	<<LISTENER_UpOrientation>>		PSVec3 for the upward orientation
 *	<<LISTENER_ForwardOrientation>>	PSVec3 for the forward orientation
 *	<<LISTENER_Position>>			PSVec3 for the position orientation
 *	<<LISTENER_Velocity>>			PSVec3 for the velocity vector
 *	<<LISTENER_Environment>>		type of environment
 *									SOUND_Generic, SOUND_Room, SOUND_Bathroom, 
									SOUND_Livingroom, SOUND_Stoneroom, SOUND_Hallway
									SOUND_CarpetedHallway, SOUND_Auditorium, SOUND_ConcertHall,
									SOUND_Alley, SOUND_Forest, SOUND_City, SOUND_Mountains,
									SOUND_Quarry, SOUND_Plain, SOUND_Parkinglot, 
									SOUND_SewerPipe, SOUND_Generic,SOUND_PaddedCell,
									SOUND_Cave, SOUND_Hangar, SOUND_Arena,
									SOUND_StoneCorridor, SOUND_Underwater,
									SOUND_Drugged, SOUND_Dizzy, SOUND_Psychotic
 *	<<LISTENER_Format>>				format of the sound file
 *	<<LISTENER_None>>				reset defaults
 * T-
 *
 * Also: SoundListener_Get
 *
 ----*/
void PSSoundListener::Set(int32 attr, void* val)
{
	switch(attr)
	{
	case LISTENER_UpOrientation:
		m_UpOrientation = *((PSVec3*)val);
		ListenerParameters.vOrientTop = PSM2D3DVec(m_UpOrientation);
		break;

	case LISTENER_ForwardOrientation:
		m_ForwardOrientation = *((PSVec3*)val);
		ListenerParameters.vOrientFront = PSM2D3DVec(m_ForwardOrientation);
		break;

	case LISTENER_Position:
		m_Position = *((PSVec3*)val);
		ListenerParameters.vPosition = PSM2D3DVec(m_Position);
		break;

	case LISTENER_Velocity:
		m_Velocity = *((PSVec3*)val);
		ListenerParameters.vVelocity = PSM2D3DVec(m_Velocity);
		break;

	case LISTENER_DistanceFactor:
		m_Distance = *((float*)val);
		m_DistanceFactorChanged = true;
		break;

	case LISTENER_Environment:
		m_Environment = *((int32*)val);
		switch(m_Environment)
		{
		case 	SOUND_Generic: 
			m_envId = EAX_ENVIRONMENT_GENERIC;
			break;
		case 	SOUND_Room: 
			m_envId = EAX_ENVIRONMENT_ROOM;
			break;
		case 	SOUND_Bathroom:
			m_envId = EAX_ENVIRONMENT_BATHROOM;
			break;
		case 	SOUND_Livingroom: 
			m_envId = EAX_ENVIRONMENT_LIVINGROOM;
			break;
		case 	SOUND_Stoneroom: 
			m_envId = EAX_ENVIRONMENT_STONEROOM;
			break;
		case 	SOUND_Hallway: 
			m_envId = EAX_ENVIRONMENT_HALLWAY;
			break;
		case 	SOUND_CarpetedHallway: 
			m_envId = EAX_ENVIRONMENT_CARPETEDHALLWAY;
			break;
		case 	SOUND_Auditorium: 
			m_envId = EAX_ENVIRONMENT_AUDITORIUM;
			break;
		case 	SOUND_ConcertHall: 
			m_envId = EAX_ENVIRONMENT_CONCERTHALL;
			break;
		case 	SOUND_Alley: 
			m_envId = EAX_ENVIRONMENT_ALLEY;
			break;
		case 	SOUND_Forest: 
			m_envId = EAX_ENVIRONMENT_FOREST;
			break;
		case 	SOUND_City: 
			m_envId = EAX_ENVIRONMENT_CITY;
			break;
		case 	SOUND_Mountains: 
			m_envId = EAX_ENVIRONMENT_MOUNTAINS;
			break;
		case 	SOUND_Quarry: 
			m_envId = EAX_ENVIRONMENT_QUARRY;
			break;
		case 	SOUND_Plain: 
			m_envId = EAX_ENVIRONMENT_PLAIN;
			break;
		case 	SOUND_Parkinglot: 
			m_envId = EAX_ENVIRONMENT_PARKINGLOT;
			break;
		case 	SOUND_SewerPipe: 
			m_envId = EAX_ENVIRONMENT_SEWERPIPE;
			break;
		case 	SOUND_PaddedCell: 
			m_envId = EAX_ENVIRONMENT_PADDEDCELL;
			break;
		case 	SOUND_Hangar: 
			m_envId = EAX_ENVIRONMENT_HANGAR;
			break;
		case 	SOUND_Arena: 
			m_envId = EAX_ENVIRONMENT_ARENA;
			break;
		case 	SOUND_StoneCorridor: 
			m_envId = EAX_ENVIRONMENT_STONECORRIDOR;
			break;
		case 	SOUND_Underwater: 
			m_envId = EAX_ENVIRONMENT_UNDERWATER;
			break;
		case 	SOUND_Drugged: 
			m_envId = EAX_ENVIRONMENT_DRUGGED;
			break;
		case 	SOUND_Dizzy: 
			m_envId = EAX_ENVIRONMENT_DIZZY;
			break;
		case 	SOUND_Psychotic:
			m_envId = EAX_ENVIRONMENT_PSYCHOTIC;
			break;
		case SOUND_Cave:
			m_envId = EAX_ENVIRONMENT_CAVE;
			break;
		}
		break;
	
	case LISTENER_Rolloff:
		m_Rolloff = *((float*)val);
		ListenerParameters.flRolloffFactor = m_Rolloff;
		break;

	case LISTENER_Doppler:
		m_Doppler = *((float*)val);
		ListenerParameters.flDopplerFactor  = m_Doppler;
		break;

	case LISTENER_None:
		//TODO
		break;
	}
}

void PSSoundListener::SetFileName(const char* filename)
{
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundListener, SOUNDLISTENER_SetFileName) << this << filename;
	PS_STREAM_END()

	if(!(strcmp(m_FileName,filename)))
	{
		m_change = false;
		return;
	}

	if (filename == NULL)
		m_FileName.Empty();
	else
		m_FileName = filename;
	m_change = true;
}

void PSSoundListener::SetEnvironment(int32 enviro)
{
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundListener, SOUNDLISTENER_SetEnvironment) << this << (int32)enviro;
	PS_STREAM_END()
	
	Set(LISTENER_Environment, (void*)&enviro);
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
void PSSoundListener::Get(int32 attr, const void* val) 
{
	switch(attr)
	{
	case LISTENER_UpOrientation:
	*((PSVec3*)val) = m_UpOrientation;
		break;

	case LISTENER_ForwardOrientation:
		*((PSVec3*)val) = m_ForwardOrientation;
		break;

	case LISTENER_Position:
		*((PSVec3*)val) = m_Position;
		break;

	case LISTENER_Velocity:
		*((PSVec3*)val) = m_Velocity;
		break;

	case LISTENER_Environment:
		*((int32*)val) = m_Environment;
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

bool PSSoundListener::CreateListener()
{
	HRESULT hr = Initialize( (HWND) PSGetScene()->GetWindow(), DSSCL_PRIORITY, 2, 22050, 16 );
	hr = Get3DListenerInterface();
	pListener->GetAllParameters( &ListenerParameters );	
	return InitDev();
}

bool PSSoundListener::OnStart()
{
	if(!m_init)
	{
		CreateListener();
		m_init = true;
	}
	
	return PSEngine::OnStart();
}

bool PSSoundListener::OnStop()
{
	PSM_TRACE(PSSoundSource::Debug, ("SoundListener::OnStop"));
	return PSEngine::OnStop();
}

bool PSSoundListener::ChangeMusic()
{
	HRESULT hr;
	pPerf->Stop(NULL, NULL, 0, 0);

	SAFE_RELEASE(pSegment);

	SAFE_RELEASE(pLoader);

	//Recreate the midi loader
	if (FAILED(CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
        IID_IDirectMusicLoader, (void**)&pLoader)))
			return false;

//Load the midi

	WCHAR				wszMidiFileName[_MAX_PATH];
	char				szMidiFileName[_MAX_PATH];
	strcpy(szMidiFileName, m_FileName);

	MULTI_TO_WIDE(wszMidiFileName, szMidiFileName);

	//describe the object to be loaded, in a DMUS_OBJECTDESC structure:
	DMUS_OBJECTDESC ObjDesc;
	ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);	
    wcscpy( ObjDesc.wszFileName, wszMidiFileName );
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

	//Now load the object and query it for the IDirectMusicSegment interface. 
	//This is done in a single call to IDirectMusicLoader::GetObject. 
	//Note that loading the object also initializes the tracks and does everything 
	//else necessary to get the MIDI data ready for playback.
	if( FAILED( hr = pLoader->GetObject(&ObjDesc, IID_IDirectMusicSegment2, (void**) &pSegment)))
		return false;

	//To ensure that the segment plays as a standard MIDI file, 
	//you now need to set a parameter on the band track.
	//let DirectMusic find the track, by passing -1 (or 0xFFFFFFFF) in the dwGroupBits method parameter.
	if( FAILED( hr = pSegment->SetParam(GUID_StandardMIDIFile,
            -1, 0, 0, (void*)pPerf)))
			return false;

	//The next step is to download the instruments. This is necessary even for playing 
	//a simple MIDI file, because the default software synthesizer needs the DLS data 
	//for the General MIDI instrument set. If you skip this step, the MIDI file will play silently. 
	//Again, you call SetParam on the segment, this time specifying the GUID_Download parameter:
    if( FAILED( hr = pSegment->SetParam(GUID_Download, -1, 0, 0, (void*)pPerf)))
		return false;

    if( FAILED( hr = pPerf->PlaySegment(pSegment, 0, 0, &pSegState)))
		return false;

	return true;	

}

bool PSSoundListener::Eval(float val)
{
	if(m_change)
	{
		m_change = false;
		if(!ChangeMusic())
			return false;
	}

	PSCamera* target = (PSCamera*) GetTarget();
	PSM_ASSERT(target);
	PSM_ASSERT(target->IsClass(PSM_Camera));
	
	m_Position = target->GetCenter();
	m_UpOrientation = target->GetDirection();

	PSModel* mod = (PSModel*) GetTarget();
	PSMatrix mtx;
	const PSMatrix* trans = mod->GetTransform();
	trans = &mtx;
	mod->TotalTransform(&mtx);
	m_ForwardOrientation.x = trans->Get(2,0);
	m_ForwardOrientation.y = trans->Get(2,1);
	m_ForwardOrientation.z = trans->Get(2,2);
	
	if (!pListener)
		return false;

	if(m_DistanceFactorChanged)
		pListener->SetDistanceFactor(m_Distance,DS3D_DEFERRED);
	m_DistanceFactorChanged = false;
	
	D3DVECTOR pos = PSM2D3DVec(m_Position);
	D3DVECTOR front = PSM2D3DVec(m_ForwardOrientation);
	D3DVECTOR top = PSM2D3DVec(m_UpOrientation);
	memcpy( &ListenerParameters.vOrientFront, &front, sizeof(D3DVECTOR) );
	memcpy( &ListenerParameters.vOrientTop, &top, sizeof(D3DVECTOR) );
    memcpy( &ListenerParameters.vPosition, &pos, sizeof(D3DVECTOR) );

	pListener->SetAllParameters( &ListenerParameters, DS3D_DEFERRED );
	pListener->CommitDeferredSettings();

	static int counter = 0;
	
	if(m_bEAX)
	{
		if(!(p3DEAXproperty))
			return false;
		else
			if(FAILED(p3DEAXproperty->Set(DSPROPSETID_EAX_ListenerProperties, 
				DSPROPERTY_EAXLISTENER_ENVIRONMENT, NULL, 0, &m_envId, sizeof(DWORD))))
					return false;
			
	}
	PSM_TRACE(sound_debug,("evalled listener"));
	return true;
}

bool PSSoundListener::InitDev()
{
	if(!pListener)
		return false;
	
	pListener->GetAllParameters(&ListenerParameters);

	Set(LISTENER_UpOrientation, (void*)&PSVec3(0.0f, 1.0f, 0.0f));
	Set(LISTENER_ForwardOrientation, (void*)&PSVec3(0.0f, 0.0f, -1.0f));
	Set(LISTENER_Position, (void*)&PSVec3(0.0f, 0.0f, 0.0f));
	int32 temp = SOUND_Auditorium;
	Set(LISTENER_Environment, (void*)&temp);
	float factor = 1.0f;
	Set(LISTENER_DistanceFactor,(void*)&factor);

	return true;
}

bool PSSoundListener::Do(PSMessenger& s, int op)
{
	char	fname[PSM_MaxPath];
	char	soundfile[PSM_MaxPath];
	int32	n;

	switch (op)
	{
	
/*
 * When loading a texture from a stream, we prepend
 * the directory of the file that was opened
 */
		case SOUNDLISTENER_SetEnvironment:
		s >> n;
		SetEnvironment(n);
		break;

		case SOUNDLISTENER_SetFileName:
		s >> fname;
		s.GetPath(fname, soundfile);
		SetFileName(soundfile);
		break;

		default:
		return PSEngine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(psm_debug << ClassName() << "::"
					   << PSSoundListener::DoNames[op - SOUNDLISTENER_SetEnvironment]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Image override for PSObj::Save
 *
 ****/
int PSSoundListener::Save(PSMessenger& s, int opts) const
{
	int32 h = PSEngine::Save(s, opts);
	if (h > 0)
	{
		const char* name = GetFileName();

		if (name && *name)
			s << OP(PSM_SoundListener, SOUNDLISTENER_SetFileName) << h << name;

		if (m_Environment)
			s << OP(PSM_SoundListener, SOUNDLISTENER_SetEnvironment) << h << m_Environment;

	}
	
	return h;
}

DebugOut& PSSoundListener::Print(DebugOut& dbg) const
{
#ifdef _DEBUG
	PSEngine::Print(dbg);
// TODO: print sound listener properties	
#endif
	return dbg;
}

bool PSSoundListener::Copy(const PSObj* obj)
{
	if (!PSEngine::Copy(obj))
		return false;
	PSSoundListener*	src = (PSSoundListener*) obj;
	pDirectSoundObj = src->pDirectSoundObj;		// Object to start DirectSound	
	pPrimaryBuf = src->pPrimaryBuf;				// The DirectSound primary buffer
	pListener = src->pListener;					// The 3D interface for the primary buffer
	p3DEAXproperty = src->p3DEAXproperty;		// the 3D EAX property interface (pReverb)
	ListenerParameters = src->ListenerParameters; // The settings for the listener
	m_b3D = src->m_b3D;							// Boolean tells whether the sound card supports 3D
	m_bEAX = src->m_bEAX ;						// Boolean tells whether the sound card supports EAX
	m_hwnd = src->m_hwnd;
	pLoader = src->pLoader;
	pPerf = src->pPerf;
	pSegment = src->pSegment;
	pSegState = src->pSegState;
	m_change = src->m_change;
	return true;
}


#ifdef _DEBUG
static const char* SourceOpnames[] =
	{ "SetFileName", "SetListener", "SetInt", "SetFloat", "SetVec"};
const char** PSSoundSource::DoNames = SourceOpnames;
#endif

PS_IMPLEMENT_CLASSID(PSSoundSource, PSEngine, PSM_SoundSource);

/*!
 * @fn PSSoundSource::PSSoundSource()
 *
 * Constructs an PSSoundSource object that describes the position, 
 * orientation, speed, and environment of a sound in the 3D world.
 *
 * @see PSSoundListener PSEngine
 */
PSSoundSource::PSSoundSource() : PSEngine()
{
	m_Listener = (PSSoundListener*)NULL;
	m_SoundData = NULL;
	pSecondaryBuf = NULL;
	p3DEAXproperty = NULL;
	m_b3D = true;
	m_bEAX = true;
	m_init = 0;
	m_SoundData = NULL;
	p3DBuf = NULL;
	Sound3DProperties.dwSize = sizeof(DS3DBUFFER);
}

void PSSoundSource::SetFileName(const char* filename)
{
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundSource, SOUNDSOURCE_SetFileName) << this << filename;
	PS_STREAM_END()

	if(filename == NULL)
	{
		m_FileName.Empty();
		m_init |= SOUND_IsLoaded;
		return;
	}
	m_FileName = filename;
	m_init &= ~SOUND_IsLoaded;
	SetStartTime(0);
	Start(0);
}

void PSSoundSource::SetListener(const PSSoundListener* listener)
{
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundSource, SOUNDSOURCE_SetListener) << this << listener;
	PS_STREAM_END()

	m_Listener = listener;
}

void PSSoundSource::Pause()
{
	if ((m_init & SOUND_IsPlaying) == 0)
		return;
	if(pSecondaryBuf)
	{
		pSecondaryBuf->Stop();
		PSM_TRACE(sound_debug,("PSSoundSource::Pause %s", GetFileName()));
	}
	m_init &= ~SOUND_IsPlaying;
}

void PSSoundSource::Resume()
{
	if (m_init & SOUND_IsPlaying)
		return;
	if(pSecondaryBuf)
	{
		pSecondaryBuf->Play(0, 0, 0 );
		PSM_TRACE(sound_debug,("PSSoundSource::Resume %s", GetFileName()));
	}
	m_init |= SOUND_IsPlaying;
}


PSSoundSource::~PSSoundSource()
{
	CleanUp();
}

void PSSoundSource::CleanUp()
{
	if(pSecondaryBuf)
	{
		m_SoundData->UnloadWAV();
		SAFE_RELEASE(pSecondaryBuf);
	}
	if(m_SoundData)
		delete m_SoundData;
	SAFE_RELEASE(p3DBuf);
	SAFE_RELEASE(p3DEAXproperty);
	SAFE_RELEASE(pSecondaryBuf);	
}

/*!
 * @fn bool PSSoundSource::CreateSound()
 *
 * Creates the sound buffer for a sound in the scene
 *
 * @see PSSoundListener PSSoundSource::Set PSSoundSource::LoadSound
 */
bool PSSoundSource::CreateSound()
{
	WAVEFORMATEX	wfx;

	if(!(SetFormat(wfx)))
		return false;

	DSBUFFERDESC bdesc;
    ZeroMemory( &bdesc, sizeof(DSBUFFERDESC) );
    bdesc.dwSize        = sizeof(DSBUFFERDESC);
    bdesc.dwFlags       = DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE ;
    bdesc.dwBufferBytes = m_size;
    bdesc.lpwfxFormat   = &wfx;

	LPDIRECTSOUND8 SoundObj = NULL;
	if ((PSSoundListener*)m_Listener)
		SoundObj = ((PSSoundListener*)m_Listener)->GetListener();
	if(SoundObj == NULL)
	{
		m_init &= ~SOUND_IsCreated;
		PSM_ERROR(("SoundSource::CreateSound %s ERROR no listener", GetFileName()), false);
	}
       
    if( FAILED(SoundObj->CreateSoundBuffer(&bdesc, &pSecondaryBuf, NULL ) ) )
	{
		m_b3D = false;
		bdesc.dwFlags       = 0;
		if( FAILED((((PSSoundListener*)m_Listener)->GetListener())->CreateSoundBuffer( &bdesc, &pSecondaryBuf, NULL ) ) )
			return false;
	}
	
	DWORD volume = DSBVOLUME_MIN/2;
	pSecondaryBuf->SetVolume(volume);
	PSM_TRACE(PSSoundSource::Debug, ("SoundSource::CreateSound %s", GetFileName()));
	m_init |= SOUND_IsCreated;
	
	if(m_b3D)
	{
		if( FAILED(pSecondaryBuf->QueryInterface(IID_IDirectSound3DBuffer, (void**)&p3DBuf)))
			return false;
		
		p3DBuf->GetAllParameters( &Sound3DProperties );
		
		if(FAILED(p3DBuf->QueryInterface(IID_IKsPropertySet, (void**)&p3DEAXproperty)))
			m_bEAX = false;	
		
		return InitDev();
	}
	return true;
}

/*!
 * @fn void PSSoundSource::Set(int attr, const PSVec3& val)
 *
 * Set the vector for the given sound source property.
 * If you do not explicitly set an source property, it will be
 * in the default state. You can  unset a property again by setting
 * it's value to  SOUND_None. The current set of vector source properties are:
 * @code
 *	SOURCE_Position				position of the sound source
 *	SOURCE_Velocity				velocity of the sound source
 *	SOURCE_ConeOrientation		orientation of the sound cone
 *	SOUND_None					unset the property
 * @endcode
 *
 * @see PSSoundSource::Get
 */
void PSSoundSource::Set(int attr, const PSVec3& val)
{	
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundSource, SOUNDSOURCE_SetVec) << this << int32(attr) << val;
	PS_STREAM_END( )

	SetChanged(true);
	switch(attr)
	{
		case SOURCE_Position:
		m_Position = val;
		Sound3DProperties.vPosition = PSM2D3DVec(m_Position);
		break;

		case SOURCE_Velocity:
		m_Velocity = val;
		Sound3DProperties.vVelocity = PSM2D3DVec(m_Velocity);
		break;

		case SOURCE_ConeOrientation:
		m_ConeOrientation = val;
		Sound3DProperties.vConeOrientation = PSM2D3DVec(m_ConeOrientation);
		break;

	}
}

/*!
 * @fn void PSSoundSource::Set(int attr, long val)
 *
 * Set the integer value of the given sound source property.
 * If you do not explicitly set an source property, it will be
 * in the default state. You can  unset a property again by setting
 * it's value to  SOUND_None. The current set of integer properties are:
 * @code
 *	SOURCE_InsideConeAngle		inside sound cone angle
 *	SOURCE_OutsideConeAngle		outside sound cone angle
 *	SOURCE_ConeOrientation		type of environment
 *	SOURCE_OutsideConeVolume	volume of sound for outside cone
 *	SOURCE_Frequency			frequency of sound
 *	SOURCE_Panning				left, right or center panning
 *	SOUND_None					unset this parameter
 * @endcode
 *
 * @see PSSoundSource::Get
 */
void PSSoundSource::Set(int attr, long val)
{	
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundSource, SOUNDSOURCE_SetInt) << this << int32(attr) << val;
	PS_STREAM_END( )

	SetChanged(true);
	switch(attr)
	{
		case SOURCE_InsideConeAngle:
		m_InsideConeAngle = val;
		Sound3DProperties.dwInsideConeAngle = m_InsideConeAngle;
		break;

		case SOURCE_OutsideConeAngle:
		m_OutsideConeAngle = val;
		Sound3DProperties.dwOutsideConeAngle =  m_OutsideConeAngle;
		break;

		case SOURCE_OutsideConeVolume:
		m_OCVolume = val;
		Sound3DProperties.lConeOutsideVolume = m_OCVolume;
		break;

		case SOURCE_Panning:
		m_Panning = val;
		break;

		case SOURCE_Frequency:
		m_Frequency = val;
		SetChanged(true);
		break;

		case SOURCE_None:
		//TODO
		break;
	}
}

void PSSoundSource::Set(int attr, float val)
{	
	PS_STREAM_BEGIN(s)
		*s << OP(PSM_SoundSource, SOUNDSOURCE_SetFloat) << this << int32(attr) << val;
	PS_STREAM_END( )

	SetChanged(true);
	switch(attr)
	{
		case SOURCE_Distance:
		m_Distance = val;
		if(m_Distance > DS3D_DEFAULTMAXDISTANCE)
			Sound3DProperties.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;
		else
			Sound3DProperties.flMaxDistance = m_Distance;
		break;

		case SOURCE_MinDistance:
		m_MinDistance = val;
		Sound3DProperties.flMinDistance = m_MinDistance;
		break;
		
	}
}

/*!
 * @fn void PSSoundSource::Get(int attr, const void* valptr)
 * @param valptr	pointer to where to store property
 *
 * The sound property is copied into the memory area referenced
 * by the input pointer. Sound properties may be vector, float
 * or integer. No check is made to verify that the memory area
 * is large enough for the property requested.
 *
 * @see PSSoundSource::Set
 */
void PSSoundSource::Get(int attr, const void* val) 
{
	switch(attr)
	{
		case SOURCE_Position:
		*((PSVec3*)val) = m_Position;
		break;

		case SOURCE_Velocity:
		*((PSVec3*)val) = m_Velocity;
		break;

		case SOURCE_InsideConeAngle:
		*((uint32*)val) = m_InsideConeAngle;
		break;

		case SOURCE_OutsideConeAngle:
		*((uint32*)val) = m_OutsideConeAngle;
		break;

		case SOURCE_ConeOrientation:
		*((PSVec3*)val) = m_ConeOrientation;
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
	}
}

bool PSSoundSource::OnStart()
{
	if (PSEngine::OnStart())
	{
		m_init |= SOUND_IsPlaying;
		PSM_TRACE(sound_debug,("SoundSource::OnStart %s", GetFileName()));
		return true;
	}
	return false;
}

bool PSSoundSource::OnStop()
{
	m_init &= ~SOUND_IsPlaying;
	
	if (pSecondaryBuf)
	{
		m_SoundData->UnloadWAV();
		SAFE_RELEASE(p3DBuf);
		SAFE_RELEASE(p3DEAXproperty);
		SAFE_RELEASE(pSecondaryBuf);
	}
	PSM_TRACE(sound_debug,("SoundSource::OnStop %s", GetFileName()));
	return true;
}

bool PSSoundSource::OnReset()
{
	m_init |= SOUND_IsPlaying;
	return true;
}

void PSSoundSource::SetActive(bool active)
{
	PSEngine::SetActive(active);
	if (active)
		Resume();
	else
		Pause();
}

bool PSSoundSource::Eval(float val)
{
	int32 state = m_init & SOUND_IsReady;
	if(state != SOUND_IsReady)
	{
		if(!Change())
			return true;
	}
	PSModel* target = (PSModel*) GetTarget();
	if(target == NULL)
		return true;

	m_Position = target->GetCenter();

	if(HasChanged() && pSecondaryBuf)
	{
		pSecondaryBuf->SetPan(m_Panning); 
		pSecondaryBuf->SetFrequency(m_Frequency);	
		Sound3DProperties.dwInsideConeAngle = m_InsideConeAngle;
		Sound3DProperties.dwOutsideConeAngle =  m_OutsideConeAngle;
		Sound3DProperties.lConeOutsideVolume = m_OCVolume;
		Sound3DProperties.vConeOrientation = PSM2D3DVec(m_ConeOrientation);
		if(m_Distance > DS3D_DEFAULTMAXDISTANCE)
			Sound3DProperties.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;
		else
			Sound3DProperties.flMaxDistance = m_Distance;
		Sound3DProperties.flMinDistance = m_MinDistance;
		SetChanged(false);
	}
	DS3DBUFFER temp;
	HRESULT hr;

	if(p3DBuf)
	{
		D3DVECTOR pos = PSM2D3DVec(m_Position);
	//	D3DVECTOR vel = PSM2D3DVec(m_Velocity);
		memcpy( &Sound3DProperties.vPosition, &pos, sizeof(D3DVECTOR) );
	//	memcpy( &Sound3DProperties.vVelocity, &vel, sizeof(D3DVECTOR) );

		hr = p3DBuf->SetAllParameters( &Sound3DProperties, DS3D_IMMEDIATE ); 
		if(hr == DSERR_INVALIDPARAM )
			hr = 0;
		
		temp.dwSize = sizeof(DS3DBUFFER);
		hr = p3DBuf->GetAllParameters( &temp); 
	} 

	
	DWORD   dwStatus;

	if( NULL == pSecondaryBuf )
		return true;

	// Restore the buffers if they are lost
	if( FAILED( hr = RestoreBuffers() ) )
		return true;

	if( FAILED( hr = pSecondaryBuf->GetStatus( &dwStatus ) ) )
		return true;

	if(!( dwStatus & DSBSTATUS_PLAYING ) && (m_init & SOUND_IsPlaying))
		// Play buffer 
   {
		if( FAILED( hr = pSecondaryBuf->Play( 0, 0, 0  ) ) )
			return true;
   }
	return true;

}

bool PSSoundSource::LoadWaveFile()
{
	const char* filename = GetFileName();
	
	if ((filename == NULL) || (*filename == NULL))
		return false;
	if(m_SoundData == NULL)
		m_SoundData = new PSSoundData();
	if(strstr(filename, ".wav"))
	{
		m_SoundData->LoadWAVFile(filename);
		if(m_SoundData->GetData())
			return true;
	}
	return false;
}

bool PSSoundSource::LoadSound()
{
	if (m_FileName.IsEmpty())
		return false;
	if (((m_init & SOUND_IsLoaded) == 0) || (m_SoundData == NULL))
	{
		if(pSecondaryBuf)
		{
			m_SoundData->UnloadWAV();
			SAFE_RELEASE(p3DBuf);
			SAFE_RELEASE(p3DEAXproperty);
			SAFE_RELEASE(pSecondaryBuf);
		}
		if (!LoadWaveFile())
			return false;
		PSM_TRACE(sound_debug,("PSSoundSource::LoadSound %s", GetFileName()));
		m_init |= SOUND_IsLoaded;
		m_format = *(m_SoundData->GetFormat());
		m_size = *(m_SoundData->GetSize());
		m_data = (BYTE*)m_SoundData->GetData();
		float time = m_SoundData->GetTime();
		SetDuration(time);
	}
	if(pSecondaryBuf == NULL)
		if(!CreateSound())
			return false;
	return LoadBuffer();
}


bool PSSoundSource::Change()
{
	if(!LoadSound())
		return false;
	long dwTemp;
	dwTemp = *(m_SoundData->GetFrequency());
	Set(SOURCE_Frequency, dwTemp);
	PSModel* target = (PSModel*) GetTarget();
	if(target == NULL)
		return true;
	PSVec3 orientation = target->GetDirection();
	Set(SOURCE_ConeOrientation, orientation);
	m_Velocity = PSVec3(0.0f, 0.0f, 0.0f);
	return true;
}

bool PSSoundSource::LoadBuffer()
{
	HRESULT		hr;
	VOID*		pbData  = NULL;
    VOID*		pbData2 = NULL;
    DWORD		dwLength = 0;
    DWORD		dwLength2 = 0;
    
	// Lock the buffer down

	if(FAILED(hr = pSecondaryBuf->Lock(0, m_size, &pbData, &dwLength, &pbData2, &dwLength2, 0L)))
		return false;

	// Copy the memory to it.
	memcpy( pbData, m_data, m_size );

	// Unlock the buffer, we don't need it anymore.
	pSecondaryBuf->Unlock( pbData, m_size, NULL, 0 );
	pbData = NULL;
	pSecondaryBuf->SetCurrentPosition(0);
	return true;
}

bool PSSoundSource::SetFormat(WAVEFORMATEX &wfx)
{
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = WAVE_FORMAT_PCM; 
	wfx.nChannels       = m_SoundData->GetChannels(); 
	wfx.nSamplesPerSec  = *(m_SoundData->GetFrequency()); 
	wfx.wBitsPerSample  = m_SoundData->GetBitsPerSample(); 
	wfx.nBlockAlign     = m_SoundData->GetBlockAlign();
	wfx.nAvgBytesPerSec = m_SoundData->GetBytesPerSecond();
    
	return true;
}

HRESULT PSSoundSource::RestoreBuffers()
{
    HRESULT hr;

    if( NULL == pSecondaryBuf )
        return S_OK;

    DWORD dwStatus;
    if( FAILED( hr = pSecondaryBuf->GetStatus( &dwStatus ) ) )
        return hr;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr = pSecondaryBuf->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( hr = pSecondaryBuf->Restore() );

        if( FAILED( hr = LoadBuffer() ) )
            return hr;
    }

    return S_OK;
}

bool PSSoundSource::InitDev()
{
	Sound3DProperties.dwMode = DS3DMODE_NORMAL ;
	return true;
}

bool PSSoundSource::Do(PSMessenger& s, int op)
{
	int32		attr;
	int32		val;
	float		fval;
	PSVec3		vecval;
	char		fname[PSM_MaxPath];
	char		soundfile[PSM_MaxPath];
	PSObj*		obj;

	switch (op)
	{
	
/*
 * When loading a texture from a stream, we prepend
 * the directory of the file that was opened
 */
		case SOUNDSOURCE_SetFileName:
		s >> fname;
		s.GetPath(fname, soundfile);
		SetFileName(soundfile);
		break;

		case SOUNDSOURCE_SetListener:
		s >> obj;
		SetListener((PSSoundListener*)obj);
		break;

		case SOUNDSOURCE_SetInt:
		s >> attr >> val;
		Set(attr, val);
		break;

		case SOUNDSOURCE_SetFloat:
		s >> attr >> fval;
		Set(attr, fval);
		break;

		case SOUNDSOURCE_SetVec:
		s >> attr >> vecval;
		Set(attr, vecval);
		break;

		default:
		return PSEngine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(psm_debug << ClassName() << "::"
					   << PSSoundSource::DoNames[op - SOUNDSOURCE_SetFileName]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class Image override for PSObj::Save
 *
 ****/
int PSSoundSource::Save(PSMessenger& s, int opts) const
{
	int32 h = PSEngine::Save(s, opts);	
	if (h > 0)
	{
		const char* name = GetFileName();

		if (name && *name)
			s << OP(PSM_SoundSource, SOUNDSOURCE_SetFileName) << h << name;

		if (m_Listener != NULL)
			s << OP(PSM_SoundSource, SOUNDSOURCE_SetListener) << h << m_Listener;
	}
	return h;
}

DebugOut& PSSoundSource::Print(DebugOut& dbg) const
{
#ifdef _DEBUG
	PSEngine::Print(dbg);
	if (!m_FileName.IsEmpty())
		endl(dbg << ClassName() << "::Load " << this << " '" << m_FileName << "'");

#endif
	return dbg;
}

bool PSSoundSource::Copy(const PSObj* obj)
{
	if (!PSEngine::Copy(obj))
		return false;
	PSSoundSource*	src = (PSSoundSource*) obj;
	m_Listener = (PSSoundListener*)src->m_Listener;				// To get to the primary sound buffer when needed
//	pSecondaryBuf = src->pSecondaryBuf;			// Instantiate a secondary buffer for each sound source
//	p3DBuf = src->p3DBuf;					// the 3D secondary buffer interface
//	p3DEAXproperty = src->p3DEAXproperty;			// the 3D EAX property interface (pReverb)
//	Sound3DProperties = src->Sound3DProperties;		// the parameters for setting the 3D sound properties
	m_b3D = src->m_b3D;					// Boolean tells whether the sound card supports 3D
	m_bEAX = src->m_bEAX;					// Boolean tells whether the sound card supports EAX
	m_size = src->m_size;					// Size of the data buffer
	m_data = src->m_data;					// points to the memory where the data is stored.
	m_format = src->m_format;				// enumerated type for the format
	m_FileName = src->m_FileName;
	m_init = src->m_init;
	m_InsideConeAngle = src->m_InsideConeAngle; 
	m_OutsideConeAngle = src->m_OutsideConeAngle; 
	m_Frequency = src->m_Frequency;
	m_Position = src->m_Position;
	m_Velocity = src->m_Velocity;
	m_OCVolume = src->m_OCVolume;
	m_ConeOrientation = src->m_ConeOrientation;
	m_Distance = src->m_Distance;
	m_MinDistance = src->m_MinDistance;
	return true;
}

#endif
