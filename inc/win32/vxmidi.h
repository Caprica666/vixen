/*
 * @file vxmidi.h
 * @brief Simple MIDI input and output.
 *
 * These MIDI classes do simple, unbuffered MIDI output and input suitable
 * for sound effects. The SoundPlayer class provides more comprehensive
 * sound support for 3D spatialized sound and audio streaming.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxsoundplayer.h
 */
#pragma once
#include <mmsystem.h>

namespace Vixen {

/*!
 * @class MidiInput
 * @brief Produces a stream of events from MIDI input.
 *
 * Midi input engine that traps midi messages and generates
 * Scene mananger midi events (Event::MIDI) that can be caught by the application.
 * You can play back the midi input by supplying a midi output
 * engine (MidiOutput) as the target for this engine.
 *
 * @see MidiEvent Event MidiOutput SoundPlayer
 */
class MidiInput : public Engine
{
public:
	VX_DECLARE_CLASS(MidiInput);
	MidiInput();
	MidiInput(int devid);
	~MidiInput();

	bool	Open(int devid = -1);	//!< open Midi input device
	void	Close();				//!< close Midi input device
	bool	OnStart();
	bool	OnStop();
	void	SetTarget(SharedObj*);	//!< connect to Midi output for playback

	//! return Midi device ID
	int32	GetDevice() const		{ return m_DeviceID; }

	/*
	 * MidiInput::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		MIDI_Open = Engine::ENG_NextOp,
		MIDI_NextOp = Engine::ENG_NextOp + 10
	};

protected:
	bool	CheckError(int r);

	HMIDIIN m_MidiHandle;
	int32	m_DeviceID;
};

/*!
 * @class MidiOutput
 * @brief Plays sound from a stream of Vixen MIDI events.
 *
 * Midi output engine that listens for Midi events (EVENT_Midi)
 * and plays them on a Midi output device. You can also
 * play Midi events under program control. This engine cannot
 * output Midi from a file and does not use Midi streams.
 *
 * @see MidiEvent Event MidiInput SoundPlayer
 */
class MidiOutput : public Engine
{
	friend class MidiInput;
public:
	VX_DECLARE_CLASS(MidiOutput);
	MidiOutput();
	MidiOutput(int devid);
	~MidiOutput();

	bool	Open(int devid = -1);	//!< open Midi output device
	void	Close();				//!< close Midi output device
	bool	OnStop();
	bool	OnEvent(Event*);
	bool	Play(int midi_data);	//!< play Midi note
	bool	Play(int message, int channel, int note, int data);

	int32	GetDevice() const		//!< return Midi device ID
		{ return m_DeviceID; }

protected:
	bool	CheckError(int r);

	HMIDIOUT	m_MidiHandle;
	int32		m_DeviceID;
};

/*!
 * @class MidiEvent
 * @brief Describes MIDI event generated from MIDI input.
 *
 * The scene manager MIDI input engine generates these events when
 * notes are played on a the MIDI input device. Your application
 * can observe these events and use them to control graphics
 * from MIDI input. You can also send these events to the
 * MIDI output engine to be played on a MIDI output device.
 *
 * Each MIDI event has a message which describes the type
 * of the event and a channel specifying the MIDI channel
 * (or voice). Some MIDI events have additional data like
 * which note was played and how long it was held.
 *
 * Scene manager MIDI Message Codes
 * @code
 *	MidiEvent::NOTE_OFF
 * 	MidiEvent::NOTE_ON
 * 	MidiEvent::KEY_VELOCITY
 * 	MidiEvent::CONTROL_CHANGE
 * 	MidiEvent::PROGRAM_CHANGE
 * 	MidiEvent::AFTER_TOUCH
 * 	MidiEvent::PITCH_BLEND
 * 	MidiEvent::SYSTEM
 * @endcode
 *
 * @see MidiInput MidiOutput Event
 */
struct MidiEvent : public Event
{
	MidiEvent()	: Event(MIDI, 2 * sizeof(int32)) {}
	MidiEvent(const Event& src);
	Event&	operator=(const Event&);

	//! Get MIDI status (message and channel).
	int		GetStatus() const	{ return LOBYTE(Data); }

	//! Get MIDI message (note on, note off).
	int		GetChannel() const	{ return GetStatus() & 0x0F; }

	//! Get MIDI channel.
	int		GetMessage() const	{ return GetStatus() & 0XF0; }

	//! Get First MIDI data byte (note played).
	int		GetNote() const		{ return HIBYTE(LOWORD(Data)); }

	//! Get Second MIDI data byte (Velocity of note).
	int		GetVelocity() const	{ return HIWORD(Data); }

	/*!
	 * MIDI Channel messages
	 */
	enum
	{
		NOTE_OFF = 0x080,
		NOTE_ON = 0x090,
		KEY_VELOCITY = 0x0A0,
		CONTROL_CHANGE = 0x0B0,
		PROGRAM_CHANGE = 0x0C0,
		AFTER_TOUCH = 0x0D0,
		PITCH_BLEND = 0x0E0,
		SYSTEM = 0X0F0
	};

	int32		Data;
	int32		DeviceID;
};

inline MidiEvent::MidiEvent(const Event& src)
{
	*this = src;
}

inline Event& MidiEvent::operator=(const Event& src)
{
	Event::operator=(src);
	if (src.Code == Event::MIDI)
	{
		MidiEvent& msrc = (MidiEvent&) src;
		DeviceID = msrc.DeviceID;
		Data = msrc.Data;
	}
	return *this;
}

} // Vixen