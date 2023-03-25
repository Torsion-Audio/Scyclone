//
//  RNBO_UniversalEvent.h
//

#ifndef _RNBO_UniversalEvent_H_
#define _RNBO_UniversalEvent_H_

#include "RNBO_EventTarget.h"
#include "RNBO_HardwareDevice.h"

namespace RNBO {

    // Note that this is not derived from anything, it just declares getTime() and can therefore be used inside an event
    // list.

	// assuming a single linear floating-point value from 0-1 as the value for the time being
	// we could add boolean, integer, enum, blob types later on

    /**
     * An event that can hold a timestamped external event (external controller, OSC, etc.)
     */
	class UniversalEvent {
	public:
		UniversalEvent()
		: _eventTime(0)
		, _id(INVALID_INDEX)
		, _value(0)
		, _eventTarget(nullptr)
		{}

		UniversalEvent(MillisecondTime eventTime, Index deviceId, ParameterValue value, EventTarget* eventTarget = nullptr)
		: _eventTime(eventTime)
		, _id(deviceId)
		, _value(value)
		, _eventTarget(eventTarget)
		{}


		UniversalEvent(MillisecondTime eventTime, String deviceName, ParameterValue value, EventTarget* eventTarget = nullptr)
		: _eventTime(eventTime)
		, _id(INVALID_INDEX)
		, _value(value)
		, _eventTarget(eventTarget)
		{
			Index sourceId = UniversalEventSource::getIdForName(deviceName);
			if (sourceId >= 0) {
				_id = sourceId;
			}
		}

		bool operator==(const UniversalEvent& rhs) const
		{
			return rhs.getTime() == getTime()
			&& rhs.getId() == getId()
			&& rhs.getValue() == getValue();
		}

		Index getId() const { return _id; }
		ParameterValue getValue() const  { return _value; }

		MillisecondTime getTime() const { return _eventTime; }
		EventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const {
			// TODO: would be good to have logging that doesn't depend on fprintf
			// fprintf(stdout, "MidiEvent: time=%.3f port=%d length=%d b1=%d b2=%d b3=%d\n", _eventTime, _portIndex, _length, _midiData[0], _midiData[1], _midiData[2]);
		}

	private:

		MillisecondTime	_eventTime;
		Index			_id;
		ParameterValue	_value;

		friend class EventVariant;

		EventTarget*	_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO

#endif // _RNBO_UniversalEvent_H_
