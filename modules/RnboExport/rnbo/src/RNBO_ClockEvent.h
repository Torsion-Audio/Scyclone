//
//  RNBO_ClockEvent.h
//
//  Created by Rob Sussman on 8/6/15.
//
//

#ifndef _RNBO_ClockEvent_H_
#define _RNBO_ClockEvent_H_

#include "RNBO_EventTarget.h"

namespace RNBO {

    /**
	 * @private
     * An clock (Timer) event
     */
	class ClockEvent {

	public:
		ClockEvent(ClockId clockIndex, MillisecondTime eventTime, EventTarget* eventTarget = nullptr)
		: _clockIndex(clockIndex)
		, _eventTime(eventTime)
		, _value(0)
		, _hasValue(false)
		, _eventTarget(eventTarget)
		{

		}

		ClockEvent(ClockId clockIndex, MillisecondTime eventTime, ParameterValue value, EventTarget* eventTarget = nullptr)
		: _clockIndex(clockIndex)
		, _eventTime(eventTime)
		, _value(value)
		, _hasValue(true)
		, _eventTarget(eventTarget)
		{

		}

		ClockEvent()
		: _clockIndex(0)
		, _eventTime(0)
		, _value(0)
		, _hasValue(false)
		, _eventTarget(nullptr)
		{
		}

		bool operator==(const ClockEvent& rhs) const
		{

			if (rhs.getClockIndex() != getClockIndex())
				return false;
			if (rhs.getTime() != getTime())
				return false;
			if (hasValue() != rhs.hasValue())
				return false;
			// we know both have same hasValue()
			if (hasValue()) {
				// we now know that the events are equal if they both have the same value
				return rhs.getValue() == getValue();
			}
			if (rhs._eventTarget != _eventTarget)
				return false;

			// if we get here we know that we don't care about the value
			return true;
		}

		bool matchesEventTargetAndClockIndex(EventTarget* eventTarget, ClockId clockIndex) const {
			return _eventTarget == eventTarget
			&& _clockIndex == clockIndex;
		}

		bool matchesEventTargetAndClockIndex(const ClockEvent& ce) const {
			return matchesEventTargetAndClockIndex(ce._eventTarget, ce._clockIndex);
		}

		bool matchesEventTargetClockIndexAndValue(EventTarget* eventTarget, ClockId clockIndex, ParameterValue value) const {
			return _eventTarget == eventTarget
			&& _clockIndex == clockIndex
			&& _value == value;
		}

		ClockId getClockIndex() const { return _clockIndex; }
		ParameterValue getValue() const { return _value; }
		bool hasValue() const { return _hasValue; }

		MillisecondTime getTime() const { return _eventTime; }
		EventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const { /* fprintf(stdout, "ClockEvent: eventTarget=%p clockIndex=%ld time=%.3f hasValue=%s value=%.4f\n", _eventTarget, _clockIndex, _eventTime, _hasValue?"true":"false", _value); */ }

	private:

		ClockId			_clockIndex;
		MillisecondTime		_eventTime;
		ParameterValue		_value;
		bool				_hasValue;

		friend class EventVariant;

		EventTarget*		_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO


#endif // #ifndef _RNBO_ClockEvent_H_
