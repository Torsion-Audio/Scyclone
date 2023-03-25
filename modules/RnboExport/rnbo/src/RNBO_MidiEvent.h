//
//  RNBO_MidiEvent.h
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_MidiEvent_H_
#define _RNBO_MidiEvent_H_


namespace RNBO {

    // Note that this is not derived from anything, it just declares getTime() and
    // can therefore be used inside an event list.

    /**
     * An event that can hold a timestamped MIDI Event (MIDI Note, pitchbend etc.)
     */
	class EventTarget;

	class MidiEvent {
	public:
		MidiEvent()
		: _eventTime(0)
		, _portIndex(0)
		, _length(0)
		, _eventTarget(nullptr)
		{
			_midiData[0] = 0;
			_midiData[1] = 0;
			_midiData[2] = 0;
		}

		MidiEvent(MillisecondTime eventTime, int portIndex, int b1, int b2, int b3, EventTarget* eventTarget = nullptr)
		: _eventTime(eventTime)
		, _portIndex(portIndex)
		, _eventTarget(eventTarget)
		{
			// todo: assert b1, b2, and b3 are valid
			_midiData[0] = uint8_t(b1);
			_midiData[1] = uint8_t(b2);
			_midiData[2] = uint8_t(b3);
			// let length based on the status
			int status = b1 & 0xF0;
			if (status == 0xC0 || status == 0xD0 || b1 == 0xF1 || b1 == 0xF3) {
				//prog change, channel pressure, quarter frame, song select
				_length = 2;
			} else if ((b1 >= 0xF8 && b1 <= 0xFF) || b1 == 0xF6) {
				//realtime, tune request
				_length = 1;
			} else {
				_length = 3;
			}
		}


		MidiEvent(MillisecondTime eventTime, int portIndex, ConstByteArray data, Index length, EventTarget* eventTarget = nullptr)
		: _eventTime(eventTime)
		, _portIndex(portIndex)
		, _eventTarget(eventTarget)
		{
			const Index maxlen = sizeof(_midiData) / sizeof(_midiData[0]);
			for (Index i = 0; i < maxlen; i++) {
				_midiData[i] = 0;
			}
			_length = length > maxlen ? maxlen : length;
			// TODO: handle larger midi packets?
			for (Index i = 0; i < _length; i++) {
				_midiData[i] = data[i];
			}
		}

		bool operator==(const MidiEvent& rhs) const
		{
			return rhs.getTime() == getTime()
			&& rhs.getPortIndex() == getPortIndex()
			&& rhs.getData() == getData()
			&& rhs.getLength() == getLength()
			&& rhs._eventTarget == _eventTarget;
		}

		int getPortIndex() const { return _portIndex; }
		ConstByteArray getData() const  { return _midiData; }
		Index getLength() const { return _length; }

		MillisecondTime getTime() const { return _eventTime; }
		EventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const {
			// TODO: would be good to have logging that doesn't depend on fprintf
			// fprintf(stdout, "MidiEvent: time=%.3f port=%d length=%d b1=%d b2=%d b3=%d\n", _eventTime, _portIndex, _length, _midiData[0], _midiData[1], _midiData[2]);
		}

	private:

		MillisecondTime	_eventTime;
		int				_portIndex;
		Index			_length;
		uint8_t			_midiData[3];

		friend class EventVariant;

		EventTarget*	_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_MidiEvent_H_

