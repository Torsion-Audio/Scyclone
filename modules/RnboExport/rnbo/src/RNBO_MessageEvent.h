//
//  RNBO_MessageEvent.h
//
//

#ifndef _RNBO_MessageEvent_H_
#define _RNBO_MessageEvent_H_

#include <memory>

#include "RNBO_Types.h"
#include "RNBO_List.h"
#include "RNBO_PlatformInterface.h"
#include "RNBO_Logger.h"
#include "RNBO_Debug.h"

#ifdef RNBO_NOSTDLIB
#define RNBO_NOMESSAGEEVENT // this is necessary until we have our own shared_ptr implementation
#endif


namespace RNBO {

	class PatcherEventTarget;

#ifdef RNBO_NOMESSAGEEVENT

	class MessageEvent {
	public:

		MillisecondTime getTime() const { return 0; }

		bool operator==(const MessageEvent& rhs) const
		{
			return false;
		}

		PatcherEventTarget* getEventTarget() const { return nullptr; }

		void dumpEvent() const {}

	private:
		friend class EventVariant;

		PatcherEventTarget*			_eventTarget;

		void setTime(MillisecondTime eventTime) { }
	};
#else
    /**
     * An event representing a message
     */
	class MessageEvent {

	public:

		enum Type {
			Invalid = -1,
			Number = 0,
			List,
			Bang,
			Max_Type
		};


		MessageEvent()
		: _tag(0)
		, _objectId(0)
		, _eventTime(0)
		, _type(Invalid)
		, _numValue(0)
		, _eventTarget(nullptr)
		{
		}

		~MessageEvent()
		{
		}

		MessageEvent(MessageTag tag, MillisecondTime eventTime, number numValue, MessageTag objectId = 0, PatcherEventTarget* eventTarget = nullptr)
		: _tag(tag)
		, _objectId(objectId)
		, _eventTime(eventTime)
		, _type(MessageEvent::Number)
		, _eventTarget(eventTarget)
		{
			_numValue = numValue;
		}

		MessageEvent(MessageTag tag, MillisecondTime eventTime, UniqueListPtr listValue, MessageTag objectId = 0, PatcherEventTarget* eventTarget = nullptr)
		: _tag(tag)
		, _objectId(objectId)
		, _eventTime(eventTime)
		, _type(MessageEvent::List)
		, _listValue(std::move(listValue))
		, _eventTarget(eventTarget)
		{
		}

		MessageEvent(MessageTag tag, MillisecondTime eventTime, MessageTag objectId = 0, PatcherEventTarget* eventTarget = nullptr)
		: _tag(tag)
		, _objectId(objectId)
		, _eventTime(eventTime)
		, _type(MessageEvent::Bang)
		, _eventTarget(eventTarget)
		{
		}

		MessageEvent(const MessageEvent& other)
		: _tag(other._tag)
		, _objectId(other._objectId)
		, _eventTime(other._eventTime)
		, _eventTarget(other._eventTarget)
		{
			if (other._type == List) {
				_listValue = other._listValue;
			}
			else if (other._type == Number){
				_numValue = other._numValue;
			}

			_type = other._type;
		}

		MessageEvent(MessageEvent&& other)
		: _tag(std::move(other._tag))
		, _objectId(std::move(other._objectId))
		, _eventTime(std::move(other._eventTime))
		, _numValue(std::move(other._numValue))
		, _listValue(std::move(other._listValue))
		, _eventTarget(std::move(other._eventTarget))
		{
			_type = other._type;
			other._type = Invalid;
		}


		MessageEvent& operator = (const MessageEvent& other)
		{
			_tag = other._tag;
			_eventTime = other._eventTime;
			_eventTarget = other._eventTarget;
			_objectId = other._objectId;

			if (other._type == List) {
				_listValue = other._listValue;
			}
			else if (other._type == Number) {
				_numValue = other._numValue;
			}

			_type = other._type;

			return *this;
		}

		MessageEvent& operator=(MessageEvent&& other)
		{
			_tag = std::move(other._tag);
			_objectId = std::move(other._objectId);
			_eventTime = std::move(other._eventTime);
			_eventTarget = std::move(other._eventTarget);
			_numValue = std::move(other._numValue);
			_listValue = std::move(other._listValue);

			_type = other._type;
			other._type = Invalid;

			return *this;
		}

		bool operator==(const MessageEvent& rhs) const
		{
			return rhs.getTag() == getTag()
			&& rhs.getObjectId() == getObjectId()
			&& rhs.getTime() == getTime()
			&& rhs.getType() == getType()
			&& ((getType() == Bang
				 || getType() == List ? rhs.getListValue() == getListValue() : rhs.getNumValue() == getNumValue()))
			&& rhs._eventTarget == _eventTarget;
		}

		MessageTag getTag() const { return _tag; }
		MessageTag getObjectId() const { return _objectId; }
		MessageEvent::Type getType() const { return _type; }

		MillisecondTime getTime() const { return _eventTime; }
		PatcherEventTarget* getEventTarget() const { return _eventTarget; }

		number getNumValue() const {
			RNBO_ASSERT(_type == Number);
			return _numValue;
		}

		std::shared_ptr<const list> getListValue() const {
			RNBO_ASSERT(_type == List);
			return _listValue;
		}

		// debugging
		void dumpEvent() const {
			// disabling for now to avoid requiring fprintf support in generated code
//			if (_type == List) { }
//			else {
//				fprintf(stdout, "MessageEvent: _tag=%d time=%.3f value=%.4f", _tag, _eventTime, _numValue);
//			}
		}

	protected:

		MessageTag			_tag;	// the tag is a hashed string value
		MessageTag			_objectId;	// the tag is a hashed string value
		MillisecondTime		_eventTime;
		MessageEvent::Type 	_type = Invalid;

		number					_numValue;
		std::shared_ptr<list>	_listValue;

		friend class EventVariant;
		friend class PatcherEventSender;

		PatcherEventTarget*			_eventTarget;

		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }
	};

#endif // RNBO_NOMESSAGEEVENT

} // namespace RNBO


#endif // #ifndef _RNBO_MessageEvent_H_
