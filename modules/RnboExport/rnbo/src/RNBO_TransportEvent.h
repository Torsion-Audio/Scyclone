//
//  RNBO_TransportEvent.h
//
//

#ifndef _RNBO_TransportEvent_H_
#define _RNBO_TransportEvent_H_

#include "RNBO_Types.h"

namespace RNBO {

	class PatcherEventTarget;

    /**
     * An event representing stopping or starting the transport
     */
	class TransportEvent {

	public:

		TransportEvent()
		: _eventTime(0)
		, _state(STOPPED)
		{
		}

		~TransportEvent()
		{
		}

		TransportEvent(
			MillisecondTime eventTime,
			TransportState state
		)
		: _eventTime(eventTime)
		, _state(state)
		{
		}

		TransportEvent(const TransportEvent& other)
		: _eventTime(other._eventTime)
		{
			_state = other._state;
		}

		TransportEvent(TransportEvent&& other)
		: _eventTime(other._eventTime)
		{
			_state = other._state;
		}

		TransportEvent& operator = (const TransportEvent& other)
		{
			_eventTime = other._eventTime;
			_state = other._state;

			return *this;
		}

		bool operator==(const TransportEvent& rhs) const
		{
			return rhs.getTime() == getTime() && rhs.getState() == getState();
		}

		MillisecondTime getTime() const { return _eventTime; }
		TransportState getState() const { return _state; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "TransportEvent: time=%.3f state=%d", _eventTime, _state);
		}

	protected:

		MillisecondTime		_eventTime;
		TransportState		_state;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

} // namespace RNBO

#endif // #ifndef _RNBO_TransportEvent_H_
