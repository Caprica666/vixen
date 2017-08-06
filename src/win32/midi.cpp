#include "vixen.h"
#include "vxutil.h"
#include "win32/vxmidi.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(MidiInput, Engine, VX_MidiInput);

VX_IMPLEMENT_CLASSID(MidiOutput, Engine, VX_MidiOutput);

/****
 *
 * MIDI INPUT CALLBACK
 * Converts the Windows MIDI event into a scene manager MIDI event and logs it
 * to the current output stream
 *
 ****/
static void FAR PASCAL MidiInFunc(HMIDIIN hMidiIn, WORD wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (wMsg != MIM_DATA)
		return;
	if (LOBYTE(dwParam1) >= MidiEvent::SYSTEM)		// ignore system messages
		return;

	MidiInput* mi = (MidiInput*) dwInstance;
	MidiEvent* event = new MidiEvent;

	VX_ASSERT(mi != NULL);
	VX_ASSERT(mi->IsClass(VX_MidiInput));
	event->Sender = mi;
	event->Time = (float) dwParam2 * 1000.0f;
	event->DeviceID = mi->GetDevice();
	event->Data = dwParam1;
	event->Log();
	VX_TRACE(MidiInput::Debug, ("MidiInput %X Message %x Channel %d Note %d Velocity %d",
				event->Data, event->GetMessage(), event->GetChannel(), event->GetNote(), event->GetVelocity()));
}

/*!
 * @fn MidiInput::MidiInput()
 *
 * Constructor for MIDI input engine. Opens the given MIDI input device.
 *
 * @see MidiInput::Open
 */
MidiInput::MidiInput() : Engine()
{
	m_MidiHandle = 0;
	m_DeviceID = -1;
}

MidiInput::MidiInput(int devid) : Engine()
{
	m_MidiHandle = 0;
	m_DeviceID = -1;
	Open(devid);
}

MidiInput::~MidiInput()
{
	Close();
}

bool MidiInput::OnStart()
{
	MMRESULT r = midiInStart(m_MidiHandle);
	return CheckError(r);
}

bool MidiInput::OnStop()
{
	MMRESULT r = midiInReset(m_MidiHandle);
	return CheckError(r);
}

/*!
 * @fn bool MidiInput::Open(int devid)
 * @param devid	MIDI device identifier (0 based).
 *
 * Opens the given MIDI input device.
 *
 * @return  true on successful open, else  false
 *
 * @see MidiInput::GetDevice MidiInput::Close
 */
bool MidiInput::Open(int devid)
{
	MMRESULT r;

	if (m_DeviceID < 0)
		m_DeviceID = devid;
	r = midiInOpen(&m_MidiHandle, m_DeviceID, (DWORD_PTR) &MidiInFunc, (DWORD_PTR) this, CALLBACK_FUNCTION);
	return CheckError(r);
} 

/*!
 * @fn void MidiInput::Close()
 *
 * Closes the MIDI input device.
 *
 * @see MidiInput::GetDevice MidiInput::Open
 */
void MidiInput::Close()
{
	midiInClose(m_MidiHandle);
	m_MidiHandle = 0;
}

/*!
 * @fn void MidiInput::SetTarget(SharedObj* obj)
 *
 * Sets the target for the MIDI input engine. If a MIDI output
 * engine is used as the target, it will play back the MIDI input
 * as it is sampled.
 *
 * @see MidiInput::GetDevice MidiInput::Open Engine::SetTarget
 */
void MidiInput::SetTarget(SharedObj* obj)
{
	MMRESULT r;

	Engine::SetTarget(obj);
	if (obj)
	   {
		MidiOutput* out = (MidiOutput*) obj;
		if (!obj->IsClass(VX_MidiOutput))
			return;
		out->Open();
		r = midiConnect(HMIDI(m_MidiHandle), out->m_MidiHandle, NULL);
	   }
	else
	   {
		const MidiOutput* out = (const MidiOutput*) GetTarget();
		if ((out == NULL) || !out->IsClass(VX_MidiOutput))
			return;
		r = midiDisconnect(HMIDI(m_MidiHandle), out->m_MidiHandle, NULL);
	   }
	CheckError(r);
}

bool MidiInput::CheckError(int r)
{
	char* err;

	switch (r)
	{
		case MMSYSERR_NOERROR: return true;
		case MMSYSERR_ALLOCATED: err = "MidiInput: ERROR device %d in use"; break;
		case MMSYSERR_BADDEVICEID: err = "MidiInput: ERROR  device identifier %d is out of range"; break;  
		case MMSYSERR_INVALFLAG:
		case MMSYSERR_INVALPARAM: err = "MidiInput: ERROR  parameter incorrect"; break;
		case MMSYSERR_NOMEM: err = "MidiInput: ERROR cannot allocate or lock memory"; break;
		case MMSYSERR_INVALHANDLE: err = "MidiInput: ERROR invalid handle"; break;
		case MIDIERR_NOTREADY: err = "MidiInput: device %d already connected"; break;
		default: err = "MidiInput: ERROR"; break;
	}
	VX_ERROR((err, m_DeviceID), false);
};

MidiOutput::MidiOutput() : Engine()
{
	m_MidiHandle = 0;
	m_DeviceID = -1;
}

MidiOutput::MidiOutput(int devid) : Engine()
{
	m_MidiHandle = 0;
	m_DeviceID = -1;
	Open(devid);
}

MidiOutput::~MidiOutput()
{
	Close();
}

bool MidiOutput::OnStop()
{
	MMRESULT r = midiOutReset(m_MidiHandle);
	return CheckError(r);
}

/*!
 * @fn bool MidiOutput::Open(int devid)
 * @param devid	MIDI device identifier (0 based).
 *
 * Opens the given MIDI output device.
 *
 * @return  true on successful open, else  false
 *
 * @see MidiOutput::GetDevice MidiOutput::Close
 */
bool MidiOutput::Open(int devid)
{
	MMRESULT r;

	if (m_DeviceID < 0)
		m_DeviceID = devid;
	r = midiOutOpen(&m_MidiHandle, m_DeviceID, NULL, (DWORD_PTR) this, CALLBACK_NULL);
	return CheckError(r);
} 

/*!
 * @fn void MidiOutput::Close()
 *
 * Closes the MIDI input device.
 *
 * @see MidiOutput::GetDevice MidiOutput::Open
 */
void MidiOutput::Close()
{
	midiOutClose(m_MidiHandle);
	m_MidiHandle = 0;
	m_DeviceID = -1;
}
 
bool MidiOutput::CheckError(int r)
{
	char* err;

	switch (r)
	{
		case MMSYSERR_NOERROR: return true;
		case MMSYSERR_ALLOCATED: err = "MidiOutput: ERROR device %d in use"; break;
		case MMSYSERR_BADDEVICEID: err = "MidiOutput: ERROR  device identifier %d is out of range"; break;  
		case MMSYSERR_INVALFLAG:
		case MMSYSERR_INVALPARAM: err = "MidiOutput: ERROR parameter incorrect"; break;
		case MMSYSERR_INVALHANDLE: err = "MidiOutput: ERROR invalid handle"; break;
		case MMSYSERR_NOMEM: err = "MidiOutput: cannot allocate or lock memory"; break;
		case MIDIERR_NOTREADY: err = "MidiOutput: device %d not ready"; break;
		default: err = "MidiOutput: ERROR"; break;
	}
	VX_ERROR((err, m_DeviceID), false);
};

bool MidiOutput::OnEvent(Event* ev)
{
	MidiEvent* me = (MidiEvent*) ev;

	if (me->Code != Event::MIDI)
		return Engine::OnEvent(ev);
	return Play(me->Data);
}

/*!
 * @fn bool MidiOutput::Play(int midi_data)
 *
 * Sends a single MIDI event to the MIDI output device.
 * This engine does not use MIDI streams and does not do buffering
 * of any kind. 
 *
 * @see MidiInput::GetDevice MidiInput::Open MidiEvent
 */
bool MidiOutput::Play(int midi_data)
{
	MMRESULT r = midiOutShortMsg(m_MidiHandle, midi_data);
	return CheckError(r);
}

/*!
 * @fn bool MidiOutput::Play(int message, int channel, int note, int data)
 * @param message	MIDI message to play
 * @param channel	voice/channel to use
 * @param note		note to play
 * @param data		MIDI data (usually note velocity)
 *
 * Sends a single MIDI event to the MIDI output device.
 * This engine does not use MIDI streams and does not do buffering
 * of any kind. 
 *
 * @see MidiInput::GetDevice MidiInput::Open MidiEvent
 */
bool MidiOutput::Play(int message, int channel, int note, int data)
{
	int32 midi_data = message | channel | (UINT(note) << 8) | (data << 16);
	return Play(midi_data);
}

}	// end Vixen