//
//  RNBO_NullPatcher.h
//
//  Created by Rob Sussman on 11/23/15.
//
//

#ifndef _RNBO_NullPatcher_H_
#define _RNBO_NullPatcher_H_

#include "RNBO.h"
#include "RNBO_PatcherInterfaceImpl.h"

namespace RNBO {

	/**
	 * @private
	 */
	class NullPatcher : public PatcherInterfaceImpl {
	public:

		Index getNumMidiInputPorts() const override {
			return 0;
		}

		Index getNumMidiOutputPorts() const override {
			return 0;
		}

		Index getNumParameters() const override {
			return 0;
		}

		ConstCharPointer getParameterName(ParameterIndex index) const override {
			RNBO_UNUSED(index);
			return "";
		}

		ConstCharPointer getParameterId(ParameterIndex index) const override {
			RNBO_UNUSED(index); return "";
		}

		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(info);
		}

		ParameterValue getParameterValue(ParameterIndex index) override {
			RNBO_UNUSED(index); return 0;
		}

		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time) override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			RNBO_UNUSED(time);
		}

		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override {
			RNBO_UNUSED(index);
			return value;
		}

		void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			RNBO_UNUSED(time);
		}

		void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			RNBO_UNUSED(time);
		}

		void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(port);
			RNBO_UNUSED(data);
			RNBO_UNUSED(length);
		}

		void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(index);
			RNBO_UNUSED(hasValue);
			RNBO_UNUSED(value);
		}

		Index getMaxBlockSize() const override { return _maxBlockSize; }
		number getSampleRate() const override { return _sampleRate; }
		bool hasFixedVectorSize() const override { return false; }

		void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) override {
			RNBO_UNUSED(force);
			_sampleRate = sampleRate;
			_maxBlockSize = maxBlockSize;
		}

		void process(const SampleValue* const* audioInputs, Index numInputs,
					 SampleValue* const* audioOutputs, Index numOutputs,
					 Index sampleFrames) override
		{
			RNBO_UNUSED(audioInputs);
			RNBO_UNUSED(numInputs);

			for (size_t i = 0; i < numOutputs; i++) {
				SampleValue* outBuffer = audioOutputs[i];
				Index n = sampleFrames;
				while (n--) {
					*outBuffer++ = 0;
				}
			}
		}

		Index getNumInputChannels() const override {
			return 0;
		}

		Index getNumOutputChannels() const override {
			// returning 2 output channels, and process method fills with silence
			// this way we make sure to output nothing when we don't really have a patcher
			return 2;
		}

		Index getProbingChannels(MessageTag outletId) const override {
			RNBO_UNUSED(outletId);
			return 0;
		}

		void processOutletEvent(EngineLink* sender, OutletIndex index, ParameterValue value,  MillisecondTime time) override {
			RNBO_UNUSED(sender);
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			RNBO_UNUSED(time);
		}

		DataRefIndex getNumDataRefs() const override {
			return 0;
		}

		ParameterIndex getNumSignalInParameters() const override
		{
			return 0;
		}

		ParameterIndex getNumSignalOutParameters() const override
		{
			return 0;
		}

		DataRef* getDataRef(DataRefIndex index) override {
			RNBO_UNUSED(index); return nullptr;
		}

		MessageTagInfo resolveTag(MessageTag tag) const override {
			RNBO_UNUSED(tag);
			return "";
		}

		MessageIndex getNumMessages() const override {
			return 0;
		}

		void processDataViewUpdate(DataRefIndex index, MillisecondTime time) override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(time);
		}

		void processOutletAtCurrentTime(EngineLink* sender, OutletIndex index, ParameterValue value) override {
			RNBO_UNUSED(sender);
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
		}

		void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) override {
			RNBO_UNUSED(tag);
			RNBO_UNUSED(objectId);
			RNBO_UNUSED(time);
			RNBO_UNUSED(payload);
		}

		void processListMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, const list& payload) override {
			RNBO_UNUSED(tag);
			RNBO_UNUSED(objectId);
			RNBO_UNUSED(time);
			RNBO_UNUSED(payload);
		}

		void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) override {
			RNBO_UNUSED(tag);
			RNBO_UNUSED(objectId);
			RNBO_UNUSED(time);
		}

		void processTempoEvent(MillisecondTime time, Tempo tempo) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(tempo);
		}

		void processTransportEvent(MillisecondTime time, TransportState state) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(state);
		}

		void processBeatTimeEvent(MillisecondTime time, BeatTime beatTime) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(beatTime);
		}

		void processTimeSignatureEvent(MillisecondTime time, int numerator, int denominator) override {
			RNBO_UNUSED(time);
			RNBO_UNUSED(numerator);
			RNBO_UNUSED(denominator);
		}

		void getState(PatcherStateInterface& state) override {
			RNBO_UNUSED(state);
		}

		void initialize() override {}

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(value);
			return 0;
		}
		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override {
			RNBO_UNUSED(index);
			RNBO_UNUSED(normalizedValue);
			return 0;
		}

	private:
		number	_sampleRate = 0;
		Index 	_maxBlockSize = 0;
	};

} // namespace RNBO

#endif // #ifndef _RNBO_NullPatcher_H_
