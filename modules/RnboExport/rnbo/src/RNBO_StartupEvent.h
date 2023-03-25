//
//  RNBO_StartupEvent.h
//
//

#ifndef _RNBO_StartupEvent_H_
#define _RNBO_StartupEvent_H_

#include <memory>

#include "RNBO_Types.h"
#include "RNBO_List.h"
#include "RNBO_PlatformInterface.h"
#include "RNBO_Logger.h"
#include "RNBO_Debug.h"
#include "RNBO_Presets.h"

namespace RNBO {

	class PatcherEventTarget;

    /**
     * An event representing startup
     */
	class StartupEvent {

	public:

		enum Type {
			Invalid = -1,
			Begin = 0,
			End,
			Max_Type
		};


		StartupEvent()
		: _eventTime(0)
		, _type(Invalid)
		{
		}

		~StartupEvent()
		{
		}

		StartupEvent(MillisecondTime eventTime, StartupEvent::Type type)
		: _eventTime(eventTime)
		, _type(type)
		{
		}

		StartupEvent(const StartupEvent& other)
		: _eventTime(other._eventTime)
		{
			_type = other._type;
		}

		StartupEvent(StartupEvent&& other)
		: _eventTime(other._eventTime)
		{
			_type = other._type;
		}

		StartupEvent& operator = (const StartupEvent& other)
		{
			_eventTime = other._eventTime;
			_type = other._type;

			return *this;
		}

		bool operator==(const StartupEvent& rhs) const
		{
			return rhs.getTime() == getTime()
			&& rhs.getType() == getType();
		}

		StartupEvent::Type getType() const { return _type; }
		MillisecondTime getTime() const { return _eventTime; }

		// we will always use the default event target (the top level patcher)
		PatcherEventTarget* getEventTarget() const { return nullptr; }

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			fprintf(stdout, "StartupEvent: time=%.3f type=%d", _eventTime, _type);
		}

	protected:

		MillisecondTime		_eventTime;
		StartupEvent::Type 	_type = Invalid;

		friend class EventVariant;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

} // namespace RNBO


#endif // #ifndef _RNBO_StartupEvent_H_
