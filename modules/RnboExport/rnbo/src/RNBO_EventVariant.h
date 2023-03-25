//
//  RNBO_EventVariant.h
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_EventVariant_H_
#define _RNBO_EventVariant_H_

#include "RNBO_Types.h"

// internal events
#include "RNBO_EmptyEvent.h"		// internal only
#include "RNBO_ClockEvent.h"		// schedule an internal Clock
#include "RNBO_DataRefEvent.h"		// update DataRefs
#include "RNBO_OutletEvent.h"		// sending data out of an external outlet
#include "RNBO_UniversalEvent.h"	// communicate hardware mappings
#include "RNBO_StartupEvent.h"		// indicates start/end of patcher initialization phase

// external events
#include "RNBO_MidiEvent.h"			// a MIDI event
#include "RNBO_ParameterEvent.h"	// a parameter change at a specific time
#include "RNBO_MessageEvent.h"		// a number, list or bang message to/from the engine
#include "RNBO_PresetEvent.h"		// getting/setting presets and touched notifications
#include "RNBO_TempoEvent.h"		// setting the tempo
#include "RNBO_TransportEvent.h"	// start/stop transport
#include "RNBO_BeatTimeEvent.h"		// set the beattime
#include "RNBO_TimeSignatureEvent.h"	// set the time signature

#ifndef RNBO_NOSTDLIB
// using an API compatible backport of the C++17 std::variant class
#include "3rdparty/MPark_variant/variant.hpp"

#endif // RNBO_NOSTDLIB

namespace RNBO {

struct Event {

	// Events must implement:
	// public:
	// MillisecondTime getTime() const
	// void dumpEvent() const
	// private:
	// void setTime(MillisecondTime)

	enum Type {
		Empty = 0,
		Clock,
		DataRef,
		Midi,
		Outlet,
		Parameter,
		Universal,
		Message,
		Preset,
		Tempo,
		Transport,
		BeatTime,
		TimeSignature,
		Startup
	};
};

#ifndef RNBO_NOSTDLIB

    /**
     * @private
	 *
     * The EventVariant provides a way to hold multiple event types inside of
     * one list. This makes it possible to sort them by time and call the
     * correct graph entry points during iteration in sequence regardless of
     * event type.
     */
	class EventVariant {

	public:

		EventVariant() {}

		template<class T> EventVariant(T event)
		: _event(event)
		{}

		/**
		 * @private
		 */
		struct GetTypeVisitor {
			constexpr Event::Type operator()(const EmptyEvent&) const { return Event::Empty; }
			constexpr Event::Type operator()(const ClockEvent&) const { return Event::Clock; }
			constexpr Event::Type operator()(const DataRefEvent&) const { return Event::DataRef; }
			constexpr Event::Type operator()(const MidiEvent&) const { return Event::Midi; }
			constexpr Event::Type operator()(const OutletEvent&) const { return Event::Outlet; }
			constexpr Event::Type operator()(const ParameterEvent&) const { return Event::Parameter; }
			constexpr Event::Type operator()(const UniversalEvent&) const { return Event::Universal; }
			constexpr Event::Type operator()(const MessageEvent&) const { return Event::Message; }
			constexpr Event::Type operator()(const PresetEvent&) const { return Event::Preset; }
			constexpr Event::Type operator()(const TempoEvent&) const { return Event::Tempo; }
			constexpr Event::Type operator()(const TransportEvent&) const { return Event::Transport; }
			constexpr Event::Type operator()(const BeatTimeEvent&) const { return Event::BeatTime; }
			constexpr Event::Type operator()(const TimeSignatureEvent&) const { return Event::TimeSignature; }
			constexpr Event::Type operator()(const StartupEvent&) const { return Event::Startup; }
		};

		Event::Type getType() const {
			return mpark::visit(GetTypeVisitor{}, _event);
		}

		bool isEmptyEvent() const { return getType() == Event::Empty; }
		bool isClockEvent() const { return getType() == Event::Clock; }
		bool isDataRefEvent() const { return getType() == Event::DataRef; }
		bool isMidiEvent() const { return getType() == Event::Midi; }
		bool isOutletEvent() const { return getType() == Event::Outlet; }
		bool isParameterEvent() const { return getType() == Event::Parameter; }
		bool isUniversalEvent() const { return getType() == Event::Universal; }
		bool isMessageEvent() const { return getType() == Event::Message; }
		bool isPresetEvent() const { return getType() == Event::Preset; }
		bool isTempoEvent() const { return getType() == Event::Tempo; }
		bool isTransportEvent() const { return getType() == Event::Transport; }
		bool isBeatTimeEvent() const { return getType() == Event::BeatTime; }
		bool isTimeSignatureEvent() const { return getType() == Event::TimeSignature; }
		bool isStartupEvent() const { return getType() == Event::Startup; }

		const EmptyEvent& getEmptyEvent() const {
			return mpark::get<EmptyEvent>(_event);
		}

		const ClockEvent& getClockEvent() const {
			return mpark::get<ClockEvent>(_event);
		}

		const DataRefEvent& getDataRefEvent() const {
			return mpark::get<DataRefEvent>(_event);
		}

		const MidiEvent& getMidiEvent() const {
			return mpark::get<MidiEvent>(_event);
		}

		const OutletEvent& getOutletEvent() const {
			return mpark::get<OutletEvent>(_event);
		}

		const ParameterEvent& getParameterEvent() const {
			return mpark::get<ParameterEvent>(_event);
		}

		const UniversalEvent& getUniversalEvent() const {
			return mpark::get<UniversalEvent>(_event);
		}

		const MessageEvent& getMessageEvent() const {
			return mpark::get<MessageEvent>(_event);
		}

		const PresetEvent& getPresetEvent() const {
			return mpark::get<PresetEvent>(_event);
		}

		const TempoEvent& getTempoEvent() const {
			return mpark::get<TempoEvent>(_event);
		}

		const TransportEvent getTransportEvent() const {
			return mpark::get<TransportEvent>(_event);
		}

		const BeatTimeEvent& getBeatTimeEvent() const {
			return mpark::get<BeatTimeEvent>(_event);
		}

		const TimeSignatureEvent& getTimeSignatureEvent() const {
			return mpark::get<TimeSignatureEvent>(_event);
		}

		const StartupEvent& getStartupEvent() const {
			return mpark::get<StartupEvent>(_event);
		}

		bool operator==(const EventVariant& rhs) const {
			return _event == rhs._event;
		}

		/**
		 * @private
		 */
		struct GetTimeVisitor {
			template <typename T> constexpr MillisecondTime operator()(const T& event) const { return event.getTime(); }
		};

		MillisecondTime getTime() const {
			return mpark::visit(GetTimeVisitor{}, _event);
		}

		/**
		 * @private
		 */
		struct GetEventTargetVisitor {
			template <typename T> constexpr EventTarget* operator()(const T& event) const { return static_cast<EventTarget*>(event.getEventTarget()); }
		};

		EventTarget* getEventTarget() const {
			return mpark::visit(GetEventTargetVisitor{}, _event);
		}

		/**
		 * @private
		 */
		struct DumpEventVisitor {
			template <typename T> void operator()(const T& event) const { event.dumpEvent(); }
		};

		void dumpEvent() const {
			mpark::visit(DumpEventVisitor{}, _event);
		}

		/**
		 * @private
		 */
		struct SetTimeVisitor {
			SetTimeVisitor(const MillisecondTime& time) : _time(time) {}
			template <typename T> void operator()(T& event) const { event.setTime(_time); }
			const MillisecondTime& _time = 0;
		};

		void setTime(MillisecondTime time) {
			mpark::visit(SetTimeVisitor{time}, _event);
		}

		/**
		 * @private
		 */
		struct SetEventTargetVisitor {
			SetEventTargetVisitor(PatcherEventTarget* eventTarget) : _eventTarget(eventTarget) {}
			void operator()(PresetEvent&) const {}
			void operator()(BeatTimeEvent&) const {}
			void operator()(TimeSignatureEvent&) const {}
			void operator()(TempoEvent&) const {}
			void operator()(TransportEvent&) const {}
			void operator()(StartupEvent&) const {}

			template <typename T> void operator()(T& event) const { event._eventTarget = _eventTarget; }
			PatcherEventTarget* _eventTarget = nullptr;
		};

		void setPatcherEventTarget(PatcherEventTarget* eventTarget) {
			mpark::visit(SetEventTargetVisitor{eventTarget}, _event);
		}

	private:

		friend class EventSender;
		friend class PatcherEventSender;

		mpark::variant<
			EmptyEvent,
			ClockEvent,
			DataRefEvent,
			MidiEvent,
			OutletEvent,
			ParameterEvent,
			UniversalEvent,
			MessageEvent,
			PresetEvent,
			TempoEvent,
			TransportEvent,
			BeatTimeEvent,
			TimeSignatureEvent,
			StartupEvent
		>	_event;
	};

#else

	class EventVariant {

		// Events MUST NOT DERIVE from a base class or the EventVariant will break.
		// Events can only contain non-object types or structs containing such.

	public:

		EventVariant()
		{}

		~EventVariant()
		{
			clear();
		}

		void clear() {
			if (_eventType == Event::Message) {
				_messageEvent.~MessageEvent();
			}
			else if (_eventType == Event::Preset) {
				_presetEvent.~PresetEvent();
			}

			_eventType = Event::Empty;
		}

		EventVariant(const EventVariant& other)
		{
			RNBO_ASSERT(_eventType == Event::Empty)
			// copy the event contents
			switch (other._eventType) {
				case Event::Empty: _emptyEvent = other._emptyEvent; break;
				case Event::Clock: _clockEvent = other._clockEvent; break;
				case Event::DataRef: _dataRefEvent = other._dataRefEvent; break;
				case Event::Midi: _midiEvent = other._midiEvent; break;
				case Event::Outlet: _outletEvent = other._outletEvent; break;
				case Event::Parameter: _parameterEvent = other._parameterEvent; break;
				case Event::Universal: _universalEvent = other._universalEvent; break;
				case Event::Message:
					// since this event contains a non-trivial member, we need to construct it first
					new (&_messageEvent) MessageEvent(other._messageEvent);
					break;
				case Event::Preset:
					new (&_presetEvent) PresetEvent(other._presetEvent);
					break;
				case Event::Tempo: _tempoEvent = other._tempoEvent; break;
				case Event::Transport: _transportEvent = other._transportEvent; break;
				case Event::BeatTime: _beatTimeEvent = other._beatTimeEvent; break;
				case Event::TimeSignature: _timeSignatureEvent = other._timeSignatureEvent; break;
				case Event::Startup: _startupEvent = other._startupEvent; break;
				// default: // no default case to ensure all cases
			}

			_eventType = other.getType();
		}

		EventVariant(Event::Type type)
		: _eventType(type)
		{
			if (type == Event::Message) {
				new (&_messageEvent) MessageEvent();
			}
			else if (type == Event::Preset) {
				new (&_presetEvent) PresetEvent();
			}
		}

		EventVariant& operator= (const EventVariant& other) {
			if (&other != this) {
				clear();

				switch (other._eventType) {
					case Event::Empty: _emptyEvent = other._emptyEvent; break;
					case Event::Clock: _clockEvent = other._clockEvent; break;
					case Event::DataRef: _dataRefEvent = other._dataRefEvent; break;
					case Event::Midi: _midiEvent = other._midiEvent; break;
					case Event::Outlet: _outletEvent = other._outletEvent; break;
					case Event::Parameter: _parameterEvent = other._parameterEvent; break;
					case Event::Universal: _universalEvent = other._universalEvent; break;
					case Event::Message:
						new (&_messageEvent) MessageEvent(other._messageEvent);
						break;
					case Event::Preset:
						new (&_presetEvent) PresetEvent(other._presetEvent);
						break;
					case Event::Tempo: _tempoEvent = other._tempoEvent; break;
					case Event::Transport: _transportEvent = other._transportEvent; break;
					case Event::BeatTime: _beatTimeEvent = other._beatTimeEvent; break;
					case Event::TimeSignature: _timeSignatureEvent = other._timeSignatureEvent; break;
					case Event::Startup: _startupEvent = other._startupEvent; break;
					// default: // no default case to ensure all cases
				}
			}
			return *this;
		}

		EventVariant(EventVariant&& other)
		{
			RNBO_ASSERT(_eventType == Event::Empty)

			switch (other._eventType) {
				case Event::Empty: _emptyEvent = other._emptyEvent; break;
				case Event::Clock: _clockEvent = other._clockEvent; break;
				case Event::DataRef: _dataRefEvent = other._dataRefEvent; break;
				case Event::Midi: _midiEvent = other._midiEvent; break;
				case Event::Outlet: _outletEvent = other._outletEvent; break;
				case Event::Parameter: _parameterEvent = other._parameterEvent; break;
				case Event::Universal: _universalEvent = other._universalEvent; break;
				case Event::Message:
					new (&_messageEvent) MessageEvent(std::move(other._messageEvent));
					break;
				case Event::Preset:
					new (&_presetEvent) PresetEvent(std::move(other._presetEvent));
					break;
				case Event::Tempo: _tempoEvent = other._tempoEvent; break;
				case Event::Transport: _transportEvent = other._transportEvent; break;
				case Event::BeatTime: _beatTimeEvent = other._beatTimeEvent; break;
				case Event::TimeSignature: _timeSignatureEvent = other._timeSignatureEvent; break;
				case Event::Startup: _startupEvent = other._startupEvent; break;
				// default: // no default case to ensure all cases
			}
			_eventType = other._eventType;
			other._eventType = Event::Empty;
		}

		EventVariant& operator = (EventVariant&& other)
		{
			if (this != &other) {
				clear();

				switch (other._eventType)
				{
				case Event::Empty: _emptyEvent = std::move(other._emptyEvent); break;
				case Event::Clock: _clockEvent = std::move(other._clockEvent); break;
				case Event::DataRef: _dataRefEvent = std::move(other._dataRefEvent); break;
				case Event::Midi: _midiEvent = std::move(other._midiEvent); break;
				case Event::Outlet: _outletEvent = std::move(other._outletEvent); break;
				case Event::Parameter: _parameterEvent = std::move(other._parameterEvent); break;
				case Event::Universal: _universalEvent = std::move(other._universalEvent); break;
				case Event::Message:
					new (&_messageEvent) MessageEvent(std::move(other._messageEvent));
					break;
				case Event::Preset:
					new (&_presetEvent) PresetEvent(std::move(other._presetEvent));
					break;
				case Event::Tempo: _tempoEvent = std::move(other._tempoEvent); break;
				case Event::Transport: _transportEvent = std::move(other._transportEvent); break;
				case Event::BeatTime: _beatTimeEvent = std::move(other._beatTimeEvent); break;
				case Event::TimeSignature: _timeSignatureEvent = std::move(other._timeSignatureEvent); break;
				case Event::Startup: _startupEvent = std::move(other._startupEvent); break;
				}

				_eventType = other._eventType;
				other._eventType = Event::Empty;
			}
			return *this;
		}

		EventVariant(const EmptyEvent& emptyEvent)
		: _eventType(Event::Empty)
		{
			_emptyEvent = emptyEvent;
		}

		EventVariant& operator=(EmptyEvent emptyEvent)
		{
			if (_eventType != Event::Empty)
			{
				this->~EventVariant();
				new (this) EventVariant(emptyEvent);
			}
			else
			{
				_emptyEvent = emptyEvent;
			}
			return *this;
		}

		EventVariant(const ClockEvent& clockEvent)
		: _eventType(Event::Clock)
		{
			_clockEvent = clockEvent;
		}

		EventVariant& operator=(ClockEvent clockEvent)
		{
			if (_eventType != Event::Clock)
			{
				this->~EventVariant();
				new (this) EventVariant(clockEvent);
			}
			else
			{
				_clockEvent = clockEvent;
			}
			return *this;
		}

		EventVariant(const DataRefEvent& dataRefEvent)
		: _eventType(Event::DataRef)
		{
			_dataRefEvent = dataRefEvent;
		}

		EventVariant& operator=(DataRefEvent dataRefEvent)
		{
			if (_eventType != Event::DataRef)
			{
				this->~EventVariant();
				new (this) EventVariant(dataRefEvent);
			}
			else
			{
				_dataRefEvent = dataRefEvent;
			}
			return *this;
		}

		EventVariant(const MidiEvent& midiEvent)
		: _eventType(Event::Midi)
		{
			_midiEvent = midiEvent;
		}

		EventVariant& operator=(MidiEvent midiEvent)
		{
			if (_eventType != Event::Midi)
			{
				this->~EventVariant();
				new (this) EventVariant(midiEvent);
			}
			else
			{
				_midiEvent = midiEvent;
			}
			return *this;
		}

		EventVariant(const OutletEvent& outletEvent)
		: _eventType(Event::Outlet)
		{
			_outletEvent = outletEvent;
		}

		EventVariant& operator=(OutletEvent outletEvent)
		{
			if (_eventType != Event::Outlet)
			{
				this->~EventVariant();
				new (this) EventVariant(outletEvent);
			}
			else
			{
				_outletEvent = outletEvent;
			}
			return *this;
		}

		EventVariant(const ParameterEvent& parameterEvent)
		: _eventType(Event::Parameter)
		{
			_parameterEvent = parameterEvent;
		}

		EventVariant& operator=(ParameterEvent parameterEvent)
		{
			if (_eventType != Event::Parameter)
			{
				this->~EventVariant();
				new (this) EventVariant(parameterEvent);
			}
			else
			{
				_parameterEvent = parameterEvent;
			}
			return *this;
		}

		EventVariant(const UniversalEvent& universalEvent)
		: _eventType(Event::Universal)
		{
			_universalEvent = universalEvent;
		}

		EventVariant& operator=(UniversalEvent universalEvent)
		{
			if (_eventType != Event::Universal)
			{
				this->~EventVariant();
				new (this) EventVariant(universalEvent);
			}
			else
			{
				_universalEvent = universalEvent;
			}
			return *this;
		}

		EventVariant(const MessageEvent& messageEvent)
		: _eventType(Event::Message)
		, _messageEvent(messageEvent)
		{
		}

		EventVariant& operator=(const MessageEvent& messageEvent)
		{
			this->~EventVariant();
			new (this) EventVariant(messageEvent);
			return *this;
		}

		EventVariant(MessageEvent&& messageEvent)
			: _eventType(Event::Message)
			, _messageEvent(std::move(messageEvent))
		{
		}

		EventVariant& operator=(MessageEvent&& messageEvent)
		{
			this->~EventVariant();
			new (this) EventVariant(std::move(messageEvent));
			return *this;
		}

		EventVariant(const PresetEvent& presetEvent)
		: _eventType(Event::Preset)
		, _presetEvent(presetEvent)
		{
		}

		EventVariant& operator=(const PresetEvent& presetEvent)
		{
			this->~EventVariant();
			new (this) EventVariant(presetEvent);
			return *this;
		}

		EventVariant(PresetEvent&& presetEvent)
			: _eventType(Event::Preset)
			, _presetEvent(std::move(presetEvent))
		{
		}

		EventVariant& operator=(PresetEvent&& presetEvent)
		{
			this->~EventVariant();
			new (this) EventVariant(std::move(presetEvent));
			return *this;
		}

		EventVariant(const TempoEvent& tempoEvent)
		: _eventType(Event::Tempo)
		{
			_tempoEvent = tempoEvent;
		}

		EventVariant& operator=(TempoEvent tempoEvent)
		{
			if (_eventType != Event::Tempo)
			{
				this->~EventVariant();
				new (this) EventVariant(tempoEvent);
			}
			else
			{
				_tempoEvent = tempoEvent;
			}
			return *this;
		}

		EventVariant(const TransportEvent& transportEvent)
		: _eventType(Event::Transport)
		{
			_transportEvent = transportEvent;
		}

		EventVariant& operator=(TransportEvent transportEvent)
		{
			if (_eventType != Event::Transport)
			{
				this->~EventVariant();
				new (this) EventVariant(transportEvent);
			}
			else
			{
				_transportEvent = transportEvent;
			}
			return *this;
		}

		EventVariant(const BeatTimeEvent& beatTimeEvent)
		: _eventType(Event::BeatTime)
		{
			_beatTimeEvent = beatTimeEvent;
		}

		EventVariant& operator=(BeatTimeEvent beatTimeEvent)
		{
			if (_eventType != Event::BeatTime)
			{
				this->~EventVariant();
				new (this) EventVariant(beatTimeEvent);
			}
			else
			{
				_beatTimeEvent = beatTimeEvent;
			}
			return *this;
		}

		EventVariant(const TimeSignatureEvent& timeSignatureEvent)
		: _eventType(Event::TimeSignature)
		{
			_timeSignatureEvent = timeSignatureEvent;
		}

		EventVariant& operator=(TimeSignatureEvent timeSignatureEvent)
		{
			if (_eventType != Event::TimeSignature)
			{
				this->~EventVariant();
				new (this) EventVariant(timeSignatureEvent);
			}
			else
			{
				_timeSignatureEvent = timeSignatureEvent;
			}
			return *this;
		}


		EventVariant(const StartupEvent& startupEvent)
		: _eventType(Event::Startup)
		{
			_startupEvent = startupEvent;
		}

		EventVariant& operator=(StartupEvent startupEvent)
		{
			if (_eventType != Event::Startup)
			{
				this->~EventVariant();
				new (this) EventVariant(startupEvent);
			}
			else
			{
				_startupEvent = startupEvent;
			}
			return *this;
		}


		Event::Type getType() const { return _eventType; }

		bool isEmptyEvent() const { return getType() == Event::Empty; }
		bool isClockEvent() const { return getType() == Event::Clock; }
		bool isDataRefEvent() const { return getType() == Event::DataRef; }
		bool isMidiEvent() const { return getType() == Event::Midi; }
		bool isOutletEvent() const { return getType() == Event::Outlet; }
		bool isParameterEvent() const { return getType() == Event::Parameter; }
		bool isUniversalEvent() const { return getType() == Event::Universal; }
		bool isMessageEvent() const { return getType() == Event::Message; }
		bool isPresetEvent() const { return getType() == Event::Preset; }
		bool isTempoEvent() const { return getType() == Event::Tempo; }
		bool isTransportEvent() const { return getType() == Event::Transport; }
		bool isBeatTimeEvent() const { return getType() == Event::BeatTime; }
		bool isTimeSignatureEvent() const { return getType() == Event::TimeSignature; }
		bool isStartupEvent() const { return getType() == Event::Startup; }

		const EmptyEvent& getEmptyEvent() const {
			RNBO_ASSERT(isEmptyEvent())
			return _emptyEvent;
		}

		const ClockEvent& getClockEvent() const {
			RNBO_ASSERT(isClockEvent())
			return _clockEvent;
		}

		const DataRefEvent& getDataRefEvent() const {
			RNBO_ASSERT(isDataRefEvent())
			return _dataRefEvent;
		}

		const MidiEvent& getMidiEvent() const {
			RNBO_ASSERT(isMidiEvent())
			return _midiEvent;
		}

		const OutletEvent& getOutletEvent() const {
			RNBO_ASSERT(isOutletEvent())
			return _outletEvent;
		}

		const ParameterEvent& getParameterEvent() const {
			RNBO_ASSERT(isParameterEvent())
			return _parameterEvent;
		}

		const UniversalEvent& getUniversalEvent() const {
			RNBO_ASSERT(isUniversalEvent())
			return _universalEvent;
		}

		const MessageEvent& getMessageEvent() const {
			RNBO_ASSERT(isMessageEvent())
			return _messageEvent;
		}

		const PresetEvent& getPresetEvent() const {
			RNBO_ASSERT(isPresetEvent())
			return _presetEvent;
		}

		const TempoEvent& getTempoEvent() const {
			RNBO_ASSERT(isTempoEvent())
			return _tempoEvent;
		}

		const TransportEvent& getTransportEvent() const {
			RNBO_ASSERT(isTransportEvent())
			return _transportEvent;
		}

		const BeatTimeEvent& getBeatTimeEvent() const {
			RNBO_ASSERT(isBeatTimeEvent())
			return _beatTimeEvent;
		}

		const TimeSignatureEvent& getTimeSignatureEvent() const {
			RNBO_ASSERT(isTimeSignatureEvent())
			return _timeSignatureEvent;
		}

		const StartupEvent& getStartupEvent() const {
			RNBO_ASSERT(isStartupEvent())
			return _startupEvent;
		}

		bool operator==(const EventVariant& rhs) const {
			if (_eventType != rhs._eventType) return false;

			switch (_eventType) {
				case Event::Empty: return _emptyEvent == rhs._emptyEvent; break;
				case Event::Clock: return _clockEvent == rhs._clockEvent; break;
				case Event::DataRef: return _dataRefEvent == rhs._dataRefEvent; break;
				case Event::Midi: return _midiEvent == rhs._midiEvent; break;
				case Event::Outlet: return _outletEvent == rhs._outletEvent; break;
				case Event::Parameter: return _parameterEvent == rhs._parameterEvent; break;
				case Event::Universal: return _universalEvent == rhs._universalEvent; break;
				case Event::Message: return _messageEvent == rhs._messageEvent; break;
				case Event::Preset: return _presetEvent == rhs._presetEvent; break;
				case Event::Tempo: return _tempoEvent == rhs._tempoEvent; break;
				case Event::Transport: return _transportEvent == rhs._transportEvent; break;
				case Event::BeatTime: return _beatTimeEvent == rhs._beatTimeEvent; break;
				case Event::TimeSignature: return _timeSignatureEvent == rhs._timeSignatureEvent; break;
				case Event::Startup: return _startupEvent == rhs._startupEvent; break;
				// default: // no default case to ensure all cases
			}
			return false;
		}

		MillisecondTime getTime() const {
			switch (_eventType) {
				case Event::Empty: return _emptyEvent.getTime(); break;
				case Event::Clock: return _clockEvent.getTime(); break;
				case Event::DataRef: return _dataRefEvent.getTime(); break;
				case Event::Midi: return _midiEvent.getTime(); break;
				case Event::Outlet: return _outletEvent.getTime(); break;
				case Event::Parameter: return _parameterEvent.getTime(); break;
				case Event::Universal: return _universalEvent.getTime(); break;
				case Event::Message: return _messageEvent.getTime(); break;
				case Event::Preset: return _presetEvent.getTime(); break;
				case Event::Tempo: return _tempoEvent.getTime(); break;
				case Event::Transport: return _transportEvent.getTime(); break;
				case Event::BeatTime: return _beatTimeEvent.getTime(); break;
				case Event::TimeSignature: return _timeSignatureEvent.getTime(); break;
				case Event::Startup: return _startupEvent.getTime(); break;
				// default: // no default case to ensure all cases
			}
			return 0;
		}

		EventTarget* getEventTarget() const {
			switch (_eventType) {
				case Event::Empty: return _emptyEvent.getEventTarget(); break;
				case Event::Clock: return _clockEvent.getEventTarget(); break;
				case Event::DataRef: return _dataRefEvent.getEventTarget(); break;
				case Event::Midi: return _midiEvent.getEventTarget(); break;
				case Event::Outlet: return _outletEvent.getEventTarget(); break;
				case Event::Parameter: return _parameterEvent.getEventTarget(); break;
				case Event::Universal: return _universalEvent.getEventTarget(); break;
				case Event::Message: return _messageEvent.getEventTarget(); break;
				case Event::Preset: return _presetEvent.getEventTarget(); break;
				case Event::Tempo: return _tempoEvent.getEventTarget(); break;
				case Event::Transport: return _transportEvent.getEventTarget(); break;
				case Event::BeatTime: return _beatTimeEvent.getEventTarget(); break;
				case Event::TimeSignature: return _timeSignatureEvent.getEventTarget(); break;
				case Event::Startup: return _startupEvent.getEventTarget(); break;
				// default: // no default case to ensure all cases
			}
			return nullptr;
		}

		// debugging
		void dumpEvent() const {
			switch (_eventType) {
				case Event::Empty: return _emptyEvent.dumpEvent(); break;
				case Event::Clock: return _clockEvent.dumpEvent(); break;
				case Event::DataRef: return _dataRefEvent.dumpEvent(); break;
				case Event::Midi: return _midiEvent.dumpEvent(); break;
				case Event::Outlet: return _outletEvent.dumpEvent(); break;
				case Event::Parameter: return _parameterEvent.dumpEvent(); break;
				case Event::Universal: return _universalEvent.dumpEvent(); break;
				case Event::Message: return _messageEvent.dumpEvent(); break;
				case Event::Preset: return _presetEvent.dumpEvent(); break;
				case Event::Tempo: return _tempoEvent.dumpEvent(); break;
				case Event::Transport: return _transportEvent.dumpEvent(); break;
				case Event::BeatTime: return _beatTimeEvent.dumpEvent(); break;
				case Event::TimeSignature: return _timeSignatureEvent.dumpEvent(); break;
				case Event::Startup: return _startupEvent.dumpEvent(); break;
				// default: // no default case to ensure all cases
			}
		}

		void setTime(MillisecondTime time) {
			switch (_eventType) {
				case Event::Empty: _emptyEvent.setTime(time); break;
				case Event::Clock: _clockEvent.setTime(time); break;
				case Event::DataRef: _dataRefEvent.setTime(time); break;
				case Event::Midi: _midiEvent.setTime(time); break;
				case Event::Outlet: _outletEvent.setTime(time); break;
				case Event::Parameter: _parameterEvent.setTime(time); break;
				case Event::Universal: _universalEvent.setTime(time); break;
				case Event::Message: _messageEvent.setTime(time); break;
				case Event::Preset: _presetEvent.setTime(time); break;
				case Event::Tempo: _tempoEvent.setTime(time); break;
				case Event::Transport: _transportEvent.setTime(time); break;
				case Event::BeatTime: _beatTimeEvent.setTime(time); break;
				case Event::TimeSignature: _timeSignatureEvent.setTime(time); break;
				case Event::Startup: _startupEvent.setTime(time); break;
				// default: // no default case to ensure all cases
			}

		}

		void setPatcherEventTarget(PatcherEventTarget* eventTarget) {
			switch (_eventType) {
				case Event::Empty: break;
				case Event::Midi: _midiEvent._eventTarget = eventTarget; break;
				case Event::Clock: _clockEvent._eventTarget = eventTarget; break;
				case Event::DataRef: _dataRefEvent._eventTarget = eventTarget; break;
				case Event::Outlet: _outletEvent._eventTarget = eventTarget; break;
				case Event::Parameter: _parameterEvent._eventTarget = eventTarget; break;
				case Event::Universal: _universalEvent._eventTarget = eventTarget; break;
				case Event::Message: _messageEvent._eventTarget = eventTarget; break;
				case Event::Preset: /* preset events do not need an event target */ break;
				case Event::Tempo: /* tempo events do not need an event target */ break;
				case Event::Transport: /* transport events do not need an event target */ break;
				case Event::BeatTime: /* beat time events do not need an event target */ break;
				case Event::TimeSignature: /* time signature events do not need an event target */ break;
				case Event::Startup: /* startup events do not need an event target */ break;
				// default: // no default case to ensure all cases
			}
		}

	private:

		friend class EventSender;
		friend class PatcherEventSender;

		Event::Type _eventType = Event::Empty;
		union {
			EmptyEvent					_emptyEvent;
			ClockEvent					_clockEvent;
			DataRefEvent				_dataRefEvent;
			MidiEvent					_midiEvent;
			OutletEvent					_outletEvent;
			ParameterEvent				_parameterEvent;
			UniversalEvent				_universalEvent;
			MessageEvent				_messageEvent;
			PresetEvent					_presetEvent;
			TempoEvent					_tempoEvent;
			TransportEvent				_transportEvent;
			BeatTimeEvent				_beatTimeEvent;
			TimeSignatureEvent			_timeSignatureEvent;
			StartupEvent				_startupEvent;
		};

		void setType(Event::Type newType) { _eventType = newType; }

	};

#endif // RNBO_NOSTDLIB

} // namespace RNBO


#endif // #ifndef _RNBO_EventVariant_H_
