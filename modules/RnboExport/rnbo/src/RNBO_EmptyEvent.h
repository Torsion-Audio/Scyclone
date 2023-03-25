//
//  RNBO_EmptyEvent.h
//
//  Created by Rob Sussman on 9/18/15.
//
//

#ifndef _RNBO_EmptyEvent_h
#define _RNBO_EmptyEvent_h

namespace RNBO {

    /**
     * The EmptyEvent allows us to make an EventVariant that does not yet have an event.
     *
     * Useful, for example, to support calling a function that takes a reference to an EventVariant
	 * and then fills in said EventVariant.
     */
	class EmptyEvent {

	public:
		EmptyEvent()
		: _eventTarget(nullptr)
		{ }

		bool operator==(const EmptyEvent& rhs) const
		{
			RNBO_UNUSED(rhs);
			return true;
		}

		// need a getTime() method for GetTimeVisitor to work.
		MillisecondTime getTime() const { return 0; }
		EventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const { /* fprintf(stdout, "EmptyEvent\n"); */ }

	private:

		friend class EventVariant;

		void setTime(MillisecondTime eventTime)
		{
			RNBO_UNUSED(eventTime);
		}

		EventTarget *_eventTarget;

	};

} // namespace RNBO


#endif // #ifndef _RNBO_EmptyEvent_h
