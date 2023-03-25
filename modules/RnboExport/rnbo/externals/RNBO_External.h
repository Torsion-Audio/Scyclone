#ifndef _RNBO_External_h_
#define _RNBO_External_h_

#include "../common/RNBO_Common.h"
#include "../common/RNBO_ExternalBase.h"
#include "../src/RNBO_DynamicSymbolRegistry.h"

namespace RNBO {

	/**
	 * @private
	 */
	class External : public ExternalBase {
	public:
		virtual ~External() override {}

		// you need to overload those for parameter communication with your external
		virtual Index getNumParameters() const override = 0;
		virtual ConstCharPointer getParameterName(Index index) const override = 0;
		virtual ConstCharPointer getParameterId(Index index) const override { return getParameterName(index); }
		virtual void getParameterInfo(Index index, ParameterInfo* info) const override = 0;

		virtual ParameterValue getParameterValue(Index index) override = 0;
		virtual void setParameterValue(Index index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override = 0;

		// initialize is called after construction and can be used to intialize your stuff
		virtual void initialize() override {}

		// audio and event processing
		virtual void prepareToProcess(double sampleRate, Index blockSize, bool force = false) override = 0;
		virtual void process(const SampleValue* const* audioInputs, Index numInputs, SampleValue* const* audioOutputs, Index numOutputs, Index sampleFrames) override = 0;

		virtual Index getNumInputChannels() const override = 0;
		virtual Index getNumOutputChannels() const override = 0;

		Index getNumMidiInputPorts() const override { return 0; }
		Index getNumMidiOutputPorts() const override { return 0; }

		// override these to process MIDI and Clock events
		void processMidiEvent(MillisecondTime, int, ConstByteArray, Index) override { }
		void processClockEvent(MillisecondTime, ClockId, bool, ParameterValue) override { }

		// override to provide custom normalizations for your parameters
		ParameterValue convertToNormalizedParameterValue(Index index, ParameterValue value) const override {
			ParameterInfo info;
			getParameterInfo(index, &info);

			if (value < info.min)
				value = info.min;
			else if (value > info.max)
				value = info.max;

			ParameterValue normalizedValue = (value - info.min) / (info.max - info.min);
			if (info.exponent != ParameterValue(1.)) {
				// TODO test if this is faster than pow()
				normalizedValue = RNBO_Math::rnbo_exp(RNBO_Math::rnbo_log(normalizedValue) * 1/info.exponent);
			}

			normalizedValue = applyStepsToNormalizedParameterValue(normalizedValue, info.steps);

			return normalizedValue;
		}

		ParameterValue convertFromNormalizedParameterValue(Index index, ParameterValue normalizedValue) const override {
			ParameterInfo info;
			getParameterInfo(index, &info);

			if (normalizedValue < 0)
				normalizedValue = 0;
			if (normalizedValue > 1)
				normalizedValue = 1;

			normalizedValue = applyStepsToNormalizedParameterValue(normalizedValue, info.steps);

			ParameterValue value;
			if (info.exponent == ParameterValue(1.)) {
				value = info.min + normalizedValue * (info.max - info.min);
			}
			else {
				// TODO test if this is faster than pow()
				value = info.min + RNBO_Math::rnbo_exp(RNBO_Math::rnbo_log(normalizedValue) * info.exponent) * (info.max - info.min);
			}
			return value;
		}

	private:

		ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
			if (steps > 0) {
				if (steps == 1) {
					if (normalizedValue > 0) {
						normalizedValue = 1.;
					}
				}
				else {
					ParameterValue oneStep = ParameterValue(1.) / (steps - 1);
					ParameterValue numberOfSteps = RNBO_Math::rnbo_fround(normalizedValue / oneStep);
					normalizedValue = numberOfSteps * oneStep;
				}
			}
			return normalizedValue;
		}
	};

} // namespace RNBO

#endif  // #ifndef _RNBO_External_h_
