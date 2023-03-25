#ifndef _RNBO_BASEINTERFACE_H_
#define _RNBO_BASEINTERFACE_H_

#include "RNBO_Types.h"
#include "RNBO_ParameterInterface.h"

namespace RNBO {

	/**
	 * Base class for objects which, like CoreObject, expose an interface to exported RNBO code.
	 */
	class BaseInterface : public ParameterInterface {

	public:

		/**
		 * Initialize data after construction of the BaseInterface
		 */
		virtual void initialize() {}

		virtual Index getNumMidiInputPorts() const = 0;
		virtual Index getNumMidiOutputPorts() const = 0;

		virtual Index getNumInputChannels() const = 0;
		virtual Index getNumOutputChannels() const = 0;

		// methods from ParameterInterface, they are here for documentation only
		ParameterIndex getNumParameters() const override = 0;
		ConstCharPointer getParameterName(ParameterIndex index) const override = 0;
		ConstCharPointer getParameterId(ParameterIndex index) const override = 0;
		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override = 0;

		ParameterValue getParameterValue(ParameterIndex index) override = 0;
		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override = 0;

	protected:

		~BaseInterface() { }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_BASEINTERFACE_H_
