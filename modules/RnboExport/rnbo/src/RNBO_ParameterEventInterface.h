#ifndef RNBO_ParameterEventInterface_h
#define RNBO_ParameterEventInterface_h

#include "RNBO_Types.h"
#include "RNBO_ParameterInterface.h"
#include "RNBO_List.h"
#include "RNBO_EventVariant.h"
#include "RNBO_UniquePtr.h"
#include "RNBO_EventList.h"

namespace RNBO {

	using ScheduleCallback = std::function<void(MillisecondTime)>;

    /**
     * The ParameterEventInterface is a lightweight interface class that can be handed around
     * to send and receive parameter values and events. It could be used, for example, by a
     * secondary UI or by a class that controls an external MIDI controller.
     */
	class ParameterEventInterface : public ParameterInterface {

		friend class EventHandler;

	public:

		virtual ~ParameterEventInterface() { }

		enum Type {
			SingleProducer,			///< a single thread queues parameter values for the audio thread
			MultiProducer,			///< multiple threads queue parameter values for the audio thread
			NotThreadSafe,			///< no queue -- good for special purposes when you know what you are doing
			                        ///< (for example, in a single threaded application with the SIMPLEENGINE flag)
			Trigger					///< no queue -- will trigger event processing on each event, for reduced latency
									///< (only use if you know what you are doing)
		};

		virtual void scheduleEvent(EventVariant event) = 0;

		void sendMessage(MessageTag tag, number payload, MessageTag objectId = 0, MillisecondTime eventTime = RNBOTimeNow)
		{
#ifdef RNBO_NOMESSAGEEVENT
			RNBO_ASSERT(false); // not support without std:lib
#else
			scheduleEvent(MessageEvent(tag, eventTime, payload, objectId));
#endif
		}

		void sendMessage(MessageTag tag, UniqueListPtr payload, MessageTag objectId = 0, MillisecondTime eventTime = RNBOTimeNow)
		{
#ifdef RNBO_NOMESSAGEEVENT
			RNBO_ASSERT(false); // not support without std:lib
#else
			scheduleEvent(MessageEvent(tag, eventTime, std::move(payload), objectId));
#endif
		}

		void sendMessage(MessageTag tag, MessageTag objectId = 0, MillisecondTime eventTime = RNBOTimeNow)
		{
#ifdef RNBO_NOMESSAGEEVENT
			RNBO_ASSERT(false); // not support without std:lib
#else
			scheduleEvent(MessageEvent(tag, eventTime, objectId));
#endif
		}

		virtual void setScheduleCallback(ScheduleCallback callback)
		{
			RNBO_UNUSED(callback)
			// allows you to get a synchronous notification whenever a clock
			// event is scheduled, makes only sense in combination with a
			// sync trigger parameter interface
			RNBO_ASSERT(false);
		}

	private:

		// your handler will call drain (preferrably in an async manner) to receive event callbacks
		virtual void drainEvents() = 0;

	};

	/**
	 * @private
	 */
	class ParameterEventInterfaceImpl : public ParameterEventInterface {

	public:

		virtual ~ParameterEventInterfaceImpl() { }

		virtual void refreshParameterCountAndValues() = 0;
		virtual void notifyParameterValueChanged(ParameterEvent sourceEvent, ParameterIndex index, ParameterValue value) = 0;
		virtual void drainIncomingQueueToEventList(EventList<EventVariant>& eventList, MillisecondTime currentTime) = 0;
		virtual void pushDirtyParameters(MillisecondTime currentTime) = 0;
		virtual void pushOutgoingEvent(EventVariant event) = 0;
		virtual void notifyOutgoingEvents() = 0;

		virtual void drainEvents() = 0;

		// those are needed for the async case
		virtual bool isActive() = 0;
		virtual void deactivate() = 0;
		virtual void notifyParameterInterfaceDeleted() = 0;

	protected:

		bool _eventsPushed = false;

	};

	/**
	 * @brief A base class for handling different kinds of RNBO events.
	 * 
	 * You can choose to implement as many or as few of the event handlers
	 * available from this class.
	 */
	class EventHandler {

	public:

		virtual ~EventHandler() {}

		// TAKE CARE if you drain events here directly, you will block the audio thread
		virtual void eventsAvailable() = 0;

		// your actual event handlers, implement the ones below to get the events in your handler
		virtual void handleParameterEvent(const ParameterEvent& event) { RNBO_UNUSED(event); }
		virtual void handleMidiEvent(const MidiEvent& event) { RNBO_UNUSED(event); }
		virtual void handleMessageEvent(const MessageEvent& event) { RNBO_UNUSED(event); }
		virtual void handlePresetEvent(const PresetEvent& event) { RNBO_UNUSED(event); }
		virtual void handleTempoEvent(const TempoEvent& event) { RNBO_UNUSED(event); }
		virtual void handleTransportEvent(const TransportEvent& event) { RNBO_UNUSED(event); }
		virtual void handleBeatTimeEvent(const BeatTimeEvent& event) { RNBO_UNUSED(event); }
		virtual void handleTimeSignatureEvent(const TimeSignatureEvent& event) { RNBO_UNUSED(event); }
		virtual void handleStartupEvent(const StartupEvent& event) { RNBO_UNUSED(event); }

		/**
		 * @brief Connect this handler class with a ParameterEventInterface
		 * 
		 * @param peInterface the interface to connect to
		 */
		void linkToParameterEventInterface(ParameterEventInterface* peInterface) {
			//we either need to be unsetting the _peInterface or setting a null
			//_peInterface, otherwise we're silently dropping an interface and
			//drainEvents won't ever be called on the previous interface
			RNBO_ASSERT(peInterface == nullptr || _peInterface == nullptr);
			_peInterface = peInterface;
		}

	protected:

		// call this in a deferred manner from eventsAvailable (NOT IN THE AUDIO THREAD)
		void drainEvents() {
			if (_peInterface) {
				_peInterface->drainEvents();
			}
		}

	private:

		ParameterEventInterface* _peInterface = nullptr;

	};

	using ParameterEventInterfaceUniquePtr = UniquePtr<ParameterEventInterface>;

} // namespace RNBO

#endif // RNBO_ParameterEventInterface_h
