#ifndef _RNBO_MIDIHelper_H_
#define _RNBO_MIDIHelper_H_

namespace RNBO {

	enum {
		MIDI_InvalidByte = -1,
		MIDI_StatusByteReceived = 0,
		MIDI_SecondByteReceived,
		MIDI_Send2Bytes,
		MIDI_NoteOff,
		MIDI_NoteOn,
		MIDI_Aftertouch,
		MIDI_CC,
		MIDI_ProgramChange,
		MIDI_ChannelPressure,
		MIDI_PitchBend,
		MIDI_Generic,

		MIDI_Sysex_Started,
		MIDI_Sysex_Complete,

		MIDI_NoteOffMask = 0x80,
		MIDI_NoteOnMask = 0x90,
		MIDI_AfterTouchMask = 0xA0,
		MIDI_CCMask = 0xB0,
		MIDI_ProgramChangeMask = 0xC0,
		MIDI_ChannelPressureMask = 0xD0,
		MIDI_PitchBendMask = 0xE0,

		MIDI_CC_Sustain = 64,
		MIDI_CC_Sostenuto = 66,
		MIDI_CC_AllNotesOff = 123,
		MIDI_CC_PressureMSB = 70,
		MIDI_CC_PressureLSB = 102,
		MIDI_CC_TimbreMSB = 74,
		MIDI_CC_TimbreLSB = 106,

		MIDI_SysexStart = 0xF0,

		//system common
		MIDI_QuarterFrame = 0xF1,
		MIDI_SongPos = 0xF2,
		MIDI_SongSel = 0xF3,
		MIDI_TuneRequest = 0xF6,

		MIDI_SysexEnd = 0xF7,

		//system realtime
		MIDI_Clock = 0xF8,
		MIDI_Start = 0xFA,
		MIDI_Continue = 0xFB,
		MIDI_Stop = 0xFC,
		MIDI_ActiveSense = 0xFE,
		MIDI_Reset = 0xFF,


		MIDI_NoteState_Off = 0,
		MIDI_NoteState_On = 1,
		MIDI_NoteState_Sustained = 2
	};

	static inline int parseMidi(int currentStatus, int byte, int statusbyte)
	{
		if (byte == 0xF0) {
			return MIDI_Sysex_Started;
		}
		else if (currentStatus == MIDI_Sysex_Started) {
			if (byte == 0xF7) {
				return MIDI_Sysex_Complete;
			}
			else if (byte <= 0x7F) {
				return currentStatus;
			}
			else {
				return MIDI_InvalidByte;
			}
		}

		if (byte > 127) return MIDI_StatusByteReceived;

		int command = statusbyte & 0xF0;   // get command

		if (currentStatus == MIDI_StatusByteReceived)
		{
			if (command == MIDI_ProgramChangeMask) return MIDI_ProgramChange;
			if (command == MIDI_ChannelPressureMask) return MIDI_ChannelPressure;
			if (statusbyte == MIDI_QuarterFrame) return MIDI_QuarterFrame;
			if (statusbyte == MIDI_SongSel) return MIDI_SongSel;
			return MIDI_SecondByteReceived;
		}
		else if (currentStatus == MIDI_SecondByteReceived) {
			if (command == MIDI_NoteOffMask || (command == MIDI_NoteOnMask && byte == 0)) return MIDI_NoteOff;
			if (command == MIDI_NoteOnMask) return MIDI_NoteOn;
			if (command == MIDI_AfterTouchMask) return MIDI_Aftertouch;
			if (command == MIDI_CCMask) return MIDI_CC;
			if (command == MIDI_PitchBendMask) return MIDI_PitchBend;
			if (statusbyte == MIDI_SongPos) return MIDI_SongPos;
			// probably shouldn't get here
			return MIDI_Generic;
		}

		return currentStatus;
	}

	static inline int getMIDIChannel(int statusbyte)
	{
		int command = statusbyte & 0xF0;

		if (command >= 0x80 && command <= 0xE0) {
			return statusbyte & 0x0F;
		}

		return 0;
	}
}



#endif  // _RNBO_MIDIHelper_H_
