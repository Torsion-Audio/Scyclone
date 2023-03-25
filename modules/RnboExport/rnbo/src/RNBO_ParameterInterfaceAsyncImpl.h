#ifndef RNBOPlugin_ParameterInterfaceAsyncImpl_h
#define RNBOPlugin_ParameterInterfaceAsyncImpl_h

#include "RNBO_ParameterEventQueue.h"

#ifdef RNBO_DEBUG
#define RNBO_DRAIN_THRESHOLD 1024
#endif

namespace RNBO {

	/**
	 * @private
	 */
	class ShadowValue {
	public:
		void operator=(const ParameterValue value) {
			_value = value;
		}

		operator ParameterValue() {
			return _value;
		}

	private:
		ParameterValue _value;
	};

	/**
	 * @private
	 * you cannot hold atomic values directly in a vector (they are not copy-constructible)
	 * so we need to wrap them here - this will make the access (reading and writing of shadow values)
	 * atomic but NOT the vector itself, this must not be edited while setting values
	 */
	class AtomicShadowValue
	{
	public:
		AtomicShadowValue(ParameterValue v = 0.0)
		: _value(v)
		{
#ifndef RNBO_ATOMIC_ALLOW_LOCK
			// If you hit this assert it means that atomic<ParameterValue> is not lock free on the platform that you're running
			// RNBO might run just fine even with this locking, but there is a potential performance hit (locking in the audio processing thread).
			// You can disable the assert and run RNBO by defining RNBO_ATOMIC_ALLOW_LOCK
			RNBO_ASSERT(std::atomic_is_lock_free(&_value))
#endif
		}

		AtomicShadowValue(const std::atomic<ParameterValue> &value)
		: AtomicShadowValue(value.load())
		{}

		AtomicShadowValue(const AtomicShadowValue &other)
		: AtomicShadowValue(other._value.load())
		{}

		AtomicShadowValue &operator=(const AtomicShadowValue &other)
		{
			_value.store(other._value.load());
			return *this;
		}

		void operator=(const ParameterValue value) {
			_value = value;
		}

		operator ParameterValue() {
			return _value;
		}

	private:

		std::atomic<ParameterValue> _value;
	};

	/**
	 * @private
	 */
	template <typename ShadowValue> class ParameterInterfaceAsyncImpl : public ParameterEventInterfaceImpl {
	public:
		ParameterInterfaceAsyncImpl(RNBO::Engine& engine, EventHandler* handler, ParameterEventQueuePtr incomingQueue, ParameterEventQueuePtr outgoingQueue)
		: _engine(engine)
		, _incomingQueue(std::move(incomingQueue))
		, _outgoingQueue(std::move(outgoingQueue))
		, _handler(handler)
		, _minimumParameterChangeNotificationPeriod(15.)	// minimum time, in milliseconds, between each outgoing parameter change notification
		, _lastOutgoingParameterUpdateTime(-1)	// negative indicates - do it immediately
		{
			_active = true;

			// this interface needs queues
			RNBO_ASSERT(_outgoingQueue);
			RNBO_ASSERT(_incomingQueue);

			refreshParameterCountAndValues();
			_engine.queueServiceEvent(ServiceNotification(ServiceNotification::ParameterInterfaceCreated, this));
		}

		// ------------- user facing functions mirrored from ParameterInterface

		ParameterIndex getNumParameters() const override
		{
			return _engine.getNumParameters();
		}

		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override
		{
			_engine.getParameterInfo(index, info);
		}

		ConstCharPointer getParameterName(ParameterIndex index) const override
		{
			return _engine.getParameterName(index);
		}

		ConstCharPointer getParameterId(ParameterIndex index) const override
		{
			return _engine.getParameterId(index);
		}

		ParameterValue getParameterValue(ParameterIndex index) override
		{
			return _parameters[index];
		}

		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time) override
		{
			value = constrainParameterValue(index, value);
			ParameterEvent	pe(index, time, value, this);
			setParameter(pe);
		}

		void scheduleEvent(EventVariant event) override
		{
			if (event.isParameterEvent()) {
				setParameter(event.getParameterEvent());
			}
			else {
				pushIncomingEvent(event);
			}
		}

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _engine.convertToNormalizedParameterValue(index, value);
		}

		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override
		{
			return _engine.convertFromNormalizedParameterValue(index, normalizedValue);
		}

		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _engine.constrainParameterValue(index, value);
		}

		// ------------- internal and Engine functions

		// used by engine to empty the incoming queue of this parameter interface
		void drainIncomingQueueToEventList(EventList<EventVariant>& eventList, MillisecondTime currentTime) override
		{
			EventVariant ev;
			while (_incomingQueue->dequeue(ev)) {
				// as long as we do not have a decent understanding of global time, there might be events with a time
				// of earlier than now (for example 0 to indicate to schedule them now), rewrite those to now
				if (ev.getTime() < currentTime) {
					ev.setTime(currentTime);
					eventList.addEvent(ev);
				} else {
					eventList.addEvent(ev);
				}
			}
		}

		// trigger events for dirty parameters
		void pushDirtyParameters(MillisecondTime currentTime) override
		{
			if (_lastOutgoingParameterUpdateTime < 0 || currentTime >= _lastOutgoingParameterUpdateTime + _minimumParameterChangeNotificationPeriod) {
				ParameterIndex paramCount = ParameterIndex(_lastOutgoingParameterEvents.size());
				for (ParameterIndex paramIndex = 0; paramIndex < paramCount; paramIndex++) {
					if (_lastOutgoingParameterEvents[paramIndex].isValid()) {
						pushOutgoingEvent(_lastOutgoingParameterEvents[paramIndex]);
						_lastOutgoingParameterEvents[paramIndex].invalidate();
					}
				}

				_lastOutgoingParameterUpdateTime = currentTime;
			}
		}

		// used by the engine to put outgoing events on the queue.
		void pushOutgoingEvent(EventVariant event) override
		{
			if (_handler) {
				_outgoingQueue->enqueue(event);
				_eventsPushed = true;
			}
			else {
				if (event.isParameterEvent()) {
					updateShadowValue(event.getParameterEvent());
				}
			}
		}


		void notifyParameterValueChanged(ParameterEvent sourceEvent, ParameterIndex index, ParameterValue value) override
		{
			if (index >= 0 && index < ParameterIndex(_lastOutgoingParameterEvents.size())) {
				_lastOutgoingParameterEvents[index] = ParameterEvent(index, sourceEvent.getTime(), value, sourceEvent.getSource());
			}
		}

		void notifyOutgoingEvents() override
		{
			if (_eventsPushed && _handler) {
#ifdef RNBO_DEBUG
//				RNBO_ASSERT(_drainEventsCounter < RNBO_DRAIN_THRESHOLD)
				_drainEventsCounter++;
#endif
				_handler->eventsAvailable();
			}

			_eventsPushed = false;
		}

		// the audio thread will deactivate a parameter interface after it removes it from its
		// list of interfaces, this indicates that it is save to free the interface
		bool isActive() override { return _active;}
		void deactivate() override { _active = false; }

		void notifyParameterInterfaceDeleted() override
		{
			_engine.queueServiceEvent(ServiceNotification(ServiceNotification::ParameterInterfaceDeleted, this));
		}

		void refreshParameterCountAndValues() override
		{
			ParameterIndex paramCount = _engine.getNumParameters();
			_parameters.resize(paramCount);

			// setup the dirty parameters vector
			_lastOutgoingParameterEvents.resize(paramCount);

			for (ParameterIndex i = 0; i < paramCount; i++) {
				_lastOutgoingParameterEvents[i].invalidate();
			}

			// update the values of all parameters
			for (ParameterIndex i = 0; i < paramCount; i++) {
				_parameters[i] = _engine.getPatcher().getParameterValue(i);
			}

			_lastOutgoingParameterUpdateTime = -1;
		}

		// used by the interface itself to push incoming events onto the queue
		void pushIncomingEvent(EventVariant event)
		{
			_incomingQueue->enqueue(event);
		}

		void drainEvents() override
		{
			RNBO_ASSERT(_handler);
#ifdef RNBO_DEBUG
			_drainEventsCounter = 0;
#endif

			EventVariant ev;
			while (_outgoingQueue->dequeue(ev)) {
				switch (ev.getType()) {
					case Event::Parameter:
					{
						ParameterEvent pe = ev.getParameterEvent();
						if (pe.getSource() == this) {
							const bool sameValue = pe.getValue() == _parameters[pe.getIndex()];
							if (!sameValue) {
								updateShadowValue(pe);
								_handler->handleParameterEvent(pe);
							}
						}
						else {
							updateShadowValue(pe);
							_handler->handleParameterEvent(pe);
						}
						break;
					}
					case Event::Midi:
						_handler->handleMidiEvent(ev.getMidiEvent());
						break;
#ifndef RNBO_NOMESSAGEEVENT
					case Event::Message:
						_handler->handleMessageEvent(ev.getMessageEvent());
						break;
#endif
#ifndef RNBO_NOPRESETS
					case Event::Preset:
						_handler->handlePresetEvent(ev.getPresetEvent());
						break;
#endif
					case Event::Transport:
						_handler->handleTransportEvent(ev.getTransportEvent());
						break;
					case Event::Tempo:
						_handler->handleTempoEvent(ev.getTempoEvent());
						break;
					case Event::BeatTime:
						_handler->handleBeatTimeEvent(ev.getBeatTimeEvent());
						break;
					case Event::TimeSignature:
						_handler->handleTimeSignatureEvent(ev.getTimeSignatureEvent());
						break;
					case Event::Startup:
						_handler->handleStartupEvent(ev.getStartupEvent());
						break;
					case Event::Empty:
					case Event::Clock:
					case Event::DataRef:
					case Event::Outlet:
					case Event::Universal:
					default:
						break;
				}
			}
		}

	private:

		// we have a "shadow" value that reflects the current value of the parameter
		// it is used for quick access to the parameter value
		void updateShadowValue(ParameterEvent event)
		{
			RNBO_ASSERT(event.getIndex() >= 0 && event.getIndex() < _parameters.size())
			_parameters[event.getIndex()] = event.getValue();
		}

		void setParameter(ParameterEvent pe)
		{
			ParameterIndex index = pe.getIndex();
			if (index >= 0 && index < ParameterIndex(_parameters.size())) {
				_parameters[index] = pe.getValue();
			}
			pushIncomingEvent(pe);
		}

		Engine&										_engine;
		ParameterEventQueuePtr						_incomingQueue;
		ParameterEventQueuePtr						_outgoingQueue;

		EventHandler * const						_handler = nullptr;

		std::atomic_bool				_active;

		// Shadow copies of values for this client.
		std::vector<ShadowValue>		_parameters;
		std::vector<ParameterEvent>		_lastOutgoingParameterEvents;
		MillisecondTime					_minimumParameterChangeNotificationPeriod;
		MillisecondTime					_lastOutgoingParameterUpdateTime;

#ifdef RNBO_DEBUG
		short							_drainEventsCounter = 0;
#endif
	};

} // namespace RNBO

#endif
