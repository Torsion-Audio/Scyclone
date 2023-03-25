//
//  RNBO_PatcherEventSender.h
//
//  Created by Rob Sussman on 1/22/19.
//  Extracted from RNBO_EventVariant.h
//

#ifndef _RNBO_PatcherEventSender_H_
#define _RNBO_PatcherEventSender_H_

#include "RNBO_Types.h"
#include "RNBO_EventSender.h"
#include "RNBO_DataRefEvent.h"
#include "RNBO_OutletEvent.h"
#include "RNBO_ParameterEvent.h"
#include "RNBO_MessageEvent.h"

namespace RNBO {

	// specialization for PatcherEventTargets
	class PatcherEventSender : public EventSender {

	public:

		PatcherEventSender(PatcherEventTarget* fallbackTarget, MillisecondTime currentTime)
		: EventSender(fallbackTarget)
		, _fallbackTarget(fallbackTarget)
		, _currentTime(currentTime)
		{ }

		PatcherEventSender(const PatcherEventSender& other) = default;
		PatcherEventSender& operator=(const PatcherEventSender& other) = default;

		~PatcherEventSender() override {}

		void sendEvent(const EventVariant& ev) const override {
			Event::Type eventType = ev.getType();

			switch (eventType) {
				case Event::DataRef: handleDataRefEvent(ev.getDataRefEvent()); break;
				case Event::Outlet: handleOutletEvent(ev.getOutletEvent()); break;
				case Event::Parameter: handleParameterEvent(ev.getParameterEvent()); break;
				case Event::Message: handleMessageEvent(ev.getMessageEvent()); break;
				case Event::Tempo: handleTempoEvent(ev.getTempoEvent()); break;
				case Event::Transport: handleTransportEvent(ev.getTransportEvent()); break;
				case Event::BeatTime: handleBeatTimeEvent(ev.getBeatTimeEvent()); break;
				case Event::TimeSignature: handleTimeSignatureEvent(ev.getTimeSignatureEvent()); break;
				case Event::Empty:
				case Event::Clock:
				case Event::Startup:
				case Event::Midi:
				case Event::Universal:
				case Event::Preset:
				default: EventSender::sendEvent(ev); // fallback to base class
			}
		}

	private:

		MillisecondTime correctTime(MillisecondTime time) const override {
			return time < _currentTime ? _currentTime : time;
		}

		// PatcherEventTarget
		void handleDataRefEvent(const DataRefEvent& de) const {
			PatcherEventTarget* eventTarget = de.getEventTarget() ? de.getEventTarget() : _fallbackTarget;
			// TODO let's make this parallel to the others wrt naming, something like
			// eventTarget->processDataRefEvent(de.getDataRefIndex(), de.getAction(), de.getTime());
			if (de.getAction() == DataRefEvent::UpdateDataRef) {
				eventTarget->processDataViewUpdate(de.getDataRefIndex(), correctTime(de.getTime()));
			}
		}

		void handleOutletEvent(const OutletEvent& oe) const {
			PatcherEventTarget* eventTarget = oe.getEventTarget() ? oe.getEventTarget() : _fallbackTarget;
			eventTarget->processOutletEvent(oe.getSender(), oe.getIndex(), oe.getValue(), correctTime(oe.getTime()));
		}

		void handleParameterEvent(const ParameterEvent& pe) const {
			PatcherEventTarget* eventTarget = pe.getEventTarget() ? pe.getEventTarget() : _fallbackTarget;
			eventTarget->processParameterEvent(pe.getIndex(), pe.getValue(), correctTime(pe.getTime()));
		}

		void handleMessageEvent(const MessageEvent& me) const {
#ifndef RNBO_NOMESSAGEEVENT
			PatcherEventTarget* eventTarget = me.getEventTarget() ? me.getEventTarget() : _fallbackTarget;
			if (me.getType() == MessageEvent::List) {
				auto listValue = me.getListValue();
				eventTarget->processListMessage(me.getTag(), me.getObjectId(), correctTime(me.getTime()), *listValue);
			}
			else if (me.getType() == MessageEvent::Number) {
				eventTarget->processNumMessage(me.getTag(), me.getObjectId(), correctTime(me.getTime()), me.getNumValue());
			}
			else if (me.getType() == MessageEvent::Bang) {
				eventTarget->processBangMessage(me.getTag(), me.getObjectId(), correctTime(me.getTime()));
			}
			else {
				RNBO_ASSERT(false);
			}
#endif
		}

		void handleTempoEvent(const TempoEvent& te) const {
			PatcherEventTarget* eventTarget = te.getEventTarget() ? te.getEventTarget() : _fallbackTarget;
			eventTarget->processTempoEvent(correctTime(te.getTime()), te.getTempo());
		}

		void handleTransportEvent(const TransportEvent& te) const {
			PatcherEventTarget* eventTarget = te.getEventTarget() ? te.getEventTarget() : _fallbackTarget;
			eventTarget->processTransportEvent(correctTime(te.getTime()), te.getState());
		}

		void handleBeatTimeEvent(const BeatTimeEvent& be) const {
			PatcherEventTarget* eventTarget = be.getEventTarget() ? be.getEventTarget() : _fallbackTarget;
			eventTarget->processBeatTimeEvent(correctTime(be.getTime()), be.getBeatTime());
		}

		void handleTimeSignatureEvent(const TimeSignatureEvent& te) const {
			PatcherEventTarget* eventTarget = te.getEventTarget() ? te.getEventTarget() : _fallbackTarget;
			eventTarget->processTimeSignatureEvent(correctTime(te.getTime()), te.getNumerator(), te.getDenominator());
		}

		PatcherEventTarget* _fallbackTarget;
		MillisecondTime		_currentTime;

	};

} // namespace RNBO


#endif // #ifndef _RNBO_PatcherEventSender_H_
