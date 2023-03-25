//
//  RNBO_DataRefEvent.h
//
//

#ifndef _RNBO_DataRefEvent_H_
#define _RNBO_DataRefEvent_H_

#include "RNBO_PatcherInterface.h"

namespace RNBO {

	/**
	 * An event for updating a Data Reference. Used internally.
	 */
	class DataRefEvent {

	public:

		enum
		{
			InvalidDataRefIndex = -1
		};

		enum DataRefAction {
			NoAction,
			UpdateDataRef
		};

		DataRefEvent()
		: _dataRefIndex(InvalidDataRefIndex)
		, _eventTime(0)
		, _action(NoAction)
		, _eventTarget(nullptr)
		{
		}

		DataRefEvent(const DataRefEvent& other) = default;
		DataRefEvent& operator = (const DataRefEvent& other) = default;

		DataRefEvent(DataRefIndex dataRefIndex, MillisecondTime eventTime, DataRefAction action, PatcherEventTarget* eventTarget = nullptr)
		: _dataRefIndex(dataRefIndex)
		, _eventTime(eventTime)
		, _action(action)
		, _eventTarget(eventTarget)
		{
		}

		bool operator==(const DataRefEvent& rhs) const
		{
			return rhs.getDataRefIndex() == getDataRefIndex()
			&& rhs.getTime() == getTime()
			&& getAction() == rhs.getAction()
			&& rhs._eventTarget == _eventTarget;
		}

		DataRefIndex getDataRefIndex() const { return _dataRefIndex; }
		DataRefAction getAction() const { return _action; }

		MillisecondTime getTime() const { return _eventTime; }
		PatcherEventTarget* getEventTarget() const { return _eventTarget; }

		// debugging
		void dumpEvent() const
		{
			// disabling for now to avoid requiring fprintf support in generated code
			// fprintf(stdout, "DataRefEvent: DataRefIndex=%d time=%.3f action=%.4f\n", _dataRefIndex, _eventTime, _action);
		}

	private:

		DataRefIndex				_dataRefIndex;
		MillisecondTime				_eventTime;
		DataRefAction				_action;

		friend class EventVariant;

		PatcherEventTarget*			_eventTarget;
		void setTime(MillisecondTime eventTime) { _eventTime = eventTime; }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_DataRefEvent_H_
