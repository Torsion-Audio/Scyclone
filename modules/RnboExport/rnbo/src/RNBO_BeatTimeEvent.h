//
//  RNBO_BeatTimeEvent.h
//
//

#ifndef _RNBO_BeatTimeEvent_H_
#define _RNBO_BeatTimeEvent_H_

#include "RNBO_Types.h"

namespace RNBO {

	class PatcherEventTarget;

	/**
	 * An event for setting the current transport time. Depending on the host,
	 * setting the beat time may simply update the internal transport, or else
	 * it may generate an event that causes the host to change its beat time.
	 * @see RNBO::BeatTime represents time in quater notes, as used by (for
	 * example) Ableton Link. @see RNBO::Millisecond time is of course the time
	 * in milliseconds.
	 */
	class BeatTimeEvent {

	public:

		BeatTimeEvent()
		: _eventTime(0)
		, _beatTime(-1)
		{
		}

		~BeatTimeEvent()
		{
		}

		BeatTimeEvent(
			MillisecondTime eventTime,
			BeatTime beatTime
		)
		: _eventTime(eventTime)
		, _beatTime(beatTime)
		{
		}

		BeatTimeEvent(const BeatTimeEvent& other)
		: _eventTime(other._eventTime)
		{
			_beatTime = other._beatTime;
		}

		BeatTimeEvent(BeatTimeEvent&& other)
		: _eventTime(other._eventTime)
		{
			_beatTime = other._beatTime;
		}

		BeatTimeEvent& operator = (const BeatTimeEvent& other)
		{
			_eventTime = other._eventTime;
			_beatTime = other._beatTime;

			return *this;
		}

		bool operator==(const BeatTimeEvent& rhs) const
		{
			return rhs.getTime() == getTime() && rhs.getBeatTime() == getBeatTime();
		}

		MillisecondTime getTime() const { return _eventTime; }
		BeatTime getBeatTime() const { return _beatTime; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "BeatTimeEvent: time=%.3f beatTime=%d", _eventTime, _beatTime);
		}

	protected:

		MillisecondTime		_eventTime;
		BeatTime			_beatTime;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

} // namespace RNBO

#endif // #ifndef _RNBO_BeatTimeEvent_H_
