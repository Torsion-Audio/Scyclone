//
//  RNBO_PresetEvent.h
//
//

#ifndef _RNBO_PresetEvent_H_
#define _RNBO_PresetEvent_H_

#include <memory>

#include "RNBO_Types.h"
#include "RNBO_List.h"
#include "RNBO_PlatformInterface.h"
#include "RNBO_Logger.h"
#include "RNBO_Debug.h"
#include "RNBO_Presets.h"

namespace RNBO {

	class PatcherEventTarget;

#ifdef RNBO_NOPRESETS

	class PresetEvent {
	public:

		MillisecondTime getTime() const { return 0; }

		bool operator==(const PresetEvent& rhs) const
		{
			return false;
		}

		PatcherEventTarget* getEventTarget() const { return nullptr; }

		void dumpEvent() const {}

	private:
		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { }
	};
#else
    /**
     * An event representing touched notifications and getting and setting presets
     */
	class PresetEvent {

	public:

		enum Type {
			Invalid = -1,
			Set = 0,
			Touched,
			Get,
			SettingBegin,
			SettingEnd,
			Max_Type
		};


		PresetEvent()
		: _eventTime(0)
		, _type(Invalid)
		{
		}

		~PresetEvent()
		{
		}

		PresetEvent(MillisecondTime eventTime, PresetEvent::Type type)
		: _eventTime(eventTime)
		, _type(type)
		{}

		PresetEvent(
			MillisecondTime eventTime,
			PresetEvent::Type type,
			PresetPtr preset,
			PresetCallback callback
		)
		: _eventTime(eventTime)
		, _type(type)
		, _preset(preset)
		, _callback(callback)
		{
		}

		PresetEvent(const PresetEvent& other)
		: _eventTime(other._eventTime)
		{
			_preset = other._preset;
			_type = other._type;
			_callback = other._callback;
		}

		PresetEvent(PresetEvent&& other)
		: _eventTime(std::move(other._eventTime))
		{
			_preset = std::move(other._preset);
			_callback = std::move(other._callback);

			_type = other._type;
			other._type = Invalid;
		}

		PresetEvent& operator = (const PresetEvent& other)
		{
			_eventTime = other._eventTime;
			_preset = other._preset;
			_type = other._type;
			_callback = other._callback;

			return *this;
		}

		PresetEvent& operator = (PresetEvent&& other)
		{
			_eventTime = std::move(other._eventTime);
			_preset = std::move(other._preset);
			_callback = std::move(other._callback);

			_type = other._type;
			other._type = Invalid;

			return *this;
		}

		bool operator==(const PresetEvent& rhs) const
		{
			return rhs.getTime() == getTime()
			&& rhs.getType() == getType()
			&& rhs.getPreset() == getPreset();
		}

		PresetEvent::Type getType() const { return _type; }
		MillisecondTime getTime() const { return _eventTime; }
		PresetPtr getPreset() const { return _preset; }
		PresetCallback getCallback() const { return _callback; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "PresetEvent: time=%.3f type=%d", _eventTime, _type);
		}

	protected:

		MillisecondTime		_eventTime;
		PresetEvent::Type 	_type = Invalid;

		PresetPtr			_preset;
		PresetCallback		_callback = nullptr;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

#endif // RNBO_NOPRESETS

} // namespace RNBO


#endif // #ifndef _RNBO_PresetEvent_H_
