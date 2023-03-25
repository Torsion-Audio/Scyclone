//
//  RNBO_EventSender.h
//
//  Created by Rob Sussman on 1/22/19.
//  Extractd from RNBO_EventVariant.h
//
//

#ifndef _RNBO_EventSender_H_
#define _RNBO_EventSender_H_

#include "RNBO_Types.h"
#include "RNBO_EventTarget.h"
#include "RNBO_EventVariant.h"
#include "RNBO_EmptyEvent.h"
#include "RNBO_ClockEvent.h"
#include "RNBO_MidiEvent.h"

namespace RNBO {

	/**
	 * @private
	 */
	class EventSender {

	public:

		virtual ~EventSender() { }

		virtual void sendEvent(const EventVariant& ev) const {
			Event::Type eventType = ev.getType();

			switch (eventType) {
				case Event::Empty: handleEmptyEvent(ev.getEmptyEvent()); break;
				case Event::Clock: handleClockEvent(ev.getClockEvent()); break;
				case Event::Midi: handleMidiEvent(ev.getMidiEvent()); break;
				case Event::DataRef:
				case Event::Outlet:
				case Event::Parameter:
				case Event::Universal:
				case Event::Message:
				case Event::Preset:
				case Event::Tempo:
				case Event::Transport:
				case Event::BeatTime:
				case Event::TimeSignature:
				case Event::Startup:
				default: break;
			}
		}

	protected:

		EventSender(EventTarget* fallbackTarget)
		: _fallbackTarget(fallbackTarget)
		{ }

		EventSender(const EventSender& other) = default;
		EventSender& operator=(const EventSender& other) = default;

	private:

		void handleEmptyEvent(const EmptyEvent& ee) const {
			RNBO_UNUSED(ee);
			// nothing to do here
		}

		void handleClockEvent(const ClockEvent& ce) const {
			EventTarget* eventTarget = ce.getEventTarget() ? ce.getEventTarget() : _fallbackTarget;

			if (!eventTarget) return; // what do we do here? is it even a possibility?
			eventTarget->processClockEvent(correctTime(ce.getTime()), ce.getClockIndex(), ce.hasValue(), ce.getValue());
		}

		void handleMidiEvent(const MidiEvent& me) const {
			EventTarget* eventTarget = me.getEventTarget() ? me.getEventTarget() : _fallbackTarget;

			if (!eventTarget) return;

			ConstByteArray data = me.getData();  // should midi be const char* or const unsigned char*
			if (data) {
				Index length = me.getLength();
				int port = me.getPortIndex();
				eventTarget->processMidiEvent(correctTime(me.getTime()), port, data, length);
			}
		}

		virtual MillisecondTime correctTime(MillisecondTime time) const {
			return time;
		}

		EventTarget* _fallbackTarget;

	};

} // namespace RNBO


#endif // #ifndef _RNBO_EventSender_H_
