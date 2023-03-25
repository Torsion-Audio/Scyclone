#ifndef RNBO_ParameterInterface_h
#define RNBO_ParameterInterface_h

#include "RNBO_Types.h"
#include "RNBO_Math.h"

namespace RNBO {

	class ParameterInterface {
		
	protected:
		~ParameterInterface() { }

	public:
		virtual ParameterIndex getNumParameters() const = 0;
		virtual ConstCharPointer getParameterName(ParameterIndex index) const = 0;
		virtual ConstCharPointer getParameterId(ParameterIndex index) const = 0;
		virtual void getParameterInfo(ParameterIndex index, ParameterInfo* info) const = 0;

		virtual ParameterValue getParameterValue(ParameterIndex index) = 0;
		virtual void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) = 0;

		virtual ParameterValue getParameterNormalized(ParameterIndex index) {
			return convertToNormalizedParameterValue(index, getParameterValue(index));
		}

		virtual void setParameterValueNormalized(ParameterIndex index, ParameterValue normalizedValue, MillisecondTime time = RNBOTimeNow) {
			setParameterValue(index, convertFromNormalizedParameterValue(index, normalizedValue), time);
		}

		virtual ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const = 0;
		virtual ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const = 0;
		virtual ParameterValue constrainParameterValue(ParameterIndex, ParameterValue value) const { return value; }
	};

} // namespace RNBO

#endif // RNBO_ParameterInterface_h
