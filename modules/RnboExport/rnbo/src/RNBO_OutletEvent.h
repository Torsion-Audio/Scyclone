
#ifndef _RNBO_OutletEvent_H_
#define _RNBO_OutletEvent_H_

#include "RNBO_Types.h"
#include "RNBO_PatcherInterface.h"

namespace RNBO {

	/**
	 * An event representing a message sent through an outlet of an external
	 */
	class OutletEvent {
	public:

		OutletEvent(const OutletEvent& other) = default;
		OutletEvent& operator = (const OutletEvent& other) = default;

		OutletEvent()
		: _eventTime(0)
		, _sender(nullptr)
		, _index(INVALID_INDEX)
		, _value(0)
		, _eventTarget(nullptr)
		{
		}

		OutletEvent(MillisecondTime eventTime, EngineLink* sender, OutletIndex index, ParameterValue value, PatcherEventTarget* eventTarget = nullptr)
		: _eventTime(eventTime)
		, _sender(sender)
		, _index(index)
		, _value(value)
		, _eventTarget(eventTarget)
		{
		}

		bool operator==(const OutletEvent& rhs) const
		{
			return rhs.getTime() == getTime()
			&& rhs.getSender() == getSender()
			&& rhs.getIndex() == getIndex()
			&& rhs.getValue() == getValue()
			&& rhs._eventTarget == _eventTarget;
		}

		EngineLink* getSender() const { return _sender; }
		OutletIndex getIndex() const { return _index; }
		ParameterValue getValue() const { return _value; }

		MillisecondTime getTime() const { return _eventTime; }
		PatcherEventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const {
//			fprintf(stdout, "OutletEvent: time: %.4f sender: %p index: %d value: %.4f eventTarget: %p\n", _eventTime, _sender, _index, _value, _eventTarget);
		}

	private:

		MillisecondTime		_eventTime;
		EngineLink*			_sender;
		OutletIndex			_index;
		ParameterValue		_value;

		friend class EventVariant;

		PatcherEventTarget*	_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_OutletEvent_H_
