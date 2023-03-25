//
//  RNBO_ParameterEvent.h
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_ParameterEvent_H_
#define _RNBO_ParameterEvent_H_

#include "RNBO_Types.h"
#include "RNBO_Math.h" // isnan

namespace RNBO {

	class PatcherEventTarget;

    // for now this is restricted to float values (so we might re-name this to be ParameterFloatEvent), but in the
    // future we might have ParameterStringEvent, ParameterBandEvent etc.

    /**
     * An event representing a parameter value change
     */
	class ParameterEvent {

	public:

		ParameterEvent()
		: _parameterIndex(INVALID_INDEX)
		, _eventTime(0)
		, _value(0)
		, _source(nullptr)
		, _eventTarget(nullptr)
		{
		}

		ParameterEvent(const ParameterEvent& other) = default;
		ParameterEvent& operator = (const ParameterEvent& other) = default;

		ParameterEvent(ParameterIndex parameterIndex, MillisecondTime eventTime, ParameterValue value, ParameterInterfaceId source, PatcherEventTarget* eventTarget = nullptr)
		: _parameterIndex(parameterIndex)
		, _eventTime(eventTime)
		, _value(value)
		, _source(source)
		, _eventTarget(eventTarget)
		{
		}

		// we want to treat two parameter events as equal if they both have values which are NaNs
		// despite the fact that comparing two NaNs will return false
		// this way we can avoid notification of non-changes in parameters
		bool valuesAreEqual(ParameterValue lhs, ParameterValue rhs) const
		{
			bool equal = lhs == rhs ? true : false;
			if (!equal) {
				// treat two NaN values as equal
				if (RNBO_Math::rnbo_isnan(lhs) && RNBO_Math::rnbo_isnan(rhs)) {
					equal = true;
				}
			}
			return equal;
		}

		bool operator==(const ParameterEvent& rhs) const
		{
			return rhs.getIndex() == getIndex()
			&& rhs.getTime() == getTime()
			&& valuesAreEqual(getValue(), rhs.getValue())
			&& rhs.getSource() == getSource()
			&& rhs._eventTarget == _eventTarget;
		}

		ParameterIndex getIndex() const { return _parameterIndex; }
		ParameterValue getValue() const { return _value; }
		ParameterInterfaceId getSource() const { return _source; }

		MillisecondTime getTime() const { return _eventTime; }
		PatcherEventTarget* getEventTarget() const { return _eventTarget; }

		bool isValid() const { return _parameterIndex != INVALID_INDEX; }
		void invalidate() { _parameterIndex = INVALID_INDEX; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
			// fprintf(stdout, "ParameterEvent: parameterIndex=%d time=%.3f value=%.4f source=%p\n", _parameterIndex, _eventTime, _value, _source);
		}

	private:

		ParameterIndex				_parameterIndex;
		MillisecondTime				_eventTime;
		ParameterValue				_value;
		ParameterInterfaceId		_source;

		friend class EventVariant;

		PatcherEventTarget*			_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_ParameterEvent_H_
