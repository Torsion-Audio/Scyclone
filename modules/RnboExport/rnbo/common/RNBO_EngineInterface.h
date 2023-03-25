//
//  RNBO_EngineInterface.h
//
//  Created by Rob Sussman on 8/25/15.
//
//

#ifndef _RNBO_EngineInterface_h_
#define _RNBO_EngineInterface_h_

#include "RNBO_EngineLink.h"

namespace RNBO {

	class EventTarget;

	/**
	 * @private
	 *
	 * The EngineInterface is passed to the generated code to allow it to talk
	 * to the Engine
	 */
	class EngineInterface {

	protected:
		// user of EngineInterface should not delete it
		~EngineInterface() { }

	public:

		virtual void scheduleClockEvent(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime delay) = 0;
		virtual void scheduleClockEventWithValue(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime delay, ParameterValue value) = 0;

		// remove all clock events from the scheduler, optionally executing the clocks
		virtual void flushClockEvents(EventTarget* eventTarget, ClockId clockIndex, bool execute) = 0;
		virtual void flushClockEventsWithValue(EventTarget* eventTarget, ClockId clockIndex, ParameterValue value, bool execute) = 0;

		// methods for generated code to send data out of the graph into the engine
		virtual void sendMidiEvent(int port, int b1, int b2, int b3) = 0;
		virtual void sendMidiEventList(int port, const list& data) = 0;

		// send an event from an external back into the RNBO graph
		virtual void sendOutlet(EngineLink* sender, OutletIndex index, ParameterValue value, SampleOffset sampleOffset = 0) = 0;

		// notify that a certain DataRef has been updated
		virtual void sendDataRefUpdated(DataRefIndex index) = 0;

		virtual void sendTempoEvent(Tempo tempo) = 0;
		virtual void sendTransportEvent(TransportState state) = 0;
		virtual void sendBeatTimeEvent(BeatTime beattime) = 0;
		virtual void sendTimeSignatureEvent(int numerator, int denominator) = 0;

		// send a message to the outside world
		virtual void sendNumMessage(MessageTag tag, MessageTag objectId, number payload, MillisecondTime time) = 0;
		virtual void sendListMessage(MessageTag tag, MessageTag objectId, const list& payload, MillisecondTime time) = 0;
		virtual void sendBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) = 0;

		// notify the outside world about an internal Parameter Change
		virtual void notifyParameterValueChanged(ParameterIndex index, ParameterValue value, bool ignoreSource) = 0;
		// schedule a parameter change event
		virtual void scheduleParameterChange(ParameterIndex index, ParameterValue value, MillisecondTime offset) = 0;

		virtual void updatePatcherEventTarget(const EventTarget *oldTarget, PatcherEventTarget *newTarget) = 0;
		virtual void rescheduleEventTarget(const EventTarget *target) = 0;

		virtual MillisecondTime getCurrentTime() = 0;

		virtual void presetTouched() = 0;

	};

} // namespace RNBO

#endif  // #ifndef _RNBO_EngineInterface_h_
