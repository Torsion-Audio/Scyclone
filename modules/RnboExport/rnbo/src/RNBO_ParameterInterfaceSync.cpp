#include "RNBO.h"
#include "RNBO_ParameterInterfaceSync.h"
#include "RNBO_EngineCore.h"

namespace RNBO {

	ParameterInterfaceSync::ParameterInterfaceSync(EngineCore& engine, EventHandler* handler)
		: _engine(engine)
		, _handler(handler)
	{
		engine.addToActiveInterfaces(this);
		if (handler) {
			handler->linkToParameterEventInterface(this);
		}

		refreshParameterCountAndValues();
	}

	ParameterInterfaceSync::~ParameterInterfaceSync() {
		_engine.removeFromActiveInterfaces(this);
	}

	ParameterIndex ParameterInterfaceSync::getNumParameters() const {
		return _engine.getNumParameters();
	}

	void ParameterInterfaceSync::getParameterInfo(ParameterIndex index, ParameterInfo* info) const {
		_engine.getParameterInfo(index, info);
	}

	ConstCharPointer ParameterInterfaceSync::getParameterName(ParameterIndex index) const {
		return _engine.getParameterName(index);
	}

	ConstCharPointer ParameterInterfaceSync::getParameterId(ParameterIndex index) const {
		return _engine.getParameterId(index);
	}

	ParameterValue ParameterInterfaceSync::getParameterValue(ParameterIndex index) {
		return _engine.getParameterValue(index);
	}

	void ParameterInterfaceSync::setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time) {
		scheduleEvent(ParameterEvent(index, time, value, this));
	}

	void ParameterInterfaceSync::scheduleEvent(EventVariant event)
	{
		const MillisecondTime currentTime = _engine.getCurrentTime();
		if (event.getTime() < currentTime) {
			event.setTime(currentTime);
		}

		if (event.isPresetEvent()) {
			PresetEvent pe = event.getPresetEvent();
			if (pe.getType() == PresetEvent::Get) {
				_engine.handlePresetEvent(pe);
			}
			else {
				_engine.scheduleEvent(event);
			}
		}
		else {
			_engine.scheduleEvent(event);
		}
	}

	void ParameterInterfaceSync::drainEvents() {
		// no need to drain events here - they have been pushed synchronously
	}

	void ParameterInterfaceSync::refreshParameterCountAndValues()
	{
		ParameterIndex paramCount = _engine.getNumParameters();
		_parameters.resize(paramCount);

		// update the values of all parameters
		for (ParameterIndex i = 0; i < paramCount; i++) {
			_parameters[i] = _engine.getPatcher().getParameterValue(i);
		}
	}

	void ParameterInterfaceSync::notifyParameterValueChanged(ParameterEvent sourceEvent, ParameterIndex index, ParameterValue value)
	{
		const auto source = sourceEvent.getSource();
		const ParameterEvent pe(index, sourceEvent.getTime(), value, source ? source : this);

		if (source == this && pe == sourceEvent && value == _parameters[index]) {
			// this notification matches the index, time, value, and is from this same source
			// so we do not care
			return;
		}

		pushOutgoingEvent(pe);
	}

	void ParameterInterfaceSync::drainIncomingQueueToEventList(EventList<EventVariant>& eventList, MillisecondTime currentTime) {
		RNBO_UNUSED(eventList);
		RNBO_UNUSED(currentTime);
		// nothing to do here, since we are synchronous, we have already scheduled the events to the engine
	}

	void ParameterInterfaceSync::pushDirtyParameters(MillisecondTime currentTime) {
		RNBO_UNUSED(currentTime);
		// nothing to do here, since we are synchronous, the parameter events have already
		// been scheduled in the _outgoingEvents array
	}

	void ParameterInterfaceSync::pushOutgoingEvent(EventVariant event) {
		if (_handler) {
			switch (event.getType()) {
				case Event::Parameter:
				{
					ParameterEvent pe = event.getParameterEvent();
					_parameters[pe.getIndex()] = pe.getValue();
					_handler->handleParameterEvent(pe);
					break;
				}
				case Event::Midi:
					_handler->handleMidiEvent(event.getMidiEvent());
					break;
#ifndef RNBO_NOMESSAGEEVENT
				case Event::Message:
					_handler->handleMessageEvent(event.getMessageEvent());
					break;
#endif
#ifndef RNBO_NOPRESETS
				case Event::Preset:
					_handler->handlePresetEvent(event.getPresetEvent());
					break;
#endif
				case Event::Transport:
					_handler->handleTransportEvent(event.getTransportEvent());
					break;
				case Event::Tempo:
					_handler->handleTempoEvent(event.getTempoEvent());
					break;
				case Event::BeatTime:
					_handler->handleBeatTimeEvent(event.getBeatTimeEvent());
					break;
				case Event::TimeSignature:
					_handler->handleTimeSignatureEvent(event.getTimeSignatureEvent());
					break;
				case Event::Startup:
					_handler->handleStartupEvent(event.getStartupEvent());
					break;
				case Event::Empty:
				case Event::Clock:
				case Event::DataRef:
				case Event::Outlet:
				case Event::Universal:
				default:
					// we are only interested in Parameter and Midi events
					break;
			}
		}
	}

	void ParameterInterfaceSync::notifyOutgoingEvents()
	{
		// nothing to be done here in the sync case
	}

	bool ParameterInterfaceSync::isActive() {
		return true;
	}

	void ParameterInterfaceSync::deactivate() {
		// synchronous parameter interfaces are always active from the engine point of view
	}

	void ParameterInterfaceSync::notifyParameterInterfaceDeleted()
	{
		// synchronous parameter interfaces are always active from the engine point of view
	}

	ParameterValue ParameterInterfaceSync::convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _engine.convertToNormalizedParameterValue(index, value);
	}

	ParameterValue ParameterInterfaceSync::convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const
	{
		return _engine.convertFromNormalizedParameterValue(index, normalizedValue);
	}

	ParameterValue ParameterInterfaceSync::constrainParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _engine.constrainParameterValue(index, value);
	}

	ParameterInterfaceTrigger::ParameterInterfaceTrigger(EngineCore& engine, EventHandler* handler)
	: ParameterInterfaceSync(engine, handler)
	{}

	void ParameterInterfaceTrigger::scheduleEvent(EventVariant event)
	{
		ParameterInterfaceSync::scheduleEvent(event);
		
		_engine.beginProcessDataRefs();
		_engine.processEventsUntil(event.getTime());
		_engine.endProcessDataRefs();
	}

	void ParameterInterfaceTrigger::setScheduleCallback(ScheduleCallback callback) {
		_engine.setScheduleCallback(callback);
	}
}
