//
//  ParameterInterface.h
//
//  Created by Stefan Brunner on 12.08.15.
//
//

#ifndef RNBOPlugin_ParameterInterfaceAsync_h
#define RNBOPlugin_ParameterInterfaceAsync_h

#include "RNBO_EventVariant.h"
#include "RNBO_EventList.h"
#include "RNBO_ParameterEventInterface.h"

namespace RNBO {

	class Engine;
	using ParameterEventInterfaceImplPtr = std::shared_ptr<ParameterEventInterfaceImpl>;

	/**
	 * @private
	 */
	class ParameterInterfaceAsync : public ParameterEventInterface
	{
	public:

		// DO NOT construct a parameter interface directly - use Engine::createParameterInterface instead
		ParameterInterfaceAsync(Engine& engine, EventHandler* handler, ParameterEventInterface::Type type);
		~ParameterInterfaceAsync() override;

		ParameterIndex getNumParameters() const override;
		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override;
		ConstCharPointer getParameterName(ParameterIndex index) const override;
		ConstCharPointer getParameterId(ParameterIndex index) const override;

		ParameterValue getParameterValue(ParameterIndex index) override;
		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override;

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override;
		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override;
		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override;

		void scheduleEvent(EventVariant event) override;

		void drainEvents() override;

	private:

		ParameterEventInterfaceImplPtr		_impl;
	};
} // namespace RNBO

#endif
