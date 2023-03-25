//
//  _RNBO_PatcherInterface_H_
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_PatcherInterface_H_
#define _RNBO_PatcherInterface_H_

#include "RNBO_Types.h"
#include "RNBO_ProcessInterface.h"
#include "RNBO_PatcherEventTarget.h"
#include "RNBO_EngineLink.h"
#include "RNBO_ProbingInterface.h"
#include "RNBO_PatcherStateInterface.h"
#include "RNBO_EngineInterface.h"

namespace RNBO {

	class DataRef;

	class PatcherInterface : public ProcessInterface,
							 public PatcherEventTarget,
							 public EngineLink,
							 public ProbingInterface
	{
	protected:
		// must be deleted by calling destroy()
		virtual ~PatcherInterface() { }

	public:
		PatcherInterface()
		{ }

		virtual void destroy() = 0;

		// Parameters:
		// - represent state of the patcher that can be set from the Engine
		// - "visible" parameters are made available to plugin environments
		// - Index identifies a parameter but might change when patcher code is regenerated
		// - Index starts at 0 and goes up to getNumParameters() - 1
		// - The parameter name is intended for users to see for plugin environments

		virtual void dump() {}

		virtual void getState(PatcherStateInterface& state)
		{
			RNBO_UNUSED(state);
		}

		void initialize() override
		{
			// you have to overload at least one initialize methods
			// assert(false); // can't use assert in Common
		}

		virtual void initialize(PatcherStateInterface&)
		{
			// the state will only be used if the Patcher overloads this function
			initialize();
		}

		virtual void getPreset(PatcherStateInterface&) {}
		virtual void setPreset(MillisecondTime, PatcherStateInterface&) {}
		virtual DataRefIndex getNumDataRefs() const = 0;
		virtual DataRef* getDataRef(DataRefIndex index) = 0;
		virtual ParameterIndex getNumSignalInParameters() const = 0;
		virtual ParameterIndex getNumSignalOutParameters() const = 0;
		virtual MessageTagInfo resolveTag(MessageTag tag) const = 0;
		virtual MessageIndex getNumMessages() const { return 0; }
		virtual const MessageInfo& getMessageInfo(MessageIndex) const { return NullMessageInfo; }
		virtual Index getMaxBlockSize() const = 0;
		virtual number getSampleRate() const = 0;
		virtual bool hasFixedVectorSize() const = 0;
		virtual void sendParameter(ParameterIndex) {}

		virtual ParameterValue getPolyParameterValue(PatcherInterface**, ParameterIndex index) {
			return getParameterValue(index);
		}

		virtual void setPolyParameterValue(PatcherInterface**, ParameterIndex index, ParameterValue v, MillisecondTime time) {
			setParameterValue(index, v, time);
		}
	};

	/**
	 * @private
	 *
	 * RNBO::default_delete allows putting PatcherInterface into RNBO::UniquePtr
	 */
	template <>
	struct default_delete<RNBO::PatcherInterface>
	{
		void operator() (RNBO::PatcherInterface* pi) const
		{
			pi->destroy();
		}
	};

} // namespace RNBO

#ifndef RNBO_NOSTDLIB

///@cond INTERNAL

// std::default_delete for RNBO::PatcherInterface allows putting PatcherInterface into std::unique_ptr
namespace std
{
	template <>
	struct default_delete<RNBO::PatcherInterface>
	{
		void operator() (RNBO::PatcherInterface* pi) const
		{
			pi->destroy();
		}
	};
}

///@endcond INTERNAL

#endif // RNBO_NOSTDLIB


#endif // #ifndef _RNBO_PatcherInterface_H_
