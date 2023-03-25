#include "RNBO.h"
#include "RNBO_ParameterInterfaceAsyncImpl.h"

namespace RNBO {

	ParameterInterfaceAsync::ParameterInterfaceAsync(Engine& engine, EventHandler* handler, ParameterEventInterface::Type type) {
		if (handler) {
			handler->linkToParameterEventInterface(this);
		}

		switch (type) {
			case ParameterEventInterface::SingleProducer:
				handler
					?	_impl = std::make_shared<ParameterInterfaceAsyncImpl<ShadowValue> >(engine, handler, make_unique<ParameterEventQueue>(), make_unique<ParameterEventQueue>())
					:	_impl = std::make_shared<ParameterInterfaceAsyncImpl<AtomicShadowValue> >(engine, handler, make_unique<ParameterEventQueue>(), make_unique<ParameterEventQueue>());
				break;
			case ParameterEventInterface::MultiProducer:
				_impl = std::make_shared<ParameterInterfaceAsyncImpl<AtomicShadowValue>>(engine, handler, make_unique<SafeParameterEventQueue>(), make_unique<SafeParameterEventQueue>());
				break;
			case ParameterEventInterface::Trigger:
			case ParameterEventInterface::NotThreadSafe:
				// for the non threadsafe version use the ParameterInterfaceSync
				RNBO_ASSERT(false);
				break;
		}
		
		engine.registerAsyncParameterInterface(_impl);
	}

	ParameterInterfaceAsync::~ParameterInterfaceAsync() {
		if (_impl->isActive()) {
			_impl->notifyParameterInterfaceDeleted();
		}
	}

ParameterIndex ParameterInterfaceAsync::getNumParameters() const {
		return _impl->getNumParameters();
	}

	void ParameterInterfaceAsync::getParameterInfo(ParameterIndex index, ParameterInfo* info) const {
		_impl->getParameterInfo(index, info);
	}

	ConstCharPointer ParameterInterfaceAsync::getParameterName(ParameterIndex index) const {
		return _impl->getParameterName(index);
	}

	ConstCharPointer ParameterInterfaceAsync::getParameterId(ParameterIndex index) const {
		return _impl->getParameterId(index);
	}

	ParameterValue ParameterInterfaceAsync::getParameterValue(ParameterIndex index) {
		return _impl->getParameterValue(index);
	}

	void ParameterInterfaceAsync::setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time) {
		_impl->setParameterValue(index, value, time);
	}

	void ParameterInterfaceAsync::scheduleEvent(EventVariant event) {
		_impl->scheduleEvent(event);
	}

	void ParameterInterfaceAsync::drainEvents() {
		_impl->drainEvents();
	}

	ParameterValue ParameterInterfaceAsync::convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _impl->convertToNormalizedParameterValue(index, value);
	}

	ParameterValue ParameterInterfaceAsync::convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const
	{
		return _impl->convertFromNormalizedParameterValue(index, normalizedValue);
	}

	ParameterValue ParameterInterfaceAsync::constrainParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _impl->constrainParameterValue(index, value);
	}


} // namespace RNBO
