//
//  RNBO_TempoEvent.h
//
//

#ifndef _RNBO_TempoEvent_H_
#define _RNBO_TempoEvent_H_

#include "RNBO_Types.h"

namespace RNBO {

	class PatcherEventTarget;

    /**
     * A tempo event
     */
	class TempoEvent {

	public:

		TempoEvent()
		: _eventTime(0)
		, _tempo(-1)
		{
		}

		~TempoEvent()
		{
		}

		TempoEvent(
			MillisecondTime eventTime,
			Tempo tempo
		)
		: _eventTime(eventTime)
		, _tempo(tempo)
		{
		}

		TempoEvent(const TempoEvent& other)
		: _eventTime(other._eventTime)
		{
			_tempo = other._tempo;
		}

		TempoEvent(TempoEvent&& other)
		: _eventTime(other._eventTime)
		{
			_tempo = other._tempo;
		}

		TempoEvent& operator = (const TempoEvent& other)
		{
			_eventTime = other._eventTime;
			_tempo = other._tempo;

			return *this;
		}

		bool operator==(const TempoEvent& rhs) const
		{
			return rhs.getTime() == getTime() && rhs.getTempo() == getTempo();
		}

		MillisecondTime getTime() const { return _eventTime; }
		Tempo getTempo() const { return _tempo; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "TempoEvent: time=%.3f tempo=%d", _eventTime, _tempo);
		}

	protected:

		MillisecondTime		_eventTime;
		Tempo				_tempo;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

} // namespace RNBO

#endif // #ifndef _RNBO_TempoEvent_H_
