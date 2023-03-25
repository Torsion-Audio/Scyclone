#ifndef RNBO_ParameterInterfaceSync_h
#define RNBO_ParameterInterfaceSync_h

#include "RNBO_ParameterEventInterface.h"

#ifdef USE_STD_VECTOR
#include <vector>
#else
#include "RNBO_Vector.h"
#endif

namespace RNBO {

	class EngineCore;

	/**
	 * @private
	 */
	class ParameterInterfaceSync : public ParameterEventInterfaceImpl
	{
	public:
		ParameterInterfaceSync(EngineCore& engine, EventHandler* handler);
		~ParameterInterfaceSync() override;

		ParameterIndex getNumParameters() const override;
		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override;
		ConstCharPointer getParameterName(ParameterIndex index) const override;
		ConstCharPointer getParameterId(ParameterIndex index) const override;
		ParameterValue getParameterValue(ParameterIndex index) override;
		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override;
		void scheduleEvent(EventVariant event) override;
		void drainEvents() override;
		void refreshParameterCountAndValues() override;
		void notifyParameterValueChanged(ParameterEvent sourceEvent, ParameterIndex index, ParameterValue value) override;
		void drainIncomingQueueToEventList(EventList<EventVariant>& eventList, MillisecondTime currentTime) override;
		void pushDirtyParameters(MillisecondTime currentTime) override;
		void pushOutgoingEvent(EventVariant event) override;
		void notifyOutgoingEvents() override;
		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override;
		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override;
		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override;

		// dummy implementations
		bool isActive() override;
		void deactivate() override;
		void notifyParameterInterfaceDeleted() override;


	protected:

		EngineCore&					_engine;
		EventHandler*				_handler;
#ifdef USE_STD_VECTOR
		std::vector<ParameterValue>	_parameters;
#else
		Vector<ParameterValue>		_parameters;
#endif
	};

	/**
	 * @private
	 */
	class ParameterInterfaceTrigger : public ParameterInterfaceSync
	{
	public:
		ParameterInterfaceTrigger(EngineCore& engine, EventHandler* handler);

		void scheduleEvent(EventVariant event) override;
		void setScheduleCallback(ScheduleCallback callback) override;
	};

} // namespace RNBO

#endif // RNBO_ParameterInterfaceSync_h
