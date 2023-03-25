//
//  RNBO_TimeSignatureEvent.h
//
//

#ifndef _RNBO_TimeSignatureEvent_H_
#define _RNBO_TimeSignatureEvent_H_

#include "RNBO_Types.h"

namespace RNBO {

	class PatcherEventTarget;

    /**
     * An event representing changing the time signature
     */
	class TimeSignatureEvent {

	public:

		TimeSignatureEvent()
		: _eventTime(0)
		, _numerator(-1)
		, _denominator(-1)
		{
		}

		~TimeSignatureEvent()
		{
		}

		TimeSignatureEvent(MillisecondTime eventTime, int numerator, int denominator)
		: _eventTime(eventTime)
		, _numerator(numerator)
		, _denominator(denominator)
		{
		}

		TimeSignatureEvent(const TimeSignatureEvent& other)
		: _eventTime(other._eventTime)
		{
			_numerator = other._numerator;
			_denominator = other._denominator;
		}

		TimeSignatureEvent(TimeSignatureEvent&& other)
		: _eventTime(other._eventTime)
		{
			_numerator = other._numerator;
			_denominator = other._denominator;
		}

		TimeSignatureEvent& operator = (const TimeSignatureEvent& other)
		{
			_eventTime = other._eventTime;
			_numerator = other._numerator;
			_denominator = other._denominator;

			return *this;
		}

		bool operator==(const TimeSignatureEvent& rhs) const
		{
			return rhs.getTime() == getTime()
				&& rhs.getNumerator() == getNumerator()
				&& rhs.getDenominator() == getDenominator();
		}

		MillisecondTime getTime() const { return _eventTime; }
		int getNumerator() const { return _numerator; }
		int getDenominator() const { return _denominator; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "TimeSignatureEvent: time=%.3f numerator=%d denominator=%d", _eventTime, _numerator, _denominator);
		}

	protected:

		MillisecondTime		_eventTime;
		int					_numerator;
		int					_denominator;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

} // namespace RNBO

#endif // #ifndef _RNBO_TimeSignatureEvent_H_
